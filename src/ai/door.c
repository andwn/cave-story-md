#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

// When the door's direction is changed to be facing right it becomes
// transparent. The CNP command does this when Balrog barges into Barr
void ai_door_onCreate(Entity *e) {
	if(e->direction) e->spriteAnim = SPRITE_DISABLE;
}

void ai_door_onUpdate(Entity *e) {
	if(e->direction) {
		SPR_SAFERELEASE(e->sprite);
	} else if(e->sprite == NULL) {
		entity_sprite_create(e);
	}
}

void ai_theDoor_onUpdate(Entity *e) {
	if(e->state == 1) {
		e->state_time--;
		if(e->state_time == 0) e->state = 0;
	} else {
		s32 x1 = e->x - block_to_sub(6), y1 = e->y - block_to_sub(6),
			x2 = e->x + block_to_sub(6), y2 = e->y + block_to_sub(6);
		if(player.x > x1 && player.x < x2 && player.y > y1 && player.y < y2) {
			SPR_SAFEFRAME(e->sprite, 2);
		} else {
			SPR_SAFEFRAME(e->sprite, 0);
		}
	}
}

// The door is in pain. You're the real monster
void ai_theDoor_onHurt(Entity *e) {
	ENTITY_SET_STATE(e, 1, 30);
	SPR_SAFEFRAME(e->sprite, 3);
}
