#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "camera.h"

void ai_pignon_onUpdate(Entity *e) {
	e->state_time++;
	switch(e->state) {
		case 0: // Standing
		if(e->state_time > 120 && (e->state_time & 31) == 0) { 
			// Either blink or walk in a random direction
			u8 rnd = random() & 7;
			if(rnd == 0) {
				ENTITY_SET_STATE(e, 1, 0);
				SPR_SAFEANIM(e->sprite, 2);
			} else if(rnd == 1) {
				ENTITY_SET_STATE(e, 2, 0);
				e->direction = random() & 1;
				e->x_speed = -0x100 + 0x200 * e->direction;
				SPR_SAFEANIM(e->sprite, 1);
				SPR_SAFEHFLIP(e->sprite, e->direction);
			}
		}
		break;
		case 1: // Blink
		if(e->state_time >= 10) {
			ENTITY_SET_STATE(e, 0, 0);
			SPR_SAFEANIM(e->sprite, 0);
		}
		break;
		case 2: // Walking
		if(e->state_time >= 30 && (random() & 31) == 0) {
			ENTITY_SET_STATE(e, 0, 0);
			e->x_speed = 0;
			SPR_SAFEANIM(e->sprite, 0);
		}
		break;
		case 3: // Hurt
		e->x_speed += -0x4 + 0x8 * e->direction; // Decellerate
		if(e->state_time >= 60) {
			ENTITY_SET_STATE(e, 0, 0);
			e->x_speed = 0;
			SPR_SAFEANIM(e->sprite, 0);
		}
		break;
	}
	if(!e->grounded) e->y_speed += gravityJump;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_pignon_onHurt(Entity *e) {
	ENTITY_SET_STATE(e, 3, 0);
	e->y_speed = -0x100;
	e->x_speed = -0x150 + 0x300 * !e->direction; // Knock backwards
	SPR_SAFEANIM(e->sprite, 3);
}

void ai_gkeeper_onUpdate(Entity *e) {
	
}
