#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "input.h"
#include "sprite.h"

void onspawn_fan(Entity *e) {
	if(e->eflags & NPC_OPTION2) e->state = e->type - 95;
	switch(e->type) {
	case 96: // Left
		e->frame = 3;
		break;
	case 97: // Up
		break;
	case 98: // Right
		e->frame = 3;
		e->dir = 1;
		break;
	case 99: // Down
		sprite_vflip(e->sprite[0], 1);
		break;
	}
}

void ai_fan(Entity *e) {
	u16 ex = sub_to_block(e->x), ey = sub_to_block(e->y);
	u16 px = sub_to_block(player.x), py = sub_to_block(player.y);
	switch(e->state) {
		case 1: // Left
		{
			ANIMATE(e, 8, 3,4,5);
			if(px > ex - 6 && px <= ex && py == ey) {
				if(player.x_speed > -MAX_WALK_SPEED * 2) player.x_speed -= 0x88;
			}
		}
		break;
		case 2: // Up
		{
			ANIMATE(e, 8, 0,1,2);
			if(py > ey - 8 && py <= ey && px == ex) {
				if(player.y_speed > -MAX_FALL_SPEED - MAX_FALL_SPEED / 2) {
					player.y_speed -= 0x88;
					player.jump_time = 10;
				}
			}
		}
		break;
		case 3: // Right
		{
			ANIMATE(e, 8, 3,4,5);
			if(px >= ex && px < ex + 6 && py == ey) {
				if(player.x_speed < MAX_WALK_SPEED * 2) player.x_speed += 0x88;
			}
		}
		break;
		case 4: // Down
		{
			ANIMATE(e, 8, 0,1,2);
			if(py >= ey && py < ey + 6 && px == ex) {
				if(player.y_speed < MAX_FALL_SPEED + MAX_FALL_SPEED / 2) player.y_speed += 0x88;
			}
		}
		break;
	}
}
