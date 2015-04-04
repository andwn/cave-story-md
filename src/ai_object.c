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
