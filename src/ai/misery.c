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
		if(e->y < -0x1000) e->state = STATE_DELETE;
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
		{
			// Wait a bit
			if(++e->state_time > TIME(25)) e->state = 1;
		}
		break;
		case 1:
		{
			// Calculate the speed it will take to reach the target in 1.5 seconds
			// Genesis can't divide 32 bit integers so here is a fun hack have fun deciphering it
			e->x_speed = (((s32)((u16)(abs(target->x - e->x) >> 5)) / TIME(75))) << 5;
			e->y_speed = (((s32)((u16)(abs(target->y - e->y) >> 5)) / TIME(75))) << 5;
			if(e->x > target->x) e->x_speed = -e->x_speed;
			if(e->y > target->y) e->y_speed = -e->y_speed;
			e->state = 2;
			e->state_time = 0;
		}
		/* no break */
		case 2:
		{
			e->x += e->x_speed;
			e->y += e->y_speed;
			// Did we reach the target?
			if(++e->state_time == TIME(75)) {
				sound_play(SND_BUBBLE, 5);
				e->state = 3;
				e->x = target->x;
				e->y = target->y;
				e->x_speed = 0;
				e->y_speed = 0;
				target->eflags |= NPC_IGNORESOLID;
				SPR_SAFEANIM(target->sprite, 1);
			}
		}
		break;
		case 3: // Carry Toroko away
		{
			e->y_speed -= SPEED(0x1C);
			e->y += e->y_speed;
			target->x = e->x + 0x200;
			target->y = e->y - 0x200;
		}
		break;
	}
}
