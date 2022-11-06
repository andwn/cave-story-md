#include "ai_common.h"

void ai_behemoth(Entity *e) {
	switch(e->state) {
		case 0: // Walking
		{
			ANIMATE(e, 12, 0,1,0,2);
			if(e->x_speed == 0) TURN_AROUND(e);
			MOVE_X(SPEED_8(0xFF));
			if(e->damage_time) {
				e->frame = 3;
				e->state = 1;
				e->timer = 0;
			}
		}
		break;
		case 1: // Hit
		{
			e->x_speed *= 7;
			e->x_speed /= 8;
			
			if(++e->timer > 40) {
				if(e->damage_time) {
					e->state = 2;
					e->frame = 4;
					e->timer = 0;
					e->animtime = 0;
					e->attack = 5;
					MOVE_X(SPEED_10(0x300));
				} else {
					e->state = 0;
					e->animtime = 0;
					e->attack = 1;
					MOVE_X(SPEED_8(0xFF));
				}
			}
		}
		break;
		case 2: // Charging
		{
			ANIMATE(e, 8, 4,5);
			if(e->x_speed == 0) TURN_AROUND(e);
			MOVE_X(SPEED_10(0x300));
			if(++e->timer > TIME_8(200)) {
				e->state = 0;
				e->attack = 1;
				e->frame = 0;
			}
		}
		break;
	}
	
	if(!e->grounded) e->y_speed += SPEED_8(0x40);
	LIMIT_Y(SPEED_12(0x5FF));
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_beetle(Entity *e) {
	switch(e->state) {
		case 0: // Initial state - begin at the wall we are facing
		{
			while(blk(e->x, e->dir ? 9 : -9, e->y, 0) != 0x41) {
				e->x += e->dir ? 0x1000 : -0x1000;
			}
			e->alwaysActive = FALSE;
			e->state++;
		} /* fallthrough */
		case 1: // wait for player on the wall
		{
			if(++e->timer > TIME_8(50) && PLAYER_DIST_Y(e, 16 << CSF)) {
				TURN_AROUND(e);
				MOVE_X(SPEED_10(0x200));
				e->state = 2;
			}
		}
		break;
		case 2: // moving left or right
		{
			ANIMATE(e, 4, 1,0);
			e->x += e->x_speed;
			if(blk(e->x, e->dir ? 8 : -8, e->y, 0) == 0x41) {
				e->state = 1;
				e->frame = 0;
				e->timer = 0;
				e->x_speed = 0;
			}
		}
		break;
	}
}

void onspawn_beetleFollow(Entity *e) {
	e->y_mark = e->y;
	e->y -= 0xC00;
}

void ai_beetleFollow(Entity *e) {
	// Don't deactivate immediately when offscreen, but do if really far away
	e->alwaysActive = TRUE;
	ANIMATE(e, 4, 1,0);
	FACE_PLAYER(e);
	e->x_speed += e->dir ? SPEED_8(12) : -SPEED_8(12);
	if(abs(e->x_speed) > SPEED_10(0x360)) 
		e->x_speed = e->dir ? SPEED_10(0x360) : -SPEED_10(0x360);
	e->y_speed += (e->y > e->y_mark) ? -4 : 4;
	LIMIT_Y(SPEED_10(0x1FF));
	if(e->damage_time) {
		e->x += e->x_speed >> 1;
		e->y += e->y_speed >> 1;
	} else {
		e->x += e->x_speed;
		e->y += e->y_speed;
	}
}

void onspawn_basu(Entity *e) {
	e->attack = 5;
	e->y_mark = e->y;
	e->y -= 0xC00;
}

void ai_basu(Entity *e) {
	ANIMATE(e, 8, 1,0);
	e->timer++;
	FACE_PLAYER(e);
	e->x_speed += e->dir ? 5 : -5;
	LIMIT_X(SPEED_10(0x2FF));
	e->y_speed += (e->y > e->y_mark) ? -2 : 2;
	LIMIT_Y(SPEED_10(0x1FF));
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(e->x_speed < 0 && collide_stage_leftwall(e)) e->x_speed = SPEED(0x200);
	if(e->x_speed > 0 && collide_stage_rightwall(e)) e->x_speed = -SPEED(0x200);
	e->x = e->x_next;
	e->y = e->y_next;
	// Fire projectile
	if(e->timer > TIME_8(150) && PLAYER_DIST_X(e, 0x14000)) {
		e->timer = 0;
		sound_play(SND_EM_FIRE, 5);
		Entity *shot = entity_create(e->x, e->y, OBJ_GIANT_BEETLE_SHOT, 0);
		THROW_AT_TARGET(shot, player.x, player.y, SPEED(0x300));
	}
}

void onspawn_basil(Entity *e) {
	e->alwaysActive = TRUE;
	e->x = player.x;
}

void ai_basil(Entity *e) {
	ANIMATE(e, 8, 0,1,2);
	if(e->x_speed == 0) { // Hit a wall
		e->dir ^= 1;
	} else if(sub_to_pixel(e->x) < sub_to_pixel(camera.x) - ScreenHalfW - 64) {
		e->dir = 1;
	} else if(sub_to_pixel(e->x) > sub_to_pixel(camera.x) + ScreenHalfW + 64) {
		e->dir = 0;
	}
	MOVE_X(SPEED(0x500));
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y; // Must store y_next for collision to work
	if(e->x_speed < 0) collide_stage_leftwall(e);
	else collide_stage_rightwall(e);
	e->x = e->x_next;
}

// Push out of the wall
void onspawn_lift(Entity *e) {
	e->x += 8 << CSF;
}

#define LIFT_SPEED 0x200
#define LIFT_MOVE_TIME 64
#define LIFT_WAIT_TIME 90

// Lift in observatory room that moves up and down
void ai_lift(Entity *e) {
	if(e->timer > 0) e->timer--;
	if(e->timer == 0) {
		e->state++;
		if(e->state == 8) e->state = 0;
		switch(e->state) {
			case 0: // bottom floor wait
			e->timer = LIFT_WAIT_TIME;
			e->y_speed = 0;
			break;
			case 1: // bottom to middle
			e->timer = LIFT_MOVE_TIME;
			e->y_speed = -LIFT_SPEED;
			break;
			case 2: // middle wait
			e->timer = LIFT_WAIT_TIME;
			e->y_speed = 0;
			break;
			case 3: // middle to top
			e->timer = LIFT_MOVE_TIME;
			e->y_speed = -LIFT_SPEED;
			break;
			case 4: // top wait
			e->timer = LIFT_WAIT_TIME;
			e->y_speed = 0;
			break;
			case 5: // top to middle
			e->timer = LIFT_MOVE_TIME;
			e->y_speed = LIFT_SPEED;
			break;
			case 6: // middle wait
			e->timer = LIFT_WAIT_TIME;
			e->y_speed = 0;
			break;
			case 7: // middle to bottom
			e->timer = LIFT_MOVE_TIME;
			e->y_speed = LIFT_SPEED;
			break;
		}
	}
	e->y += e->y_speed;
}

void ai_terminal(Entity *e) {
	switch(e->state) {
		case 0:
		case 1:
		{
			e->frame = 0;
			if (PLAYER_DIST_X(e, 8<<CSF) && PLAYER_DIST_Y2(e, 16<<CSF, 8<<CSF)) {
				sound_play(SND_COMPUTER_BEEP, 5);
				if(e->flags & NPC_OPTION2) e->frame = 2;
				e->state = 10;
			}
		}
		break;
		case 10:
		{
			e->frame ^= 1;
		}
		break;
	}
}
