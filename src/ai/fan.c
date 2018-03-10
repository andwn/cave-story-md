#include "ai_common.h"

#define FAN_HSPEED (pal_mode ? 0x5FF : 0x550)

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
	// Fans in Last Cave turn on when player touches them (they use NPC_OPTION1)
	if(e->eflags & NPC_OPTION1) {
		if(PLAYER_DIST_X(8 << CSF) && PLAYER_DIST_Y(18 << CSF)) e->state = 2;
	}
	switch(e->state) {
		case 1: // Left
		{
			if((++e->animtime & 3) == 0 && ++e->frame > 5) {
				e->frame = 3;
				effect_create_misc(EFF_FANL, ex - 8, ey + ((random() & 15) - 8));
			}
			if(px > ex - (6<<4) && px < ex && py > ey - 12 && py < ey + 12) {
				player.x_speed -= SPEED_8(0x88);
				if(player.x_speed < -FAN_HSPEED) player.x_speed = -FAN_HSPEED;
			}
		}
		break;
		case 2: // Up
		{
			if((++e->animtime & 3) == 0 && ++e->frame > 2) {
				e->frame = 0;
				effect_create_misc(EFF_FANU, ex + ((random() & 15) - 8), ey - 8);
			}
			if(py > ey - (6<<4) && py < ey && px > ex - 12 && px < ex + 12) {
				player.y_speed -= SPEED_8(0x88);
				if(player.y_speed < -SPEED(0x5FF)) player.y_speed = -SPEED(0x5FF);
				if(player.y_speed < 0) player.jump_time = 4;
			}
		}
		break;
		case 3: // Right
		{
			if((++e->animtime & 3) == 0 && ++e->frame > 5) {
				e->frame = 3;
				effect_create_misc(EFF_FANR, ex + 8, ey + ((random() & 15) - 8));
			}
			if(px > ex && px < ex + (6<<4) && py > ey - 12 && py < ey + 12) {
				player.x_speed += SPEED_8(0x88);
				if(player.x_speed > FAN_HSPEED) player.x_speed = FAN_HSPEED;
			}
		}
		break;
		case 4: // Down
		{
			if((++e->animtime & 3) == 0 && ++e->frame > 2) {
				e->frame = 0;
				effect_create_misc(EFF_FAND, ex + ((random() & 15) - 8), ey + 8);
			}
			if(py > ey && py < ey + (6<<4) && px > ex - 12 && px < ex + 12) {
				player.y_speed += SPEED_8(0x88);
				if(player.y_speed > SPEED(0x5FF)) player.y_speed = SPEED(0x5FF);
			}
		}
		break;
	}
}
