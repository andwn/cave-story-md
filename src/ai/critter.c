#include "ai_common.h"

void ai_critter(Entity *e) {
	enum {
		STATE_WAITING = 0, 
		STATE_ATTENTION = 10, 
		STATE_HOPPING = 20, 
		STATE_FLYING = 30
	};
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	/*
	if((e->x_speed > 0 && (blk(e->x, e->hit_box.right, e->y, 0) & 0x41 == 0x41)) ||
			(e->x_speed < 0 && (blk(e->x, -e->hit_box.left, e->y, 0) & 0x41 == 0x41))) {
		e->y_speed = 0;
	}
	* */
	if(e->x_speed > 0) collide_stage_rightwall(e);
	if(e->x_speed < 0) collide_stage_leftwall(e);
	if(e->y_speed < 0) collide_stage_ceiling(e);
	else if(!e->grounded) e->grounded = collide_stage_floor(e);
	
	switch(e->state) {
		case STATE_WAITING:
		{
			if(e->type == OBJ_POWER_CRITTER) e->attack = 2;
			e->frame = 0;
			e->x_speed = e->y_speed = 0;
			e->state++;
		} /* fallthrough */
		case STATE_WAITING+1:
		{
			if(e->damage_time || (PLAYER_DIST_X(0x60 << CSF) && PLAYER_DIST_Y(0x40 << CSF))) {
				FACE_PLAYER(e);
				e->state = STATE_ATTENTION;
			}
		}
		break;
		case STATE_ATTENTION:
		{
			e->frame = 1;
			e->state++;
			e->timer = 0;
		} /* fallthrough */
		case STATE_ATTENTION+1:
		{
			if(++e->timer > TIME(25)) {
				e->state = STATE_HOPPING;
			}
		}
		break;
		case STATE_HOPPING:
		{
			e->frame = 2;
			e->grounded = FALSE;
			e->y_speed = -SPEED(0x600);
			sound_play(SND_ENEMY_JUMP, 5);
			e->state++;
			e->timer = 0;
		} /* fallthrough */
		case STATE_HOPPING+1:
		{
			// Keep trying to move after hitting a wall
			if(e->type != OBJ_CRITTER_SHOOTING_PURPLE) MOVE_X(SPEED(0x100));
			// Handle critters that can hover
			if(e->y_speed > SPEED(0x100) && (e->type == OBJ_CRITTER_FLYING || 
					e->type == OBJ_POWER_CRITTER || e->type == OBJ_CRITTER_SHOOTING_PURPLE)) {
				e->state = STATE_FLYING;
			} else if(e->grounded) {
				e->state = STATE_WAITING;
			}
		}
		break;
		case STATE_FLYING:
		{
			e->frame = 3;
		} /* fallthrough */
		case STATE_FLYING+1:
		{
			if(!(++e->timer % 8)) {
				sound_play(SND_CRITTER_FLY, 2);
				if(++e->frame > 5) e->frame = 3;
			}
			e->y_speed -= SPEED(0x50);
			if(e->timer == TIME(25)) {
				if(e->type == OBJ_CRITTER_SHOOTING_PURPLE) {
					sound_play(SND_EM_FIRE, 3);
					Entity *shot = entity_create(e->x, e->y, OBJ_CRITTER_SHOT, 0);
					THROW_AT_TARGET(shot, player.x, player.y, 0x200);
				}
			} else if(e->timer > TIME(50)) {
				e->frame = 1;
				e->state++;
			}
		}
		break;
		case STATE_FLYING+2:
		{
			if(e->grounded) {
				e->state = STATE_WAITING;
			} else if(e->type == OBJ_POWER_CRITTER) {
				// Crush player
				if(player.y > e->y + (16<<CSF)) {
					e->attack = 10;
				} else {
					e->attack = 2;
				}
			}
		}
		break;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
	if(!e->grounded) {
		e->y_speed += SPEED(0x40);
		LIMIT_Y(SPEED(0x5FF));
	}
}
