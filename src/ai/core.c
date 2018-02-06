#include "ai_common.h"

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
#define OPEN_MOUTH ({                                                                          \
	if(e->damage_time & 2) {                                                                   \
		                                                                                       \
	} else {                                                                                   \
		                                                                                       \
	}                                                                                          \
	if(!pieces[CFRONT]->mouth_open) {                                                          \
		pieces[CFRONT]->frame = 1;                                                             \
		pieces[CFRONT]->mouth_open = TRUE;                                                     \
	}                                                                                          \
})

// makes the core close his mouth
#define CLOSE_MOUTH ({                                                                         \
	if(pieces[CFRONT]->mouth_open) {                                                           \
		pieces[CFRONT]->frame = 0;                                                             \
		pieces[CFRONT]->mouth_open = FALSE;                                                    \
	}                                                                                          \
})

// bring the water up if it's not already up, but don't keep it up
// if it's already been up on it's own because that's not fair
#define START_WATER_STREAM ({                                                                  \
	if (water_entity->state == WL_DOWN) water_entity->state = WL_UP;                           \
	camera_shake(100);                                                                         \
})

// bring the water down again if it's not already
#define STOP_WATER_STREAM ({                                                                   \
	if (water_entity->state == WL_UP) water_entity->state = WL_CYCLE;                          \
})

// Minicore frame indeces
static const uint16_t mframeindex[5] = {
	TILE_BACKINDEX, 		// Face - mouth closed
	TILE_BACKINDEX + 16,	// Face - mouth open
	TILE_BACKINDEX + 32,	// Back
	TILE_BACKINDEX + 44,	// Bottom part of face
	TILE_BACKINDEX + 48		// Bottom part of back
};
	
// called at the entry to the Core room.
// initilize all the pieces of the Core boss.
void onspawn_core(Entity *e) {
	e->state = CORE_SLEEP;
	e->eflags = NPC_SHOWDAMAGE;
	
	e->x = (1150 << CSF);
	e->y = (212 << CSF);
	e->x_speed = 0;
	e->y_speed = 0;
	e->health = 650;
	e->hurtSound = SND_ENEMY_HURT_COOL;
	e->hit_box = (bounding_box) { 3*8, 4*8, 3*8, 4*8 };
	
	// spawn all the pieces in the correct z-order
	pieces[3] = entity_create(0, 0, OBJ_MINICORE, 0);
	pieces[4] = entity_create(0, 0, OBJ_MINICORE, 0);
	pieces[CFRONT] = entity_create(0, 0, OBJ_CORE_FRONT, 0);
	pieces[CBACK] = entity_create(0, 0, OBJ_CORE_BACK, 0);
	pieces[0] = entity_create(0, 0, OBJ_MINICORE, 0);
	pieces[1] = entity_create(0, 0, OBJ_MINICORE, 0);
	pieces[2] = entity_create(0, 0, OBJ_MINICORE, 0);
	
	// set up the front piece
	pieces[CFRONT]->linkedEntity = e;
	pieces[CFRONT]->eflags = NPC_SHOOTABLE | NPC_INVINCIBLE;
	pieces[CFRONT]->hit_box = (bounding_box) { 3*8+4, 4*8+4, 3*8+4, 4*8+4 };
	pieces[CFRONT]->display_box = (bounding_box) { 4*8, 6*8, 4*8, 6*8 };
	
	// set up our back piece
	pieces[CBACK]->linkedEntity = e;
	pieces[CBACK]->eflags |= NPC_SHOOTABLE | NPC_INVINCIBLE;
	pieces[CBACK]->hit_box = (bounding_box) { 6*8, 5*8, 2*8, 5*8 };
	pieces[CBACK]->display_box = (bounding_box) { 6*8, 6*8, 6*8, 6*8 };
	
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

	for(uint8_t i = 0; i < 5; i++) {
		pieces[i]->eflags = (NPC_SHOOTABLE | NPC_INVINCIBLE | NPC_IGNORESOLID);
		pieces[i]->health = 1000;
		pieces[i]->state = MC_SLEEP;
		pieces[i]->hurtSound = SND_ENEMY_HURT_BIG;
		pieces[i]->hit_box = (bounding_box) { 24, 12, 0, 12 };
	}
	
	// Upload some tile data for the minicore sprites into the background section
	
	SHEET_LOAD(&SPR_MiniCore1, 1, 16, mframeindex[0], 1, 0,0);
	SHEET_LOAD(&SPR_MiniCore2, 1, 16, mframeindex[1], 1, 0,0);
	SHEET_LOAD(&SPR_MiniCore3, 1, 12, mframeindex[2], 1, 0,0);
	SHEET_LOAD(&SPR_MiniCore4, 1, 4, mframeindex[3], 1, 0,0);
	SHEET_LOAD(&SPR_MiniCore5, 1, 2, mframeindex[4], 1, 0,0);
	
}

// We never need to know the core controller's ID but need an extra uint16_t variable to save hp
#define savedhp		id
#define mouth_open	grounded

void ai_core(Entity *e) {
	uint8_t do_thrust = FALSE;

	switch(e->state) {
		case CORE_SLEEP: break;			// core is asleep
		// Core's mouth is closed.
		// Core targets player point but does not update it during the state.
		// This is also the state set via BOA to awaken the core.
		case CORE_CLOSED:
		{
			e->state++;
			e->timer = 0;
			
			STOP_WATER_STREAM;
			e->x_mark = player.x;
			e->y_mark = player.y;
		}
		/* fallthrough */
		case CORE_CLOSED+1:
		{
			// open mouth after 400 ticks
			if (e->timer > TIME(400)) {
				if (++e->timer2 > 3) {
					// every 3rd time do gusting left and big core blasts
					e->timer2 = 0;
					e->state = CORE_GUST;
				} else {
					e->state = CORE_OPEN;
				}
				
				do_thrust = TRUE;
			}
		}
		break;
		// Core's mouth is open.
		// Core moves towards player, and updates the position throughout
		// the state (is "aggressive" about seeking him).
		// Core fires ghosties, and curly targets it.
		case CORE_OPEN:
		{
			e->state++;
			e->timer = 0;
			// gonna open mouth, so save the current HP so we'll
			// know how much damage we've taken this time.
			e->savedhp = e->health;
		}
		/* fallthrough */
		case CORE_OPEN+1:
		{
			e->x_mark = player.x;
			e->y_mark = player.y;
			
			// must call constantly for red-flashing when hit
			OPEN_MOUTH;
			
			e->timer++;
			// hint curly to target us
			if ((e->timer % TIME(64)) == 1) {
				CURLY_TARGET_HERE(e);
			}
			
			// spawn ghosties
			if (e->timer < TIME(200)) {
				if ((e->timer % TIME(20))==0) {
					entity_create(e->x + ((-48 + (random() & 63)) << CSF),
						     	  e->y + ((-64 + (random() & 127)) << CSF),
							 	  OBJ_CORE_GHOSTIE, 0);
				}
			}
			
			// close mouth when 400 ticks have passed or we've taken more than 200 damage
			if (e->timer > TIME(400) || (e->savedhp - e->health) >= 200) {
				e->state = CORE_CLOSED;
				CLOSE_MOUTH;
				do_thrust = TRUE;
			}
		}
		break;
		case CORE_GUST:
		{
			e->state++;
			e->timer = 0;
			
			START_WATER_STREAM;
		}
		/* fallthrough */
		case CORE_GUST+1:
		{
			// Instead of spawning a bunch of laggy 1 pixel objects push the water to the left
			backScrollTimer++;
			
			player.x_speed -= SPEED(0x20);
			
			OPEN_MOUTH;
			
			e->timer++;
			// spawn the big white blasts
			if (e->timer==TIME(300) || e->timer==TIME(350) || e->timer==TIME(400)) {
				Entity *shot = entity_create(pieces[CFRONT]->x, pieces[CFRONT]->y,
											 OBJ_CORE_BLAST, 0);
				THROW_AT_TARGET(shot, player.x, player.y, SPEED(0x600));
				sound_play(SND_LIGHTNING_STRIKE, 5);
			}
			
			if (e->timer > TIME(400)) {
				e->state = CORE_CLOSED;
				CLOSE_MOUTH;
				do_thrust = TRUE;
			}
		}
		break;
		case 500:		// defeated!!
		{
			STOP_WATER_STREAM;
			water_entity->state = WL_CALM;
			
			e->state = 501;
			e->timer = 0;
			e->x_speed = e->y_speed = 0;
			curly_target_time = 0;
			
			CLOSE_MOUTH;
			
			camera_shake(20);
			
			// tell all the MC's to retreat
			for(uint8_t i = 0; i < 5; i++) {
				pieces[i]->eflags &= ~(NPC_SHOOTABLE & NPC_INVINCIBLE);
				pieces[i]->state = MC_RETREAT;
			}
		}
		/* fallthrough */
		case 501:
		{
			e->timer++;
			if ((e->timer & 15) == 1) {
				effect_create_smoke((e->x >> CSF) - 80 + (random() % 160), 
									(e->y >> CSF) - 50 + (random() % 100));
			}
			
			if (e->timer & 2)
				e->x -= (1 << CSF);
			else
				e->x += (1 << CSF);
			
			#define CORE_DEATH_TARGET_X		0x72000
			#define CORE_DEATH_TARGET_Y		0x16000
			e->x_speed += (e->x > CORE_DEATH_TARGET_X) ? -SPEED(0x80) : SPEED(0x80);
			e->y_speed += (e->y > CORE_DEATH_TARGET_Y) ? -SPEED(0x80) : SPEED(0x80);
		}
		break;
		case 600:			// teleported away by Misery
		{
			e->x_speed = 0;
			e->y_speed = 0;
			e->state++;
			e->timer = 60;
		}
		/* fallthrough */
		case 601:
		{
			if (--e->timer == 0) {
				pieces[CFRONT]->hidden = TRUE;
				pieces[CBACK]->hidden = TRUE;
				
				for(uint8_t i = 0; i < 7; i++) pieces[i]->state = STATE_DELETE;
				e->state = STATE_DELETE;
				return;
			}
		}
		break;
	}
	
	if (do_thrust) {
		// tell all the minicores to jump to a new position
		for(uint8_t i = 0; i < 5; i++) {
			pieces[i]->state = MC_THRUST;
		}
		
		camera_shake(20);
		sound_play(SND_CORE_THRUST, 5);
	}
	
	// fire the minicores in any awake non-dead state
	if (e->state >= CORE_CLOSED && e->state < 500) {
		e->timer++;
		
		// fire off each minicore sequentially...
		if(e->timer == TIME(80+  0)) pieces[0]->state = MC_CHARGE_FIRE;
		if(e->timer == TIME(80+ 30)) pieces[1]->state = MC_CHARGE_FIRE;
		if(e->timer == TIME(80+ 60)) pieces[2]->state = MC_CHARGE_FIRE;
		if(e->timer == TIME(80+ 90)) pieces[3]->state = MC_CHARGE_FIRE;
		if(e->timer == TIME(80+120)) pieces[4]->state = MC_CHARGE_FIRE;
		
		// move main core towards a spot in front of target
		e->x_speed += (e->x > (e->x_mark + (48<<CSF))) ? -4 : 4;
		e->y_speed += (e->y > e->y_mark) ? -4 : 4;
	}
	
	// set up our shootable status--you never actually hit the core (CFRONT),
	// but if it's mouth is open, make us, the invisible controller object, shootable.
	if (pieces[CFRONT]->mouth_open) {
		e->eflags |= NPC_SHOOTABLE;
		pieces[CFRONT]->eflags &= ~(NPC_INVINCIBLE | NPC_SHOOTABLE);
	} else {
		e->eflags &= ~NPC_SHOOTABLE;
		pieces[CFRONT]->eflags |= NPC_INVINCIBLE | NPC_SHOOTABLE;
	}
	
	LIMIT_X(SPEED(0x80));
	LIMIT_Y(SPEED(0x80));
	
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ondeath_core(Entity *e) {
	e->state = 500;
	e->timer = 0;
	e->eflags = 0;
	tsc_call_event(e->event);
}

// the front (mouth) piece of the main core
void ai_core_front(Entity *e) {
	if (!bossEntity) { e->state = STATE_DELETE; return; }
	
	e->x = bossEntity->x;
	e->y = bossEntity->y;
}

// the back (unanimated) piece of the main core
void ai_core_back(Entity *e) {
	if (!bossEntity) { e->state = STATE_DELETE; return; }
	// Align with front so we look like one big entity
	e->x = bossEntity->x + ((pieces[CFRONT]->display_box.right + e->display_box.left) << CSF);
	e->y = bossEntity->y;
}

void ai_minicore(Entity *e) {
	if (!bossEntity) { e->state = STATE_DELETE; return; }
	
	switch(e->state) {
		case MC_SLEEP:		// idle & mouth closed
		{
			e->mouth_open = FALSE;
			e->state = MC_SLEEP+1;
		}
		/* fallthrough */
		case MC_SLEEP+1:
		{
			e->x_mark = e->x;
			e->y_mark = e->y;
		}
		break;
		case MC_THRUST:			// thrust (move to random new pos)
		{
			e->state = MC_THRUST+1;
			e->mouth_open = FALSE;
			e->timer = 0;
			e->x_mark = e->x + ((-128 + (random() % 160)) << CSF);
			e->y_mark = e->y + ((-64 + (random() % 128)) << CSF);
		}
		/* fallthrough */
		case MC_THRUST+1:
		{
			if (++e->timer > TIME(50)) e->mouth_open = TRUE;
		}
		break;
		case MC_CHARGE_FIRE:			// charging for fire
		{
			e->state = MC_CHARGE_FIRE+1;
			e->timer = 0;
		}
		/* fallthrough */
		case MC_CHARGE_FIRE+1:			// flash blue
		{
			if(++e->timer > TIME(20)) e->state = MC_FIRE;
		}
		break;
		case MC_FIRE:			// firing
		{
			e->state = MC_FIRE+1;
			e->mouth_open = FALSE;	// close mouth again;
			e->timer = 0;
			e->x_mark = e->x + ((24 + (random() & 15)) << CSF);
			e->y_mark = e->y + ((-4 + (random() & 7)) << CSF);
		}
		/* fallthrough */
		case MC_FIRE+1:
		{
			if (++e->timer > TIME(50)) {
				e->state = MC_FIRED;
				e->mouth_open = TRUE;
			} else if (e->timer==1 || e->timer==3) {
				// fire at player at speed (2<<CSF) with 2 degrees of variance
				Entity *shot = entity_create(e->x - 0x1000, e->y,
											 OBJ_MINICORE_SHOT, 0);
				THROW_AT_TARGET(shot, player.x, player.y, SPEED(0x400));
				sound_play(SND_EM_FIRE, 5);
			}
		}
		break;
		case MC_RETREAT:		// defeated!
		{
			e->state = MC_RETREAT+1;
			e->mouth_open = FALSE;
			e->x_speed = e->y_speed = 0;
		}
		/* fallthrough */
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
		if (e->damage_time) e->x_mark += 0x400;
		
		e->x += (e->x_mark - e->x) >> 4;
		e->y += (e->y_mark - e->y) >> 4;
	}
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	// Have to deal with sprites manually
	e->sprite[0] = (VDPSprite) { // Face
		.x = (e->x>>CSF) - (camera.x>>CSF) + SCREEN_HALF_W - 28 + 128,
		.y = (e->y>>CSF) - (camera.y>>CSF) + SCREEN_HALF_H - 20 + 128,
		.size = SPRITE_SIZE(4, 4),
		.attribut = TILE_ATTR_FULL(PAL2,0,0,0,mframeindex[e->mouth_open])
	};
	e->sprite[1] = (VDPSprite) { // Back
		.x = (e->x>>CSF) - (camera.x>>CSF) + SCREEN_HALF_W + 4 + 128,
		.y = (e->y>>CSF) - (camera.y>>CSF) + SCREEN_HALF_H - 20 + 128,
		.size = SPRITE_SIZE(3, 4),
		.attribut = TILE_ATTR_FULL(PAL2,0,0,0,mframeindex[2])
	};
	e->sprite[2] = (VDPSprite) { // Bottom-Face
		.x = (e->x>>CSF) - (camera.x>>CSF) + SCREEN_HALF_W - 28 + 128,
		.y = (e->y>>CSF) - (camera.y>>CSF) + SCREEN_HALF_H + 12 + 128,
		.size = SPRITE_SIZE(4, 1),
		.attribut = TILE_ATTR_FULL(PAL2,0,0,0,mframeindex[3])
	};
	e->sprite[3] = (VDPSprite) { // Bottom-Back
		.x = (e->x>>CSF) - (camera.x>>CSF) + SCREEN_HALF_W + 4 + 128,
		.y = (e->y>>CSF) - (camera.y>>CSF) + SCREEN_HALF_H + 12 + 128,
		.size = SPRITE_SIZE(2, 1),
		.attribut = TILE_ATTR_FULL(PAL2,0,0,0,mframeindex[4])
	};
	
	// don't let them kill us
	e->health = 1000;
	
	// invincible when mouth is closed
	if (e->mouth_open)
		e->eflags &= ~NPC_INVINCIBLE;
	else
		e->eflags |= NPC_INVINCIBLE;
}

void ai_minicore_shot(Entity *e) {
	e->x += e->x_speed;
	e->y += e->y_speed;
	ANIMATE(e, 4, 0,1,2);
	if (blk(e->x, 0, e->y, 0) == 0x41) {
		effect_create_smoke(e->x >> CSF, e->y >> CSF);
		e->state = STATE_DELETE;
	}
}

void ai_core_ghostie(Entity *e) {
	e->x_speed -= SPEED(0x20);
	if(e->x_speed < -SPEED(0x400)) e->x_speed = -SPEED(0x400);
	e->x += e->x_speed;
	ANIMATE(e, 4, 0,1,2);
	if (blk(e->x, 0, e->y, 0) == 0x41) {
		effect_create_smoke(e->x >> CSF, e->y >> CSF);
		e->state = STATE_DELETE;
	}
}

void ai_core_blast(Entity *e) {
	e->x += e->x_speed;
	e->y += e->y_speed;
	if (++e->timer > TIME(200)) e->state = STATE_DELETE;
}
