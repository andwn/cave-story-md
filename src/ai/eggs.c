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
		e->dir = !e->dir;
		SPR_SAFEHFLIP(e->sprite, e->dir);
		e->x_speed = -0x100 + 0x200 * e->dir;
	}
	if(!e->grounded) e->y_speed += GRAVITY_JUMP;
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
			e->x_speed = -0x200 + 0x400 * e->dir;
		} else if((sub_to_pixel(e->x) & 15) == 7) {
			if(!e->dir && stage_get_block_type(x - 1, y) == 0x41) {
				e->state = 1;
				e->state_time = 0;
				e->x_speed = 0;
				SPR_SAFEHFLIP(e->sprite, 0);
				SPR_SAFEANIM(e->sprite, 0);
			} else if(e->dir && stage_get_block_type(x + 1, y) == 0x41) {
				e->state = 1;
				e->state_time = 0;
				e->x_speed = 0;
				SPR_SAFEHFLIP(e->sprite, 1);
				SPR_SAFEANIM(e->sprite, 0);
			} 
		}
		break;
		case 1: // On wall
		if(++e->state_time > 60) {
			u16 py = sub_to_block(player.y);
			if(py >= y - 1 || py <= y + 1) {
				e->state = 0;
				e->dir = !e->dir;
				e->x_speed = -0x200 + 0x400 * e->dir;
				SPR_SAFEHFLIP(e->sprite, e->dir);
				SPR_SAFEANIM(e->sprite, 1);
			}
		}
		break;
	}
	e->x += e->x_speed;
}

void ai_beetleFollow_onCreate(Entity *e) {
	e->frame = 1;
	e->state_time = 45;
}

void ai_beetleFollow_onUpdate(Entity *e) {
	e->state_time++;
	u8 dir = player.x >= e->x;
	if(dir != e->dir) {
		e->dir = dir;
		SPR_SAFEHFLIP(e->sprite, dir);
	}
	e->x_speed += dir ? 6 : -6;
	if(abs(e->x_speed) > 0x300) e->x_speed = dir ? 0x300 : -0x300;
	e->y_speed += (e->state_time % 180) >= 90 ? -4 : 4;
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_basu_onCreate(Entity *e) {
	e->state_time = 45;
	e->attack = 5;
}

void ai_basu_onUpdate(Entity *e) {
	e->state_time++;
	u8 dir = player.x >= e->x;
	if(dir != e->dir) {
		e->dir = dir;
		SPR_SAFEHFLIP(e->sprite, dir);
	}
	e->x_speed += dir ? 5 : -5;
	if(abs(e->x_speed) > 0x200) e->x_speed = dir ? 0x200 : -0x200;
	e->y_speed += (e->state_time % 180) >= 90 ? -3 : 3;
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
		e->dir = !e->dir;
		SPR_SAFEHFLIP(e->sprite, e->dir);
		e->x_speed = -0x400 + 0x800 * e->dir;
	} else if(sub_to_pixel(e->x) < sub_to_pixel(camera.x) - SCREEN_HALF_W - 64) {
		e->dir = 1;
		SPR_SAFEHFLIP(e->sprite, e->dir);
		e->x_speed = 0x400;
	} else if(sub_to_pixel(e->x) > sub_to_pixel(camera.x) + SCREEN_HALF_W + 64) {
		e->dir = 0;
		SPR_SAFEHFLIP(e->sprite, e->dir);
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
	//e->y += pixel_to_sub(8);
}

#define LIFT_SPEED 0x200
#define LIFT_MOVE_TIME 64
#define LIFT_WAIT_TIME 90

// Moves through floors 1, 2, 3, 1, 2, 3 - doesn't stop at 2 on the way down
void ai_lift_onUpdate(Entity *e) {
	if(e->state_time > 0) e->state_time--;
	if(e->state_time == 0) {
		e->state++;
		if(e->state == 6) e->state = 0;
		switch(e->state) {
			case 0: // bottom floor wait
			e->state_time = LIFT_WAIT_TIME;
			e->y_speed = 0;
			break;
			case 1: // bottom to middle
			e->state_time = LIFT_MOVE_TIME;
			e->y_speed = -LIFT_SPEED;
			break;
			case 2: // middle wait
			e->state_time = LIFT_WAIT_TIME;
			e->y_speed = 0;
			break;
			case 3: // middle to top
			e->state_time = LIFT_MOVE_TIME;
			e->y_speed = -LIFT_SPEED;
			break;
			case 4: // top wait
			e->state_time = LIFT_WAIT_TIME;
			e->y_speed = 0;
			break;
			case 5: // top to bottom
			e->state_time = LIFT_MOVE_TIME * 2;
			e->y_speed = LIFT_SPEED;
			break;
		}
	}
	e->y += e->y_speed;
}

void ai_terminal_onUpdate(Entity *e) {
	switch(e->state) {
		case 0:
		case 1:
		SPR_SAFEANIM(e->sprite, 0);
		if(player.x > e->x - 0x1000 && player.x < e->x + 0x1000 && 
			player.y > e->y - 0x2000 && player.y < e->y + 0x1000) {
			sound_play(SND_COMPUTER_BEEP, 5);
			SPR_SAFEANIM(e->sprite, 1 + e->state);
			e->state = 10;
		}
		break;
		case 10:
		break;
	}
}
