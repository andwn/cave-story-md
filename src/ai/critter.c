#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void ai_critterHop_onUpdate(Entity *e) {
	if(!e->grounded) {
		e->y_speed += gravity;
		e->x_next = e->x + e->x_speed;
		e->y_next = e->y + e->y_speed;
		entity_update_collision(e);
		e->x = e->x_next;
		e->y = e->y_next;
		if(e->grounded) ENTITY_SET_STATE(e, 0, 0);
	} else if(e->state == 1) {
		e->state_time--;
		if(e->state_time == 0) ENTITY_SET_STATE(e, 2, 0);
	} else {
		s32 x1 = e->x - block_to_sub(6), y1 = e->y - block_to_sub(6),
			x2 = e->x + block_to_sub(6), y2 = e->y + block_to_sub(6);
		if(player.x > x1 && player.x < x2 && player.y > y1 && player.y < y2) {
			e->direction = player.x > e->x;
			ENTITY_SET_STATE(e, 1, 30);
		}
	}
}

void ai_critterHop_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) {
		e->state = STATE_DESTROY;
		return;
	}
	switch(e->state) {
	case 0: // Still on the ground
		e->y_speed = 0;
		e->x_speed = 0;
		SPR_SAFEFRAME(e->sprite, 0);
		break;
	case 1: // Preparing to jump
		SPR_SAFEFRAME(e->sprite, 1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		break;
	case 2: // Hop
		e->grounded = false;
		e->x_speed = -0x100 + 0x200 * e->direction;
		e->y_speed = pixel_to_sub(-3);
		SPR_SAFEFRAME(e->sprite, 2);
		sound_play(SOUND_HOP, 5);
		break;
	}
}
