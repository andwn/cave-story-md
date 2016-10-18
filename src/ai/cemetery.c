#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "camera.h"

void ai_pignon(Entity *e) {
	e->timer++;
	if(e->state < 3 && e->damage_time == 29) {
		//FACE_PLAYER(e);
		e->state = 3;
		e->timer = 0;
		e->y_speed = -0x100;
		if(e->type == OBJ_GIANT_MUSHROOM_ENEMY)
			e->x_speed = e->x > player.x ? -0x100 : 0x100;
		else
			e->x_speed = 0;
		//MOVE_X(-0x120);
		e->frame = 4;
	}
	switch(e->state) {
		case 0: // Standing
		{
			if(e->timer > 120 && (e->timer & 31) == 0) { 
				// Either blink or walk in a random direction
				u8 rnd = random() & 7;
				if(rnd == 0) {
					e->state = 1;
					e->timer = 0;
					e->frame = 2;
				} else if(rnd == 1) {
					e->state = 2;
					e->timer = 0;
					e->dir = random() & 1;
					e->x_speed = e->dir ? 0x100 : -0x100;
					e->frame = 1;
				}
			}
		}
		break;
		case 1: // Blink
		{
			if(e->timer >= 10) {
				e->state = 0;
				e->timer = 0;
				e->frame = 0;
			}
		}
		break;
		case 2: // Walking
		{
			ANIMATE(e, 8, 1,0,2,0);
			if(e->timer >= 30 && (random() & 31) == 0) {
				e->state = 0;
				e->timer = 0;
				e->x_speed = 0;
				e->frame = 0;
			}
		}
		break;
		case 3: // Hurt
		{
			e->x_speed -= e->x_speed > 0 ? 4 : -4; // Decellerate
			if(e->timer >= TIME(30)) {
				e->state = 0;
				e->timer = 0;
				e->x_speed = 0;
				e->frame = 0;
			}
		}
		break;
	}
	if(!e->grounded) e->y_speed += SPEED(0x20);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	// Don't test ceiling, only test sticking to ground while moving
	if(e->x_speed < 0) {
		collide_stage_leftwall(e);
	} else if(e->x_speed > 0) {
		collide_stage_rightwall(e);
	}
	if(e->grounded) {
		if(e->x_speed != 0) e->grounded = collide_stage_floor_grounded(e);
	} else {
		e->grounded = collide_stage_floor(e);
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

void onspawn_gkeeper(Entity *e) {
	e->nflags &= ~NPC_INVINCIBLE;
	e->eflags |= NPC_SHOOTABLE | NPC_INVINCIBLE;
	// I expanded the sprite to 32 width so hflip will be symmetrical
	e->hit_box = (bounding_box) { 8, 11, 8, 11 };
	e->display_box = (bounding_box) { 16, 12, 16, 12 };
	e->attack = 0;
}

void ai_gkeeper(Entity *e) {
	switch(e->state) {
		case 0: // Standing
		e->frame = 0;
		FACE_PLAYER(e);
		// start walking when player comes near
		if(PLAYER_DIST_X(pixel_to_sub(128)) && 
			PLAYER_DIST_Y2(pixel_to_sub(48), pixel_to_sub(32))) {
			e->state = 1;
			e->frame = 1;
		}
		// start walking if shot
		if(e->damage_time > 0) {
			e->state = 1;
			e->frame = 1;
			e->eflags |= NPC_INVINCIBLE;
		}
		break;
		case 1: // Walking
		ANIMATE(e, 8, 1,0,2,0);
		FACE_PLAYER(e);
		e->x_speed = e->dir ? 0x100 : -0x100;
		// reached knife range of player?
		if(PLAYER_DIST_X(pixel_to_sub(10))) {
			e->state = 2;
			e->timer = 0;
			e->x_speed = 0;
			sound_play(SND_FIREBALL, 5);
			e->frame = 3;
			e->eflags &= ~NPC_INVINCIBLE;
		}
		break;
		case 2: // Knife raised
		if(++e->timer > 40) {
			e->state = 3;
			e->timer = 0;
			e->attack = 10;
			e->hit_box.left += 6;
			sound_play(SND_SLASH, 5);
			e->frame = 4;
		}
		break;
		case 3: // Knife frame 2
		if(++e->timer > 2) {
			e->state = 4;
			e->timer = 0;
			e->frame = 5;
		}
		break;
		case 4: // Knife frame 3
		if(++e->timer > 60) {
			e->state = 0;
			e->frame = 0;
			e->eflags |= NPC_INVINCIBLE;
			e->attack = 0;
			e->hit_box.left -= 6;
		}
		break;
	}
	if(!e->grounded) e->y_speed += GRAVITY_JUMP;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	// Don't test ceiling, only test sticking to ground while moving
	if(e->x_speed < 0) {
		collide_stage_leftwall(e);
	} else if(e->x_speed > 0) {
		collide_stage_rightwall(e);
	}
	if(e->grounded) {
		if(e->x_speed != 0) e->grounded = collide_stage_floor_grounded(e);
	} else {
		e->grounded = collide_stage_floor(e);
	}
	e->x = e->x_next;
	e->y = e->y_next;
}
