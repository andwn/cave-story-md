#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

// The range is a bit too high so here is my lazy way to fix it
void ai_batVertical_onCreate(Entity *e) {
	e->y += pixel_to_sub(24);
}

// Just up and down gotta go up and down
void ai_batVertical_onUpdate(Entity *e) {
	if(e->state == 0) {
		e->y_speed -= 8;
		if(e->y_speed <= pixel_to_sub(-1)) e->state = 1;
	} else if(e->state == 1) {
		e->y_speed += 8;
		if(e->y_speed >= pixel_to_sub(1)) e->state = 0;
	}
	FACE_PLAYER(e);
	e->y += e->y_speed;
}

void ai_batHang_onCreate(Entity *e) {
	e->spriteAnim = 1; // Hanging anim
}

void ai_batHang_onUpdate(Entity *e) {
	if(e->state == 0) {
		s32 x1 = e->x - 0x1000, y1 = e->y - 0x1000,
			x2 = e->x + 0x1000, y2 = e->y + 0x9000;
		if(player.x > x1 && player.x < x2 && player.y > y1 && player.y < y2) {
			e->direction = player.x > e->x;
			SPR_SAFEANIM(e->sprite, 3);
			ENTITY_SET_STATE(e, 1, 0);
		}
	} else if(e->state == 1) {
		e->y_speed += 0x1D;
		e->y_next = e->y + e->y_speed;
		if(++e->state_time > 24 || collide_stage_floor(e)) {
			SPR_SAFEANIM(e->sprite, 0);
			ENTITY_SET_STATE(e, 2, 0);
		} else {
			e->y = e->y_next;
		}
	} else {
		e->state_time++;
		FACE_PLAYER(e);
		e->x_speed += (e->x > player.x) ? -0x1D : 0x1D;
		e->y_speed += (e->state_time % 60) < 30 ? -0x0F : 0x0F;
		e->x_next = e->x + e->x_speed;
		e->y_next = e->y + e->y_speed;
		if(collide_stage_floor(e)) e->y_speed = -0x200;
		if(e->direction ? collide_stage_rightwall(e) : collide_stage_leftwall(e)) {
			e->y_speed = 0x200;
		}
		e->x = e->x_next;
		e->y = e->y_next;
	}
}
