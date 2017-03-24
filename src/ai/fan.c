#include "ai_common.h"

#ifdef PAL
#define FAN_HSPEED 0x680
#else
#define FAN_HSPEED 0x550
#endif

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
		e->odir = e->dir = 1;
		break;
	case 99: // Down
		sprite_vflip(e->sprite[0], 1);
		break;
	}
}

void ai_fan(Entity *e) {
	uint16_t ex = e->x >> CSF, ey = e->y >> CSF;
	uint16_t px = player.x >> CSF, py = player.y >> CSF;
	// Script in Egg Corridor turns on fan by switching direction
	if(e->dir != e->odir) e->state = 2;
	switch(e->state) {
		case 1: // Left
		{
			ANIMATE(e, 4, 3,4,5);
			if(px > ex - (6<<4) && px < ex && py > ey - 12 && py < ey + 12) {
				player.x_speed -= SPEED(0x88);
				if(player.x_speed < -FAN_HSPEED) player.x_speed = -FAN_HSPEED;
			}
		}
		break;
		case 2: // Up
		{
			ANIMATE(e, 4, 0,1,2);
			if(py > ey - (6<<4) && py < ey && px > ex - 12 && px < ex + 12) {
				player.y_speed -= SPEED(0x88);
				if(player.y_speed < -SPEED(0x5FF)) player.y_speed = -SPEED(0x5FF);
				if(player.y_speed < 0) player.jump_time = 4;
			}
		}
		break;
		case 3: // Right
		{
			ANIMATE(e, 4, 3,4,5);
			if(px > ex && px < ex + (6<<4) && py > ey - 12 && py < ey + 12) {
				player.x_speed += SPEED(0x88);
				if(player.x_speed > FAN_HSPEED) player.x_speed = FAN_HSPEED;
			}
		}
		break;
		case 4: // Down
		{
			ANIMATE(e, 4, 0,1,2);
			if(py > ey && py < ey + (6<<4) && px > ex - 12 && px < ex + 12) {
				player.y_speed += SPEED(0x88);
				if(player.y_speed > SPEED(0x5FF)) player.y_speed = SPEED(0x5FF);
			}
		}
		break;
	}
}
