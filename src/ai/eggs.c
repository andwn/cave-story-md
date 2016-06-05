#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "camera.h"

void ai_behemoth_onUpdate(Entity *e) {
	if(e->x_speed == 0) {
		e->direction = !e->direction;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		e->x_speed = -0x100 + 0x200 * e->direction;
	}
	if(!e->grounded) e->y_speed += gravityJump;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_behemoth_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) e->state = STATE_DESTROY;
}

void ai_beetle_onUpdate(Entity *e) {
	u16 x = sub_to_block(e->x), y = sub_to_block(e->y);
	switch(e->state) {
		case 0: // Initial state / moving left or right
		if(e->x_speed == 0) {
			e->x_speed = -0x200 + 0x400 * e->direction;
		} else if((sub_to_pixel(e->x) & 15) == 7) {
			if(!e->direction && (stage_get_block_type(x - 1, y) & 0xF) == BLOCK_SOLID) {
				e->state = 1;
				e->state_time = 0;
				e->x_speed = 0;
				SPR_SAFEHFLIP(e->sprite, 0);
				//SPR_SAFEANIM(e->sprite, 1);
			} else if(e->direction && (stage_get_block_type(x + 1, y) & 0xF) == BLOCK_SOLID) {
				e->state = 1;
				e->state_time = 0;
				e->x_speed = 0;
				SPR_SAFEHFLIP(e->sprite, 1);
				//SPR_SAFEANIM(e->sprite, 1);
			} 
		}
		break;
		case 1: // On wall
		if(++e->state_time > 60) {
			u16 py = sub_to_block(player.y);
			if(py >= y - 1 || py <= y + 1) {
				e->state = 0;
				e->direction = !e->direction;
				e->x_speed = -0x200 + 0x400 * e->direction;
				SPR_SAFEHFLIP(e->sprite, e->direction);
				//SPR_SAFEANIM(e->sprite, 0);
			}
		}
		break;
	}
	e->x += e->x_speed;
}

void ai_beetleFollow_onUpdate(Entity *e) {
	u8 dir = player.x >= e->x;
	if(dir != e->direction) {
		e->direction = dir;
		SPR_SAFEHFLIP(e->sprite, dir);
	}
	e->x_speed += dir ? 6 : -6;
	if(abs(e->x_speed) > 0x300) e->x_speed = dir ? 0x300 : -0x300;
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_basil_onCreate(Entity *e) {
	e->alwaysActive = true;
	e->x = player.x;
	e->x_speed = -0x400;
}

void ai_basil_onUpdate(Entity *e) {
	if(e->x_speed == 0) { // Hit a wall
		e->direction = !e->direction;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		e->x_speed = -0x400 + 0x800 * e->direction;
	} else if(sub_to_pixel(e->x) < sub_to_pixel(camera.x) - SCREEN_HALF_W - 64) {
		e->direction = 1;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		e->x_speed = 0x400;
	} else if(sub_to_pixel(e->x) > sub_to_pixel(camera.x) + SCREEN_HALF_W + 64) {
		e->direction = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		e->x_speed = -0x400;
	}
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

// Push out of the wall and align with bottom floor
void ai_lift_onCreate(Entity *e) {
	e->x += pixel_to_sub(8);
	e->y += pixel_to_sub(8);
}

// Moves through floors 1, 2, 3, 1, 2, 3 - doesn't stop at 2 on the way down
void ai_lift_onUpdate(Entity *e) {
	
}
