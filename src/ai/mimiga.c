#include "ai_common.h"

void onspawn_flower(Entity *e) {
	e->frame = random() % 6;
}

void ai_jack(Entity *e) {
	if(!e->grounded) e->y_speed += SPEED(0x50);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	generic_npc_states(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_santa(Entity *e) {
	if(!tscState) FACE_PLAYER(e);
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
			e->frame = 4;
		}
		break;
		case 11: break;
		default: generic_npc_states(e);
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_mahin(Entity *e) {
	switch(e->state) {
		case 0:
		{
			SNAP_TO_GROUND(e);
			e->frame = 2;
			e->state = 1;
			e->dir = 1;
		}
		case 1: break;
		case 2:
		{
			e->frame = 0;
			RANDBLINK(e, 1, 200);
		}
		break;
	}
}

void ai_sanda(Entity *e) {
	if (!e->state) {
		if (e->eflags & NPC_OPTION2) {	
			// Sandaime gone
			e->x -= 16 << CSF;
			e->state = 2;
			e->frame = 2;
		} else {	
			// Sandaime present
			e->state = 1;
		}
	}
	if (e->state == 1) {
		e->frame = 0;
		RANDBLINK(e, 1, 200);
	}
}

void ai_cthulu(Entity *e) {
	if (!e->state) {
		SNAP_TO_GROUND(e);
		e->state = 1;
	}
	// open eyes when player comes near
	e->frame = (PLAYER_DIST_X((48<<CSF)) && PLAYER_DIST_Y2((48<<CSF), (16<<CSF))) ? 1 : 0;
}
