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
/*
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

// makes the core open his mouth and handles flashing red when hit
#define OPEN_MOUTH		\
{						\
	RunOpenMouth();	\
}

// makes the core close his mouth
#define CLOSE_MOUTH		\
{		\
	pieces[CFRONT]->frame = 2;		\
	pieces[CBACK]->frame = 0;		\
}

// called at the entry to the Core room.
// initilize all the pieces of the Core boss.
void create_core() {
	e = entity_create_boss(0, 0, BOSS_CORE, 1000);
	//game.stageboss.object = o;
	
	e->state = 10;
	
	e->eflags = (FLAG_SHOW_FLOATTEXT | FLAG_IGNORE_SOLID | FLAG_SCRIPTONDEATH);
	//e->id2 = 1000;
	
	e->x = (1207 << CSF);
	e->y = (212 << CSF);
	e->x_speed = 0;
	e->y_speed = 0;
	e->health = 650;
	
	//e->sprite = SPR_CORESHOOTMARKER;
	
	// spawn all the pieces in the correct z-order
	pieces[3] = CreateMinicore(e);
	pieces[4] = CreateMinicore(e);
	pieces[CFRONT] = CreateEntity(0, 0, OBJ_CORE_FRONT);
	pieces[CBACK] = CreateEntity(0, 0, OBJ_CORE_BACK);
	pieces[0] = CreateMinicore(e);
	pieces[1] = CreateMinicore(e);
	pieces[2] = CreateMinicore(e);
	
	// set up the front piece
	pieces[CFRONT]->sprite = SPR_CORE_FRONT;
	pieces[CFRONT]->state = CORE_SLEEP;
	pieces[CFRONT]->linkedobject = e;
	pieces[CFRONT]->flags |= (FLAG_IGNORE_SOLID | FLAG_INVULNERABLE);
	pieces[CFRONT]->frame = 2;			// mouth closed
	
	// set up our back piece
	pieces[CBACK]->sprite = SPR_CORE_BACK;
	pieces[CBACK]->state = CORE_SLEEP;
	pieces[CBACK]->linkedobject = e;
	pieces[CBACK]->flags |= (FLAG_IGNORE_SOLID | FLAG_INVULNERABLE);
	pieces[CBACK]->frame = 0;
	
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
	
	//this->hittimer = 0;
}

// We never need to know the core controller's ID but need an extra u16 variable to save hp
#define savedhp	id

void ai_core(Entity *e) {
	bool do_thrust = false;
	int i;

	switch(e->state) {
		case CORE_SLEEP: break;			// core is asleep
		// Core's mouth is closed.
		// Core targets player point but does not update it during the state.
		// This is also the state set via BOA to awaken the core.
		case CORE_CLOSED:
		{
			e->state = CORE_CLOSED+1;
			e->state_time = 0;
			
			StopWaterStream();
			e->x_mark = player.x;
			e->y_mark = player.y;
		}
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
			if (e->state_time > TIME(400) || (e->savedhp - e->hp) >= 200) {
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
			
			StartWaterStream();
		}
		case CORE_GUST+1:
		{
			// spawn water droplet effects and push player
			//Entity *droplet = CreateEntity(player->x + ((random(-50, 150)<<CSF)*2), \
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
			StopWaterStream();
			water_entity->state = WL_CALM;
			
			e->state = 501;
			e->state_time = 0;
			e->x_speed = e->y_speed = 0;
			curly_target_time = 0;
			
			CLOSE_MOUTH;
			
			camera_shake(20);
			//SmokeXY(pieces[CBACK]->x, pieces[CBACK]->CenterY(), 20, 128, 64);
			
			// tell all the MC's to retreat
			for(i=0;i<5;i++)
			{
				pieces[i]->flags &= ~(NPC_SHOOTABLE & NPC_INVINCIBLE);
				pieces[i]->state = MC_RETREAT;
			}
		}
		case 501:
		{
			e->state_time++;
			if ((e->state_time & 0x0f) != 0)
			{
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
		case 601:
		{
			//pieces[CFRONT]->display_xoff = pieces[CBACK]->display_xoff = random(-8, 8);
			
			//pieces[CFRONT]->clipy2 = e->state_time;
			//pieces[CBACK]->clipy2 = e->state_time;
			
			if (--e->state_time < 0) {
				//pieces[CFRONT]->invisible = true;
				//pieces[CBACK]->invisible = true;
				
				// restore status bars
				//game.stageboss.object = NULL;
				//game.bossbar.object = NULL;
				e->state = STATE_DELETE;
				return;
			}
		}
		break;
	}
	
	if (do_thrust) {
		// tell all the minicores to jump to a new position
		for(i=0;i<5;i++) {
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
			case 80+0:   pieces[0]->state = MC_CHARGE_FIRE; break;
			case 80+30:  pieces[1]->state = MC_CHARGE_FIRE; break;
			case 80+60:  pieces[2]->state = MC_CHARGE_FIRE; break;
			case 80+90:  pieces[3]->state = MC_CHARGE_FIRE; break;
			case 80+120: pieces[4]->state = MC_CHARGE_FIRE; break;
		}
		
		// move main core towards a spot in front of target
		e->x_speed += (e->x > (e->xmark + (160<<CSF))) ? -4 : 4;
		e->y_speed += (e->y > e->ymark - (e->Height() / 2)) ? -4 : 4;
	}
	
	// set up our shootable status--you never actually hit the core (CFRONT),
	// but if it's mouth is open, make us, the invisible controller object, shootable.
	if (pieces[CFRONT]->frame==2) {
		e->flags &= ~FLAG_SHOOTABLE;
		pieces[CFRONT]->flags |= FLAG_INVULNERABLE;
	} else {
		e->flags |= FLAG_SHOOTABLE;
		pieces[CFRONT]->flags &= ~FLAG_INVULNERABLE;
	}
	
	LIMITX(SPEED(0x80));
	LIMITY(SPEED(0x80));
}

void RunOpenMouth() {
	// flash red when struck, else stay in Mouth Open frame
	pieces[CFRONT]->frame = 0;
	pieces[CBACK]->frame = 0;
	
	if (e->shaketime)
	{
		this->hittimer++;
		if (this->hittimer & 2)
		{
			pieces[CFRONT]->frame = 1;
			pieces[CBACK]->frame = 1;
		}
	}
	else
	{
		this->hittimer = 0;
	}
}

void StartWaterStream() {
	// bring the water up if it's not already up, but don't keep it up
	// if it's already been up on it's own because that's not fair
	if (water_entity->state == WL_DOWN)
		water_entity->state = WL_UP;
	
	camera_shake(100);
	//StartStreamSound(400);
}

void StopWaterStream() {
	// bring the water down again if it's not already
	if (water_entity->state == WL_UP)
		water_entity->state = WL_CYCLE;
	
	//StopLoopSounds();
}

// the front (mouth) piece of the main core
void ai_core_front(Entity *e) {
	Entity *core = e->linkedEntity;
	if (!core) { e->state == STATE_DELETE; return; }
	
	e->x = core->x - 0x4800;
	e->y = core->y - 0x5e00;
}

// the back (unanimated) piece of the main core
void ai_core_back(Entity *e) {
	Entity *core = e->linkedEntity;
	if (!core) { e->state == STATE_DELETE; return; }
	
	e->x = core->x + (0x5800 - (8 << CSF));
	e->y = core->y - 0x5e00;
}

static Entity *CreateMinicore(Entity *core) {
	Entity *e = CreateEntity(0, 0, OBJ_MINICORE);
	e->linkedEntity = core;
	e->eflags = (FLAG_SHOOTABLE | FLAG_INVULNERABLE | FLAG_IGNORE_SOLID);
	e->health = 1000;
	e->state = MC_SLEEP;
	
	return o;
}

void ai_minicore(Entity *e) {
	Entity *core = e->linkedEntity;
	if (!core) { e->state == STATE_DELETE; return; }
	
	switch(e->state)
	{
		case MC_SLEEP:		// idle & mouth closed
			e->frame = 2;
			e->xmark = e->x;
			e->ymark = e->y;
		break;
		
		case MC_THRUST:			// thrust (move to random new pos)
			e->state = MC_THRUST+1;
			e->frame = 2;
			e->state_time = 0;
			e->xmark = core->x + (random(-128, 32) << CSF);
			e->ymark = core->y + (random(-64, 64) << CSF);
		case MC_THRUST+1:
			if (++e->state_time > 50)
			{
				e->frame = 0;
			}
		break;
		
		case MC_CHARGE_FIRE:			// charging for fire
			e->state = MC_CHARGE_FIRE+1;
			e->state_time = 0;
		case MC_CHARGE_FIRE+1:			// flash blue
			e->state_time++;
			e->frame = ((e->state_time >> 1) & 1);
			if (e->state_time > 20)
			{
				e->state = MC_FIRE;
			}
		break;
		
		case MC_FIRE:			// firing
			e->state = MC_FIRE+1;
			e->frame = 2;	// close mouth again
			e->state_time = 0;
			e->xmark = e->x + (random(24, 48) << CSF);
			e->ymark = e->y + (random(-4, 4) << CSF);
		case MC_FIRE+1:
			if (++e->state_time > 50)
			{
				e->state = MC_FIRED;
				e->frame = 0;
			}
			else if (e->state_time==1 || e->state_time==3)
			{
				// fire at player at speed (2<<CSF) with 2 degrees of variance
				EmFireAngledShot(o, OBJ_MINICORE_SHOT, 2, 2<<CSF);
				sound(SND_EM_FIRE);
			}
		break;
		
		
		case MC_RETREAT:		// defeated!
			e->state = MC_RETREAT+1;
			e->frame = 2;
			e->x_speed = e->y_speed = 0;
		case MC_RETREAT+1:		// retreat back into the abyss
			e->x_speed += 0x20;
			if (e->x > ((map.xsize*TILE_W)<<CSF) + 0x4000)
			{
				e->Delete();
			}
		break;
	}
	
	if (e->state < MC_RETREAT)
	{
		// jump back when shot
		if (e->shaketime)
		{
			e->xmark += 0x400;
		}
		
		e->x += (e->xmark - e->x) / 16;
		e->y += (e->ymark - e->y) / 16;
	}
	
	// don't let them kill us
	e->hp = 1000;
	
	// invincible when mouth is closed
	if (e->frame != 2)
		e->flags &= ~FLAG_INVULNERABLE;
	else
		e->flags |= FLAG_INVULNERABLE;
}

void ai_minicore_shot(Entity *e)
{
	if (++e->state_time2 > 150)
	{
		effect(e->CenterX(), e->CenterY(), EFFECT_FISHY);
		e->Delete();
	}
	
	ai_animate2(o);
}
// shutter made noise when opening
// curly looks up at no 4

void ai_core_ghostie(Entity *e)
{
char hit = 0;

	if (e->x_speed > 0 && e->blockr) hit = 1;
	if (e->x_speed < 0 && e->blockl) hit = 1;
	if (e->y_speed > 0 && e->blockd) hit = 1;
	if (e->y_speed < 0 && e->blocku) hit = 1;
	
	e->x_speed -= 0x20;
	LIMITX(0x400);
	
	if (hit)
	{
		effect(e->CenterX(), e->CenterY(), EFFECT_FISHY);
		e->Delete();
	}
	
	ai_animate2(o);
}

void ai_core_blast(Entity *e)
{
	if (++e->state_time > 200) e->Delete();
	ANIMATE(2, 0, 1);
}
*/
