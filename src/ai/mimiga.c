#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

#ifdef PAL
#define MIMIGA_WALKSPEED 0x200
#else
#define MIMIGA_WALKSPEED 0x1D0
#endif

void ai_flower_onCreate(Entity *e) {
	e->spriteFrame = random() % 6;
}

void ai_jack_onUpdate(Entity *e) {
	if(!e->grounded) e->y_speed += GRAVITY;
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
		MOVE_X(MIMIGA_WALKSPEED);
		SPR_SAFEANIM(e->sprite, 1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		break;
	}
}

void ai_santa_onUpdate(Entity *e) {
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_santa_onState(Entity *e) {
	switch(e->state) {
		case 0: // Stand
		e->x_speed = 0;
		SPR_SAFEANIM(e->sprite, 0);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		break;
		case 3: // Walk
		case 4:
		MOVE_X(MIMIGA_WALKSPEED);
		SPR_SAFEANIM(e->sprite, 1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		break;
	}
}

void ai_chaco_onUpdate(Entity* e) {
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_chaco_onState(Entity *e) {
	switch(e->state) {
		case 0: // Stand
		e->x_speed = 0;
		SPR_SAFEANIM(e->sprite, 0);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		break;
		case 3: // Walk
		case 4:
		MOVE_X(MIMIGA_WALKSPEED);
		SPR_SAFEANIM(e->sprite, 1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		break;
		case 10: // Sleeping
		e->x_speed = 0;
		SPR_SAFEANIM(e->sprite, 3);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		break;
	}
}
