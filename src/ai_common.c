#include "behavior.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

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
		s32 x1 = e->x - block_to_sub(6), y1 = e->y - block_to_sub(6),
			x2 = e->x + block_to_sub(6), y2 = e->y + block_to_sub(6);
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
		SPR_setFrame(e->sprite, 0);
		break;
	case 1: // Preparing to jump
		SPR_setFrame(e->sprite, 1);
		SPR_setHFlip(e->sprite, e->direction);
		e->state_time = 30;
		break;
	case 2: // Hop
		e->grounded = false;
		e->x_speed = -0x100 + 0x200 * e->direction;
		e->y_speed = pixel_to_sub(-3);
		SPR_setFrame(e->sprite, 2);
		sound_play(SOUND_HOP, 5);
		break;
	}
	return false;
}

void ai_update_door_enemy(Entity *e) {
	if(e->state == 1) {
		e->state_time--;
		if(e->state_time == 0) e->state = 0;
	} else {
		s32 x1 = e->x - block_to_sub(6), y1 = e->y - block_to_sub(6),
			x2 = e->x + block_to_sub(6), y2 = e->y + block_to_sub(6);
		if(player.x > x1 && player.x < x2 && player.y > y1 && player.y < y2) {
			SPR_setFrame(e->sprite, 2);
		} else {
			SPR_setFrame(e->sprite, 0);
		}
	}
}

void ai_hurt_door_enemy(Entity *e) {
	e->state = 1;
	e->state_time = 30;
	SPR_setFrame(e->sprite, 3);
}

void ai_setup(Entity *e) {
	switch(e->type) {
	case 1: // Weapon Energy
		e->x_speed = 0x200 - (random() % 0x400);
		e->update = &ai_update_energy;
		break;
	case 7: // Basil
		e->alwaysActive = true;
		e->x = player.x;
		break;
	case 9:
	case 12: // Balrog (Cutscene)
	case 19:
		e->update = &ai_update_balrog_scene;
		e->set_state = &ai_setstate_balrog_scene;
		e->grounded = false;
		break;
	case 18: // Door
		e->activate = &ai_activate_door;
		e->activate(e);
		break;
	case 43: // Blackboard
		e->y -= block_to_sub(1);
		if(e->eflags&NPC_OPTION2) SPR_setFrame(e->sprite, 1);
		break;
	case 46: // Trigger
		e->update = &ai_update_trigger;
		break;
	case 59: // Door Enemy
		e->update = &ai_update_door_enemy;
		e->hurt = &ai_hurt_door_enemy;
		break;
	case 60: // Toroko
		e->update = &ai_update_toroko;
		break;
	case 63: // Toroko attacking with stick
		e->y -= block_to_sub(1);
		e->update = &ai_update_toroko;
		e->state = 3; // Running back and forth
		SPR_setAnim(e->sprite, 2);
		break;
	case 5: // Critter - Green
	case 64: // Critter - Blue
		e->update = &ai_update_critter_hop;
		e->set_state = &ai_setstate_critter_hop;
		break;
	case 65: // Bat - First Cave
		e->y_speed = pixel_to_sub(1);
		e->update = &ai_update_bat_vert;
		e->set_state(e, 0);
		break;
	case 67: // Misery (floating)
		e->update = &ai_update_misery_float;
		e->set_state = &ai_setstate_misery_float;
		break;
	case 68: // Boss: Balrog (Mimiga Village)
		e->update = &ai_update_balrog_boss1;
		e->set_state = &ai_setstate_balrog_boss1;
		break;
	case 21: // Open Chest
	case 30: // Gunsmith
		if(!(stage_get_block_type(sub_to_block(e->x), sub_to_block(e->y) + 1) & BLOCK_SOLID)) {
			e->y += block_to_sub(1);
		}
		break;
	case 62: // Kazuma on computer
	case 79: // Mahin
		e->y += block_to_sub(1);
		break;
	case 96: // Fans
	case 97:
	case 98:
	case 99:
		if(e->eflags&NPC_OPTION2) {
			e->state = e->type - 95;
			e->update = &ai_update_fan;
		}
		e->activate = &ai_activate_fan;
		if(e->sprite != NULL) e->activate(e);
		break;
	case 102: // Power Wave
		e->y += block_to_sub(1);
		break;
	case 211: // Spikes
		e->activate = &ai_activate_spike;
		if(e->sprite != NULL) e->activate(e);
		break;
	default:
		break;
	}
}
