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
#include "sheet.h"
#include "resources.h"
#include "npc.h"
#include "sprite.h"

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

#define saved_health		curly_target_time

enum Pieces {
	TREADUL, TREADUR, TREADLL, TREADLR,
	TARGET1, TARGET2, TARGET3, TARGET4,
	DOORL, DOORR,
};

// the treads start moving at slightly different times
// which we change direction, etc.
static const u16 tread_turnon_times[] = { 4, 8, 10, 12 };

// return true if all the targets behind the doors have been destroyed.
static u8 all_targets_destroyed() {
	for(u8 i=TARGET1;i<TARGET1+4;i++) {
		if (!pieces[i]->hidden) return FALSE;
	}
	return TRUE;
}

// sets state on an array on objects
static void set_states(Entity *e[], u8 n, u16 state) {
	while(--n) e[n]->state = state;
}

// sets direction on an array on objects
//static void set_dirs(Entity *e[], u8 n, u8 dir) {
//	while(--n) e[n]->dir = dir;
//}

void onspawn_monsterx(Entity *e) {
	e->alwaysActive = TRUE;
	e->health = 700;
	e->state = STATE_X_APPEAR;
	e->x = (128 * 16) << CSF;
	e->y = (200 << CSF);
	e->eflags = NPC_IGNORESOLID;
	
	// Since this entity was created first it will draw the background portion of the boss.
	// All the other pieces will be loaded in back -> front order
	
	// create internals
	e->linkedEntity = entity_create(0, 0, OBJ_X_INTERNALS, 0);
	// create targets
	pieces[TARGET1] = entity_create(0, 0, OBJ_X_TARGET, 0);
	pieces[TARGET2] = entity_create(0, 0, OBJ_X_TARGET, NPC_OPTION1);
	pieces[TARGET3] = entity_create(0, 0, OBJ_X_TARGET, NPC_OPTION2);
	pieces[TARGET4] = entity_create(0, 0, OBJ_X_TARGET, NPC_OPTION1|NPC_OPTION2);
	// create treads
	pieces[TREADUL] = entity_create(0xf8000, 0x12000,           OBJ_X_TREAD, 0);
	pieces[TREADUR] = entity_create(0x108000,0x12000,           OBJ_X_TREAD, NPC_OPTION1);
	pieces[TREADLL] = entity_create(0xf8000, 0x20000-(16<<CSF), OBJ_X_TREAD, NPC_OPTION2);
	pieces[TREADLR] = entity_create(0x108000,0x20000-(16<<CSF), OBJ_X_TREAD, NPC_OPTION1|NPC_OPTION2);
	// create doors
	pieces[DOORL] = entity_create(0, 0, OBJ_X_DOOR, 0);
	pieces[DOORR] = entity_create(0, 0, OBJ_X_DOOR, NPC_OPTION2);
	
	// Link them all to us
	for(u8 i = 0; i < 10; i++) pieces[i]->linkedEntity = e;
}

// The 4 green things look slightly different
void onspawn_x_target(Entity *e) {
	e->alwaysActive = TRUE;
	e->health = 60;
	e->nflags &= ~NPC_SHOWDAMAGE;
	if(e->eflags & NPC_OPTION1) e->frame += 1;
	if(e->eflags & NPC_OPTION2) e->frame += 2;
}

// Change sprite vflip for top treads
// The real game uses 4 different sprites
void onspawn_x_tread(Entity *e) {
	e->alwaysActive = TRUE;
	if(!(e->eflags & NPC_OPTION2)) {
		sprite_vflip(e->sprite[0], 1);
		sprite_vflip(e->sprite[1], 1);
	}
}

// Door on the right uses the second frame
void onspawn_x_door(Entity *e) {
	e->alwaysActive = TRUE;
	if(e->eflags & NPC_OPTION2) e->frame = 1;
}

void onspawn_x_internals(Entity *e) {
	e->alwaysActive = TRUE;
}

void ai_monsterx(Entity *e) {
	switch(e->state) {
		// script triggered us to initilize/appear
		// (there is a hvtrigger, right before player first walks by us
		// and sees us inactive, which sends us this ANP).
		case STATE_X_APPEAR: break;
		
		// script has triggered the fight to begin
		case STATE_X_FIGHT_BEGIN:
		{
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
			for(int i=0;i<4;i++) {
				if (e->timer == tread_turnon_times[i]) {
					pieces[TREADUL+i]->state = STATE_TREAD_RUN;
					pieces[TREADUL+i]->dir = e->dir;
				}
			}
			
			if (e->timer > 120)
			{
				// time to attack? we attack every 3rd travel
				// if so skid to a stop, that's the first step.
				if (e->timer2 >= 3)
				{
					e->timer2 = 0;
					
					e->dir ^= 1;
					e->state = STATE_X_BRAKE;
					e->timer = 0;
				}
				else
				{
					// passed player? skid and turn around.
					if ((e->dir == 1 && e->x > player.x) ||
					 	(e->dir == 0  && e->x < player.x)) {
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
			for(int i=0;i<4;i++) {
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
			
			if (++e->timer > 300 || all_targets_destroyed()) {
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
				
				//set_states(fishspawners, 4, STATE_FISHSPAWNER_FIRE);
				e->eflags |= NPC_SHOOTABLE;
			}
			
			if (++e->timer > 300 || (saved_health - e->health) > 200) {
				e->state = STATE_X_CLOSE_DOORS;
				e->timer = 0;
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
				//set_states(fishspawners, 4, 0);
				e->eflags &= ~NPC_SHOOTABLE;
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
			//SetStates(fishspawners, 4, 0);
			//entities_clear_by_type(OBJ_X_FISHY_MISSILE);
			
			e->timer = 0;
			e->state++;
		}
		case STATE_X_EXPLODING+1:
		{
			camera_shake(2);
			e->timer++;
			
			if ((e->timer % 8) == 0)
				sound_play(SND_ENEMY_HURT_BIG, 5);
			
			//SmokePuff(e->CenterX() + (random(-72, 72) << CSF),
			//		  e->CenterY() + (random(-64, 64) << CSF));
			
			if (e->timer > 100) {
				//starflash.Start(e->CenterX(), e->CenterY());
				sound_play(SND_EXPLOSION1, 5);
				e->timer = 0;
				e->state++;
			}
		}
		break;
		case STATE_X_EXPLODING+2:
		{
			camera_shake(40);
			if (++e->timer > 50) {
				entity_create(e->x, e->y - (24 << CSF), OBJ_X_DEFEATED, 0);
				entities_clear_by_type(OBJ_X_TARGET);
				entities_clear_by_type(OBJ_X_DOOR);
				return;
			}
		}
		break;
	}
	
	// main object pulled along as treads move
	s32 tread_center = (pieces[TREADUL]->x + pieces[TREADUR]->x +
					 	pieces[TREADLL]->x + pieces[TREADLR]->x) / 4;
	e->x += (tread_center - e->x) / 16;
	
	// Fill in sprites for the body
	e->sprite[0] = (VDPSprite) {
		.x = (e->x >> CSF) - 64 + 128, 
		.y = (e->y >> CSF) - 32 + 128, 
		.size = SPRITE_SIZE(4, 4), 
		.attribut = TILE_ATTR_FULL(PAL3,0,0,0,sheets[12].index),
	};
	e->sprite[1] = (VDPSprite) {
		.x = (e->x >> CSF) + 64 + 128,
		.y = (e->y >> CSF) - 32 + 128,
		.size = SPRITE_SIZE(4, 4), 
		.attribut = TILE_ATTR_FULL(PAL3,0,0,1,sheets[12].index),
	};
	e->sprite[2] = (VDPSprite) {
		.x = (e->x >> CSF) - 64 + 128,
		.y = (e->y >> CSF) + 32 + 128,
		.size = SPRITE_SIZE(4, 4), 
		.attribut = TILE_ATTR_FULL(PAL3,0,1,0,sheets[12].index),
	};
	e->sprite[3] = (VDPSprite) {
		.x = (e->x >> CSF) + 64 + 128,
		.y = (e->y >> CSF) + 32 + 128,
		.size = SPRITE_SIZE(4, 4), 
		.attribut = TILE_ATTR_FULL(PAL3,0,1,1,sheets[12].index),
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
			e->frame = 2;
			e->animtime = 0;
			
			e->state++;
		}
		case STATE_TREAD_RUN+1:
		{
			ANIMATE(e, 8, 2,3);
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
			e->frame = 2;
			e->animtime = 0;
			
			e->eflags |= NPC_BOUNCYTOP;
			e->state++;
		}
		case STATE_TREAD_BRAKE+1:
		{
			ANIMATE(e, 8, 2,3);
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
	//switch(e->state)
	//{
	//	case STATE_TREAD_RUN+1:
	//	case STATE_TREAD_BRAKE+1:
	//	{
	//		if (e->timer & 1)
	//			sound_play(SND_MOTOR_SKIP, 3);
	//	}
	//	break;
	//	
	//	case STATE_TREAD_RUN+2:
	//	{
	//		if ((e->timer % 4) == 1)
	//			sound_play(SND_MOTOR_RUN, 3);
	//	}
	//	break;
	//}
	
	// determine if player is in a position where he could get run over.
	if (e->state > STATE_TREAD_STOPPED && e->x_speed) {
		if (abs(player.y - e->y) <= (5 << CSF))
			e->attack = 10;
		else
			e->attack = 0;
	} else {
		e->attack = 0;
	}
	
	LIMIT_X(0x400);
}

void ai_x_internals(Entity *e) {
	e->x = bossEntity->x;
	e->y = bossEntity->y;
	
	// eye open after targets destroyed
	e->frame = (bossEntity->state < 10) ? 1 : 0;
	
	// link damage to main object
	if (e->health < 1000) {
		if(1000 - e->health > bossEntity->health) {
			bossEntity->health = 0;
			e->nflags &= ~(NPC_SHOOTABLE | NPC_SHOWDAMAGE);
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
	if (e->eflags & NPC_OPTION2) {
		e->x = (bossEntity->x - e->x_mark);
	} else {
		e->x = (bossEntity->x + e->x_mark);
	}
	e->y = bossEntity->y;
}
/*
void XBoss::run_fishy_spawner(int index)
{
	Entity *o = fishspawners[index];
	
	switch(e->state)
	{
		case STATE_FISHSPAWNER_FIRE:
		{
			e->timer = 20 + (index * 20);
			e->state++;
		}
		case STATE_FISHSPAWNER_FIRE+1:
		{
			if (e->timer)
			{
				e->timer--;
				break;
			}
			
			// keep appropriate position relative to main object
			//                               UL          UR         LL         LR
			static const int xoffs[]   = { -64 <<CSF,  76 <<CSF, -64 <<CSF,  76 <<CSF };
			static const int yoffs[]   = {  27 <<CSF,  27 <<CSF, -16 <<CSF, -16 <<CSF };
			e->x = (mainobject->x + xoffs[index]);
			e->y = (mainobject->y + yoffs[index]);
			
			Entity *missile = CreateEntity(e->x, e->y, OBJ_X_FISHY_MISSILE);
			missile->dir = index;
			
			sound_play(SND_EM_FIRE);
			e->timer = 120;
		}
		break;
	}
}
*/
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
			if (--e->timer <= 16)
			{
				// flash shortly before firing
				if (e->timer & 2) e->frame |= 4;
							 else e->frame &= 3;
				
				if (e->timer <= 0)
				{
					e->timer = 40;
					//EmFireAngledShot(e, OBJ_GAUDI_FLYING_SHOT, 2, 0x500);
					sound_play(SND_EM_FIRE, 3);
				}
			}
		}
		break;
	}
	
	// keep appropriate position on internals
	//                               UL          UR         LL         LR
	//static const int xoffs[] = { -22 <<CSF,  28 <<CSF, -15 <<CSF,  17 <<CSF };
	//static const int yoffs[] = { -16 <<CSF, -16 <<CSF,  14 <<CSF,  14 <<CSF };
	
	//e->x = internals->x + xoffs[index];
	//e->y = internals->y + yoffs[index];
}

void ondeath_monsterx(Entity *e) {
	e->state = STATE_X_EXPLODING;
	e->nflags &= ~(NPC_SHOOTABLE | NPC_SHOWDAMAGE);
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
	if (e->state == 0) {
		//static const int angle_for_dirs[] = { 160, 224, 96, 32 };
		
		//e->angle = angle_for_dirs[e->dir];
		e->dir = 1;
		
		e->state = 1;
	}
	
	//vector_from_angle(e->angle, 0x400, &e->x_speed, &e->y_speed);
	//int desired_angle = GetAngle(e->x, e->y, player.x, player.y);
	
	//if (e->angle >= desired_angle) {
	//	if ((e->angle - desired_angle) < 128) {
	//		e->angle--;
	//	} else {
	//		e->angle++;
	//	}
	//} else {
	//	if ((e->angle - desired_angle) < 128) {
	//		e->angle++;
	//	} else {
	//		e->angle--;
	//	}
	//}
	
	// smoke trails
	if (++e->timer2 > 2) {
		e->timer2 = 0;
		//Caret *c = effect(e->ActionPointX(), e->ActionPointY(), EFFECT_SMOKETRAIL_SLOW);
		//c->x_speed = -e->x_speed >> 2;
		//c->y_speed = -e->y_speed >> 2;
	}
	
	//e->frame = (e->angle + 16) / 32;
	//if (e->frame > 7) e->frame = 7;
}


// this is the cat that falls out after you defeat him
void ai_x_defeated(Entity *e) {
	e->timer++;
	if ((e->timer % 4) == 0) {
		//SmokeClouds(o, 1, 16, 16);
	}
	
	switch(e->state)
	{
		case 0:
		{
			//SmokeClouds(o, 8, 16, 16);
			e->state = 1;
		}
		case 1:
		{
			if (e->timer > 50) {
				e->state = 2;
				e->x_speed = -0x100;
			}
			
			// three-position shake
			e->x += (e->timer & 2) ? (1 << CSF) : -(1 << CSF);
		}
		break;
		
		case 2:
		{
			e->y_speed += 0x40;
			if (e->y > (stageHeight * 16) << CSF) e->state = STATE_DELETE;
		}
		break;
	}
}
