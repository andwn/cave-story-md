#include "ai_common.h"
#include "ai_object.h"
#include "ai_mimiga.h"
#include "ai_balrog.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "sprite.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

#define entity_within_rect(p, x1, y1, x2, y2) (p->x > x1 && p->x < x2 && p->y > y1 && p->y < y2)

void ai_activate_base(Entity *e) {

}
void ai_update_base(Entity *e) {

}
bool ai_setstate_base(Entity *e, u16 state) {
	e->state = state;
	if(state == STATE_DEFEATED) {
		return true;
	}
	return false;
}

void ai_update_bat_vert(Entity *e) {
	if(e->state == 0) {
		e->y_speed -= 8;
		if(e->y_speed <= pixel_to_sub(-1)) e->state = 1;
	} else if(e->state == 1) {
		e->y_speed += 8;
		if(e->y_speed >= pixel_to_sub(1)) e->state = 0;
	}
	e->y += e->y_speed;
}

void ai_update_critter_hop(Entity *e) {
	if(!e->grounded) {
		e->y_speed += gravity;
		e->x_next = e->x + e->x_speed;
		e->y_next = e->y + e->y_speed;
		entity_update_collision(e);
		e->x = e->x_next;
		e->y = e->y_next;
		if(e->grounded) e->set_state(e, 0);
	} else if(e->state == 1) {
		e->state_time--;
		if(e->state_time == 0) e->set_state(e, 2);
	} else {
		s32 x1 = e->x - block_to_sub(8), y1 = e->y - block_to_sub(8),
			x2 = e->x + block_to_sub(8), y2 = e->y + block_to_sub(8);
		if(player.x > x1 && player.x < x2 && player.y > y1 && player.y < y2) {
			e->direction = player.x > e->x;
			e->set_state(e, 1);
		}
	}
}

bool ai_setstate_critter_hop(Entity *e, u16 state) {
	e->state = state;
	if(state == STATE_DEFEATED) {
		return true;
	}
	switch(state) {
	case 0: // Still on the ground
		e->y_speed = 0;
		e->x_speed = 0;
		sprite_set_frame(e->sprite, 0);
		break;
	case 1: // Preparing to jump
		sprite_set_frame(e->sprite, 1);
		sprite_set_attr(e->sprite, TILE_ATTR(PAL2, 0, 0, e->direction));
		e->state_time = 30;
		break;
	case 2: // Hop
		e->grounded = false;
		e->x_speed = -0x100 + 0x200 * e->direction;
		e->y_speed = pixel_to_sub(-3);
		sprite_set_frame(e->sprite, 2);
		sound_play(SOUND_HOP, 5);
		break;
	}
	return false;
}

void entity_create_special(Entity *e, bool option1, bool option2) {
	switch(e->type) {
	case 1: // Weapon Energy
		e->x_speed = 0x200 - (random() % 0x400);
		e->update = &ai_update_energy;
		break;
	case 12: // Balrog (Cutscene)
		e->update = &ai_update_balrog_scene;
		e->set_state = &ai_setstate_balrog_scene;
		break;
	case 18: // Door
		e->activate = &ai_activate_door;
		e->activate(e);
		break;
	case 46: // Trigger
		e->update = &ai_update_trigger;
		break;
	case 60: // Toroko
		e->update = &ai_update_toroko;
		break;
	case 63: // Toroko attacking with stick
		e->y -= block_to_sub(1);
		e->update = &ai_update_toroko;
		e->state = 3; // Running back and forth
		sprite_set_animation(e->sprite, 2);
		break;
	//case 26:
	//case 31:
	case 64: // Critter - Blue
		e->update = &ai_update_critter_hop;
		e->set_state = &ai_setstate_critter_hop;
		break;
	case 65: // Bat - First Cave
		e->y_speed = pixel_to_sub(1);
		e->update = &ai_update_bat_vert;
		e->set_state(e, 0);
		break;
	case 68: // Boss: Balrog (Mimiga Village)
		e->update = &ai_update_balrog_boss1;
		e->set_state = &ai_setstate_balrog_boss1;
		break;
	case 211: // Spikes
		e->activate = &ai_activate_spike;
		if(e->sprite != SPRITE_NONE) e->activate(e);
		break;
	default:
		break;
	}
}
