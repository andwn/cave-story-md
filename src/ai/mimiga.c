#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void ai_flower_onCreate(Entity *e) {
	e->spriteFrame = random() % 6;
}

void ai_jack_onUpdate(Entity *e) {
	if(!e->grounded) e->y_speed += gravity;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_jack_onState(Entity *e) {
	switch(e->state) {
		case 0: // Stand
		e->x_speed = 0;
		SPR_SAFEANIM(e->sprite, 0);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		break;
		case 8: // Walk
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEANIM(e->sprite, 1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		break;
	}
}
