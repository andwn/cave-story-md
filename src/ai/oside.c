#include "ai_common.h"

#define NS_WAIT					1
#define NS_SEEK_PLAYER			2
#define NS_PREPARE_FIRE			3
#define NS_FIRING				4
#define NS_RETURN_TO_SET_POINT	5
#define NS_GUARD_SET_POINT		6

void ai_night_spirit(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->state = NS_WAIT;
			e->hidden = TRUE;
			e->y_mark = e->y + (12 << CSF);
		} /* fallthrough */
		case NS_WAIT:
		{
			if (PLAYER_DIST_Y(e, 8 << CSF)) {
				static const int32_t distance = pixel_to_sub(240);
				e->y += e->dir ? distance : -distance;
				
				e->state = NS_SEEK_PLAYER;
				e->timer = 0;
				e->hidden = FALSE;
				
				e->y_speed = 0;
				e->flags |= NPC_SHOOTABLE;
			}
		}
		break;
		
		case NS_SEEK_PLAYER:
		{
			ANIMATE(e, 4, 0,1,2);
			
			if (++e->timer > TIME_8(200)) {
				e->state = NS_PREPARE_FIRE;
				e->timer = 0;
				e->frame += 3;
			}
		}
		break;
		
		case NS_PREPARE_FIRE:
		{
			ANIMATE(e, 4, 3,4,5);
			if (++e->timer > TIME_8(50)) {
				e->state = NS_FIRING;
				e->timer = 0;
				e->frame += 3;
			}
		}
		break;
		
		case NS_FIRING:
		{
			ANIMATE(e, 4, 6,7,8);
			
			if (!(e->timer & 7)) {
				Entity *shot = entity_create(e->x, e->y, OBJ_NIGHT_SPIRIT_SHOT, 0);
				shot->x_speed = SPEED_10(0x180) + SPEED_10(rand() & 0x3FF);
				shot->y_speed = -SPEED_10(0x200) + SPEED_10(rand() & 0x3FF);
				
				sound_play(SND_BUBBLE, 3);
			}
			
			if (++e->timer > TIME(50)) {
				e->state = NS_SEEK_PLAYER;
				e->timer = 0;
				e->frame -= 6;
			}
		}
		break;
		
		case NS_RETURN_TO_SET_POINT:
		{
			ANIMATE(e, 4, 3,4,5);
			
			// lie in wait at original set point
			e->y_speed += (e->y > e->y_mark) ? -SPEED(0x40) : SPEED(0x40);
			LIMIT_Y(SPEED(0x400));
			
			if (abs(e->y - e->y_mark) < pixel_to_sub(ScreenHalfH)) {
				e->state = NS_GUARD_SET_POINT;
			}
		}
		break;
		
		case NS_GUARD_SET_POINT:
		{
			ANIMATE(e, 4, 3,4,5);
			
			// lie in wait at original set point
			e->y_speed += (e->y > e->y_mark) ? -SPEED(0x40) : SPEED(0x40);
			LIMIT_Y(SPEED(0x400));
			
			// and if player appears again...
			if (PLAYER_DIST_Y(e, pixel_to_sub(ScreenHeight))) {
				// ..jump out and fire immediately
				e->state = NS_PREPARE_FIRE;
				e->timer = 0;
			}
		}
		break;
	}
	
	if (e->state >= NS_SEEK_PLAYER && e->state < NS_GUARD_SET_POINT) {
		// sinusoidal player seek
		e->y_speed += (e->y < player.y) ? SPEED(0x19) : -SPEED(0x19);
		
		// rarely seen, but they do bounce off walls
		e->x_next = e->x;
		e->y_next = e->y + e->y_speed;
		if (collide_stage_ceiling(e)) e->y_speed = SPEED(0x200);
		if (collide_stage_floor(e)) e->y_speed = -SPEED(0x200);
		e->y = e->y_next;
		
		// avoid leaving designated area
		if (abs(e->y - e->y_mark) > pixel_to_sub(ScreenHeight)) {
			if (e->state != NS_FIRING) {
				e->state = NS_RETURN_TO_SET_POINT;
			}
		}
	}
	LIMIT_Y(SPEED(0x400));
}

void ai_night_spirit_sh(Entity *e) {
	ANIMATE(e, 4, 0,1,2);
	e->x_speed -= SPEED(0x19);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if (e->x_speed < 0 && collide_stage_leftwall(e)) {
		effect_create_smoke(e->x >> CSF, e->y >> CSF);
		sound_play(SND_SHOT_HIT, 3);
		e->state = STATE_DELETE;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

// Hoppy jumps off the left walls, physics needs to be handled a bit weird here
void ai_hoppy(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->enableSlopes = FALSE;
			e->grounded = TRUE;
		} /* fallthrough */
		case 1:		// wait for player...
		{
			e->frame = 0;
			if (PLAYER_DIST_Y(e, 0x10000)) {
				e->state = 2;
				e->timer = 0;
				e->frame = 1;
			}
		}
		break;
		
		case 2:	// jump
		{
			e->timer++;
			
			if (e->timer == 4)
				e->frame = 2;
			
			if (e->timer > 12) {
				e->state = 3;
				e->frame = 3;
				
				sound_play(SND_HOPPY_JUMP, 5);
				e->x_speed = SPEED(0x700);
				e->grounded = FALSE;
			}
		}
		break;
		
		case 3:	// in air...
		{
			if (e->y < player.y)	  e->y_speed = SPEED(0xAA);
			else if (e->y > player.y) e->y_speed = -SPEED(0xAA);
			
			// Top/bottom
			if (e->x_speed > 0) {
				collide_stage_rightwall(e);
			} else if (e->x_speed < 0) {
				if((e->grounded = collide_stage_leftwall(e))) {
					e->y_speed = 0;
					
					e->state = 4;
					e->frame = 2;
					e->timer = 0;
				}
			}
			
			// Sides
			if(e->y_speed < 0) {
				collide_stage_ceiling(e);
			} else if(e->y_speed > 0) {
				collide_stage_floor(e);
			}
		}
		break;
		
		case 4:
		{
			e->timer++;
			if (e->timer == 2) e->frame = 1;
			if (e->timer == 6) e->frame = 0;
			
			if (e->timer > 16)
				e->state = 1;
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
	
	if(!e->grounded) e->x_speed -= SPEED(0x2A);
	LIMIT_X(SPEED(0x5ff));
}

void onspawn_skydragon(Entity *e) {
	if(stageID == STAGE_FALLING) {
		e->alwaysActive = TRUE;
	}
}

void ai_sky_dragon(Entity *e) {
	switch(e->state) {
		case 0:		// standing
		{
			e->state++;
			e->y -= 4 << CSF; // Push up 4 pixels cause we in the ground
		} /* fallthrough */
		case 1:
		{
			ANIMATE(e, 30, 0,1);
		}
		break;
		
		case 10:	// player and kazuma gets on, dragon floats up
		{
			e->state = 11;
			// There is a separate 2 frames for the player wearing the Mimiga Mask
			if(playerEquipment & EQUIP_MIMIMASK)
				e->frame = 4;
			else
				e->frame = 2;
			e->animtime = 0;
			
			e->x_mark = e->x - (6 << CSF);
			e->y_mark = e->y - (16 << CSF);
			
			e->y_speed = 0;
		} /* fallthrough */
		case 11:
		{
			moveMeToFront = TRUE; // For normal ending, stay above clouds
			if(++e->animtime > 8) {
				e->frame ^= 1; // swap between 2-3 or 4-5 for mimiga mask
				e->animtime = 0;
			}
			e->x_speed += (e->x < e->x_mark) ? 0x08 : -0x08;
			e->y_speed += (e->y < e->y_mark) ? 0x08 : -0x08;
		}
		break;
		
		case 20:	// fly away
		{
			if(++e->animtime > 8) {
				e->frame ^= 1;
				e->animtime = 0;
			}
			e->y_speed += (e->y < e->y_mark) ? 0x08 : -0x08;
			e->x_speed += 0x20;
			LIMIT_X(SPEED(0x600));
		}
		break;
		
		case 30:	// spawn a Sue hanging from mouth
		{
			Entity *sue = entity_create(e->x, e->y, OBJ_SUE, 0);
			sue->linkedEntity = e;
			sue->state = 50;
			e->state++;
		}
		break;
	}
	e->dir = 0; // Script makes us face right, but sprite already does that
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_pixel_cat(Entity *e) {
	if (e->state == 0) {
		e->y -= (32 << CSF);
		e->state = 1;
	}
}

void ai_little_family(Entity *e) {
	e->frame &= 1;
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	
	switch(e->state) {
		case 0: // Standing
		{
			e->enableSlopes = TRUE;
			e->state = 1;
			e->frame = 0;
			e->x_speed = 0;
		} /* fallthrough */
		case 1:
		{
			if(!(rand() & 63)) {
				e->state = (rand() & 1) ? 2 : 10;
				e->timer = 0;
				e->frame = 1;
			}
		}
		break;
		case 2:
		{
			if (++e->timer > 8) {
				e->state = 1;
				e->frame = 0;
			}
		}
		break;
		case 10: // Walking
		{
			e->state = 11;
			e->frame = 0;
			e->animtime = 0;
			e->dir = rand() & 1;
			e->timer = 16 + (rand() & 15);
		} /* fallthrough */
		case 11:
		{
			if ((!e->dir && collide_stage_leftwall(e)) ||
				(e->dir && collide_stage_rightwall(e))) {
				e->dir ^= 1;
			}
			
			MOVE_X(SPEED(0x100));
			ANIMATE(e, 4, 0,1);
			
			if (e->timer == 0) e->state = 0;
			else e->timer--;
		}
		break;
	}
	
	switch(e->event) {
		case 210: e->frame += 2; break;		// red mom
		case 220: e->frame += 4; break;		// little son
	}
	
	if(!e->grounded) e->y_speed += SPEED(0x20);
	LIMIT_Y(SPEED(0x5ff));
	
	e->x = e->x_next;
	e->y = e->y_next;
}
