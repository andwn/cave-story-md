#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void ai_omega_onCreate(Entity *e) {
	e->direction = 1;
	e->health = 300;
	e->attack = 5;
	e->hurtSound = 52;
	e->deathSound = 72;
	e->deathSmoke = 3;
	e->hit_box = (bounding_box) { 32, 24, 32, 32 };
	e->display_box = (bounding_box) { 40, 32, 40, 32 };
	//SYS_die("Hi I am create");
}

void ai_omega_onUpdate(Entity *e) {
	if(e->state == 20) {
		if(++e->state_time > 120) {
			ENTITY_SET_STATE(e, 21, 0);
		}
	} else {
		
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_omega_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) {
		tsc_call_event(e->event); // Boss defeated event
		e->state = STATE_DESTROY;
		bossEntity = NULL;
	}
	switch(e->state) {
		case 20:
		e->y_speed = -0x0A0;
		break;
		case 21:
		e->y_speed = 0;
		break;
	}
}

void ai_sunstone_onCreate(Entity *e) {
	e->x += pixel_to_sub(8);
	e->y += pixel_to_sub(8);
}

void ai_sunstone_onUpdate(Entity *e) {
	
}

void ai_sunstone_onState(Entity *e) {
	
}

void ai_puppy_onCreate(Entity *e) {
	e->eflags |= NPC_INTERACTIVE; // Yeah..
}

void ai_puppy_onUpdate(Entity *e) {
	
}

void ai_puppyCarry_onCreate(Entity *e) {
	e->alwaysActive = true;
	// One's all you can manage. One's all you can manage. One's all you can manage.
	e->eflags &= ~NPC_INTERACTIVE;
	e->nflags &= ~NPC_INTERACTIVE;
}

void ai_puppyCarry_onUpdate(Entity *e) {
	if(player.direction != e->direction) {
		e->direction ^= 1;
		SPR_SAFEHFLIP(e->sprite, e->direction);
	}
	e->x = player.x + pixel_to_sub(e->direction ? -4 : 4);
	e->y = player.y - pixel_to_sub(5);
}

void ai_torokoBoss_onCreate(Entity *e) {
	e->eflags |= NPC_SHOOTABLE;
}

void ai_torokoBoss_onUpdate(Entity *e) {
	
}

void ai_torokoBoss_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) {
		e->state = STATE_DELETE;
		e->eflags &= ~NPC_SHOOTABLE;
		e->nflags &= ~NPC_SHOOTABLE;
		tsc_call_event(e->event);
	}
}

void ai_jenka_onCreate(Entity *e) {
	if(e->type == OBJ_JENKA_COLLAPSED) {
		e->spriteAnim = 2;
	}
}
