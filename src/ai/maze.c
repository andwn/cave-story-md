#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"
#include "camera.h"
#include "system.h"

#ifdef PAL
#define BLOCK_SOUND_INTERVAL	10
#define BLOCK_TRAVEL_ACCEL		0x20
#define BLOCK_TRAVEL_SPEED		0x200
#else
#define BLOCK_SOUND_INTERVAL	12
#define BLOCK_TRAVEL_ACCEL		0x1B
#define BLOCK_TRAVEL_SPEED		0x1B0
#endif

void ai_block_onCreate(Entity *e) {
	e->x += pixel_to_sub(8);
	e->y += pixel_to_sub(8);
	e->hit_box = (bounding_box) { 16, 16, 16, 16 };
	e->display_box = (bounding_box) { 16, 16, 16, 16 };
	e->eflags |= NPC_SOLID;
	e->eflags |= NPC_IGNORE44;
	e->enableSlopes = false;
	e->attack = 0;
	e->state = (e->eflags & NPC_OPTION2) ? 20 : 10;
}

void ai_blockh_onUpdate(Entity *e) {
	switch(e->state) {
		case 10:
		if((player.x > e->x && player.x - e->x < 0x3200) ||
			(player.x < e->x && e->x - player.x < 0x32000)) {
			if(PLAYER_DIST_Y(0x3200)) {
				e->state = 30;
				e->state_time = 0;
			}
		}
		break;
		case 20:
		if((player.x > e->x && player.x - e->x < 0x32000) ||
			(player.x < e->x && e->x - player.x < 0x3200)) {
			if(PLAYER_DIST_Y(0x3200)) {
				e->state = 30;
				e->state_time = 0;
			}
		}
		break;
		case 30:
		{
			u16 dir = e->eflags & NPC_OPTION2;
			e->x_speed += dir ? BLOCK_TRAVEL_ACCEL : -BLOCK_TRAVEL_ACCEL;
			if(e->x_speed > BLOCK_TRAVEL_SPEED) e->x_speed = BLOCK_TRAVEL_SPEED;
			if(e->x_speed < -BLOCK_TRAVEL_SPEED) e->x_speed = -BLOCK_TRAVEL_SPEED;
			e->x_next = e->x + e->x_speed;
			// hit edge
			if(stage_get_block_type(
					sub_to_block(e->x_next + 0x3200), sub_to_block(e->y) == 0x41 ||
				stage_get_block_type(
					sub_to_block(e->x_next - 0x3200), sub_to_block(e->y) == 0x41))) {
				camera_shake(10);
				e->x_speed = 0;
				e->eflags ^= NPC_OPTION2;
				e->state = dir ? 10 : 20;
			} else {
				e->x = e->x_next;
				if((++e->state_time % BLOCK_SOUND_INTERVAL) == 6) {
					sound_play(SND_BLOCK_MOVE, 2);
				}
			}
		}
		break;
	}
}

void ai_blockv_onUpdate(Entity *e) {
	switch(e->state) {
		case 10:
		if((player.y > e->y && player.y - e->y < 0x3200) ||
			(player.y < e->y && e->y - player.y < 0x32000)) {
			if(PLAYER_DIST_X(0x3200)) {
				e->state = 30;
				e->state_time = 0;
			}
		}
		break;
		case 20:
		if((player.y > e->y && player.y - e->y < 0x32000) ||
			(player.y < e->y && e->y - player.y < 0x3200)) {
			if(PLAYER_DIST_X(0x3200)) {
				e->state = 30;
				e->state_time = 0;
			}
		}
		break;
		case 30:
		e->y_speed += e->direction ? BLOCK_TRAVEL_ACCEL : -BLOCK_TRAVEL_ACCEL;
		if(e->y_speed > BLOCK_TRAVEL_SPEED) e->y_speed = BLOCK_TRAVEL_SPEED;
		if(e->y_speed < -BLOCK_TRAVEL_SPEED) e->y_speed = -BLOCK_TRAVEL_SPEED;
		e->y_next = e->y + e->y_speed;
		// hit edge
		if((e->direction && collide_stage_floor(e)) || 
			(!e->direction && collide_stage_ceiling(e))) {
			
			camera_shake(10);
			
			e->y_speed = 0;
			e->direction ^= 1;
			e->state = (e->direction) ? 20 : 10;
		}
		e->y = e->y_next;
		if((++e->state_time % BLOCK_SOUND_INTERVAL) == 6) {
			sound_play(SND_BLOCK_MOVE, 2);
		}
		break;
	}
}
