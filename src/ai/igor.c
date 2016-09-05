#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "input.h"
#include "effect.h"

#define fireatk curly_target_x

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
	fireatk = 0;
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
				if(e->x <= player.x + pixel_to_sub(22)) ENTITY_SET_STATE(e, STATE_PUNCH, 0);
			} else {
				if(e->x >= player.x - pixel_to_sub(22)) ENTITY_SET_STATE(e, STATE_PUNCH, 0);
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
				e->hit_box.left -= 10;
			} else {
				e->hit_box.right -= 10;
			}
			ENTITY_SET_STATE(e, STATE_STAND, 0);
		}
		break;
		case STATE_JUMPING:
		if(e->grounded) {
			sound_play(SND_ENEMY_JUMP, 5);
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
				sound_play(SND_BLOCK_DESTROY, 5);
				Entity *shot = entity_create(sub_to_block(e->x) + (e->direction ? 1 : -1),
					sub_to_block(e->y), 0, 0, 0x0B, 0, e->direction);
				shot->x_speed = 0x4A0 * (e->direction ? 1 : -1);
				shot->y_speed = 0x100 - (random() % 0x300);
			}
			// fires 6 shots
			if(e->state_time > 150) ENTITY_SET_STATE(e, STATE_STAND, 0);
		}
		break;
	}
	if(!e->grounded) e->y_speed += GRAVITY_JUMP;
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
		}
		/* no break */
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
		sound_play(SND_EXPL_SMALL, 5);
		// sprite appears identical, but has a wider bounding box.
		if(e->direction == 0) {
			e->hit_box.left += 10;
		} else {
			e->hit_box.right += 10;
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
		e->x_speed = 0;
		tsc_call_event(e->event); // Boss defeated event
		break;
	}
}

void ai_igorscene_onUpdate(Entity *e) {
	if(e->state >= 4) {
		e->state_time--;
		if(e->state_time == 0) ENTITY_SET_STATE(e, e->state == 4 ? 5 : 0, 0);
	}
	if(!e->grounded) e->y_speed += SPEED(0x40);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
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
		{
			e->x_speed = 0;
			SPR_SAFEHFLIP(e->sprite, e->direction);
			SPR_SAFEANIM(e->sprite, 0);
		}
		break;
		case 2:
		case 3:
		{
			MOVE_X(SPEED(0x200));
			SPR_SAFEANIM(e->sprite, 1);
		}
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

void ai_igordead_onUpdate(Entity *e) {
	switch(e->state) {
		case 0:
		FACE_PLAYER(e);
		//sound(SND_BIG_CRASH);
		//SmokeBoomUp(e);
		e->x_speed = 0;
		e->state_time = 0;
		e->state = 1;
		break;
		case 1:
		// Puffs of smoke
		if((++e->state_time % 10) == 1) {
			effect_create_smoke(0, sub_to_pixel(e->x) - 24 + (random() % 48), 
				sub_to_pixel(e->y) - 32 + (random() % 64));
		}
		// Shake
		if((e->state_time & 3) == 1) {
			e->display_box.left -= 1;
		} else if((e->state_time & 3) == 3) {
			e->display_box.left += 1;
		}
		if(e->state_time > 100) {
			e->state_time = 0;
			e->state = 2;
		}
		break;
		case 2:
		// Slower smoke puff
		if((++e->state_time & 15) == 0) {
			effect_create_smoke(0, sub_to_pixel(e->x) - 24 + (random() % 48), 
				sub_to_pixel(e->y) - 32 + (random() % 64));
		}
		// alternate between big and small sprites
		// (frenzied/not-frenzied forms)
		if((e->state_time & 3) == 1) {
			SPR_SAFEANIM(e->sprite, 9);
		} else if((e->state_time & 3) == 3) {
			SPR_SAFEANIM(e->sprite, 7);
		}
		if(e->state_time > 160) {
			SPR_SAFEANIM(e->sprite, 9);
			e->state = 3;
			e->state_time = 0;
		}
		break;
		case 3:
		if(++e->state_time > 60) {
			if(e->sprite->animInd >= 11) {
				SPR_SAFEVISIBILITY(e->sprite, HIDDEN);
				e->state = 4;
			} else {
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, e->sprite->animInd + 1);
			}
		}
		//if((e->timer % 24) == 0)
		//	smoke_puff(o, false);
		break;
		case 4: break;
	}
}
