#include "ai_balrog.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "sprite.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"


// 12 - Balrog (Cutscene)
void ai_update_balrog_scene(Entity *e) {
	if(e->state_time > 0) {
		e->state_time--;
		if(e->state_time == 0) {
			switch(e->state) {
			case 20:
			case 21: e->set_state(e, 10); break;
			case 30: e->set_state(e, 0); break;
			default: break;
			}
		}
	}
	if(!e->grounded) e->y_speed += gravityJump;
	e->y_next = e->y + e->y_speed;
	e->x_next = e->x + e->x_speed;
	if(e->state != 10 && e->y > block_to_sub(6)) entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

bool ai_setstate_balrog_scene(Entity *e, u16 state) {
	e->state = state;
	switch(state) {
	case 0: // Standing around
		sprite_set_animation(e->sprite, 0);
		break;
	case 10: // Going up!
	case 11:
		sprite_set_animation(e->sprite, 3);
		e->y_speed = pixel_to_sub(-2);
		break;
	case 20: // Smoking, going up!
	case 21:
		sprite_set_animation(e->sprite, 5);
		//e->y_speed = pixel_to_sub(-2);
		e->state_time = 160;
		break;
	case 30: // Smile
		sprite_set_animation(e->sprite, 6);
		e->state_time = 90;
		break;
	case 70: // Vanish
	case 71:
		return true;
	}
	return false;
}

// 68 - Boss: Balrog (Mimiga Village)
void ai_update_balrog_boss1(Entity *e) {
	if(e->state_time > 0) e->state_time--;
	switch(e->state) {
	case 0:
	case 2:
	case 4:
		if(e->x_speed > 0) e->x_speed -= 0x10;
		else if(e->x_speed < 0) e->x_speed += 0x10;
		if(abs(e->x_speed) < 0x10) {
			e->x_speed = 0;
			sprite_set_animation(e->sprite, 0);
		}
		if(e->state_time == 0) e->set_state(e, e->state + 1);
		break;
	case 1:
	case 3:
	case 5:
		e->x_speed -= 0x10 - (0x20 * e->direction);
		if(e->state_time == 0 || (abs(e->x - player.x) < block_to_sub(2)))
			e->set_state(e, e->state + 1);
		break;
	case 6:
		if(e->grounded) {
			e->x_speed >>= 1;
			e->set_state(e, 0);
		} else e->y_speed += gravityJump;
		break;
	case 7: // Grabbed player
		player.x = e->x;
		player.y = e->y;
		if(!e->grounded) e->y_speed += gravityJump;
		if(e->state_time == 0) e->set_state(e, 8);
		break;
	case 8:
		if(e->state_time == 0) e->set_state(e, 0);
		break;
	default:
		break;
	}
	// Grab/throw player
	if(e->state < 7 && !player_invincible() && entity_overlapping(&player, e)) {
		e->set_state(e, 7);
	}
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

bool ai_setstate_balrog_boss1(Entity *e, u16 state) {
	e->state = state;
	if(state == STATE_DEFEATED) {
		e->state += 1;
		tsc_call_event(e->event); // Boss defeated event
		return false;
	}
	switch(state) {
	case 0: // Stand still
	case 2:
	case 4:
		e->state_time = 160;
		break;
	case 1: // Run towards player
	case 3: // Run towards player (2)
	case 5: // Run towards player and jump
		e->direction = e->x < player.x;
		sprite_set_animation(e->sprite, 1);
		sprite_set_direction(e->sprite, e->direction);
		e->state_time = 120;
		break;
	case 6: // Jumping
		e->grounded = false;
		e->y_speed = pixel_to_sub(-2);
		sprite_set_animation(e->sprite, 3);
		e->state_time = 160;
		break;
	case 7: // Grab player
		player_lock_controls();
		player.x = e->x;
		player.y = e->y;
		player.x_speed = 0;
		player.y_speed = 0;
		player.grounded = true;
		e->x_speed = 0;
		sprite_set_animation(e->sprite, 8);
		e->state_time = 120;
		break;
	case 8: // Throw player
		player_unlock_controls();
		sprite_set_animation(e->sprite, 3);
		if(player_inflict_damage(1)) break;
		player.y_speed = pixel_to_sub(-1);
		player.x_speed = pixel_to_sub(2) - (pixel_to_sub(4) * e->direction);
		e->state_time = 60;
		break;
	default:
		break;
	}
	return false;
}
