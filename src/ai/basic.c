#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

// Tons of NPCs are placed 1 block above where they are meant
// to appear. This function pushes them down to the right spot
void ai_pushdn_onCreate(Entity *e) {
	u16 x = sub_to_block(e->x), y = sub_to_block(e->y);
	if(stage_get_block_type(x, y + 1) != 0x41) e->y += block_to_sub(1);
}
// King faces right on NPC_OPTION2
void ai_op2flip_onCreate(Entity *e) {
	if(e->eflags & NPC_OPTION2) {
		e->direction = 1;
	}
}
// Save sign frame advance on NPC_OPTION2
void ai_op2frame_onCreate(Entity *e) {
	if(e->eflags & NPC_OPTION2) {
		e->spriteFrame = 1;
	}
}
// Computer changes animation on NPC_OPTION2
void ai_op2anim_onCreate(Entity *e) {
	if(e->eflags & NPC_OPTION2) {
		e->spriteAnim = 1;
	}
}
// Blackboard needs to be pushed up, and changes frame on NPC_OPTION2
void ai_blackboard_onCreate(Entity *e) {
	e->y -= block_to_sub(1);
	if(e->eflags & NPC_OPTION2) e->spriteFrame = 1;
}
// This rotates the spikes so they are always sticking out of a solid area
// Spikes use a second frame for 90 degree rotation
// In the actual game, option 1 & 2 are used for this, but whatever
void ai_spike_onCreate(Entity *e) {
	u16 x = sub_to_block(e->x), y = sub_to_block(e->y);
	if(stage_get_block_type_ROM(x, y+1) == 0x41) return; // Solid on bottom
	if(stage_get_block_type_ROM(x, y-1) == 0x41) { // Solid on top
		e->spriteVFlip = 1;
		return;
	}
	if(stage_get_block_type_ROM(x-1, y) == 0x41) { // Solid on left
		e->spriteFrame = 1;
		return;
	}
	if(stage_get_block_type_ROM(x+1, y) == 0x41) { // Solid on right
		e->spriteFrame = 1;
		e->direction = 1;
		return;
	}
}

// For objects that need to fall when the floor disappears beneath them
void ai_grav_onUpdate(Entity *e) {
	if(!e->grounded) e->y_speed += gravity;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

// Default onState just explodes on death
void ai_default_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) e->state = STATE_DESTROY;
}

void ai_trigger_onUpdate(Entity *e) {
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
