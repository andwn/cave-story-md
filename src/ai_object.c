#include "ai_object.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "sprite.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void ai_update_energy(Entity *e) {
	// Bounce when hitting the ground
	if(e->grounded) {
		e->y_speed = pixel_to_sub(-2);
		e->grounded = false;
		sound_play(SOUND_EXPBOUNCE, 0);
	}
	e->y_speed += gravity;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	s16 xsp = e->x_speed;
	entity_update_collision(e);
	// Reverse direction when hitting a wall
	if(e->x_speed == 0) {
		e->direction = !e->direction;
		e->x_speed = -xsp;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

// When the door's direction is changed to be facing right it becomes
// transparent. The CNP command does this when Balrog barges into Barr
void ai_activate_door(Entity *e) {
	if(e->direction) {
		sprite_delete(e->sprite);
		e->sprite = SPRITE_NONE;
	}
}

void ai_update_trigger(Entity *e) {
	if(tsc_running()) return;
	bool activate = false;
	if(e->eflags&NPC_OPTION2) { // Vertical
		if(player.x - pixel_to_sub(player.hit_box.left) < e->x + pixel_to_sub(e->hit_box.right) &&
			player.x + pixel_to_sub(player.hit_box.right) > e->x - pixel_to_sub(e->hit_box.left)) {
			activate = true;
		}
	} else { // Horizontal
		if(entity_overlapping(&player, e) && player.y_speed < 0) activate = true;
	}
	if(activate) tsc_call_event(e->event);
}

void ai_activate_fan(Entity *e) {
	u8 anim = 0;
	if(e->eflags & NPC_OPTION2) anim = 2;
	switch(e->type) {
	case 96: // Left
		// Nothing
		break;
	case 97: // Up
		anim += 1;
		break;
	case 98: // Right
		sprite_set_attr(e->sprite, TILE_ATTR(PAL1, 0, 0, 1));
		break;
	case 99: // Down
		anim += 1;
		sprite_set_attr(e->sprite, TILE_ATTR(PAL1, 0, 1, 0));
		break;
	}
	sprite_set_animation(e->sprite, anim);
}

void ai_update_fan(Entity *e) {
	u16 ex = sub_to_block(e->x), ey = sub_to_block(e->y);
	u16 px = sub_to_block(player.x), py = sub_to_block(player.y);
	switch(e->state) {
	case 1: // Left
		if(px > ex - 8 && px <= ex && py == ey) {
			player.x_speed -= 0x50;
		}
		break;
	case 2: // Up
		if(py > ey - 8 && py <= ey && px == ex) {
			player.y_speed -= 0x50;
		}
		break;
	case 3: // Right
		if(px >= ex && px < ex + 8 && py == ey) {
			player.x_speed += 0x50;
		}
		break;
	case 4: // Down
		if(py >= ey && py < ey + 8 && px == ex) {
			player.y_speed += 0x50;
		}
		break;
	default:
		break;
	}
}

// This rotates the spikes so they are always sticking out of a solid area
// Spikes use a second frame for 90 degree rotation
void ai_activate_spike(Entity *e) {
	u16 x = sub_to_block(e->x), y = sub_to_block(e->y);
	if(stage_get_block_type(x, y+1) == 0x41) return; // Solid on bottom
	if(stage_get_block_type(x, y-1) == 0x41) { // Solid on top
		// Flip vertical
		sprite_set_attr(e->sprite, TILE_ATTR(PAL1, false, true, false));
		return;
	}
	if(stage_get_block_type(x-1, y) == 0x41) { // Solid on left
		sprite_set_frame(e->sprite, 1);
		return;
	}
	if(stage_get_block_type(x+1, y) == 0x41) { // Solid on right
		sprite_set_frame(e->sprite, 1);
		// Flip horizontal
		sprite_set_attr(e->sprite, TILE_ATTR(PAL1, false, false, true));
		return;
	}
}
