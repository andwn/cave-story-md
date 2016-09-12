#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "resources.h"
#include "sheet.h"

enum {
	STATE_WAITING = 0, 
	STATE_ATTENTION = 10, 
	STATE_HOPPING = 20, 
	STATE_FLYING = 30
};

void ai_critter(Entity *restrict e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	switch(e->state) {
		case STATE_WAITING:
		{
			e->frame = 0;
			e->x_speed = e->y_speed = 0;
			e->state++;
		}
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
		}
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
			e->grounded = false;
			if(e->type != OBJ_CRITTER_SHOOTING_PURPLE) MOVE_X(SPEED(0x100));
			e->y_speed = SPEED(-0x600);
			sound_play(SND_ENEMY_JUMP, 5);
			e->state++;
			e->timer = 0;
		}
		case STATE_HOPPING+1:
		{
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
		}
		case STATE_FLYING+1:
		{
			ANIMATE(e, 4, 3,4,5);
			e->timer++;
			e->y_speed -= SPEED(0x50);
			if(e->timer % 8 == 1) sound_play(SND_CRITTER_FLY, 2);
			if(e->timer > TIME(50)) {
				e->frame = 1;
				e->state++;
			} else if(e->grounded) {
				e->state = STATE_WAITING;
			}
		}
		break;
		case STATE_FLYING+2:
		{
			// Falling
			if(e->grounded) {
				e->state = STATE_WAITING;
			}
		}
		break;
	}
	if(e->x_speed > 0) collide_stage_rightwall(e);
	else if(e->x_speed < 0) collide_stage_leftwall(e);
	e->x = e->x_next;
	e->y = e->y_next;
	if(!e->grounded) {
		e->y_speed += SPEED(0x40);
		LIMIT_Y(SPEED(0x5FF));
	}
}
