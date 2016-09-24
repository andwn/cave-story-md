#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void onspawn_flower(Entity *e) {
	e->frame = random() % 6;
}

void ai_jack(Entity *e) {
	if(!e->grounded) e->y_speed += GRAVITY;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	generic_npc_states(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_santa(Entity *e) {
	generic_npc_states(e);
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_chaco(Entity* e) {
	switch(e->state) {
		case 10: // Sleeping
		{
			e->state++;
			e->x_speed = 0;
			e->frame = 3;
		}
		break;
		case 11: break;
		default:
			generic_npc_states(e);
		break;
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}
