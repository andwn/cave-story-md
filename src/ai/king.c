#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void ai_king_onUpdate(Entity *e) {
	if(!e->grounded) e->y_speed += gravityJump;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
	e->state_time++;
	switch(e->state) {
		case 0: // Stand & Blink randomly
		if(e->state_time > 200 && (e->state_time & 31) == 0 && (random() & 7) == 0) {
			e->state = 1;
			e->state_time = 0;
			SPR_SAFEANIM(e->sprite, 2);
		}
		break;
		case 1: // Blink
		if(e->state_time == 10) {
			ENTITY_SET_STATE(e, 0, 0);
		}
		break;
		case 5: // Fall over, change animation after hitting the ground
		case 6:
		case 7:
		if(abs(e->x_speed) > 0 && e->grounded) {
			e->x_speed = 0;
			SPR_SAFEANIM(e->sprite, 4);
		}
		break;
	}
}

void ai_king_onState(Entity *e) {
	switch(e->state) {
		case 0: // Stand around
		SPR_SAFEANIM(e->sprite, 0);
		e->x_speed = 0;
		e->grounded = true;
		break;
		case 5: // Fall over
		case 6:
		case 7:
		SPR_SAFEANIM(e->sprite, 3);
		e->x_speed = -0x150 + 0x300 * e->direction;
		e->y_speed = -0x150;
		e->grounded = false;
		break;
		case 8: // Walk forward
		SPR_SAFEANIM(e->sprite, 1);
		e->x_speed = -0x200 + 0x400 * e->direction;
		break;
		case 9: // Flop down, walk forward
		break;
		case 10: // Run forward
		case 11:
		break;
		case 20: // Brandish sword
		break;
		case 30: // Knocked way over
		case 31:
		break;
		case 40: // Die, leave sword
		case 42:
		break;
		case 60: // Leap to the right
		break;
	}
}
