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
				e->x_speed = e->direction ? 0x100 : -0x100;
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
		if(e->x_speed != 0) e->grounded = collide_stage_floor_grounded(e);
	} else {
		e->grounded = collide_stage_floor(e);
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_pignon_onHurt(Entity *e) {
	ENTITY_SET_STATE(e, 3, 0);
	e->y_speed = -0x100;
	e->x_speed = -0x150 + 0x300 * !e->direction; // Knock backwards
	SPR_SAFEANIM(e->sprite, 3);
}

void ai_gkeeper_onCreate(Entity *e) {
	e->eflags |= NPC_SHOOTABLE;
	e->eflags |= NPC_INVINCIBLE;
	e->attack = 0;
}

void ai_gkeeper_onUpdate(Entity *e) {
	switch(e->state) {
		case 0: // Standing
		FACE_PLAYER(e);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		// start walking when player comes near
		if(PLAYER_DIST_X(pixel_to_sub(128)) && 
			PLAYER_DIST_Y2(pixel_to_sub(48), pixel_to_sub(32))) {
			e->state = 1;
			SPR_SAFEANIM(e->sprite, 1);
		}
		// start walking if shot
		if(e->damage_time > 0) {
			e->state = 1;
			SPR_SAFEANIM(e->sprite, 1);
			e->eflags |= NPC_INVINCIBLE;
		}
		break;
		case 1: // Walking
		FACE_PLAYER(e);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		e->x_speed = e->direction ? 0x100 : -0x100;
		// reached knife range of player?
		if(PLAYER_DIST_X(pixel_to_sub(10))) {
			e->state = 2;
			e->state_time = 0;
			e->x_speed = 0;
			sound_play(SND_FIREBALL, 5);
			SPR_SAFEANIM(e->sprite, 2);
			e->eflags &= ~NPC_INVINCIBLE;
		}
		break;
		case 2: // Knife raised
		if(++e->state_time > 40) {
			e->state = 3;
			e->state_time = 0;
			e->attack = 10;
			sound_play(SND_SLASH, 5);
			SPR_SAFEANIM(e->sprite, 3);
		}
		break;
		case 3: // Knife frame 2
		if(++e->state_time > 2) {
			e->state = 4;
			e->state_time = 0;
			SPR_SAFEANIM(e->sprite, 4);
		}
		break;
		case 4: // Knife frame 3
		if(++e->state_time > 60) {
			e->state = 0;
			SPR_SAFEANIM(e->sprite, 0);
			e->eflags |= NPC_INVINCIBLE;
			e->attack = 0;
		}
		break;
	}
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
		if(e->x_speed != 0) e->grounded = collide_stage_floor_grounded(e);
	} else {
		e->grounded = collide_stage_floor(e);
	}
	e->x = e->x_next;
	e->y = e->y_next;
}
