#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"

void onspawn_snap(Entity *e) {
	SNAP_TO_GROUND(e);
}

void onspawn_op2flip(Entity *e) {
	if(e->eflags & NPC_OPTION2) e->dir = 1;
}

void onspawn_snapflip(Entity *e) {
	if(e->eflags & NPC_OPTION2) e->dir = 1;
	SNAP_TO_GROUND(e);
}

void onspawn_op2anim(Entity *e) {
	if(e->eflags & NPC_OPTION2) e->frame = 1;
}

void onspawn_op2snap(Entity *e) {
	if(e->eflags & NPC_OPTION2) SNAP_TO_GROUND(e);
}

void onspawn_blackboard(Entity *e) {
	e->y -= 16<<CSF;
	if(e->eflags & NPC_OPTION2) e->frame = 1;
}

void onspawn_persistent(Entity *e) {
	e->alwaysActive = true;
}

// Spikes use a second frame for 90 degree rotation
// In the actual game, option 1 & 2 are used for this, but whatever
void onspawn_spike(Entity *e) {
	if(stageID == 0x2D) {
		// Disable sprite in Labyrinth M
		// The map has a brown version overlapping us so it's pointless
		e->hidden = true;
		return;
	}
	// Shrink hitbox slightly -- test this in First Cave
	// Player should not collide with the first spike to the left when not jumping
	e->hit_box.left -= 2;
	e->hit_box.top -= 2;
	e->hit_box.right -= 2;
	e->hit_box.bottom -= 2;
	
	u16 x = sub_to_block(e->x), y = sub_to_block(e->y);
	if(stage_get_block_type(x, y+1) == 0x41) { // Solid on bottom
	} else if(stage_get_block_type(x, y-1) == 0x41) { // Solid on top
		e->sprite[0].attribut |= TILE_ATTR_VFLIP_MASK;
	} else if(stage_get_block_type(x-1, y) == 0x41) { // Solid on left
		e->frame = 1;;
	} else if(stage_get_block_type(x+1, y) == 0x41) { // Solid on right
		e->frame = 1;;
		e->dir = 1;
	}
}

void ai_grav(Entity *e) {
	if(!e->grounded) e->y_speed += GRAVITY;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_trigger(Entity *e) {
	if(tscState) return;
	// Hack to skip Monster X
	if(stageID == 0x27) return;
	bool activate = false;
	if(e->eflags&NPC_OPTION2) { // Vertical
		if(player.x - pixel_to_sub(player.hit_box.left) < e->x + pixel_to_sub(e->hit_box.right) &&
			player.x + pixel_to_sub(player.hit_box.right) > e->x - pixel_to_sub(e->hit_box.left)) {
			activate = true;
		}
	} else { // Horizontal (Egg Corridor eggs)
		e->hit_box.left = 32;
		e->hit_box.right = 32;
		if(entity_overlapping(&player, e) && player.y_speed < 0) activate = true;
	}
	if(activate) tsc_call_event(e->event);
}

void ai_genericproj(Entity *e) {
	if(++e->timer > TIME(250) ||
		stage_get_block_type(sub_to_block(e->x), sub_to_block(e->y)) == 0x41) {
		e->state = STATE_DELETE;
	} else {
		e->x += e->x_speed;
		e->y += e->y_speed;
	}
}

void ondeath_default(Entity *e) {
	e->state = STATE_DESTROY;
}

void onspawn_teleIn(Entity *e) {
	e->x += pixel_to_sub(16);
	e->y -= pixel_to_sub(8);
	e->frame = 2;;
	sound_play(SND_TELEPORT, 5);
}

void ai_teleIn(Entity *e) {
	switch(e->state) {
		case 0: // Appear
		{
			if(++e->timer >= 5*14) {
				e->timer = 0;
				e->state++;
				e->grounded = false;
				e->frame = 0;
			}
		}
		break;
		case 1: // Drop
		{
			if(!e->grounded) {
				e->y_speed += SPEED(0x40);
				e->y += e->y_speed;
				// For whatever reason, collide_stage_floor() doesn't work here
				e->grounded = stage_get_block_type(
						sub_to_block(e->x), sub_to_block(e->y + (8<<CSF))) == 0x41;
			}
		}
		break;
	}
}

void onspawn_teleOut(Entity *e) {
	e->y -= pixel_to_sub(24);
	SNAP_TO_GROUND(e);
	e->y_speed = SPEED(-0x400);
}

void ai_teleOut(Entity *e) {
	switch(e->state) {
		case 0: // Hopping up
		{
			if(++e->timer >= TIME(20)) {
				e->state++;
				e->timer = 0;
				e->y_speed = 0;
				e->frame = 1;
				sound_play(SND_TELEPORT, 5);
			} else {
				e->y_speed += SPEED(0x43);
				e->y += e->y_speed;
			}
		}
		break;
		case 1: // Show teleport animation
		{
			if(++e->timer >= 5*14) {
				e->state++;
				e->timer = 0;
				e->hidden = true;
				////SPR_SAFEVISIBILITY(e->sprite, HIDDEN);
			}
		}
		break;
	}
}

void onspawn_teleLight(Entity *e) {
	e->hidden = true;
	e->x += pixel_to_sub(8);
	e->y += pixel_to_sub(8);
}

void ai_teleLight(Entity *e) {
	
}

void ondeath_teleLight(Entity *e) {
	
}

void ai_player(Entity *e) {
	if(!e->grounded) e->y_speed += SPEED(0x40);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ondeath_player(Entity *e) {
	switch(e->state) {
		case 0:
		e->x_speed = 0;
		e->frame = 0;
		break;
		case 2:		// looking up
		e->frame = 2;
		break;
		case 10:	// he gets flattened
		sound_play(SND_LITTLE_CRASH, 5);
		for(u8 i = 0; i < 4; i++) {
			effect_create_smoke(sub_to_pixel(e->x) - 16 + (random() % 32), 
				sub_to_pixel(e->y) - 16 + (random() % 32));
		}
		e->state++;
		/* no break */
		case 11:
		e->frame = 9;
		break;
		case 20:	// he teleports away
		e->type = 0x6F;
		e->state = 0;
		e->timer = 0;
		onspawn_teleOut(e);
		break;
		case 50:	// walking
		MOVE_X(SPEED(0x200));
		e->frame = 1;
		break;
		// falling, upside-down (from good ending; Fall stage)
		case 60:
		e->frame = 0;
		//SPR_SAFEVFLIP(e->sprite, 1);
		break;
		case 80:	// face away
		e->frame = 4;
		break;
		// walking in place during credits
		case 99:
		case 100:
		case 101:
		case 102:
		e->frame = 1;
		break;
	}
}

void ai_computer(Entity *e) {
	ANIMATE(e, 5, 1,2);
}

void ai_savepoint(Entity *e) {
	ANIMATE(e, 4, 0,1,2,3,4,5,6,7);
	ai_grav(e);
}

void ai_refill(Entity *e) {
	ANIMATE(e, 2, 0,1);
	ai_grav(e);
}
