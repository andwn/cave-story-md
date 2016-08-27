#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"
#include "camera.h"
#include "system.h"

#define CFRONT				5
#define CBACK				6

// states for the core
#define CORE_SLEEP			10
#define CORE_CLOSED			200
#define CORE_OPEN			210
#define CORE_GUST			220

// and the states for the minicores
#define MC_SLEEP			0
#define MC_THRUST			10
#define MC_CHARGE_FIRE		20
#define MC_FIRE				30
#define MC_FIRED			40
#define MC_RETREAT			50

// flash red when struck, else stay in Mouth Open frame
// TODO: Palette swapping
#define OPEN_MOUTH ({ \
	if(e->damage_time & 2) { \
		 \
	} else { \
		 \
	} \
	if(!e->mouth_open) { \
		e->mouth_open = true; \
	} \
})

// makes the core close his mouth
#define CLOSE_MOUTH ({ \
	if(e->mouth_open) { \
		e->mouth_open = false; \
	} \
})

// bring the water up if it's not already up, but don't keep it up
// if it's already been up on it's own because that's not fair
#define START_WATER_STREAM ({ \
	if (water_entity->state == WL_DOWN) water_entity->state = WL_UP; \
	camera_shake(100); \
})

// bring the water down again if it's not already
#define STOP_WATER_STREAM ({ \
	if (water_entity->state == WL_UP) water_entity->state = WL_CYCLE; \
})

//typedef struct { u8 x1, x2, y1, y2; } DrawArea;
//DrawArea drawn_area = {};
Entity *pieces[7];

// called at the entry to the Core room.
// initilize all the pieces of the Core boss.
void oncreate_core(Entity *e) {
	e->state = CORE_SLEEP;
	e->eflags = (NPC_SHOWDAMAGE | NPC_IGNORESOLID | NPC_EVENTONDEATH);
	
	e->x = (1207 << CSF);
	e->y = (212 << CSF);
	e->x_speed = 0;
	e->y_speed = 0;
	e->health = 650;
	e->hit_box = (bounding_box) { 9*8, 5*8, 0, 5*8 };
	
	//e->sprite = SPR_CORESHOOTMARKER;
	
	// spawn all the pieces in the correct z-order
	pieces[3] = entity_create(0, 0, 0, 0, OBJ_MINICORE, 0, 0);
	pieces[4] = entity_create(0, 0, 0, 0, OBJ_MINICORE, 0, 0);
	pieces[CFRONT] = entity_create(0, 0, 0, 0, OBJ_CORE_FRONT, 0, 0);
	pieces[CBACK] = entity_create(0, 0, 0, 0, OBJ_CORE_BACK, 0, 0);
	pieces[0] = entity_create(0, 0, 0, 0, OBJ_MINICORE, 0, 0);
	pieces[1] = entity_create(0, 0, 0, 0, OBJ_MINICORE, 0, 0);
	pieces[2] = entity_create(0, 0, 0, 0, OBJ_MINICORE, 0, 0);
	
	// set up the front piece
	pieces[CFRONT]->linkedEntity = e;
	pieces[CFRONT]->eflags |= (NPC_IGNORESOLID | NPC_SHOOTABLE | NPC_INVINCIBLE);
	pieces[CFRONT]->hit_box = (bounding_box) { 4*8, 5*8, 4*8, 5*8 };
	pieces[CFRONT]->display_box = (bounding_box) { 4*8+4, 7*8, 4*8+4, 7*8 };
	
	// set up our back piece
	pieces[CBACK]->linkedEntity = e;
	pieces[CBACK]->eflags |= (NPC_IGNORESOLID | NPC_SHOOTABLE | NPC_INVINCIBLE);
	pieces[CBACK]->display_box = (bounding_box) { 5*8+4, 7*8, 5*8+4, 7*8 };
	
	// set the positions of all the minicores
	pieces[0]->x = (e->x - 0x1000);
	pieces[0]->y = (e->y - 0x8000);
	
	pieces[1]->x = (e->x + 0x2000);
	pieces[1]->y = e->y;
	
	pieces[2]->x = (e->x - 0x1000);
	pieces[2]->y = (e->y + 0x8000);
	
	pieces[3]->x = (e->x - 0x6000);
	pieces[3]->y = (e->y + 0x4000);
	
	pieces[4]->x = (e->x - 0x6000);
	pieces[4]->y = (e->y - 0x4000);

	for(u8 i = 0; i < 5; i++) {
		pieces[i]->eflags = (NPC_SHOOTABLE | NPC_INVINCIBLE | NPC_IGNORESOLID);
		pieces[i]->health = 1000;
		pieces[i]->state = MC_SLEEP;
		pieces[i]->hit_box = (bounding_box) { 28, 16, 20, 16 };
		pieces[i]->display_box = (bounding_box) { 32, 20, 32, 20 };
	}
}

// We never need to know the core controller's ID but need an extra u16 variable to save hp
#define savedhp		id
#define mouth_open	grounded

void ai_core(Entity *e) {
	bool do_thrust = false;

	switch(e->state) {
		case CORE_SLEEP: break;			// core is asleep
		// Core's mouth is closed.
		// Core targets player point but does not update it during the state.
		// This is also the state set via BOA to awaken the core.
		case CORE_CLOSED:
		{
			e->state = CORE_CLOSED+1;
			e->state_time = 0;
			
			STOP_WATER_STREAM;
			e->x_mark = player.x;
			e->y_mark = player.y;
		}
		/* no break */
		case CORE_CLOSED+1:
		{
			// open mouth after 400 ticks
			if (e->state_time > TIME(400)) {
				if (++e->state_time2 > 3) {
					// every 3rd time do gusting left and big core blasts
					e->state_time2 = 0;
					e->state = CORE_GUST;
				} else {
					e->state = CORE_OPEN;
				}
				
				do_thrust = true;
			}
		}
		break;
		// Core's mouth is open.
		// Core moves towards player, and updates the position throughout
		// the state (is "aggressive" about seeking him).
		// Core fires ghosties, and curly targets it.
		case CORE_OPEN:
		{
			e->state = CORE_OPEN+1;
			e->state_time = 0;
			// gonna open mouth, so save the current HP so we'll
			// know how much damage we've taken this time.
			e->savedhp = e->health;
		}
		/* no break */
		case CORE_OPEN+1:
		{
			e->x_mark = player.x;
			e->y_mark = player.y;
			
			// must call constantly for red-flashing when hit
			OPEN_MOUTH;
			
			// hint curly to target us
			if ((e->state_time % TIME(64)) == 1) {
				CURLY_TARGET_HERE(e);
			}
			
			// spawn ghosties
			if (e->state_time < TIME(200)) {
				if ((e->state_time % TIME(20))==0) {
					//CreateEntity(e->x + (random(-48, -16) << CSF),
					//	     	 e->y + (random(-64, 64) << CSF),
					//		 	 OBJ_CORE_GHOSTIE);
				}
			}
			
			// close mouth when 400 ticks have passed or we've taken more than 200 damage
			if (e->state_time > TIME(400) || (e->savedhp - e->health) >= 200) {
				e->state = CORE_CLOSED;
				CLOSE_MOUTH;
				do_thrust = true;
			}
		}
		break;
		case CORE_GUST:
		{
			e->state = CORE_GUST+1;
			e->state_time = 0;
			
			START_WATER_STREAM;
		}
		/* no break */
		case CORE_GUST+1:
		{
			// spawn water droplet effects and push player
			//Entity *droplet = CreateEntity(player->x + ((random(-50, 150)<<CSF)*2),
			//					   		   player->y + (random(-160, 160)<<CSF),
			//					   		   OBJ_FAN_DROPLET);
			//droplet->dir = LEFT;
			player.x_speed -= SPEED(0x20);
			
			OPEN_MOUTH;
			
			// spawn the big white blasts
			if (e->state_time==TIME(300) || e->state_time==TIME(350) || e->state_time==TIME(400)) {
				//EmFireAngledShot(pieces[CFRONT], OBJ_CORE_BLAST, 0, 3<<CSF);
				sound_play(SND_LIGHTNING_STRIKE, 5);
			}
			
			if (e->state_time > TIME(400)) {
				e->state = CORE_CLOSED;
				CLOSE_MOUTH;
				do_thrust = true;
			}
		}
		break;
		case 500:		// defeated!!
		{
			STOP_WATER_STREAM;
			water_entity->state = WL_CALM;
			
			e->state = 501;
			e->state_time = 0;
			e->x_speed = e->y_speed = 0;
			curly_target_time = 0;
			
			CLOSE_MOUTH;
			
			camera_shake(20);
			//SmokeXY(pieces[CBACK]->x, pieces[CBACK]->CenterY(), 20, 128, 64);
			
			// tell all the MC's to retreat
			//for(i=0;i<5;i++) {
			//	pieces[i]->flags &= ~(NPC_SHOOTABLE & NPC_INVINCIBLE);
			//	pieces[i]->state = MC_RETREAT;
			//}
		}
		/* no break */
		case 501:
		{
			e->state_time++;
			if ((e->state_time & 0x0f) != 0) {
				//SmokeXY(pieces[CBACK]->x, pieces[CBACK]->CenterY(), 1, 64, 32);
			}
			
			if (e->state_time & 2)
				e->x -= (1 << CSF);
			else
				e->x += (1 << CSF);
			
			#define CORE_DEATH_TARGET_X		0x7a000
			#define CORE_DEATH_TARGET_Y		0x16000
			e->x_speed += (e->x > CORE_DEATH_TARGET_X) ? SPEED(-0x80) : SPEED(0x80);
			e->y_speed += (e->y > CORE_DEATH_TARGET_Y) ? SPEED(-0x80) : SPEED(0x80);
		}
		break;
		case 600:			// teleported away by Misery
		{
			e->x_speed = 0;
			e->y_speed = 0;
			e->state++;
			//sound(SND_TELEPORT);
			
			//pieces[CFRONT]->clip_enable = pieces[CBACK]->clip_enable = 1;
			e->state_time = 60;//sprites[pieces[CFRONT]->sprite].h;
		}
		/* no break */
		case 601:
		{
			//pieces[CFRONT]->display_xoff = pieces[CBACK]->display_xoff = random(-8, 8);
			
			//pieces[CFRONT]->clipy2 = e->state_time;
			//pieces[CBACK]->clipy2 = e->state_time;
			
			if (--e->state_time == 0) {
				//pieces[CFRONT]->invisible = true;
				//pieces[CBACK]->invisible = true;
				
				// restore status bars
				//game.stageboss.object = NULL;
				//game.bossbar.object = NULL;
				for(u8 i = 0; i < 7; i++) pieces[i]->state = STATE_DELETE;
				e->state = STATE_DELETE;
				return;
			}
		}
		break;
	}
	
	if (do_thrust) {
		// tell all the minicores to jump to a new position
		for(u8 i=0;i<5;i++) {
			pieces[i]->state = MC_THRUST;
		}
		
		camera_shake(20);
		sound_play(SND_CORE_THRUST, 5);
	}
	
	// fire the minicores in any awake non-dead state
	if (e->state >= CORE_CLOSED && e->state < 500) {
		e->state_time++;
		
		// fire off each minicore sequentially...
		switch(e->state_time) {
			case TIME(80+0):   pieces[0]->state = MC_CHARGE_FIRE; break;
			case TIME(80+30):  pieces[1]->state = MC_CHARGE_FIRE; break;
			case TIME(80+60):  pieces[2]->state = MC_CHARGE_FIRE; break;
			case TIME(80+90):  pieces[3]->state = MC_CHARGE_FIRE; break;
			case TIME(80+120): pieces[4]->state = MC_CHARGE_FIRE; break;
		}
		
		// move main core towards a spot in front of target
		e->x_speed += (e->x > (e->x_mark + (160<<CSF))) ? -4 : 4;
		e->y_speed += (e->y > e->y_mark) ? -4 : 4;
	}
	
	// set up our shootable status--you never actually hit the core (CFRONT),
	// but if it's mouth is open, make us, the invisible controller object, shootable.
	if (pieces[CFRONT]->mouth_open) {
		e->eflags &= ~NPC_SHOOTABLE;
		pieces[CFRONT]->eflags |= NPC_INVINCIBLE;
	} else {
		e->eflags |= NPC_SHOOTABLE;
		pieces[CFRONT]->eflags &= ~NPC_INVINCIBLE;
	}
	
	LIMIT_X(SPEED(0x80));
	LIMIT_Y(SPEED(0x80));

	// Draw blank core tiles that come onscreen, erase those that go offscreen
	//SYS_disableInts();
	//s16 hscroll = (e->x - camera.x) >> CSF;
	//s16 vscroll = (e->y - camera.y) >> CSF;
	//DrawArea visible_area = {
	//		.x1 = ,
	//};
	//SYS_enableInts();
}

void ondeath_core(Entity *e) {
	if(e->state == STATE_DEFEATED) {
		e->state = 500;
		e->state_time = 0;
		e->eflags = 0;
		tsc_call_event(e->event);
	}
}

// the front (mouth) piece of the main core
void ai_core_front(Entity *e) {
	Entity *core = e->linkedEntity;
	if (core == NULL) { e->state = STATE_DELETE; return; }
	
	e->x = core->x - (36 << CSF);
	e->y = core->y - (48 << CSF);
}

// the back (unanimated) piece of the main core
void ai_core_back(Entity *e) {
	Entity *core = e->linkedEntity;
	if (core == NULL) { e->state = STATE_DELETE; return; }
	
	e->x = core->x + (0x5800 - (8 << CSF));
	e->y = core->y - 0x5e00;
}

void ai_minicore(Entity *e) {
	Entity *core = e->linkedEntity;
	if (core == NULL) { e->state = STATE_DELETE; return; }
	
	switch(e->state) {
		case MC_SLEEP:		// idle & mouth closed
		{
			SPR_SAFEANIM(e->sprite, 2);
			e->state = MC_SLEEP+1;
		}
		/* no break */
		case MC_SLEEP+1:
		{
			e->x_mark = e->x;
			e->y_mark = e->y;
		}
		break;
		case MC_THRUST:			// thrust (move to random new pos)
		{
			e->state = MC_THRUST+1;
			SPR_SAFEANIM(e->sprite, 2);
			e->state_time = 0;
			e->x_mark = e->x + ((-128 + (random() % 160)) << CSF);
			e->y_mark = e->y + ((-64 + (random() % 128)) << CSF);
		}
		/* no break */
		case MC_THRUST+1:
		{
			if (++e->state_time > TIME(50)) {
				SPR_SAFEANIM(e->sprite, 0);
			}
		}
		break;
		case MC_CHARGE_FIRE:			// charging for fire
		{
			e->state = MC_CHARGE_FIRE+1;
			e->state_time = 0;
		}
		/* no break */
		case MC_CHARGE_FIRE+1:			// flash blue
		{
			e->state_time++;
			if(e->state_time % 4 == 0) {
				SPR_SAFEANIM(e->sprite, 0);
			} else if(e->state_time % 4 == 2) {
				SPR_SAFEANIM(e->sprite, 1);
			}
			if(e->state_time > TIME(20)) {
				e->state = MC_FIRE;
			}
		}
		break;
		case MC_FIRE:			// firing
		{
			e->state = MC_FIRE+1;
			SPR_SAFEANIM(e->sprite, 2);	// close mouth again
			e->state_time = 0;
			e->x_mark = e->x + ((24 + (random() % 16)) << CSF);
			e->y_mark = e->y + ((-4 + (random() % 8)) << CSF);
		}
		/* no break */
		case MC_FIRE+1:
		{
			if (++e->state_time > TIME(50)) {
				e->state = MC_FIRED;
				SPR_SAFEANIM(e->sprite, 0);
			} else if (e->state_time==1 || e->state_time==3) {
				// fire at player at speed (2<<CSF) with 2 degrees of variance
				//EmFireAngledShot(o, OBJ_MINICORE_SHOT, 2, 2<<CSF);
				sound_play(SND_EM_FIRE, 5);
			}
		}
		break;
		case MC_RETREAT:		// defeated!
		{
			e->state = MC_RETREAT+1;
			SPR_SAFEANIM(e->sprite, 2);
			e->x_speed = e->y_speed = 0;
		}
		/* no break */
		case MC_RETREAT+1:		// retreat back into the abyss
		{
			e->x_speed += SPEED(0x20);
			if (e->x > block_to_sub(stageWidth) + 0x4000) {
				e->state = STATE_DELETE;
			}
		}
		break;
	}
	
	if (e->state < MC_RETREAT) {
		// jump back when shot
		if (e->damage_time) {
			e->x_mark += 0x400;
		}
		
		e->x += (e->x_mark - e->x) / 16;
		e->y += (e->y_mark - e->y) / 16;
	}
	
	// don't let them kill us
	e->health = 1000;
	
	// invincible when mouth is closed
	//if (e->frame != 2)
	//	e->flags &= ~FLAG_INVULNERABLE;
	//else
	//	e->flags |= FLAG_INVULNERABLE;
}

void ai_minicore_shot(Entity *e) {
	e->x += e->x_speed;
	e->y += e->y_speed;
	if (++e->state_time2 > TIME(150)) {
		//effect(e->CenterX(), e->CenterY(), EFFECT_FISHY);
		e->state = STATE_DELETE;
	}
}
// shutter made noise when opening
// curly looks up at no 4

void ai_core_ghostie(Entity *e) {
	char hit = 0;

	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;

	if (e->x_speed > 0 && collide_stage_rightwall(e)) hit = 1;
	if (e->x_speed < 0 && collide_stage_leftwall(e)) hit = 1;
	if (e->y_speed > 0 && collide_stage_floor(e)) hit = 1;
	if (e->y_speed < 0 && collide_stage_ceiling(e)) hit = 1;
	
	e->x = e->x_next;
	e->y = e->y_next;

	e->x_speed -= SPEED(0x20);
	LIMIT_X(SPEED(0x400));
	
	if(hit) {
		//effect(e->CenterX(), e->CenterY(), EFFECT_FISHY);
		e->state = STATE_DELETE;
	}
}

void ai_core_blast(Entity *e) {
	e->x += e->x_speed;
	e->y += e->y_speed;
	if (++e->state_time > TIME(200)) e->state = STATE_DELETE;
}
