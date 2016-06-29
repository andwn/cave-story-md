#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void ai_torokoAtk_onCreate(Entity *e) {
	e->y -= block_to_sub(1);
	e->x_speed = 0x300; // 1.5px
	e->state = 3; // Running back and forth
	e->spriteAnim = 2;
}

void ai_toroko_onUpdate(Entity *e) {
	switch(e->state) {
	case 0: // Stand still
		SPR_SAFEANIM(e->sprite, 0);
		break;
	case 3: // Run back and forth
	case 4:
		if(e->attack == 0) {
			SPR_SAFEANIM(e->sprite, 1);
		} else {
			SPR_SAFEANIM(e->sprite, 2);
			Bullet *b = bullet_colliding(e);
			if(b != NULL) {
				sound_play(e->hurtSound, 10); // Squeak
				e->attack = 0; // Don't hurt the player anymore
				e->eflags |= NPC_INTERACTIVE; // Enable interaction
				e->state = 10; // Change animation to falling on ground
				e->y_speed = pixel_to_sub(-1);
				e->x_speed /= 2;
				e->grounded = false;
				SPR_SAFEANIM(e->sprite, 3);
				b->ttl = 0;
				SPR_SAFERELEASE(b->sprite);
			}
		}
		// Switch direction in specific range
		if((e->x_speed > 0 && e->x > block_to_sub(15)) || 
			(e->x_speed < 0 && e->x < block_to_sub(10))) {
			e->direction = !e->direction;
			e->x_speed = -0x300 + 0x600 * e->direction;
			SPR_SAFEHFLIP(e->sprite, e->direction);
		}
		break;
	case 6: // Jump then run
		if(e->grounded && abs(e->x_speed) < 0x300) {
			e->x_speed = -0x300 + 0x600 * e->direction;
			e->state = 7; // Toroko stops after hitting a wall so don't keep doing this
		}
		break;
	case 7:
		break;
	case 8: // Jump in place (don't run after)
		break;
	case 10: // Falling down
		if(e->grounded) {
			e->x_speed = 0;
			e->state = 11;
			SPR_SAFEANIM(e->sprite, 4);
		}
		break;
	case 11: // After falling on ground
		e->direction = 0;
		break;
	default:
		break;
	}
	if(!e->grounded) e->y_speed += GRAVITY_JUMP;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_toroko_onState(Entity *e) {
	switch(e->state) {
	case 6: // Jump
		SPR_SAFEANIM(e->sprite, 1);
		e->y_speed = -0x150;
		e->x_speed = -0x150 + 0x300 * e->direction;
		e->grounded = false;
		break;
	case 8:
		e->y_speed = 0x150;
		e->grounded = false;
		break;
	default:
		break;
	}
}
