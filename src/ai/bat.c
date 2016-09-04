#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "resources.h"
#include "sheet.h"

#ifdef PAL
#define BAT_FALL_ACCEL		0x20
#define BAT_FALL_SPEED		0x5FF
#define BAT_FALL_TIME		20
#define BAT_WAIT_TIME		100
#define BAT_BLINK_TIME		8
#define BAT_BOUNCE_SPEED	0x200
#define BAT_FLY_SPEED		0x200
#define BAT_FLY_XACCEL		0x20
#define BAT_FLY_YACCEL		0x10
#else
#define BAT_FALL_ACCEL		0x1B
#define BAT_FALL_SPEED		0x4FF
#define BAT_FALL_TIME		24
#define BAT_WAIT_TIME		120
#define BAT_BLINK_TIME		10
#define BAT_BOUNCE_SPEED	0x1B0
#define BAT_FLY_SPEED		0x1B0
#define BAT_FLY_XACCEL		0x1B
#define BAT_FLY_YACCEL		0xD
#endif

// The range is a bit too high so here is my lazy way to fix it
void ai_batVertical_onCreate(Entity *e) {
	e->y += pixel_to_sub(24);
}

// Just up and down gotta go up and down
void ai_batVertical_onUpdate(Entity *e) {
	if(e->sprite == NULL) {
		SPRITE_FROM_SHEET(&SPR_Bat, SHEET_BAT);
		e->state_time2 = 0;
	} else if(++e->state_time2 > 4) {
		if(++e->spriteFrame >= 3) e->spriteFrame = 0;
		SPR_SAFETILEINDEX(e->sprite, sheets[e->spriteAnim].index + e->spriteFrame * 4);
		e->state_time2 = 0;
	}
	if(e->state == 0) {
		e->y_speed -= 8;
		if(e->y_speed <= pixel_to_sub(-1)) e->state = 1;
	} else if(e->state == 1) {
		e->y_speed += 8;
		if(e->y_speed >= pixel_to_sub(1)) e->state = 0;
	}
	FACE_PLAYER(e);
	e->y += e->y_speed;
}

void ai_batHang_onCreate(Entity *e) {
	//e->spriteAnim = 1; // Hanging anim
}

void ai_batHang_onUpdate(Entity *e) {
	if(e->sprite == NULL) {
		SPRITE_FROM_SHEET(&SPR_Bat, SHEET_BAT);
		if(!e->state) e->spriteFrame = 3;
		e->state_time2 = 0;
	}
	if(e->state == 0) { // Hanging and waiting
		if(random() % BAT_WAIT_TIME == 0) {
			e->state = 1;
			e->state_time = 0;
			e->spriteFrame = 4;
			SPR_SAFETILEINDEX(e->sprite, sheets[e->spriteAnim].index + e->spriteFrame * 4);
		}
		if(player.x > e->x - 0x1000 && player.x < e->x + 0x1000 && 
			player.y > e->y - 0x1000 && player.y < e->y + 0x9000) {
			e->state = 2;
			e->state_time = 0;
		}
	} else if(e->state == 1) { // Blinking
		if(++e->state_time > BAT_BLINK_TIME) {
			e->state = 0;
			e->state_time = 0;
			e->spriteFrame = 3;
			SPR_SAFETILEINDEX(e->sprite, sheets[e->spriteAnim].index + e->spriteFrame * 4);
		}
	} else if(e->state == 2) { // At attention
		if(e->damage_time > 0 || (player.x > e->x - 0x2800 && player.x < e->x + 0x2800)) {
			e->state = 3;
			e->state_time = 0;
			e->spriteFrame = 5;
			SPR_SAFETILEINDEX(e->sprite, sheets[e->spriteAnim].index + e->spriteFrame * 4);
		}
	} else if(e->state == 3) { // Falling
		e->y_speed += BAT_FALL_ACCEL;
		if(e->y_speed > BAT_FALL_SPEED) e->y_speed = BAT_FALL_SPEED;
		
		e->state_time++;
		e->x_next = e->x; // x_next needs to be set for collision to work properly
		e->y_next = e->y + e->y_speed;
		bool collided = collide_stage_floor(e);
		if(collided || (e->state_time > BAT_FALL_TIME && player.y - 0x2000 < e->y)) {
			e->state = 4;
			e->state_time = 0;
			e->y_mark = e->y;
			//SPR_SAFEANIM(e->sprite, 0);
			if(collided) e->y_speed = -BAT_BOUNCE_SPEED;
		} else {
			e->y = e->y_next;
		}
	} else { // Flying
		if(++e->state_time2 > 4) {
			if(++e->spriteFrame >= 3) e->spriteFrame = 0;
			SPR_SAFETILEINDEX(e->sprite, sheets[e->spriteAnim].index + e->spriteFrame * 4);
			e->state_time2 = 0;
		}
		if((e->direction && player.x < e->x) || (!e->direction && player.x > e->x)) {
			FACE_PLAYER(e);
		}
		e->x_speed += (e->x > player.x) ? -BAT_FLY_XACCEL : BAT_FLY_XACCEL;
		e->y_speed += (e->y > e->y_mark) ? -BAT_FLY_YACCEL : BAT_FLY_YACCEL;
		// Limit speed
		if(e->x_speed > BAT_FLY_SPEED) e->x_speed = BAT_FLY_SPEED;
		if(e->y_speed > BAT_FLY_SPEED) e->y_speed = BAT_FLY_SPEED;
		if(e->x_speed < -BAT_FLY_SPEED) e->x_speed = -BAT_FLY_SPEED;
		if(e->y_speed < -BAT_FLY_SPEED) e->y_speed = -BAT_FLY_SPEED;
		e->x_next = e->x + e->x_speed;
		e->y_next = e->y + e->y_speed;
		// Bounce against floor and walls
		if(e->x_speed < 0) {
			collide_stage_leftwall(e);
		} else if(e->x_speed > 0) {
			collide_stage_rightwall(e);
		}
		if(collide_stage_floor(e)) {
			e->y_speed = -BAT_BOUNCE_SPEED;
		}
		if(e->y_speed <= 0) {
			collide_stage_ceiling(e);
		}
		e->x = e->x_next;
		e->y = e->y_next;
	}
}

void ai_batCircle_onUpdate(Entity *e) {
	if(e->sprite == NULL) {
		SPRITE_FROM_SHEET(&SPR_Bat, SHEET_BAT);
		e->state_time2 = 0;
	} else if(e->state == 1 && ++e->state_time2 > 4) {
		if(++e->spriteFrame >= 3) e->spriteFrame = 0;
		SPR_SAFETILEINDEX(e->sprite, sheets[e->spriteAnim].index + e->spriteFrame * 4);
		e->state_time2 = 0;
	}
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		{
			// set up initial direction and target x,y
			// Angles are between 0-1024 where 1024 is 360 degrees
			u16 angle = random() % 1024;
			// SGDK has a sine table already available, but it is meant to be used with
			// its own 'fix32' type which is a 10-bit fixed point number. Cave Story's fixed
			// point numbers use a 9-bit decimal so we shift right by 1 bit
			e->x_speed = sintab32[angle] >> 1;
			angle = (angle + 256) % 1024; // Add 90 degrees to get cosine
			e->x_mark = e->x + (sintab32[angle] >> 1) * 8;
			// Starting Y speed
			angle = random() % 1024;
			e->y_speed = sintab32[angle] >> 1;
			// Target Y position
			angle = (angle + 256) % 1024;
			e->y_mark = e->y + (sintab32[angle] >> 1) * 8;
			
			e->state = 1;
		}
		/* no break */
		case 1:
			// circle around our target point
			if(e->direction && player.x < e->x) TURN_AROUND(e);
			if(!e->direction && player.x > e->x) TURN_AROUND(e);
			e->x_speed += (e->x > e->x_mark) ? -0x10 : 0x10;
			e->y_speed += (e->y > e->y_mark) ? -0x10 : 0x10;
			LIMIT_X(SPEED(0x200));
			LIMIT_Y(SPEED(0x200));
			if(!e->state_time) {
				if(PLAYER_DIST_X(0x1000) && (player.y > e->y) && PLAYER_DIST_Y(0xC000)) {
					// dive attack
					e->x_speed /= 2;
					e->y_speed = 0;
					e->state = 2;
					//SPR_SAFEANIM(e->sprite, 2);		// mouth showing teeth
					e->spriteFrame = 5;
					SPR_SAFETILEINDEX(e->sprite, 
							sheets[e->spriteAnim].index + e->spriteFrame * 4);
				}
			} else {
				e->state_time--;
			}
		break;
		
		case 2:	// dive attack
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5ff));
			if(collide_stage_floor(e)) {
				e->y_speed = 0;
				e->x_speed *= 2;
				e->state_time = TIME(120);		// delay before can dive again
				e->state = 1;
				//SPR_SAFEANIM(e->sprite, 0);
			}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}
