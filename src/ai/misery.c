#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void ai_miseryFloat_onUpdate(Entity *e) {
	switch(e->state) {
	case 14: // Standing
		break;
	case 15: // Spawning bubble
	case 16:
		sound_play(SND_BUBBLE, 5);
		entity_create(sub_to_block(e->x), sub_to_block(e->y) - 1, 0, 0, 0x42, 0, 0);
		e->state = 14;
		break;
	case 20:
	case 21:
		e->y_speed -= 0x20;
		break;
	default:
		break;
	}
	e->y += e->y_speed;
}

void ai_miseryFloat_onState(Entity *e) {
	switch(e->state) {
	case 20: // Hover, going up
	case 21:
		break;
	default:
		break;
	}
}

// https://github.com/libretro/nxengine-libretro/blob/master/nxengine/ai/npc/misery.cpp#L126

void ai_miseryBubble_onUpdate(Entity *e) {
	// find the Toroko object we are to home in on
	Entity *target = entity_find_by_type(0x3C);
	if(target == NULL) {
		e->state = STATE_DELETE;
		return;
	}
	switch(e->state) {
		case 0:
		e->x_speed = e->x > target->x ? -0x200 : 0x200;
		e->y_speed = e->y > target->y ? -0x200 : 0x200;
		e->x += e->x_speed;
		e->y += e->y_speed;
		if(e->x > target->x - 0x200 && e->y > target->y - 0x200 &&
			e->x < target->x + 0x200 && e->y < target->y + 0x200) {
			sound_play(SND_BUBBLE, 5);
			ENTITY_SET_STATE(e, 1, 0);
			e->x_speed = 0;
			e->y_speed = 0;
			//SPR_SAFEANIM(e->sprite, 1);
			SPR_SAFEANIM(target->sprite, 1);
		}
		break;
		case 1: // Carry Toroko away
		//if(e->x_speed > 0) e->x_speed -= 0x1C;
		//else if(e->x_speed < 0) e->x_speed += 0x1C;
		e->y_speed -= 0x1C;
		e->y += e->y_speed;
		target->y = e->y;
		break;
	}
}
