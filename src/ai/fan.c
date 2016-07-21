#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "input.h"

void ai_fan_onCreate(Entity *e) {
	u8 anim = 0;
	if(e->eflags & NPC_OPTION2) {
		e->state = e->type - 95;
		anim = 2;
	}
	switch(e->type) {
	case 96: // Left
		// Nothing
		break;
	case 97: // Up
		anim += 1;
		break;
	case 98: // Right
		e->direction = 1;
		break;
	case 99: // Down
		anim += 1;
		e->direction = 1;
		break;
	}
	e->spriteAnim = anim;
}

void ai_fan_onUpdate(Entity *e) {
	u16 ex = sub_to_block(e->x), ey = sub_to_block(e->y);
	u16 px = sub_to_block(player.x), py = sub_to_block(player.y);
	switch(e->state) {
	case 1: // Left
		if(px > ex - 6 && px <= ex && py == ey) {
			if(player.x_speed > -MAX_WALK_SPEED * 2) player.x_speed -= 0x88;
		}
		break;
	case 2: // Up
		if(py > ey - 6 && py <= ey && px == ex) {
			if(player.y_speed > -MAX_FALL_SPEED) player.y_speed -= 0x88;
		}
		break;
	case 3: // Right
		if(px >= ex && px < ex + 6 && py == ey) {
			if(player.x_speed < MAX_WALK_SPEED * 2) player.x_speed += 0x88;
		}
		break;
	case 4: // Down
		if(py >= ey && py < ey + 8 && px == ex) {
			if(player.y_speed < MAX_FALL_SPEED) player.y_speed += 0x88;
		}
		break;
	default:
		break;
	}
}
