#include "ai_common.h"

// Just up and down gotta go up and down
void ai_batVertical(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->y_mark = e->y;
			e->timer = rand() & 63;
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			if (!e->timer) {
				e->state = 2;
				e->y_speed = SPEED_10(0x300);
			} else e->timer--;
		}
		break;
		case 2:
		{
			if (e->y >= e->y_mark)
				e->y_speed -= SPEED_8(0x10);
			else
				e->y_speed += SPEED_8(0x10);
			LIMIT_Y(SPEED_10(0x300));
		}
		break;
	}
	ANIMATE(e, 4, 0,1,2);
	FACE_PLAYER(e);
	e->y += e->y_speed;
}

void onspawn_batHang(Entity *e) {
	e->frame = 3; // Hanging anim;
}

void ai_batHang(Entity *e) {
	if(e->state == 0) { // Hanging and waiting
		if(!(rand() & 127)) {
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
		if(++e->timer > 8) {
			e->state = 0;
			e->timer = 0;
			e->frame = 3;
		}
	} else if(e->state == 2) { // At attention
		if(e->damage_time > 0 || (player.x > e->x - 0x2800 && player.x < e->x + 0x2800)) {
			e->state = 3;
			e->timer = 0;
			e->frame = 5;
			moveMeToFront = TRUE;
		}
	} else if(e->state == 3) { // Falling
		e->y_speed += SPEED_8(0x20);
		if(e->y_speed > SPEED_12(0x5FF)) e->y_speed = SPEED_12(0x5FF);
		
		e->timer++;
		e->x_next = e->x; // x_next needs to be set for collision to work properly
		e->y_next = e->y + e->y_speed;
		uint8_t collided = collide_stage_floor(e);
		if(collided || (e->timer > TIME_8(20) && player.y - 0x2000 < e->y)) {
			e->state = 4;
			e->timer = 0;
			e->y_mark = e->y;
			e->frame = 0;
			if(collided) e->y_speed = -SPEED_10(0x200);
		} else {
			e->y = e->y_next;
		}
	} else { // Flying
		if(++e->timer2 > 4) {
			if(++e->frame >= 3) e->frame = 0;
			e->timer2 = 0;
		}
		FACE_PLAYER(e);
		e->x_speed += (e->x > player.x) ? -SPEED_8(0x20) : SPEED_8(0x20);
		e->y_speed += (e->y > e->y_mark) ? -SPEED_8(0x10) : SPEED_8(0x10);
		// Limit speed
		LIMIT_X(SPEED_10(0x200));
		LIMIT_Y(SPEED_10(0x200));
		e->x_next = e->x + e->x_speed;
		e->y_next = e->y + e->y_speed;
		// Bounce against floor and walls
		if(e->x_speed < 0) {
			collide_stage_leftwall(e);
		} else if(e->x_speed > 0) {
			collide_stage_rightwall(e);
		}
		if(collide_stage_floor(e)) {
			e->y_speed = -SPEED_10(0x200);
		}
		if(e->y_speed <= 0) {
			collide_stage_ceiling(e);
		}
		e->x = e->x_next;
		e->y = e->y_next;
	}
}

void ai_batCircle(Entity *e) {
	switch(e->state) {
		case 0:
		{
			uint8_t angle = rand();
			e->x_speed = cos[angle];
			e->y_speed = sin[angle];
			e->x_mark = e->x + (e->x_speed << 3);
			e->y_mark = e->y + (e->x_speed << 3);
			e->state++;
		}
		/* fallthrough */
		case 1:
			// circle around our target point
			if((++e->animtime & 3) == 0) {
				FACE_PLAYER(e);
				LIMIT_X(SPEED_10(0x1E0));
				LIMIT_Y(SPEED_10(0x1E0));
				if(++e->frame > 2) e->frame = 0;
			}
			e->x_speed += (e->x > e->x_mark) ? -0x10 : 0x10;
			e->y_speed += (e->y > e->y_mark) ? -0x10 : 0x10;
			if(!e->timer) {
				if(PLAYER_DIST_X(e, 0x1000) && PLAYER_DIST_Y2(e, 0, pixel_to_sub(64))) {
					// dive attack
					e->x_speed >>= 1;
					e->y_speed = 0;
					e->state++;
					e->frame = 5;
				}
			} else e->timer--;
		break;
		case 2:	// dive attack
			if(e->y_speed < SPEED_12(0x5C0)) e->y_speed += SPEED_8(0x40);
			if(blk(e->x, 0, e->y, 8) == 0x41) {
				e->y_speed = 0;
				e->x_speed <<= 1;
				e->timer = TIME_8(100);		// delay before can dive again
				e->state = 1;
				e->frame = 0;
			}
		break;
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}
