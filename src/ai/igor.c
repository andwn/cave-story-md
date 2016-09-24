#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "input.h"
#include "effect.h"

// Rewrite this whole file tbh

/*
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
*/
void onspawn_igor(Entity *e) {
	/*
	fireatk = 0;
	e->attack = 0;
	e->hit_box.bottom += 4;
	e->hit_box.top -= 4;
	if(e->type == 0x59) e->frame = 7;;
	* */
}

void ai_igor(Entity *e) {
	/*
	switch(e->state) {
		case STATE_STAND:
		if(++e->timer > 60) ENTITY_SET_STATE(e, STATE_BEGIN_ATTACK, 0);
		break;
		case STATE_WALK:
		if(fireatk == -1) {	// begin mouth-blast attack
			if(++e->timer > 20) ENTITY_SET_STATE(e, STATE_MOUTH_BLAST, 0);
		} else {
			if(e->dir == 0) {
				if(e->x <= player.x + pixel_to_sub(22)) ENTITY_SET_STATE(e, STATE_PUNCH, 0);
			} else {
				if(e->x >= player.x - pixel_to_sub(22)) ENTITY_SET_STATE(e, STATE_PUNCH, 0);
			}
			// if we don't reach him after a while, do a jump
			if(++e->timer > 60) ENTITY_SET_STATE(e, STATE_JUMPING, 0);
		}
		break;
		case STATE_PUNCH:
		if(++e->timer > 16) ENTITY_SET_STATE(e, STATE_PUNCH_2, 0);
		break;
		case STATE_PUNCH_2:
		if(++e->timer > 12) {
			// return to normal-size bounding box
			if(e->dir == 0) {
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
			effect_create_smoke(sub_to_pixel(e->x), sub_to_pixel(e->y) + e->hit_box.bottom);
			ENTITY_SET_STATE(e, STATE_LANDED, 0);
		}
		break;
		case STATE_LANDED:
		if(++e->timer > 12) ENTITY_SET_STATE(e, STATE_STAND, 0);
		break;
		case STATE_MOUTH_BLAST:
		e->timer++;
		// flash mouth
		if(e->timer > 60 && (e->timer & 4)) {
			if(e->sprite->animInd != 8) e->frame = 8;
		} else {
			if(e->sprite->animInd != 4) e->frame = 4;
		}
		// fire shots
		if(e->timer > 120) {
			if((e->timer % 8) == 1) {
				sound_play(SND_BLOCK_DESTROY, 5);
				Entity *shot = entity_create(sub_to_block(e->x) + (e->dir ? 1 : -1),
					sub_to_block(e->y), 0, 0, 0x0B, 0, e->dir);
				shot->x_speed = 0x4A0 * (e->dir ? 1 : -1);
				shot->y_speed = 0x100 - (random() % 0x300);
			}
			// fires 6 shots
			if(e->timer > 150) ENTITY_SET_STATE(e, STATE_STAND, 0);
		}
		break;
	}
	if(!e->grounded) e->y_speed += GRAVITY_JUMP;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
	* */
}

void ondeath_igor(Entity *e) {
	e->attack = 0;
	e->eflags &= ~(NPC_SHOOTABLE|NPC_SHOWDAMAGE);
	e->x_speed = 0;
	tsc_call_event(e->event); // Boss defeated event
	/*
	switch(e->state) {
		case STATE_STAND:
		//SPR_SAFEHFLIP(e->sprite, e->dir);
		e->frame = 0;
		e->attack = 0;
		break;
		case STATE_BEGIN_ATTACK:
		e->state = STATE_WALK;
		FACE_PLAYER(e);
		// when health is less than halfway, then use
		// the mouth blast attack every third time.
		if(++fireatk >= 3 && e->health <= npc_health(e->type) / 2) {
			fireatk = -1;
			e->dir ^= 1;	// walk away from player
		}
		
		case STATE_WALK:
		//SPR_SAFEHFLIP(e->sprite, e->dir);
		e->frame = 1;
		e->x_speed = pixel_to_sub(e->dir ? 1 : -1);
		break;
		case STATE_PUNCH:
		e->frame = 2;
		e->x_speed = 0;
		break;
		case STATE_PUNCH_2:
		sound_play(SND_EXPL_SMALL, 5);
		// sprite appears identical, but has a wider bounding box.
		if(e->dir == 0) {
			e->hit_box.left += 10;
		} else {
			e->hit_box.right += 10;
		}
		e->frame = 3;
		e->attack = 5;
		break;
		case STATE_JUMPING:
		e->frame = 5;
		e->y_speed = -0x400;
		e->attack = 2;
		e->x_speed *= 2;
		e->x_speed /= 3;
		break;
		case STATE_LANDED:
		e->frame = 6;
		e->x_speed = 0;
		break;
		case STATE_MOUTH_BLAST:
		FACE_PLAYER(e);
		//SPR_SAFEHFLIP(e->sprite, e->dir);
		e->frame = 4;
		e->x_speed = 0;
		break;
		case STATE_DEFEATED:
		e->attack = 0;
		e->x_speed = 0;
		tsc_call_event(e->event); // Boss defeated event
		break;
	}
	* */
}

void ai_igorscene(Entity *e) {
	/*
	if(e->state >= 4) {
		e->timer--;
		if(e->timer == 0) ENTITY_SET_STATE(e, e->state == 4 ? 5 : 0, 0);
	}
	if(!e->grounded) e->y_speed += SPEED(0x40);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	e->x = e->x_next;
	e->y = e->y_next;
	* */
}

void ondeath_igorscene(Entity *e) {
	/*
	switch(e->state) {
		case 0:
		case 1:
		{
			e->x_speed = 0;
			//SPR_SAFEHFLIP(e->sprite, e->dir);
			e->frame = 0;
		}
		break;
		case 2:
		case 3:
		{
			MOVE_X(SPEED(0x200));
			e->frame = 1;
		}
		break;
		case 4:
		e->x_speed = 0;
		//SPR_SAFEHFLIP(e->sprite, e->dir);
		e->frame = 2;
		e->timer = 20;
		break;
		case 5:
		e->x_speed = 0;
		//SPR_SAFEHFLIP(e->sprite, e->dir);
		e->frame = 3;
		e->timer = 20;
		break;
	}
	* */
}

void ai_igordead(Entity *e) {
	/*
	switch(e->state) {
		case 0:
		FACE_PLAYER(e);
		//sound(SND_BIG_CRASH);
		//SmokeBoomUp(e);
		e->x_speed = 0;
		e->timer = 0;
		e->state = 1;
		break;
		case 1:
		// Puffs of smoke
		if((++e->timer % 10) == 1) {
			effect_create_smoke(sub_to_pixel(e->x) - 24 + (random() % 48), 
				sub_to_pixel(e->y) - 32 + (random() % 64));
		}
		// Shake
		if((e->timer & 3) == 1) {
			e->display_box.left -= 1;
		} else if((e->timer & 3) == 3) {
			e->display_box.left += 1;
		}
		if(e->timer > 100) {
			e->timer = 0;
			e->state = 2;
		}
		break;
		case 2:
		// Slower smoke puff
		if((++e->timer & 15) == 0) {
			effect_create_smoke(sub_to_pixel(e->x) - 24 + (random() % 48), 
				sub_to_pixel(e->y) - 32 + (random() % 64));
		}
		// alternate between big and small sprites
		// (frenzied/not-frenzied forms)
		if((e->timer & 3) == 1) {
			e->frame = 9;
		} else if((e->timer & 3) == 3) {
			e->frame = 7;
		}
		if(e->timer > 160) {
			e->frame = 9;
			e->state = 3;
			e->timer = 0;
		}
		break;
		case 3:
		if(++e->timer > 60) {
			if(e->sprite->animInd >= 11) {
				//SPR_SAFEVISIBILITY(e->sprite, HIDDEN);
				e->state = 4;
			} else {
				e->timer = 0;
				e->frame = e->sprite->animInd + 1;
			}
		}
		//if((e->timer % 24) == 0)
		//	smoke_puff(o, FALSE);
		break;
		case 4: break;
	}
	* */
}
