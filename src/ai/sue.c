#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void ai_sue_onUpdate(Entity *e) {
	if(e->state == 20 || e->state == 21) {
		if(player.x - block_to_sub(2) > e->x) ENTITY_SET_STATE(e, 0, 0);
	}
	if(!e->grounded) e->y_speed += GRAVITY;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	// Don't test ceiling, only test sticking to ground while moving
	if(e->x_speed < 0) {
		collide_stage_leftwall(e);
	} else if(e->x_speed > 0) {
		collide_stage_rightwall(e);
	}
	if(e->grounded) {
		if(e->x_speed != 0) collide_stage_floor_grounded(e);
	} else {
		collide_stage_floor(e);
		if(e->grounded && e->state == 8) ENTITY_SET_STATE(e, 10, 0);
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

/*
Sue [0042] -By Shmitz
0000	Stand around. Eyes blink.
0003	Walk forward			
0004	Walk forward			
0005	Back turned			
0008	Knocked a bit over		
0009	Fall over			
0010	Fall over			
0011	Throw a fit			
0012	Fall over, throw a fit.
0014	CRASH GAME
0015	Summons red crystal		
0017	Looks afraid
0020	Run while to left of main char		
0021	Run while to left of main char		
0030	Run forward			
0031	Run forward			
0040	Hops once
 */
void ai_sue_onState(Entity *e) {
	switch(e->state) {
		case 0:
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 0);
		break;
		case 3:
		case 4:
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 5:
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 3);
		break;
		case 8:
		e->x_speed = e->direction ? 0x100 : -0x100;
		e->y_speed = -0x300;
		e->grounded = false;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 4);
		sound_play(0x32, 6);
		break;
		case 9:
		case 10:
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 5);
		break;
		case 11:
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 7);
		break;
		case 12:
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 5);
		break;
		case 14: // Maybe when grabbed by Igor
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 6);
		break;
		case 15:
		break;
		case 17:
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 9);
		break;
		case 20:
		case 21:
		e->direction = 1;
		e->x_speed = pixel_to_sub(2);
		SPR_SAFEHFLIP(e->sprite, 1);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 30:
		case 31:
		e->x_speed = pixel_to_sub(e->direction ? 2 : -2);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 40:
		e->y_speed = -0x300;
		e->grounded = false;
		break;
	}
}
