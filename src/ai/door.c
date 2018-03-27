#include "ai_common.h"

// A door is a moving mechanism used to block off and allow access to, an entrance to or within 
// an enclosed space, such as a building, room or vehicle. Doors normally consist of one or two 
// solid panels, with or without windows, that swing using hinges horizontally. These hinges 
// are attached to the door's edge but there are also doors that slide, fold or spin. The main 
// purpose of a door is to control physical access.
// You can find out more about doors by watching this video:
// https://www.youtube.com/watch?v=4p9_GxicLlw

void onspawn_door(Entity *e) {
	// When the door's direction is changed to be facing right it becomes transparent
	if(e->dir) e->hidden = TRUE;
	uint16_t x = sub_to_block(e->x), y = sub_to_block(e->y);
	if(stage_get_block_type(x, y + 1) != 0x41) { // Push down if too high
		e->y += block_to_sub(1);
	} else if(stage_get_block_type(x, y) == 0x41) { // Push up if too low
		e->y -= block_to_sub(1);
	}
}

void ai_door(Entity *e) {
	e->hidden = e->dir;
}

void ai_theDoor(Entity *e) {
	if(e->damage_time) {
		e->frame = 3;
	} else if(PLAYER_DIST_X(e, pixel_to_sub(80)) && PLAYER_DIST_Y(e, pixel_to_sub(80))) {
		e->frame = ++e->timer > 8 ? 2 : 1;
	} else {
		e->timer = e->frame = 0;
	}
}

void onspawn_doorway(Entity *e) {
	if(e->eflags & NPC_OPTION2) {
		// Right door
		e->frame = 11;
		e->x -= 8 << CSF;
	} else {
		// Left door
		e->x += 8 << CSF;
	}
}

void ai_doorway(Entity *e) {
	e->dir = 0;
	// Opening animation
	if(e->state) {
		if(++e->animtime > TIME_8(10)) {
			e->animtime = 0;
			e->frame++;
			if(e->frame == 11 || e->frame == 22) e->state = STATE_DELETE;
		}
	} else if(++e->timer > TIME_8(5)) {
		// This makes sure the 2 doors draw on top of the doorway
		e->timer = 0;
		moveMeToFront = TRUE;
	}
}
