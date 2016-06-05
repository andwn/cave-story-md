#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void ai_miseryFloat_onUpdate(Entity *e) {
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

void ai_miseryFloat_onState(Entity *e) {
	switch(e->state) {
	case 20: // Hover, going up
	case 21:
		break;
	default:
		break;
	}
}
