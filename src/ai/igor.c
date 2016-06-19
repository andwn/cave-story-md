#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "input.h"
#include "effect.h"

// I copied stuff from NXEngine here

s8 fireatk = 0;

enum {
	STATE_STAND = 0,
	STATE_BEGIN_ATTACK,
	STATE_WALK,
	STATE_JUMPING,
	STATE_LANDED,
	STATE_PUNCH,
	STATE_PUNCH_2,
	STATE_MOUTH_BLAST
};

void ai_igor_onCreate(Entity *e) {
	e->attack = 0;
	e->hit_box.bottom += 4;
	e->hit_box.top -= 4;
	if(e->type == 0x59) e->spriteAnim = 7;
}

void ai_igor_onUpdate(Entity *e) {
	switch(e->state) {
		case STATE_STAND:
		if(++e->state_time > 60) ENTITY_SET_STATE(e, STATE_BEGIN_ATTACK, 0);
		break;
		case STATE_WALK:
		if(fireatk == -1) {	// begin mouth-blast attack
			if(++e->state_time > 20) ENTITY_SET_STATE(e, STATE_MOUTH_BLAST, 0);
		} else {
			if(e->direction == 0) {
				if(e->x <= player.x + pixel_to_sub(24)) ENTITY_SET_STATE(e, STATE_PUNCH, 0);
			} else {
				if(e->x >= player.x - pixel_to_sub(24)) ENTITY_SET_STATE(e, STATE_PUNCH, 0);
			}
			// if we don't reach him after a while, do a jump
			if(++e->state_time > 60) ENTITY_SET_STATE(e, STATE_JUMPING, 0);
		}
		break;
		case STATE_PUNCH:
		if(++e->state_time > 16) ENTITY_SET_STATE(e, STATE_PUNCH_2, 0);
		break;
		case STATE_PUNCH_2:
		if(++e->state_time > 12) {
			// return to normal-size bounding box
			if(e->direction == 0) {
				e->hit_box.left -= 8;
			} else {
				e->hit_box.right -= 8;
			}
			ENTITY_SET_STATE(e, STATE_STAND, 0);
		}
		break;
		case STATE_JUMPING:
		if(e->grounded) {
			sound_play(0x1A, 5);
			effect_create_smoke(0, sub_to_pixel(e->x), sub_to_pixel(e->y) + e->hit_box.bottom);
			ENTITY_SET_STATE(e, STATE_LANDED, 0);
		}
		break;
		case STATE_LANDED:
		if(++e->state_time > 12) ENTITY_SET_STATE(e, STATE_STAND, 0);
		break;
		case STATE_MOUTH_BLAST:
		e->state_time++;
		// flash mouth
		if(e->state_time > 60 && (e->state_time & 4)) {
			if(e->sprite->animInd != 8) SPR_SAFEANIM(e->sprite, 8);
		} else {
			if(e->sprite->animInd != 4) SPR_SAFEANIM(e->sprite, 4);
		}
		// fire shots
		if(e->state_time > 120) {
			if((e->state_time % 8) == 1) {
				sound_play(SOUND_BREAK, 5);
				//Entity *shot = entity_create(e->x + block_to_sub(e->direction ? 2 : -2),
				//	e->y, 0, 0, ??, 0, e->direction);
				//int angle = (e->dir == LEFT) ? 136 : 248;
				//angle += random(-16, 16);
				//ThrowObjectAtAngle(shot, angle, 0x580);
			}
			// fires 6 shots
			if(e->state_time > 150) ENTITY_SET_STATE(e, STATE_STAND, 0);
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

void ai_igor_onState(Entity *e) {
	switch(e->state) {
		case STATE_STAND:
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 0);
		e->attack = 0;
		break;
		case STATE_BEGIN_ATTACK:
		e->state = STATE_WALK;
		FACE_PLAYER(e);
		// when health is less than halfway, then use
		// the mouth blast attack every third time.
		if(++fireatk >= 3 && e->health <= npc_health(e->type) / 2) {
			fireatk = -1;
			e->direction ^= 1;	// walk away from player
		} // fall thru
		case STATE_WALK:
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		break;
		case STATE_PUNCH:
		SPR_SAFEANIM(e->sprite, 2);
		e->x_speed = 0;
		break;
		case STATE_PUNCH_2:
		sound_play(0x46, 5);
		// sprite appears identical, but has a wider bounding box.
		if(e->direction == 0) {
			e->hit_box.left += 8;
		} else {
			e->hit_box.right += 8;
		}
		SPR_SAFEANIM(e->sprite, 3);
		e->attack = 5;
		break;
		case STATE_JUMPING:
		SPR_SAFEANIM(e->sprite, 5);
		e->y_speed = -0x400;
		e->attack = 2;
		e->x_speed *= 2;
		e->x_speed /= 3;
		break;
		case STATE_LANDED:
		SPR_SAFEANIM(e->sprite, 6);
		e->x_speed = 0;
		break;
		case STATE_MOUTH_BLAST:
		FACE_PLAYER(e);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 4);
		e->x_speed = 0;
		break;
		case STATE_DEFEATED:
		e->attack = 0;
		tsc_call_event(e->event); // Boss defeated event
		break;
	}
}

void ai_igorscene_onUpdate(Entity *e) {
	if(e->state >= 4) {
		e->state_time--;
		if(e->state_time == 0) ENTITY_SET_STATE(e, e->state == 4 ? 5 : 0, 0);
	}
	if(!e->grounded) e->y_speed += gravity;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

/*
 * 0000	Standing
 * 0001	Standing
 * 0002	Walk
 * 0003	Walk
 * 0004	Bend arm and Jab
 * 0005	Quick Jab
 */
void ai_igorscene_onState(Entity *e) {
	switch(e->state) {
		case 0:
		case 1:
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 0);
		break;
		case 2:
		case 3:
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 4:
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 2);
		e->state_time = 20;
		break;
		case 5:
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 3);
		e->state_time = 20;
		break;
	}
}
