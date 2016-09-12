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
void onspawn_batVertical(Entity *e) {
	e->y += pixel_to_sub(24);
}

// Just up and down gotta go up and down
void ai_batVertical(Entity *e) {
	ANIMATE(e, 4, 0,1,2);
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

void onspawn_batHang(Entity *e) {
	e->frame = 3; // Hanging anim;
}

void ai_batHang(Entity *e) {
	if(e->state == 0) { // Hanging and waiting
		if(random() % BAT_WAIT_TIME == 0) {
			e->state = 1;
			e->timer = 0;
			e->frame = 4;
		}
		if(player.x > e->x - 0x1000 && player.x < e->x + 0x1000 && 
			player.y > e->y - 0x1000 && player.y < e->y + 0x9000) {
			e->state = 2;
			e->timer = 0;
		}
	} else if(e->state == 1) { // Blinking
		if(++e->timer > BAT_BLINK_TIME) {
			e->state = 0;
			e->timer = 0;
			e->frame = 3;
		}
	} else if(e->state == 2) { // At attention
		if(e->damage_time > 0 || (player.x > e->x - 0x2800 && player.x < e->x + 0x2800)) {
			e->state = 3;
			e->timer = 0;
			e->frame = 5;
		}
	} else if(e->state == 3) { // Falling
		e->y_speed += BAT_FALL_ACCEL;
		if(e->y_speed > BAT_FALL_SPEED) e->y_speed = BAT_FALL_SPEED;
		
		e->timer++;
		e->x_next = e->x; // x_next needs to be set for collision to work properly
		e->y_next = e->y + e->y_speed;
		bool collided = collide_stage_floor(e);
		if(collided || (e->timer > BAT_FALL_TIME && player.y - 0x2000 < e->y)) {
			e->state = 4;
			e->timer = 0;
			e->y_mark = e->y;
			e->frame = 0;
			if(collided) e->y_speed = -BAT_BOUNCE_SPEED;
		} else {
			e->y = e->y_next;
		}
	} else { // Flying
		if(++e->timer2 > 4) {
			if(++e->frame >= 3) e->frame = 0;
			e->timer2 = 0;
		}
		if((e->dir && player.x < e->x) || (!e->dir && player.x > e->x)) {
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

void ai_batCircle(Entity *e) {
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
			if(e->dir && player.x < e->x) TURN_AROUND(e);
			if(!e->dir && player.x > e->x) TURN_AROUND(e);
			e->x_speed += (e->x > e->x_mark) ? -0x10 : 0x10;
			e->y_speed += (e->y > e->y_mark) ? -0x10 : 0x10;
			LIMIT_X(SPEED(0x200));
			LIMIT_Y(SPEED(0x200));
			if(!e->timer) {
				if(PLAYER_DIST_X(0x1000) && (player.y > e->y) && PLAYER_DIST_Y(0xC000)) {
					// dive attack
					e->x_speed /= 2;
					e->y_speed = 0;
					e->state = 2;
					//e->frame = 2;		// mouth showing teeth;
					e->frame = 5;
				}
			} else {
				e->timer--;
			}
		break;
		
		case 2:	// dive attack
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5ff));
			if(collide_stage_floor(e)) {
				e->y_speed = 0;
				e->x_speed *= 2;
				e->timer = TIME(120);		// delay before can dive again
				e->state = 1;
				e->frame = 0;
			}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}
