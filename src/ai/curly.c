#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"

/*
 * Curly [0117] -By Shmitz
0000	Stand around. Eyes blink.
0003	Walk forward
0004	Walk forward
0005	Poof defeat
0006	Defeat
0010	Walk towards main char until 1 away
0011	Walk forward no falling
0020	Back turned
0021	Look up
0030	Knocked a bit over
0031	Fall over
0032	Fall over
0070	Moonwalk
0071	Moonwalk
* */

void ai_curly_onUpdate(Entity *e) {
	if(!e->grounded) e->y_speed += GRAVITY_JUMP;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	// Don't test ceiling, only test sticking to ground while moving
	if(e->x_speed < 0) {
		collide_stage_leftwall(e);
	} else if(e->x_speed > 0) {
		collide_stage_rightwall(e);
	}
	if(e->grounded) {
		if(e->state != 11 && e->x_speed != 0) e->grounded = collide_stage_floor_grounded(e);
	} else if(e->y_speed > 0) {
		e->grounded = collide_stage_floor(e);
		if(e->grounded && e->state == 30) ENTITY_SET_STATE(e, 31, 0);
	}
	//if(e->state == 10 && e->direction ? 
	//	(e->x > player.x - block_to_sub(2)) : (e->x < player.x + block_to_sub(2))) {
	//	ENTITY_SET_STATE(e, 0, 0);
	//}
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_curly_onState(Entity *e) {
	switch(e->state) {
		case 0: // Stand
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 0);
		break;
		case 3:
		case 4: // Walk
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 5:
		effect_create_smoke(0, sub_to_block(e->x), sub_to_block(e->y));
		case 6: // Defeated
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 9);
		break;
		case 10: // Walk towards player until 1 block away
		FACE_PLAYER(e);
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 11: // Walk with no gravity
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 20: // Back turned
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 4);
		break;
		case 21: // Look up
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 2);
		break;
		case 30: // Knocked over
		SPR_SAFEANIM(e->sprite, 10);
		e->x_speed = -0x150 + 0x300 * e->direction;
		e->y_speed = -0x150;
		e->grounded = false;
		break;
		case 31:
		case 32: // Fell over
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 9);
		break;
		case 70:
		case 71: // Walk backwards
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, !e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
	}
}
