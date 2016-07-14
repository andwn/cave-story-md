#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void ai_critter_onUpdate(Entity *e) {
	if(!e->grounded) { 
		if(e->sprite->animInd == 3) { // Flying
			e->state_time++;
			e->y_speed -= GRAVITY_JUMP / 2;
			if(e->state_time % 8 == 0) sound_play(SND_CRITTER_FLY, 2);
			if(e->state_time > 60) {
				SPR_SAFEANIM(e->sprite, 1);
			}
			e->x_next = e->x + e->x_speed;
			e->y_next = e->y + e->y_speed;
			entity_update_collision(e);
			e->x = e->x_next;
			e->y = e->y_next;
			if(e->grounded) ENTITY_SET_STATE(e, 0, 0);
		} else if((e->type == 0x18 || e->type == 0x1C) &&
			e->sprite->animInd == 2 && e->y_speed > 0x100) { // Begin to fly
			SPR_SAFEANIM(e->sprite, 3);
			e->state_time = 0;
		} else { // Jumping/falling
			e->y_speed += GRAVITY;
			e->x_next = e->x + e->x_speed;
			e->y_next = e->y + e->y_speed;
			entity_update_collision(e);
			e->x = e->x_next;
			e->y = e->y_next;
			if(e->grounded) ENTITY_SET_STATE(e, 0, 0);
		}
	} else if(e->state == 1) { // Preparing to jump
		e->state_time--;
		if(e->state_time == 0) ENTITY_SET_STATE(e, 2, 0);
	} else { // Hop towards player
		s32 x1 = e->x - block_to_sub(6), y1 = e->y - block_to_sub(4),
			x2 = e->x + block_to_sub(6), y2 = e->y + block_to_sub(4);
		if(player.x > x1 && player.x < x2 && player.y > y1 && player.y < y2) {
			e->direction = player.x > e->x;
			ENTITY_SET_STATE(e, 1, 30);
		}
	}
}

void ai_critter_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) {
		e->state = STATE_DESTROY;
		return;
	}
	switch(e->state) {
	case 0: // Still on the ground
		e->y_speed = 0;
		e->x_speed = 0;
		SPR_SAFEANIM(e->sprite, 0);
		break;
	case 1: // Preparing to jump
		SPR_SAFEANIM(e->sprite, 1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		break;
	case 2: // Hop
		e->grounded = false;
		e->x_speed = -0x100 + 0x200 * e->direction;
		e->y_speed = pixel_to_sub(-3);
		SPR_SAFEANIM(e->sprite, 2);
		sound_play(SND_ENEMY_JUMP, 5);
		break;
	}
}

void ai_critter_onHurt(Entity *e) {
	if(e->state < 1) {
		ENTITY_SET_STATE(e, 1, 10);
	}
}
