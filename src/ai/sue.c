#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void ai_sue_onUpdate(Entity *e) {
	if(e->state == 13) { // Carried by Igor
		Entity *igor = entity_find_by_type(0x53);
		if(igor != NULL) { // This should never be NULL but just in case
			e->y = igor->y;
			e->x = igor->x + igor->direction ? block_to_sub(1) : block_to_sub(-1);
			SPR_SAFEHFLIP(e->sprite, igor->direction);
		}
	} else if(e->state == 6) {
		if(++e->state_time > 40) {
			ENTITY_SET_STATE(e, 0, 0);
		}
	} else {
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
		if(e->state == 8 && e->grounded) { 
			e->x_speed = 0;
			ENTITY_SET_STATE(e, 10, 0);
		}
	}
}

void ai_sue_onState(Entity *e) {
	switch(e->state) {
		case 0: // Stand
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 0);
		break;
		case 3: // Walking
		case 4: 
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 5: // Face away
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 3);
		break;
		case 6: // Punched by Igor
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 4);
		sound_play(SND_ENEMY_SQUEAK, 6);
		break;
		case 8: // Punched harder
		e->x_speed = e->direction ? -0x100 : 0x100;
		e->y_speed = -0x300;
		e->grounded = false;
		SPR_SAFEHFLIP(e->sprite, !e->direction);
		SPR_SAFEANIM(e->sprite, 4);
		sound_play(SND_ENEMY_SQUEAK, 6);
		break;
		case 9:
		case 10: // On the ground
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 5);
		break;
		case 11: // Punching in the air
		case 12:
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 7);
		break;
		case 13: // Grabbed by Igor
		case 14:
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 6);
		break;
		case 15: // Red crystal
		break;
		case 17:
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 9);
		break;
		case 20: // Run away from doctor and hide behind player
		case 21:
		e->direction = 1;
		e->x_speed = pixel_to_sub(2);
		SPR_SAFEHFLIP(e->sprite, 1);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 30: // Run in ending scene
		case 31:
		e->x_speed = pixel_to_sub(e->direction ? 2 : -2);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 40: // Jump
		e->y_speed = -0x300;
		e->grounded = false;
		break;
	}
}
