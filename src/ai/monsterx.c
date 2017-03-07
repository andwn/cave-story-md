#include "ai_common.h"

#define STATE_X_APPEAR				1		// script-triggered: must stay constant
#define STATE_X_FIGHT_BEGIN			10		// script-triggered: must stay constant
#define STATE_X_TRAVEL				20
#define STATE_X_BRAKE				30
#define STATE_X_OPEN_DOORS			40
#define STATE_X_FIRE_TARGETS		50
#define STATE_X_FIRE_FISHIES		60
#define STATE_X_CLOSE_DOORS			70
#define STATE_X_EXPLODING			80

#define STATE_DOOR_OPENING			10		// makes the doors open
#define STATE_DOOR_OPENING_PARTIAL	20		// makes the doors open part-way
#define STATE_DOOR_CLOSING			30		// closes the doors
#define STATE_DOOR_FINISHED			40		// doors are finished moving

#define STATE_TREAD_STOPPED			20
#define STATE_TREAD_RUN				30
#define STATE_TREAD_BRAKE			40

#define STATE_FISHSPAWNER_FIRE		10
#define STATE_TARGET_FIRE			10

#define DOORS_OPEN_DIST			(32 << CSF)		// how far the doors open
#define DOORS_OPEN_FISHY_DIST	(20 << CSF)		// how far the doors open during fish-missile phase

#define BODY_UL_X	-64
#define BODY_UL_Y	-40
#define BODY_UR_X	 32
#define BODY_UR_Y	-40
#define BODY_LL_X	-64
#define BODY_LL_Y	 8
#define BODY_LR_X	 32
#define BODY_LR_Y	 8

#define ARENA_LEFT		0x106800
#define ARENA_RIGHT		0x15B800
#define ARENA_BOTTOM	0x21800

#define saved_health		curly_target_time
#define alt_sheet			jump_time
#define fish_timer			id

// For fish missile only
#define want_angle	jump_time
#define cur_angle	underwater

enum Pieces {
	TREADUL, TREADUR, TREADLL, TREADLR,
	TARGET1, TARGET2, TARGET3, TARGET4,
	DOORL, DOORR,
};

// the treads start moving at slightly different times
// which we change direction, etc.
static const uint16_t tread_turnon_times[] = { 4, 8, 10, 12 };

// return true if all the targets behind the doors have been destroyed.
static uint8_t all_targets_destroyed() {
	for(uint8_t i=TARGET1;i<TARGET1+4;i++) {
		if (!pieces[i]->hidden) return FALSE;
	}
	return TRUE;
}

static void spawn_fish(uint8_t index) {
	// keep appropriate position relative to main object
	//                               UL          UR          LL          LR
	static const int32_t xoffs[]   = { -(64<<CSF),  (76<<CSF), -(64<<CSF),  (76<<CSF) };
	static const int32_t yoffs[]   = {  (27<<CSF),  (27<<CSF), -(16<<CSF), -(16<<CSF) };
	static const uint8_t angles[]  = { A_LEFT+0x20,  A_UP+0x20,A_DOWN+0x20, A_RIGHT+0x20 };
	// Create manually because cur_angle needs to be set
	// Then let the missile itself deal with setting speeds with sin/cos
	Entity *fish = entity_create(bossEntity->x + xoffs[index], bossEntity->y + yoffs[index],
								 OBJ_X_FISHY_MISSILE, 0);
	fish->cur_angle = angles[index];
	fish->nflags &= ~NPC_INVINCIBLE;
	fish->eflags &= ~NPC_INVINCIBLE;
	fish->nflags &= ~NPC_SHOOTABLE;
	fish->eflags |= NPC_SHOOTABLE;
	fish->health = 1;
	//FIRE_ANGLED_SHOT(OBJ_X_FISHY_MISSILE, 
	//				 bossEntity->x + xoffs[index], bossEntity->y + yoffs[index], 
	//				 0x80 + index * 0x100, SPEED(0x400));
	sound_play(SND_EM_FIRE, 3);
}

// sets state on an array on objects
static void set_states(Entity *e[], uint8_t n, uint16_t state) {
	for(uint8_t i = 0; i < n; i++) e[i]->state = state;
}

void onspawn_monsterx(Entity *e) {
	e->alwaysActive = TRUE;
	e->health = 700;
	e->state = STATE_X_APPEAR;
	e->x = (130 * 16) << CSF;
	e->y = (208 << CSF);
	e->eflags = NPC_IGNORESOLID;
	SHEET_FIND(e->alt_sheet, SHEET_XBODY);
}

// The 4 green things look slightly different
void onspawn_x_target(Entity *e) {
	e->alwaysActive = TRUE;
	e->hurtSound = SND_ENEMY_HURT;
	e->health = 60;
	e->eflags |= NPC_SHOWDAMAGE;
	if(e->eflags & NPC_OPTION1) e->frame += 1;
	if(e->eflags & NPC_OPTION2) e->frame += 2;
	
	static const int32_t xoffs[] = { -(22 <<CSF),  28 <<CSF, -(15 <<CSF),  17 <<CSF };
	static const int32_t yoffs[] = { -(16 <<CSF), -(16 <<CSF),  14 <<CSF,  14 <<CSF };
	e->x_mark = xoffs[e->frame];
	e->y_mark = yoffs[e->frame];
}

// Change sprite vflip for top treads
// The real game uses 4 different sprites
void onspawn_x_tread(Entity *e) {
	e->alwaysActive = TRUE;
	e->eflags |= NPC_SPECIALSOLID;
	SHEET_FIND(e->alt_sheet, SHEET_XTREAD);
	e->hit_box = (bounding_box) { 32, 8, 32, 16 };
	e->display_box = (bounding_box) { 32, 16, 32, 16 };
}

// Door on the right uses the second frame
void onspawn_x_door(Entity *e) {
	e->alwaysActive = TRUE;
	e->display_box = (bounding_box) { 24, 24, 24, 24 };
	if(e->eflags & NPC_OPTION2) e->frame = 1;
}

void onspawn_x_internals(Entity *e) {
	e->alwaysActive = TRUE;
	e->eflags |= NPC_SHOWDAMAGE;
	e->hurtSound = SND_ENEMY_HURT_COOL;
	e->hit_box = (bounding_box) { 24, 16, 24, 16 };
	e->display_box = (bounding_box) { 36, 24, 36, 24 };
	e->health = 1000;
}

void ai_monsterx(Entity *e) {
	switch(e->state) {
		// script triggered us to initilize/appear
		// (there is a hvtrigger, right before player first walks by us
		// and sees us inactive, which sends us this ANP).
		case STATE_X_APPEAR: 
		{
			e->state++;
			// back -> front order
			// create internals
			e->linkedEntity = entity_create(0, 0, OBJ_X_INTERNALS, 0);
			// create targets
			pieces[TARGET1] = entity_create(0, 0, OBJ_X_TARGET, 0);
			pieces[TARGET2] = entity_create(0, 0, OBJ_X_TARGET, NPC_OPTION1);
			pieces[TARGET3] = entity_create(0, 0, OBJ_X_TARGET, NPC_OPTION2);
			pieces[TARGET4] = entity_create(0, 0, OBJ_X_TARGET, NPC_OPTION1|NPC_OPTION2);
			// create treads
			pieces[TREADUL] = entity_create(0xfc000, 0x14000, OBJ_X_TREAD, 0);
			pieces[TREADUR] = entity_create(0x10c000,0x14000, OBJ_X_TREAD, NPC_OPTION1);
			pieces[TREADLL] = entity_create(0xfc000, 0x20000, OBJ_X_TREAD, NPC_OPTION2);
			pieces[TREADLR] = entity_create(0x10c000,0x20000, OBJ_X_TREAD, NPC_OPTION1|NPC_OPTION2);
			// create doors
			pieces[DOORL] = entity_create(0, 0, OBJ_X_DOOR, 0);
			pieces[DOORR] = entity_create(0, 0, OBJ_X_DOOR, NPC_OPTION2);
		}
		break;
		case STATE_X_APPEAR+1: break;
		
		// script has triggered the fight to begin
		case STATE_X_FIGHT_BEGIN:
		{
			// For some reason the boss entity changes to a flying gaudi... why though
			bossEntity = e;
			e->timer = 0;
			e->state++;
		}
		case STATE_X_FIGHT_BEGIN+1:
		{
			if (++e->timer > 100) {
				FACE_PLAYER(e);
				e->timer = 0;
				e->state = STATE_X_TRAVEL;
			}
		}
		break;
		
		// starts the treads and moves us in the currently-facing direction
		case STATE_X_TRAVEL:
		{
			// count number of times we've traveled, we brake
			// and attack every third time.
			e->timer2++;
			
			e->timer = 0;
			e->state++;
		}
		case STATE_X_TRAVEL+1:
		{
			e->timer++;
			
			// trigger the treads to start moving,
			// and put them slightly out of sync with each-other.
			for(uint8_t i=0;i<4;i++) {
				if (e->timer == tread_turnon_times[i]) {
					pieces[TREADUL+i]->state = STATE_TREAD_RUN;
					pieces[TREADUL+i]->dir = e->dir;
				}
			}
			
			if (e->timer > 120) {
				// time to attack? we attack every 3rd travel
				// if so skid to a stop, that's the first step.
				if (e->timer2 >= 3) {
					e->timer2 = 0;
					
					e->dir ^= 1;
					e->state = STATE_X_BRAKE;
					e->timer = 0;
				} else {
					// passed player? skid and turn around.
					if ((e->dir && e->x > player.x) ||
					 	(!e->dir && e->x < player.x)) {
						e->dir ^= 1;
						e->state = STATE_X_TRAVEL;
					}
				}
			}
		}
		break;
		
		// skidding to a stop in preparation to attack
		case STATE_X_BRAKE:
		{
			e->timer = 0;
			e->state++;
		}
		case STATE_X_BRAKE+1:
		{
			e->timer++;
			
			// trigger the treads to start braking,
			// and put them slightly out of sync with each-other.
			for(uint8_t i=0;i<4;i++) {
				if (e->timer == tread_turnon_times[i]) {
					pieces[TREADUL+i]->state = STATE_TREAD_BRAKE;
					pieces[TREADUL+i]->dir = e->dir;
				}
			}
			
			if (e->timer > 50) {
				e->state = STATE_X_OPEN_DOORS;
				e->timer = 0;
			}
		}
		break;
		
		// doors opening to attack
		case STATE_X_OPEN_DOORS:
		{
			e->timer = 0;
			saved_health = e->health;
			
			// select type of attack depending on where we are in the battle
			if (!all_targets_destroyed()) {
				set_states(&pieces[DOORL], 2, STATE_DOOR_OPENING);
				e->state = STATE_X_FIRE_TARGETS;
			} else {
				set_states(&pieces[DOORL], 2, STATE_DOOR_OPENING_PARTIAL);
				e->state = STATE_X_FIRE_FISHIES;
			}
		}
		break;
		
		// firing targets (early battle)
		case STATE_X_FIRE_TARGETS:
		{
			if (pieces[DOORL]->state == STATE_DOOR_FINISHED) {
				pieces[DOORL]->state = 0;
				set_states(&pieces[TARGET1], 4, STATE_TARGET_FIRE);
			}
			
			if (++e->timer > TIME(300) || all_targets_destroyed()) {
				e->state = STATE_X_CLOSE_DOORS;
				e->timer = 0;
			}
		}
		break;
		
		// firing fishy missiles (late battle)
		case STATE_X_FIRE_FISHIES:
		{
			if (pieces[DOORL]->state == STATE_DOOR_FINISHED) {
				pieces[DOORL]->state = 0;
				e->linkedEntity->eflags |= NPC_SHOOTABLE;
			}
			
			if (++e->timer > TIME(300) || (saved_health - e->health) > 200) {
				e->state = STATE_X_CLOSE_DOORS;
				e->timer = 0;
			} else if(e->timer > TIME(50) && e->timer % TIME(50) == 1) {
				// Recycling useless underwater var to cycle fish index
				if(e->underwater > 3) e->underwater = 0;
				spawn_fish(e->underwater);
				e->underwater++;
			}
		}
		break;
		
		// doors closing after attack
		case STATE_X_CLOSE_DOORS:
		{
			e->timer = 0;
			e->state++;
			
			set_states(&pieces[DOORL], 2, STATE_DOOR_CLOSING);
		}
		case STATE_X_CLOSE_DOORS+1:
		{
			if (pieces[DOORL]->state == STATE_DOOR_FINISHED) {
				pieces[DOORL]->state = 0;
				
				// just turn off everything for both types of attacks;
				// turning off the attack type that wasn't enabled isn't harmful.
				set_states(&pieces[TARGET1], 4, 0);
				e->linkedEntity->eflags &= ~NPC_SHOOTABLE;
			}
			
			if (++e->timer > 50) {
				FACE_PLAYER(e);
				e->state = STATE_X_TRAVEL;
				e->timer = 0;
			}
		}
		break;
		
		// exploding
		case STATE_X_EXPLODING:
		{
			entities_clear_by_type(OBJ_X_FISHY_MISSILE);
			
			e->timer = 0;
			e->state++;
		}
		case STATE_X_EXPLODING+1:
		{
			camera_shake(2);
			e->timer++;
			
			if ((e->timer % TIME(8)) == 0)
				sound_play(SND_ENEMY_HURT_BIG, 5);
			
			//SmokePuff(e->CenterX() + (random(-72, 72) << CSF),
			//		  e->CenterY() + (random(-64, 64) << CSF));
			
			if (e->timer > TIME(100)) {
				SCREEN_FLASH(30);
				sound_play(SND_EXPLOSION1, 5);
				e->timer = 0;
				e->state++;
			}
		}
		break;
		case STATE_X_EXPLODING+2:
		{
			camera_shake(40);
			e->timer++;
			// Delete this stuff once the screen starts to flash as opposed
			// to the real game, where there is an explosion that takes time to
			// cover the screen
			if (e->timer == 1) {
				entity_create(e->x, e->y - (24 << CSF), OBJ_X_DEFEATED, 0);
				entities_clear_by_type(OBJ_X_TARGET);
				entities_clear_by_type(OBJ_X_DOOR);
				entities_clear_by_type(OBJ_X_TREAD);
				entities_clear_by_type(OBJ_X_INTERNALS);
				player.grounded = FALSE; // Stop player from floating where treads were
				playerPlatform = NULL; // I SAID STOP DAMMIT
				e->state = STATE_DELETE;
				return;
			}
		}
		break;
	}
	
	// Prevent clipping through walls/floor
	if(e->state >= STATE_X_FIGHT_BEGIN) {
		if(player.x < ARENA_LEFT) player.x = ARENA_LEFT;
		if(player.x > ARENA_RIGHT) player.x = ARENA_RIGHT;
		if(player.y > ARENA_BOTTOM) player.y = ARENA_BOTTOM;
	}
	
	// main object pulled along as treads move
	int32_t tread_center = (pieces[TREADUL]->x + pieces[TREADUR]->x +
					 	pieces[TREADLL]->x + pieces[TREADLR]->x) / 4;
	e->x += (tread_center - e->x) / 16;
	
	// Fill in sprites for the body
	e->sprite[0] = (VDPSprite) {
		.x = (e->x >> CSF) - (camera.x>>CSF) + SCREEN_HALF_W + BODY_UL_X + 128, 
		.y = (e->y >> CSF) - (camera.y>>CSF) + SCREEN_HALF_H + BODY_UL_Y + 128, 
		.size = SPRITE_SIZE(4, 4), 
		.attribut = TILE_ATTR_FULL(PAL3,0,0,0,sheets[e->alt_sheet].index),
	};
	e->sprite[1] = (VDPSprite) {
		.x = (e->x >> CSF) - (camera.x>>CSF) + SCREEN_HALF_W + BODY_UR_X + 128,
		.y = (e->y >> CSF) - (camera.y>>CSF) + SCREEN_HALF_H + BODY_UR_Y + 128,
		.size = SPRITE_SIZE(4, 4), 
		.attribut = TILE_ATTR_FULL(PAL3,0,0,1,sheets[e->alt_sheet].index),
	};
	e->sprite[2] = (VDPSprite) {
		.x = (e->x >> CSF) - (camera.x>>CSF) + SCREEN_HALF_W + BODY_LL_X + 128,
		.y = (e->y >> CSF) - (camera.y>>CSF) + SCREEN_HALF_H + BODY_LL_Y + 128,
		.size = SPRITE_SIZE(4, 4), 
		.attribut = TILE_ATTR_FULL(PAL3,0,1,0,sheets[e->alt_sheet].index),
	};
	e->sprite[3] = (VDPSprite) {
		.x = (e->x >> CSF) - (camera.x>>CSF) + SCREEN_HALF_W + BODY_LR_X + 128,
		.y = (e->y >> CSF) - (camera.y>>CSF) + SCREEN_HALF_H + BODY_LR_Y + 128,
		.size = SPRITE_SIZE(4, 4), 
		.attribut = TILE_ATTR_FULL(PAL3,0,1,1,sheets[e->alt_sheet].index),
	};
}

void ai_x_tread(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->eflags |= (NPC_SOLID | NPC_INVINCIBLE | NPC_FRONTATKONLY);
			e->state = STATE_TREAD_STOPPED;
		}
		case STATE_TREAD_STOPPED:
		{
			e->frame = 0;
			e->attack = 0;
			e->eflags &= ~NPC_BOUNCYTOP;
		}
		break;
		
		case STATE_TREAD_RUN:
		{
			e->eflags |= NPC_BOUNCYTOP;
			e->timer = 0;
			e->animtime = 0;
			
			e->state++;
		}
		case STATE_TREAD_RUN+1:
		{
			ANIMATE(e, 8, 1,0);
			ACCEL_X(0x20);
			
			if (++e->timer > 30) {
				e->eflags &= ~NPC_BOUNCYTOP;
				e->frame = 0;
				e->animtime = 0;
				e->state++;
			}
		}
		break;
		case STATE_TREAD_RUN+2:
		{
			ANIMATE(e, 8, 0,1);
			ACCEL_X(0x20);
			
			e->timer++;
		}
		break;
		
		case STATE_TREAD_BRAKE:
		{
			e->frame = 0;
			e->animtime = 0;
			
			e->eflags |= NPC_BOUNCYTOP;
			e->state++;
		}
		case STATE_TREAD_BRAKE+1:
		{
			ACCEL_X(0x20);
			
			if ((e->dir == 1 && e->x_speed > 0) ||
				(e->dir == 0 && e->x_speed < 0)) {
				e->x_speed = 0;
				e->state = STATE_TREAD_STOPPED;
			}
		}
		break;
	}
	
	// make motor noise
	switch(e->state) {
		case STATE_TREAD_RUN+1:
		case STATE_TREAD_BRAKE+1:
		{
			if (e->timer & 2)
				sound_play(SND_MOTOR_SKIP, 3);
		}
		break;
		
		case STATE_TREAD_RUN+2:
		{
			if ((e->timer % 8) == 1)
				sound_play(SND_MOTOR_RUN, 3);
		}
		break;
	}
	
	// determine if player is in a position where he could get run over.
	if (e->state > STATE_TREAD_STOPPED && e->x_speed) {
		if (abs(player.y - e->y) <= (5 << CSF))
			e->attack = 10;
		else
			e->attack = 0;
	} else {
		e->attack = 0;
	}
	
	LIMIT_X(SPEED(0x400));
	e->x += e->x_speed;
	
	// Sprite
	e->sprite[0] = (VDPSprite) {
		.x = (e->x>>CSF) - e->display_box.left - (camera.x>>CSF) + SCREEN_HALF_W + 128,
		.y = (e->y>>CSF) - e->display_box.top - (camera.y>>CSF) + SCREEN_HALF_H + 128,
		.size = SPRITE_SIZE(4, 4),
		.attribut = TILE_ATTR_FULL(PAL3,0,e->y < 0x18000,0,
				sheets[e->alt_sheet].index + (e->frame?32:0)),
	};
	e->sprite[1] = (VDPSprite) {
		.x = (e->x>>CSF) - e->display_box.left + 32 - (camera.x>>CSF) + SCREEN_HALF_W + 128,
		.y = (e->y>>CSF) - e->display_box.top - (camera.y>>CSF) + SCREEN_HALF_H + 128,
		.size = SPRITE_SIZE(4, 4),
		.attribut = TILE_ATTR_FULL(PAL3,0,e->y < 0x18000,0,
				sheets[e->alt_sheet].index+16 + (e->frame?32:0)),
	};
}

void ai_x_internals(Entity *e) {
	e->x = bossEntity->x;
	e->y = bossEntity->y + 0x800;
	
	// eye open during fight
	e->frame = (bossEntity->state < 10) ? 0 : 1;
	
	// link damage to main object
	if (e->health < 1000) {
		if(1000 - e->health > bossEntity->health) {
			bossEntity->health = 0;
			e->eflags &= ~(NPC_SHOOTABLE | NPC_SHOWDAMAGE);
			ENTITY_ONDEATH(bossEntity);
		} else {
			bossEntity->health -= 1000 - e->health;
		}
		e->health = 1000;
	}
}

void ai_x_door(Entity *e) {
	switch(e->state) {
		// doors opening all the way
		case STATE_DOOR_OPENING:
		{
			e->x_mark += (1 << CSF);
			if (e->x_mark >= DOORS_OPEN_DIST) {
				e->x_mark = DOORS_OPEN_DIST;
				e->state = STATE_DOOR_FINISHED;
			}
		}
		break;
		
		// doors opening partially for fish-missile launchers to fire
		case STATE_DOOR_OPENING_PARTIAL:
		{
			e->x_mark += (1 << CSF);
			if (e->x_mark >= DOORS_OPEN_FISHY_DIST) {
				e->x_mark = DOORS_OPEN_FISHY_DIST;
				e->state = STATE_DOOR_FINISHED;
			}
		}
		break;
		
		// doors closing
		case STATE_DOOR_CLOSING:
		{
			e->x_mark -= (1 << CSF);
			if (e->x_mark <= 0) {
				e->x_mark = 0;
				e->state = STATE_DOOR_FINISHED;
			}
		}
		break;
		
		// this is a signal to the main object that the doors
		// are finished with the last command.
		case STATE_DOOR_FINISHED:
		break;
	}
	
	// set position relative to main object.
	// doors open in opposite directions.
	if (!(e->eflags & NPC_OPTION2)) {
		e->x = bossEntity->x - e->x_mark - (24<<CSF);
	} else {
		e->x = bossEntity->x + e->x_mark + (24<<CSF);
	}
	e->y = bossEntity->y;
}

void ai_x_target(Entity *e) {
	// has this target been destroyed?
	// (we don't really kill the object until the battle is over,
	// to avoid having to deal with dangling pointers).
	if (e->hidden) return;
	
	switch(e->state) {
		case 0:
			e->eflags &= ~NPC_SHOOTABLE;
			e->frame &= 3;
			e->state = 1;
		break;
		
		case STATE_TARGET_FIRE:
		{
			e->timer = 40 + (NPC_OPTION1 ? 10 : 0) + (NPC_OPTION2 ? 20 : 0);
			e->eflags |= NPC_SHOOTABLE;
			e->state++;
		}
		case STATE_TARGET_FIRE+1:
		{
			if (--e->timer <= 16) {
				// flash shortly before firing
				if (e->timer & 2) e->frame |= 4;
							 else e->frame &= 3;
				
				if (e->timer == 0) {
					e->timer = 40;
					Entity *shot = entity_create(e->x, e->y, OBJ_GAUDI_FLYING_SHOT, 0);
					THROW_AT_TARGET(shot, player.x, player.y, SPEED(0x500));
					//EmFireAngledShot(e, OBJ_GAUDI_FLYING_SHOT, 2, 0x500);
					sound_play(SND_EM_FIRE, 3);
				}
			}
		}
		break;
	}
	
	e->x = bossEntity->linkedEntity->x + e->x_mark;
	e->y = bossEntity->linkedEntity->y + e->y_mark;
}

void ondeath_monsterx(Entity *e) {
	e->state = STATE_X_EXPLODING;
	e->eflags &= ~(NPC_SHOOTABLE | NPC_SHOWDAMAGE);
	e->damage_time = 150;
	tsc_call_event(e->event);
}

void ondeath_x_target(Entity *e) {
	//SmokeClouds(o, 8, 8, 8);
	sound_play(SND_LITTLE_CRASH, 5);
	
	e->eflags &= ~NPC_SHOOTABLE;
	e->hidden = TRUE;
}

void ai_x_fishy_missile(Entity *e) {
	e->eflags ^= NPC_SHOOTABLE;
	if(e->timer > TIME(600)) {
		effect_create_smoke(e->x >> CSF, e->y >> CSF);
		e->state = STATE_DELETE;
		return;
	}
	
	// Find angle needed to reach player
	// Don't do this every frame, arctan is expensive
	if(e->timer++ % TIME(20) == 0) {
		e->want_angle = get_angle(e->x, e->y, player.x, player.y);
	}
	// Turn towards desired angle
	if(e->cur_angle < e->want_angle) {
		if(abs(e->cur_angle - e->want_angle) < 0x80) e->cur_angle++;
		else e->cur_angle--;
	} else {
		if(abs(e->cur_angle - e->want_angle) < 0x80) e->cur_angle--;
		else e->cur_angle++;
	}
	
	// smoke trails
	//if (++e->timer2 > 2) {
	//	e->timer2 = 0;
		//Caret *c = effect(e->ActionPointX(), e->ActionPointY(), EFFECT_SMOKETRAIL_SLOW);
		//c->x_speed = -e->x_speed >> 2;
		//c->y_speed = -e->y_speed >> 2;
	//}
	
	//e->cur_angle %= 0x400;
	e->frame = ((e->cur_angle + (uint8_t)0x20) >> 5) & 7;
	
	e->x_speed = cos[e->cur_angle];
	e->y_speed = sin[e->cur_angle];
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	e->dir = 0; // Never flip sprite, let angles handle it all
}


// this is the cat that falls out after you defeat him
void ai_x_defeated(Entity *e) {
	e->timer++;
	if ((e->timer % 4) == 0) {
		//SmokeClouds(o, 1, 16, 16);
	}
	
	switch(e->state) {
		case 0:
		{
			//SmokeClouds(o, 8, 16, 16);
			e->state = 1;
		}
		case 1:
		{
			if (e->timer > TIME(50)) {
				e->state = 2;
				e->x_speed = -SPEED(0x100);
			}
			
			// three-position shake
			e->x += (e->timer & 2) ? (1 << CSF) : -(1 << CSF);
		}
		break;
		
		case 2:
		{
			e->y_speed += SPEED(0x40);
			if (e->y > (stageHeight * 16) << CSF) e->state = STATE_DELETE;
		}
		break;
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}
