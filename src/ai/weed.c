#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"

void ai_jelly_onCreate(Entity *e) {
	e->x_next = e->x;
	e->y_next = e->y;
	e->state_time = (random() % 20) + 11;
	e->x_speed = e->direction ? -0x200 : 0x200;
}

void ai_jelly_onUpdate(Entity *e) {
	if(e->state == 0) {
		if(--e->state_time == 0) {
			SPR_SAFEFRAME(e->sprite, 0);
			ENTITY_SET_STATE(e, 1, 0);
		}
	} else if(e->state == 1) {
		if(++e->state_time > 15) {
			ENTITY_SET_STATE(e, 2, 0);
		} else if(e->state_time > 10) {
			e->x_speed += e->direction ? 0x100 : -0x100;
			e->y_speed -= 0x200;
		}
	} else {
		if(++e->state_time > 10 && e->y > e->y_next) {
			ENTITY_SET_STATE(e, 1, 0);
		}
	}
	e->direction = e->x < e->x_next;
	e->y_speed += 0x20;
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_jelly_onHurt(Entity *e) {
	e->x_speed /= 2;
	e->y_speed /= 2;
}

void ai_mannan_onUpdate(Entity *e) {
	if(e->state == 1 && ++e->state_time > 24) {
		ENTITY_SET_STATE(e, 0, 0);
	}
}

void ai_mannan_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) {
		sound_play(0x1D, 5);
		effect_create_smoke(0, sub_to_pixel(e->x), sub_to_pixel(e->y));
		entity_drop_powerup(e);
		// Face sprite remains after defeated
		e->eflags &= ~NPC_SHOOTABLE;
		e->nflags &= ~NPC_SHOOTABLE;
		SPR_SAFEANIM(e->sprite, 2);
		e->attack = 0;
		e->state = 3;
		return;
	}
	switch(e->state) {
		case 0:
		SPR_SAFEANIM(e->sprite, 0);
		break;
		case 1: {
			SPR_SAFEANIM(e->sprite, 1);
			Entity *shot = entity_create(sub_to_block(e->x), sub_to_block(e->y), 
				0, 0, 0x67, 0, e->direction);
			shot->direction = e->direction;
		}
		break;
	}
}

void ai_mannan_onHurt(Entity *e) {
	// Fire projectile
	if(e->state == 0) ENTITY_SET_STATE(e, 1, 0);
}

void ai_mannanShot_onUpdate(Entity *e) {
	e->x_speed += e->direction ? 0x1D : -0x1D;
	if((e->state_time % 8) == 1) {
		//sound_play(SND_IRONH_SHOT_FLY, 2);
	}
	if(++e->state_time > 120) e->state = STATE_DELETE;
}

void ai_malco_onUpdate(Entity *e) {
	switch(e->state) {
		case 10:
		if(++e->state_time < 100) {
			if((e->state_time % 4) == 0) {
				//sound_play(SND_COMPUTER_BEEP);
			}
		} else if(e->state_time > 150) {
			ENTITY_SET_STATE(e, 15, 0);
		}
		break;
		case 15:		// shaking
		if(e->state_time % 4 == 0) {
			e->x += sub_to_pixel(1);
			//sound_play(SOUND_DOOR, 5);
		} else if(e->state_time % 4 == 2) {
			e->x -= sub_to_pixel(1);
		}
		if(++e->state_time > 50) ENTITY_SET_STATE(e, 16, 0);
		break;
		case 16:		// stand up
		if(++e->state_time > 150) ENTITY_SET_STATE(e, 18, 0);
		break;
		case 18:		// gawking/bobbing up and down
		if(++e->state_time % 8 == 0) sound_play(0x0B, 5);
		if(e->state_time > 100) ENTITY_SET_STATE(e, 20, 0);
		break;
	}
}

void ai_malco_onState(Entity *e) {
	switch(e->state) {
		case 0:
		SPR_SAFEANIM(e->sprite, 0);
		break;
		case 10:
		effect_create_smoke(0, sub_to_pixel(e->x), sub_to_pixel(e->y));
		break;
		case 16:
		SPR_SAFEANIM(e->sprite, 2);
		sound_play(SOUND_BREAK, 5);
		effect_create_smoke(0, sub_to_pixel(e->x), sub_to_pixel(e->y));
		break;
		case 18:
		SPR_SAFEANIM(e->sprite, 3);
		break;
		case 20: 
		SPR_SAFEANIM(e->sprite, 4);
		effect_create_smoke(0, sub_to_pixel(e->x), sub_to_pixel(e->y));
		break;
		case 21:	// got smushed!
		SPR_SAFEANIM(e->sprite, 5);
		//sound_play(SND_ENEMY_HURT);
		break;
	}
}

void ai_malcoBroken_onState(Entity *e) {
	switch(e->state) {
		case 0:
		SPR_SAFEANIM(e->sprite, 0);
		FACE_PLAYER(e);
		break;
		case 10:	// set when pulled out of ground
		sound_play(SOUND_BREAK, 5);
		effect_create_smoke(0, sub_to_pixel(e->x), sub_to_pixel(e->y));
		e->state = 0;
		break;
	}
}
