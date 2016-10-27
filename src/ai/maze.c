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
#include "sprite.h"

#ifdef PAL
#define BLOCK_SOUND_INTERVAL	10
#define BLOCK_TRAVEL_ACCEL		0x20
#define BLOCK_TRAVEL_SPEED		0x200
#else
#define BLOCK_SOUND_INTERVAL	12
#define BLOCK_TRAVEL_ACCEL		0x1B
#define BLOCK_TRAVEL_SPEED		0x1B0
#endif

void onspawn_block(Entity *e) {
	e->x += pixel_to_sub(8);
	e->y += pixel_to_sub(8);
	e->hit_box = (bounding_box) { 16, 16, 16, 16 };
	e->display_box = (bounding_box) { 16, 16, 16, 16 };
	e->eflags |= NPC_SPECIALSOLID;
	e->nflags &= ~NPC_SOLID;
	e->eflags |= NPC_IGNORE44;
	e->enableSlopes = FALSE;
	e->attack = 0;
	e->state = (e->eflags & NPC_OPTION2) ? 20 : 10;
}

void ai_blockh(Entity *e) {
	switch(e->state) {
		case 10:
		if((player.x > e->x && player.x - e->x < 0x3200) ||
			(player.x < e->x && e->x - player.x < 0x32000)) {
			if(PLAYER_DIST_Y(0x3200)) {
				e->state = 30;
				e->timer = 0;
			}
		}
		break;
		case 20:
		if((player.x > e->x && player.x - e->x < 0x32000) ||
			(player.x < e->x && e->x - player.x < 0x3200)) {
			if(PLAYER_DIST_Y(0x3200)) {
				e->state = 30;
				e->timer = 0;
			}
		}
		break;
		case 30:
		{
			u16 dir = e->eflags & NPC_OPTION2;
			e->x_speed += dir ? BLOCK_TRAVEL_ACCEL : -BLOCK_TRAVEL_ACCEL;
			if(e->x_speed > BLOCK_TRAVEL_SPEED) e->x_speed = BLOCK_TRAVEL_SPEED;
			if(e->x_speed < -BLOCK_TRAVEL_SPEED) e->x_speed = -BLOCK_TRAVEL_SPEED;
			e->x_next = e->x + e->x_speed;
			// hit edge
			if((e->x_speed > 0 && stage_get_block_type(
					sub_to_block(e->x_next + 0x1000), sub_to_block(e->y)) == 0x41) ||
				(e->x_speed < 0 && stage_get_block_type(
					sub_to_block(e->x_next - 0x1000), sub_to_block(e->y)) == 0x41)) {
				camera_shake(10);
				e->x_speed = 0;
				e->eflags ^= NPC_OPTION2;
				e->state = dir ? 10 : 20;
			} else {
				e->x = e->x_next;
				if((++e->timer % BLOCK_SOUND_INTERVAL) == 6) {
					sound_play(SND_BLOCK_MOVE, 2);
				}
			}
		}
		break;
	}
}

void ai_blockv(Entity *e) {
	switch(e->state) {
		case 10:
		if((player.y > e->y && player.y - e->y < 0x3200) ||
			(player.y < e->y && e->y - player.y < 0x32000)) {
			if(PLAYER_DIST_X(0x3200)) {
				e->state = 30;
				e->timer = 0;
			}
		}
		break;
		case 20:
		if((player.y > e->y && player.y - e->y < 0x32000) ||
			(player.y < e->y && e->y - player.y < 0x3200)) {
			if(PLAYER_DIST_X(0x3200)) {
				e->state = 30;
				e->timer = 0;
			}
		}
		break;
		case 30:
		{
			u16 dir = e->eflags & NPC_OPTION2;
			e->y_speed += dir ? BLOCK_TRAVEL_ACCEL : -BLOCK_TRAVEL_ACCEL;
			if(e->y_speed > BLOCK_TRAVEL_SPEED) e->y_speed = BLOCK_TRAVEL_SPEED;
			if(e->y_speed < -BLOCK_TRAVEL_SPEED) e->y_speed = -BLOCK_TRAVEL_SPEED;
			e->y_next = e->y + e->y_speed;
			// hit edge
			if((e->y_speed > 0 && stage_get_block_type(
					sub_to_block(e->x - 0x200), sub_to_block(e->y_next + 0x1000)) == 0x41) ||
				(e->y_speed < 0 && stage_get_block_type(
					sub_to_block(e->x - 0x200), sub_to_block(e->y_next - 0x1000)) == 0x41)) {
				camera_shake(10);
				e->y_speed = 0;
				e->eflags ^= NPC_OPTION2;
				e->state = dir ? 10 : 20;
			} else {
				e->y = e->y_next;
				if((++e->timer % BLOCK_SOUND_INTERVAL) == 6) {
					sound_play(SND_BLOCK_MOVE, 2);
				}
			}
		}
		break;
	}
}

void ai_boulder(Entity *e) {
	//e->inback = TRUE;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		// shaking
		case 10:
		{
			e->state = 11;
			e->timer = 0;
			e->x_mark = e->x;
		}
		/* no break */
		case 11:
		{
			if ((++e->timer % 3) != 0)
				e->x = e->x_mark + (1 << 9);
			else
				e->x = e->x_mark;
		}
		break;
		
		// thrown away by Balrog
		case 20:
		{
			e->y_speed = SPEED(-0x400);
			e->x_speed = SPEED(0x100);
			sound_play(SND_FUNNY_EXPLODE, 5);
			
			e->state = 21;
			e->timer = 0;
		}
		/* no break */
		case 21:
		{
			e->y_speed += SPEED(0x10);
			
			if (e->y_speed >= 0 && collide_stage_floor(e)) {
				sound_play(SND_EXPLOSION1, 5);
				camera_shake(40);
				
				e->x_speed = 0;
				e->y_speed = 0;
				e->state = 0;
			}
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

#define GAUDI_HP 15
#define GAUDI_FLYING_HP 15
#define GAUDI_ARMORED_HP 15

void ai_gaudiDying(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:		// just died (initializing)
		{
			e->eflags &= ~(NPC_SHOOTABLE | NPC_IGNORESOLID | NPC_SHOWDAMAGE);
			e->nflags &= ~(NPC_SHOOTABLE | NPC_IGNORESOLID | NPC_SHOWDAMAGE);
			e->attack = 0;
			
			e->frame = 4;
			
			e->y_speed = SPEED(-0x200);
			MOVE_X(SPEED(-0x100));
			sound_play(SND_ENEMY_HURT_SMALL, 5);
			e->grounded = FALSE;
			
			e->state = 1;
		}
		break;
		
		case 1:		// flying backwards through air
		{
			if (e->y_speed >= 0 && (e->grounded = collide_stage_floor(e))) {
				e->frame = 5;
				e->state = 2;
				e->timer = 0;
			}
		}
		break;
		
		case 2:		// landed, shake
		{
			e->x_speed -= e->x_speed >> 4;
			e->frame = (e->timer % 8) > 3 ? 6 : 5;
			
			if (++e->timer > TIME(50)) {
				// this deletes Entity while generating smoke effects and boom
				e->state = STATE_DESTROY;
			}
		}
		break;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	if(!e->grounded) e->y_speed += SPEED(0x20);
	LIMIT_Y(SPEED(0x5ff));
}

void ai_gaudi(Entity *e) {
	if (e->health <= (1000 - GAUDI_HP)) {
		e->type = OBJ_GAUDI_DYING;
		e->attack = 0;
		e->state = 0;
		e->timer = 0;
		ai_gaudiDying(e);
		return;
	}
	
	// Use different palette in Labyrinth Shop
	if(stageID == 0x2A) sprite_pal(e->sprite[0], PAL3);
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	switch(e->state) {
		case 0:
		{
			// Gaudi's in shop
			if (e->eflags & NPC_INTERACTIVE) {
				e->attack = 0;
				e->nflags &= ~NPC_SHOOTABLE;
			}
			
			e->x_speed = 0;
			e->state = 1;
		}
		/* no break */
		case 1:
		{
			e->frame = 0;
			RANDBLINK(e, 3, 200);
			
			if (!(random() % TIME(100))) {
				if (random() & 1) {
					TURN_AROUND(e);
				} else {
					e->state = 10;
				}
			}
		}
		break;
		case 10:		// walking
		{
			e->state = 11;
			e->timer = (random() % TIME(75)) + TIME(25);		// how long to walk for
		}
		/* no break */
		case 11:
		{
			ANIMATE(e, 8, 1,0,2,0);
			// time to stop walking?
			if (--e->timer <= 0) e->state = 0;
				
			MOVE_X(SPEED(0x200));
			
			// try to jump over any walls we come to
			if ((e->x_speed < 0 && collide_stage_leftwall(e)) || \
				(e->x_speed > 0 && collide_stage_rightwall(e))) {
				e->y_speed = -SPEED(0x5ff);
				e->grounded = FALSE;
				e->state = 20;
				e->timer = 0;
				
				if (!controlsLocked)	// no sound during ending cutscene
					sound_play(SND_ENEMY_JUMP, 5);
			}
		}
		break;
		case 20:		// jumping
		{
			// landed?
			if (e->y_speed >= 0 && (e->grounded = collide_stage_floor(e))) {
				e->x_speed = 0;
				e->state = 21;
				e->timer = 0;
				
				if (!controlsLocked)	// no sound during ending cutscene
					sound_play(SND_THUD, 5);
			}
			
			// count how long we've been touching the wall
			// we're trying to jump over..if it's not working
			// go the other way.
			if ((e->dir == 0 && collide_stage_leftwall(e)) || \
				(e->dir == 1 && collide_stage_rightwall(e))) {
				if (++e->timer > TIME(10)) {
					e->timer = 0;
					TURN_AROUND(e);
				}
			} else {
				e->timer = 0;
			}
			
			MOVE_X(SPEED(0x100));
		}
		break;
		case 21:	// landed from jump
		{
			if (++e->timer > TIME(10)) e->state = 0;
		}
		break;
	}
	if(e->y_speed >= 0) {
		if(!e->grounded) e->grounded = collide_stage_floor(e);
		else e->grounded = collide_stage_floor_grounded(e);
	} else {
		collide_stage_ceiling(e);
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	if(!e->grounded) e->y_speed += SPEED(0x40);
	LIMIT_Y(SPEED(0x5ff));
}

void ai_gaudiFlying(Entity *e) {
	if (e->health <= (1000 - GAUDI_FLYING_HP)) {
		if (e->dir == 0)
			e->x -= (2 << 9);
		else
			e->x += (2 << 9);
		
		e->type = OBJ_GAUDI_DYING;
		e->attack = 0;
		e->state = 0;
		e->timer = 0;
		ai_gaudiDying(e);
		return;
	}
	
	switch(e->state) {
		case 0:
		{
			u16 angle = random() % 1024;
			e->x_speed = sintab32[angle] >> 1;
			e->y_speed = sintab32[(angle + 256) % 1024] >> 1;
			e->x_mark = e->x + (e->x_speed * 8);
			e->y_mark = e->y + (e->y_speed * 8);
			e->state = 1;
			e->timer2 = 120;
		}
		/* no break */
		case 1:
		{
			e->timer = TIME(70) + (random() % TIME(80));
			e->state = 2;
		}
		/* no break */
		case 2:
		{
			ANIMATE(e, 4, 7,8);
			
			if (!e->timer) {
				e->state = 3;
			} else e->timer--;
		}
		break;
		
		case 3:		// preparing to fire
		{
			ANIMATE(e, 4, 9,8);
			
			e->timer++;
			if (++e->timer > TIME(30)) {
				Entity *shot = entity_create(e->x, e->y, OBJ_GAUDI_FLYING_SHOT, 0);
				THROW_AT_TARGET(shot, player.x, player.y, SPEED(0x400));
				sound_play(SND_EM_FIRE, 5);
				
				e->state = 1;
			}
		}
	}
	
	FACE_PLAYER(e);
	// sinusoidal circling pattern
	e->x_speed += (e->x > e->x_mark) ? SPEED(-0x10) : SPEED(0x10);
	e->y_speed += (e->y > e->y_mark) ? SPEED(-0x10) : SPEED(0x10);
	LIMIT_X(SPEED(0x200));
	LIMIT_Y(SPEED(0x5ff));
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_gaudiArmored(Entity *e) {
	if (e->health <= (1000 - GAUDI_ARMORED_HP)) {
		e->type = OBJ_GAUDI_DYING;
		e->attack = 0;
		e->state = 0;
		e->timer = 0;
		ai_gaudiDying(e);
		return;
	}
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	FACE_PLAYER(e);
	
	switch(e->state) {
		case 0:
		{
			e->frame = 0;
			e->x_mark = e->x;
			e->state = 1;
		}
		/* no break */
		case 1:
		{
			e->x_speed = 0;
			
			if (++e->timer >= TIME(5)) {
				if (PLAYER_DIST_X(192 << 9) && PLAYER_DIST_Y(160 << 9)) {	// begin hopping
					e->state = 10;
					e->timer = 0;
					e->frame = 1;
				}
			}
		}
		break;
		case 10:	// on ground inbetween hops
		{
			if (++e->timer > 3) {
				sound_play(SND_ENEMY_JUMP, 5);
				e->frame = 2;
				e->timer = 0;
				
				if (++e->timer2 < 3) {	// hopping back and forth
					e->state = 20;
					e->y_speed = SPEED(-0x200);
					e->grounded = FALSE;
					e->x_speed = (e->x < e->x_mark) ? SPEED(0x200) : SPEED(-0x200);
				} else {	// big jump and attack
					e->state = 30;
					e->y_speed = SPEED(-0x600);
					e->grounded = FALSE;
					e->x_speed = (e->x < e->x_mark) ? SPEED(0x80) : SPEED(-0x80);
					
					e->timer2 = 0;
				}
			}
		}
		break;
		case 20:	// jumping (small hop)
		{
			// landed?
			if (e->y_speed >= 0 && (e->grounded = collide_stage_floor(e))) {
				// drop sub-pixel precision
				// (required to maintain stability of back-and-forth sequence).
				e->y >>= 9; e->y <<= 9;
				
				sound_play(SND_THUD, 5);
				e->state = 40;
				e->frame = 1;
				e->timer = 0;
			}
		}
		break;
		case 30:	// jumping (big jump + attack)
		{
			e->timer++;
			
			// throw attacks at player
			if (e->timer == TIME(30) || e->timer == TIME(40)) {
				FIRE_ANGLED_SHOT(OBJ_GAUDI_ARMORED_SHOT, e->x, e->y, 
						e->dir ? A_RIGHT+64 : A_LEFT-64, 0x600);
				sound_play(SND_EM_FIRE, 5);
				
				e->frame = 3;
				CURLY_TARGET_HERE(e);
			}
			
			// stop throwing animation
			if (e->timer == TIME(35) || e->timer == TIME(45)) e->frame = 2;
			
			if (e->y_speed > 0 && (e->grounded = collide_stage_floor(e))) {
				sound_play(SND_THUD, 5);
				e->state = 40;
				e->frame = 1;
				e->timer = 0;
			}
		}
		break;
		case 40:		// landed
		{
			e->x_speed -= e->x_speed >> 4;
			
			if (++e->timer >= 2) {
				e->frame = 0;
				e->x_speed = 0;
				
				e->state = 1;
				e->timer = 0;
			}
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
	
	if(!e->grounded) e->y_speed += SPEED(0x33);
	LIMIT_Y(SPEED(0x5ff));
}

void ai_gaudiArmoredShot(Entity *e) {
	ANIMATE(e, 4, 0,1,2);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		{
			u8 bounced = FALSE;
			if (e->x_speed <= 0 && collide_stage_leftwall(e)) { 
				e->x_speed = SPEED(0x200);
				bounced = TRUE; 
			}
			if (e->x_speed >= 0 && collide_stage_rightwall(e)) { 
				e->x_speed = -SPEED(0x200);
				bounced = TRUE; 
			}
			if (e->y_speed >= 0 && collide_stage_floor(e)) { 
				e->y_speed = -SPEED(0x200);
				bounced = TRUE; 
			}
			if (e->y_speed <= 0 && collide_stage_ceiling(e)) { 
				e->y_speed = SPEED(0x200);
				bounced = TRUE; 
			}
			
			if (bounced) {
				e->state = 1;
				sound_play(SND_TINK, 5);
			}
		}
		break;
		
		case 1:
		{
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5ff));
			
			if (e->y_speed >= 0 && collide_stage_floor(e)) {
				e->state = STATE_DELETE;
				return;
			}
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

#define FRAME_STAND		0
#define FRAME_DYING		1
#define FRAME_LANDED	2
#define FRAME_FLYING	3

#define bubble_xmark curly_target_x
#define bubble_ymark curly_target_y

void ai_pooh_black(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;

	switch(e->state) {
		case 0:
		{
			e->alwaysActive = TRUE;

			e->frame = FRAME_FLYING;
			FACE_PLAYER(e);
			
			e->y_speed = SPEED(0xA00);
			e->eflags |= NPC_IGNORESOLID;
			
			if (e->y >= block_to_sub(8)) {
				e->eflags &= ~NPC_IGNORESOLID;
				e->state = 1;
			}
		}
		break;
		case 1:
		{
			e->frame = FRAME_FLYING;
			e->y_speed = SPEED(0xA00);
			
			if ((e->grounded = collide_stage_floor(e))) {
				//SmokeSide(o, 8, DOWN);
				sound_play(SND_BIG_CRASH, 5);
				camera_shake(30);
				
				entities_clear_by_type(OBJ_POOH_BLACK_BUBBLE);
				e->state = 2;
			}
			// damage player if he falls on him
			e->attack = (e->y < player.y && player.grounded) ? 20 : 0;
		}
		break;
		case 2:		// landed, showing landed frame
		{
			e->frame = FRAME_LANDED;
			e->attack = 0;
			if (++e->timer > TIME(24)) {
				e->state = 3;
				e->timer = 0;
			}
		}
		break;
		case 3:		// standing, stare at player till he shoots us.
		{
			e->frame = FRAME_STAND;
			bubble_xmark = e->x;
			bubble_ymark = e->y;
			
			// spawn bubbles when hit
			if (e->damage_time && (e->damage_time % TIME(5)) == 1) {
				Entity *bubble = entity_create(e->x, e->y, OBJ_POOH_BLACK_BUBBLE, 0);
				bubble->alwaysActive = TRUE;
				bubble->x = e->x - 0x1800 + (random() % 0x3000);
				bubble->y = e->y - 0x1800 + (random() % 0x3000);
				// Don't wrap the whole thing in 1 SPEED(), gcc can't optimize that
				bubble->x_speed = -SPEED(0x600) + (random() % SPEED(0xC00));
				bubble->y_speed = -SPEED(0x600) + (random() % SPEED(0xC00));
				
				// fly away after hit enough times
				if (++e->timer > TIME(30)) {
					e->state = 4;
					e->timer = 0;
					
					e->eflags |= NPC_IGNORESOLID;
					e->y_speed = -SPEED(0xC00);
				}
			}
		}
		break;
		case 4:		// flying away off-screen
		{
			e->frame = FRAME_FLYING;;
			e->timer++;
			
			// bubbles shoot down past player just before
			// he falls.
			if (e->timer == TIME(60)) {
				bubble_xmark = player.x;
				bubble_ymark = (10000 << CSF);
			} else if (e->timer < TIME(60)) {
				bubble_xmark = e->x;
				bubble_ymark = e->y;
			}
			if (e->timer >= TIME(170)) {
				// Fall on player, but keep outside the walls
				e->x_next = player.x;
				if(e->x_next < (5 * 16) << CSF) e->x_next = (5 * 16) << CSF;
				if(e->x_next > (15 * 16) << CSF) e->x_next = (15 * 16) << CSF;
				e->y_next = 0;
				e->y_speed = SPEED(0x5ff);
				
				e->state = 0;
				e->timer = 0;
			}
		}
		break;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
}


void ai_pooh_black_bubble(Entity *e) {
	if (e->health < 100 || (e->state && e->y < 0)) {
		e->state = STATE_DELETE;
		return;
	}
	if (!(random() % 8)) e->frame = 0;
	else e->frame = 1;
	e->x_speed += (e->x > bubble_xmark) ? -SPEED(0x40) : SPEED(0x40);
	e->y_speed += (e->y > bubble_ymark) ? -SPEED(0x40) : SPEED(0x40);
	LIMIT_X(SPEED(0x1000));
	LIMIT_Y(SPEED(0x1000));
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ondeath_pooh_black(Entity *e) {
	e->type = OBJ_POOH_BLACK_DYING;
	e->state = 0;
	e->attack = 0;
	e->nflags &= ~(NPC_SHOOTABLE|NPC_SOLID);
	e->eflags &= ~(NPC_SHOOTABLE|NPC_SOLID);
	tsc_call_event(e->event);
}

void ai_pooh_black_dying(Entity *e) {
	bubble_xmark = e->x;
	bubble_ymark = -(10000 << CSF);

	switch(e->state) {
		case 0:
		{
			e->frame = FRAME_DYING;
			FACE_PLAYER(e);
			
			sound_play(SND_BIG_CRASH, 5);
			//SmokeClouds(o, 10, 12, 12);
			entities_clear_by_type(OBJ_POOH_BLACK_BUBBLE);
			
			e->state = 1;
			e->timer = 0;
			e->timer2 = 0;
		}
		break;
		
		case 1:
		case 2:
		{
			camera_shake(2);
			if (++e->timer > 200) {
				e->state = 2;
				e->timer2++;
				if ((e->timer2 % 6) == 3) {
					e->hidden = FALSE;
					sound_play(SND_BUBBLE, 5);
				} else if((e->timer2 % 6) == 0) {
					e->hidden = TRUE;
				}
				if(e->timer2 > 60) {
					//entities_clear_by_type(OBJ_POOH_BLACK_BUBBLE);
					e->state = STATE_DELETE;
					return;
				}
			}
		}
		break;
	}
	
	if ((e->timer % 4) == 1) {
		Entity *bubble = entity_create(
				e->x - 0x1800 + (random() % 0x3000), 
				e->y - 0x1800 + (random() % 0x3000), OBJ_POOH_BLACK_BUBBLE, 0);
		bubble->alwaysActive = TRUE;
		bubble->attack = 0;
		bubble->state = 1;
		bubble->x_speed = -0x200 + (random() % 0x400);
		bubble->y_speed = -0x100;
	}

}

// MazeM enemies

void ai_firewhirr(Entity *e) {
	Entity *shot;
	FACE_PLAYER(e);
	
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->timer = random() % TIME(50);
			e->y_mark = e->y;
		}
		/* no break */
		case 1:
		{
			if (!e->timer) {
				e->state = 10;
				e->timer = TIME(100);
				e->y_speed = SPEED(-0x200);
			}
			else e->timer--;
		}
		/* no break */
		case 10:
		{
			e->y_speed += (e->y < e->y_mark) ? SPEED(0x10) : SPEED(-0x10);
			LIMIT_Y(SPEED(0x200));
			
			// inc time-to-fire while player near
			if (PLAYER_DIST_Y(80 << 9)) {
				if (!e->dir && player.x < e->x && PLAYER_DIST_X(160 << 9)) e->timer2++;
				if (e->dir && player.x > e->x && PLAYER_DIST_X(160 << 9)) e->timer2++;
			}
			
			// if time to fire, spawn a shot
			if (e->timer2 > TIME(120)) {
				shot = entity_create(e->x, e->y, OBJ_FIREWHIRR_SHOT, 0);
				shot->dir = e->dir;
				shot->alwaysActive = TRUE;
				shot->x = e->x;
				shot->y = e->y;
				e->timer2 = random() % TIME(20);
				// tell Curly to acquire us as a target
				CURLY_TARGET_HERE(e);
			}
		}
		break;
	}
}

void ai_firewhirr_shot(Entity *e) {
	ANIMATE(e, 8, 0,1,2);
	e->x_next = e->x + (!e->dir ? SPEED(-0x200) : SPEED(0x200));
	e->y_next = e->y;
	
	if ((!e->dir && collide_stage_leftwall(e)) ||
		(e->dir && collide_stage_rightwall(e))) {
		//effect(e->x, e->CenterY(), EFFECT_FISHY);
		e->state = STATE_DELETE;
	}

	e->x = e->x_next;
}

void ai_gaudi_egg(Entity *e) {
	if (!e->state) {
		if (!(e->eflags & NPC_OPTION2)) {	// on floor
			// align properly with ground
			e->y -= (4<<9);
			e->x -= (4<<9);
		} else {	// on ceiling
			sprite_vflip(e->sprite[0], 1);
			// for the egg @ entrance point that is on a ceiling slope
			if (!collide_stage_ceiling(e)) {
				e->y -= (14 << 9);
			}
		}
		e->state = 1;
	} else if (e->state == 1) {
		if (e->health < 90) {
			e->frame = 1;
			e->attack = 0;
			e->eflags &= ~NPC_SHOOTABLE;
			e->nflags &= ~NPC_SHOOTABLE;
			entity_drop_powerup(e);
			sound_play(e->deathSound, 5);
			//SmokeSide(o, 6, (e->dir==0)?DOWN:UP);
			e->state = 2;
		}
	}
}

void ai_fuzz_core(Entity *e) {
	switch(e->state) {
		case 0:
		{
			// spawn mini-fuzzes
			u16 angle = 0;
			for(u16 i=0;i<5;i++) {
				Entity *f = entity_create(e->x, e->y, OBJ_FUZZ, 0);
				f->x = e->x;
				f->y = e->y;
				f->linkedEntity = e;
				f->timer = angle;
				angle += (1024 / 5);
			}
			e->timer = random() % TIME(50);
			e->state = 1;
		}
		/* no break */
		case 1:		// de-syncs the Y positions when multiple cores are present at once
		{
			if (e->timer == 0) {
				e->state = 2;
				e->y_speed = SPEED(0x300);
				e->y_mark = e->y;
			} else e->timer--;
		}
		break;
		case 2:
		{
			FACE_PLAYER(e);
			
			if (e->y > e->y_mark) e->y_speed -= SPEED(0x10);
			if (e->y < e->y_mark) e->y_speed += SPEED(0x10);
			LIMIT_Y(SPEED(0x355));
		}
		break;
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_fuzz(Entity *e) {
	FACE_PLAYER(e);
	
	switch(e->state) {
		case 0:
		{
			e->timer = (e->timer + TIME(16)) % 1024; // Increase angle
			e->timer2 = (e->timer + 256) % 1024; // add 90 degrees for cosine
			if (e->linkedEntity->state == STATE_DESTROY) {
				e->x_speed = SPEED(-0x200) + (random() % SPEED(0x400));
				e->y_speed = SPEED(-0x200) + (random() % SPEED(0x400));
				e->state = 1;
			} else {
				e->x = e->linkedEntity->x + ((sintab32[e->timer] >> 1) << 5);
				e->y = e->linkedEntity->y + ((sintab32[e->timer2] >> 1) << 5);
			}
		}
		break;
		// base destroyed, simple sinusoidal player-seek
		case 1:
		{
			e->x_speed += (e->x > player.x) ? SPEED(-0x20) : SPEED(0x20);
			e->y_speed += (e->y > player.y) ? SPEED(-0x20) : SPEED(0x20);
			
			LIMIT_X(SPEED(0x800));
			LIMIT_Y(SPEED(0x200));
			e->x += e->x_speed;
			e->y += e->y_speed;
		}
		break;
	}
}

#define BUYOBUYO_BASE_HP		60

void ai_buyobuyo_base(Entity *e) {
	if (e->state < 3 && e->health < (1000 - BUYOBUYO_BASE_HP)) {
		//SmokeClouds(o, objprop[e->type].death_smoke_amt, 8, 8);
		e->attack = 0;
		e->eflags &= ~NPC_SHOOTABLE;
		e->nflags &= ~NPC_SHOOTABLE;
		entity_drop_powerup(e);
		sound_play(e->deathSound, 5);
		e->state = 10;
		e->frame = 2;
	}
	
	switch(e->state) {
		case 0:
		{
			// ceiling has different bounding box and action point
			//if (e->dir == 1)
			//	e->sprite = SPR_BUYOBUYO_BASE_CEILING;
			if(e->eflags & NPC_OPTION2) {
				e->dir = 1;
				e->x -= 16 << CSF;
			}
			e->state = 1;
			e->timer = TIME(10);
		}
		/* no break */
		case 1:
		{
			if (PLAYER_DIST_X(0x14000)) {
				if ((!e->dir && PLAYER_DIST_Y2(0x14000, 0x2000)) ||
					(e->dir && PLAYER_DIST_Y2(0x2000, 0x14000))) {
					if (--e->timer == 0) {
						e->state = 2;
						e->timer = 0;
						e->frame = 1;
					}
				}
			}
		}
		break;
		case 2:
		{
			if (++e->timer > TIME(10)) {
				//Entity *buyo = entity_create(e->x, e->y, OBJ_BUYOBUYO, 0);
				//buyo->dir = e->dir;
				//buyo->x = e->x;
				//buyo->y = e->y;
				
				sound_play(SND_EM_FIRE, 5);
				CURLY_TARGET_HERE(e);
				
				// cyclic: three firings then pause
				e->state = 1;
				if (++e->timer2 > 2) {
					e->timer = TIME(100);
					e->timer2 = 0;
				} else {
					e->timer = TIME(20);
				}
				e->frame = 0;
			}
		}
		break;
	}
}

void ai_buyobuyo(Entity *e) {
	u8 deleteme = FALSE;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		{
			// shoot up down at player...
			e->y_speed = e->dir ? SPEED(0x600) : -SPEED(0x600);
			e->state = 1;
			e->timer = 0;
		}
		/* no break */
		case 1:
		{
			e->timer++;		// inc fly time
			// reached height of player yet?
			if (PLAYER_DIST_Y(0x2000)) {
				e->state = 2;
			} else break;
		}
		/* no break */
		case 2:
		{
			// this slight "minimum fly time" keeps the underwater ones from
			// smacking into the floor if the player is underwater with them
			if (++e->timer > 3) {
				FACE_PLAYER(e);
				e->x_mark = e->x;
				e->y_mark = e->y;
				
				e->x_speed = (random() & 1) ? SPEED(0x200) : -SPEED(0x200);
				e->y_speed = (random() & 1) ? SPEED(0x200) : -SPEED(0x200);
				
				e->state = 3;
			}
		}
		break;
		case 3:
		{
			if (e->x > e->x_mark) e->x_speed -= SPEED(0x20);
			if (e->x < e->x_mark) e->x_speed += SPEED(0x20);
			if (e->y > e->y_mark) e->y_speed -= SPEED(0x20);
			if (e->y < e->y_mark) e->y_speed += SPEED(0x20);
			LIMIT_X(SPEED(0x400));
			LIMIT_Y(SPEED(0x400));
			
			// move the point we are bobbling around
			e->x_mark += e->dir ? SPEED(1 << 9) : SPEED(-(1 << 9));
			
			if (++e->timer > TIME(300)) deleteme = TRUE;
		}
		break;
	}
	if ((e->x_speed < 0 && collide_stage_leftwall(e)) ||
		(e->x_speed > 0 && collide_stage_rightwall(e)) ||
		(e->y_speed < 0 && collide_stage_ceiling(e)) ||
		(e->y_speed > 0 && collide_stage_floor(e))) {
		deleteme = TRUE;
	}
	if (deleteme) {
		//effect(e->CenterX(), e->CenterY(), EFFECT_STARPOOF);
		e->state = STATE_DELETE;
		return;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

void onspawn_gero(Entity *e) {
	e->x -= 8 << CSF;
}

void ai_gero(Entity *e) {
	e->frame = 0;
	RANDBLINK(e, 1, 200);
}
