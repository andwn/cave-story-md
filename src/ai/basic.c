#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"

void oncreate_snap(Entity *e) {
	SNAP_TO_GROUND(e);
}

void oncreate_op2flip(Entity *e) {
	if(e->eflags & NPC_OPTION2) e->direction = 1;
}

void oncreate_snapflip(Entity *e) {
	if(e->eflags & NPC_OPTION2) e->direction = 1;
	SNAP_TO_GROUND(e);
}

void oncreate_op2anim(Entity *e) {
	if(e->eflags & NPC_OPTION2) e->spriteAnim = 1;
}

void oncreate_op2snap(Entity *e) {
	if(e->eflags & NPC_OPTION2) SNAP_TO_GROUND(e);
}

void oncreate_blackboard(Entity *e) {
	e->y -= block_to_sub(1);
	if(e->eflags & NPC_OPTION2) e->spriteFrame = 1;
}

void oncreate_persistent(Entity *e) {
	e->alwaysActive = true;
}

// Spikes use a second frame for 90 degree rotation
// In the actual game, option 1 & 2 are used for this, but whatever
void oncreate_spike(Entity *e) {
	u16 x = sub_to_block(e->x), y = sub_to_block(e->y);
	if(stage_get_block_type(x, y+1) == 0x41) { // Solid on bottom
	} else if(stage_get_block_type(x, y-1) == 0x41) { // Solid on top
		e->spriteVFlip = 1;
	} else if(stage_get_block_type(x-1, y) == 0x41) { // Solid on left
		e->spriteAnim = 1;
	} else if(stage_get_block_type(x+1, y) == 0x41) { // Solid on right
		e->spriteAnim = 1;
		e->direction = 1;
	}
}

void ai_grav_onUpdate(Entity *e) {
	if(!e->grounded) e->y_speed += GRAVITY;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_trigger_onUpdate(Entity *e) {
	if(tsc_running()) return;
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

void ai_genericproj_onUpdate(Entity *e) {
	if(++e->state_time > TIME(300) ||
		stage_get_block_type(sub_to_block(e->x), sub_to_block(e->y)) == 0x41) {
		e->state = STATE_DELETE;
	} else {
		e->x += e->x_speed;
		e->y += e->y_speed;
	}
}

void ai_default_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) e->state = STATE_DESTROY;
}

void ai_teleIn_onCreate(Entity *e) {
	e->x += pixel_to_sub(16);
	e->y -= pixel_to_sub(8);
	e->spriteAnim = 2;
	sound_play(SND_TELEPORT, 5);
}

void ai_teleIn_onUpdate(Entity *e) {
	switch(e->state) {
		case 0: // Appear
		{
			if(++e->state_time >= 5*14) {
				e->state_time = 0;
				e->state++;
				e->grounded = false;
				SPR_SAFEANIM(e->sprite, 0);
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

void ai_teleOut_onCreate(Entity *e) {
	e->y -= pixel_to_sub(24);
	SNAP_TO_GROUND(e);
	e->y_speed = SPEED(-0x400);
}

void ai_teleOut_onUpdate(Entity *e) {
	switch(e->state) {
		case 0: // Hopping up
		{
			if(++e->state_time >= TIME(20)) {
				e->state++;
				e->state_time = 0;
				e->y_speed = 0;
				SPR_SAFEANIM(e->sprite, 1);
				sound_play(SND_TELEPORT, 5);
			} else {
				e->y_speed += SPEED(0x43);
				e->y += e->y_speed;
			}
		}
		break;
		case 1: // Show teleport animation
		{
			if(++e->state_time >= 5*14) {
				e->state++;
				e->state_time = 0;
				SPR_SAFEVISIBILITY(e->sprite, HIDDEN);
			}
		}
		break;
	}
}

void ai_teleLight_onCreate(Entity *e) {
	e->spriteAnim = SPRITE_DISABLE;
	e->x += pixel_to_sub(8);
	e->y += pixel_to_sub(8);
}

void ai_teleLight_onUpdate(Entity *e) {
	
}

void ai_teleLight_onState(Entity *e) {
	
}

void ai_player_onUpdate(Entity *e) {
	if(!e->grounded) e->y_speed += SPEED(0x40);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_player_onState(Entity *e) {
	switch(e->state) {
		case 0:
		e->x_speed = 0;
		SPR_SAFEANIM(e->sprite, 0);
		break;
		case 2:		// looking up
		SPR_SAFEANIM(e->sprite, 2);
		break;
		case 10:	// he gets flattened
		sound_play(SND_LITTLE_CRASH, 5);
		for(u8 i = 0; i < 4; i++) {
			effect_create_smoke(0, sub_to_pixel(e->x) - 16 + (random() % 32), 
				sub_to_pixel(e->y) - 16 + (random() % 32));
		}
		e->state++;
		/* no break */
		case 11:
		SPR_SAFEANIM(e->sprite, 9);
		break;
		case 20:	// he teleports away
		e->type = 0x6F;
		e->state = 0;
		e->state_time = 0;
		ai_teleOut_onCreate(e);
		break;
		case 50:	// walking
		MOVE_X(SPEED(0x200));
		SPR_SAFEANIM(e->sprite, 1);
		break;
		// falling, upside-down (from good ending; Fall stage)
		case 60:
		SPR_SAFEANIM(e->sprite, 0);
		SPR_SAFEVFLIP(e->sprite, 1);
		break;
		case 80:	// face away
		SPR_SAFEANIM(e->sprite, 4);
		break;
		// walking in place during credits
		case 99:
		case 100:
		case 101:
		case 102:
		SPR_SAFEANIM(e->sprite, 1);
		break;
	}
}
