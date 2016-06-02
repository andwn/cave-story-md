#include "behavior.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void ai_update_toroko(Entity *e) {
	switch(e->state) {
	case 0: // Stand still
		SPR_setAnim(e->sprite, 0);
		break;
	case 3: // Run back and forth
	case 4:
		if(e->attack == 0) {
			SPR_setAnim(e->sprite, 1);
		} else {
			SPR_setAnim(e->sprite, 2);
			Bullet *b = bullet_colliding(e);
			if(b != NULL) {
				sound_play(e->hurtSound, 10); // Squeak
				e->attack = 0; // Don't hurt the player anymore
				e->eflags |= NPC_INTERACTIVE; // Enable interaction
				e->state = 10; // Change animation to falling on ground
				e->y_speed = pixel_to_sub(-1);
				e->x_speed /= 2;
				e->grounded = false;
				SPR_setAnim(e->sprite, 3);
				b->ttl = 0;
				SPR_SAFERELEASE(b->sprite);
			}
		}
		if(e->x_speed == 0) { // Stop after hitting a wall
			e->direction = !e->direction;
			e->x_speed = pixel_to_sub(-2 + 4*e->direction);
			SPR_setHFlip(e->sprite, e->direction);
		}
		break;
	case 6: // Jump then run
		if(e->x_speed == 0 && e->grounded) {
			e->x_speed = pixel_to_sub(-2 + 4*e->direction);
		}
		break;
	case 8: // Jump in place (don't run after)
		break;
	//case 9: // Run off screen and delete self (after 6)
	//	if(e->x_speed == 0 && e->grounded) {
	//		e->x_speed = pixel_to_sub(-2 + 4*e->direction);
	//	}
		//if(!entity_on_screen(e)) { }
	//	break;
	case 10: // Falling down
		if(e->grounded) {
			e->x_speed = 0;
			e->state = 11;
			SPR_setAnim(e->sprite, 4);
		}
		break;
	case 11: // After falling on ground
		e->direction = 0;
		break;
	default:
		break;
	}
	if(!e->grounded) e->y_speed += gravity;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

bool ai_setstate_toroko(Entity *e, u16 state) {
	e->state = state;
	switch(state) {
	case 6: // Jump
	case 8:
		e->y_speed = pixel_to_sub(-1);
		e->grounded = false;
		break;
	default:
		break;
	}
	return false;
}

void ai_update_misery_float(Entity *e) {
	switch(e->state) {
	case 20:
	case 21:
		e->y_speed -= 0x20;
		break;
	default:
		break;
	}
	e->y += e->y_speed;
}

bool ai_setstate_misery_float(Entity *e, u16 state) {
	e->state = state;
	switch(state) {
	case 20: // Hover, going up
	case 21:
		break;
	default:
		break;
	}
	return false;
}
