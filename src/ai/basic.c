#include "ai_common.h"

void ai_null(Entity *e) { (void)(e); }

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
	if(e->eflags & NPC_OPTION2) {
		SNAP_TO_GROUND(e);
	}
}

void onspawn_pushup(Entity *e) {
	e->y -= 16 << CSF;
}

void onspawn_blackboard(Entity *e) {
	e->y -= 16<<CSF;
	if(e->eflags & NPC_OPTION2) e->frame = 1;
}

void onspawn_persistent(Entity *e) {
	e->alwaysActive = TRUE;
}

// Needed for save point after sisters fight
void onspawn_interactive(Entity *e) {
	e->eflags |= NPC_INTERACTIVE;
}

// Spikes use a second frame for 90 degree rotation
// In the actual game, option 1 & 2 are used for this, but whatever
void onspawn_spike(Entity *e) {
	if(stageID == 0x2D) {
		// Disable sprite in Labyrinth M
		// The map has a brown version overlapping us so it's pointless
		e->hidden = TRUE;
		return;
	}
	// Shrink hitbox slightly -- test this in First Cave
	// There are hidden spikes in the wall near the gunsmith room
	e->hit_box.left -= 2;
	e->hit_box.top -= 2;
	e->hit_box.right -= 2;
	e->hit_box.bottom -= 2;
	
	uint16_t x = sub_to_block(e->x), y = sub_to_block(e->y);
	if(stage_get_block_type(x, y+1) == 0x41) { // Solid on bottom
	} else if(stage_get_block_type(x, y-1) == 0x41) { // Solid on top
		e->sprite[0].attribut |= TILE_ATTR_VFLIP_MASK;
	} else if(stage_get_block_type(x-1, y) == 0x41) { // Solid on left
		e->frame = 1;
	} else if(stage_get_block_type(x+1, y) == 0x41) { // Solid on right
		e->frame = 1;
		e->dir = 1;
	}
}

// The big spike, collision box is too high
void onspawn_trap(Entity *e) {
	e->hit_box.top -= 7;
}

void ondeath_event(Entity *e) {
	tsc_call_event(e->event);
}

void ai_grav(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(e->grounded) {
		e->grounded = collide_stage_floor_grounded(e);
	} else {
		e->y_speed += SPEED(0x40);
		e->grounded = collide_stage_floor(e);
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

// For type 0, this actually does something with OPTION1 apparently
void ai_nothing(Entity *e) {
	if(!e->state) {
		e->state = 1;
		if(e->eflags & NPC_OPTION1) {
			e->type = OBJ_HVTRIGGER;
			e->eflags &= ~(NPC_OPTION1 | NPC_OPTION2);
			e->hit_box.top = 4; e->hit_box.bottom = 0;
			e->hit_box.left = 128; e->hit_box.right = 128;
		}
	}
}

void onspawn_trigger(Entity *e) {
	e->alwaysActive = TRUE;
	if(e->eflags & NPC_OPTION2) { // Vertical
		// First test if the trigger is placed outside of the map, if so change the type
		// to OOB trigger which will activate based on player position instead of collision
		int16_t ex = sub_to_block(e->x);
		if(ex <= 0) { // Left OOB
			//e->alwaysActive = TRUE;
			e->type = OBJ_TRIGGER_SPECIAL;
			e->eflags &= ~NPC_OPTION1;
		} else if(ex >= stageWidth - 1) { // Right OOB
			//e->alwaysActive = TRUE;
			e->type = OBJ_TRIGGER_SPECIAL;
			e->eflags |= NPC_OPTION1;
		} else { // Not OOB
			e->hit_box.left = 2; e->hit_box.right = 2;
			// Don't expand immediately
			e->hit_box.top = 1; e->hit_box.bottom = 1;
			//for(; e->hit_box.top <= 240; e->hit_box.top += 16) {
			//	if(stage_get_block_type((e->x>>CSF)/16, 
			//			((e->y>>CSF)-e->hit_box.top)/16) == 0x41) break;
			//}
			//for(; e->hit_box.bottom <= 240; e->hit_box.bottom += 16) {
			//	if(stage_get_block_type((e->x>>CSF)/16, 
			//			((e->y>>CSF)+e->hit_box.bottom)/16) == 0x41) break;
			//}
		}
	} else { // Horizontal
		int16_t ey = sub_to_block(e->y);
		if(ey <= 0) { // Top OOB
			//e->alwaysActive = TRUE;
			e->type = OBJ_TRIGGER_SPECIAL;
			e->eflags &= ~NPC_OPTION1;
		} else if(ey >= stageHeight - 1) { // Bottom OOB
			//e->alwaysActive = TRUE;
			e->type = OBJ_TRIGGER_SPECIAL;
			e->eflags |= NPC_OPTION1;
		} else {  // Not OOB
			e->hit_box.top = 4; e->hit_box.bottom = 0;
			// Don't expand immediately so Sisters skip works
			e->hit_box.left = 1; e->hit_box.right = 1;
			//for(; e->hit_box.left <= 240; e->hit_box.left += 16) {
			//	if(stage_get_block_type(((e->x>>CSF)-e->hit_box.left)/16, 
			//			(e->y>>CSF)/16) == 0x41) break;
			//}
			//for(; e->hit_box.right <= 240; e->hit_box.right += 16) {
			//	if(stage_get_block_type(((e->x>>CSF)+e->hit_box.right)/16, 
			//			(e->y>>CSF)/16) == 0x41) break;
			//}
		}
	}
}

// Triggers will activate on player's collision
// OPTION2 off: horizontal, on: vertical
void ai_trigger(Entity *e) {
	if(tscState) return;
	if(e->state == 0) {
		e->alwaysActive = TRUE;
		e->state++;
		e->timer = 0;
	} else if(e->state == 1) { // Expansion
		if(e->eflags & NPC_OPTION2) {
			if(blk(e->x, 0, e->y, -e->hit_box.top) != 0x41) e->hit_box.top += 2;
			if(blk(e->x, 0, e->y, e->hit_box.bottom) != 0x41) e->hit_box.bottom += 2;
			if(++e->timer >= 126) e->state++;
		} else {
			if(blk(e->x, -e->hit_box.left, e->y, 0) != 0x41) e->hit_box.left += 2;
			if(blk(e->x, e->hit_box.right, e->y, 0) != 0x41) e->hit_box.right += 2;
			if(++e->timer >= 126) e->state++;
		}
	}
	// Prevent getting stuck in the eggs in egg corridor
	//if((e->eflags & NPC_OPTION2) || player.y_speed < 0 || stageID != 0x02) {
	if(e->timer > 8 && entity_overlapping(&player, e)) tsc_call_event(e->event);
	//}
}

// Special H/V trigger logic for Outer Wall and Balcony
// OPTION2 off: horizontal, on: vertical
void ai_trigger_special(Entity *e) {
	if(tscState) return;
	if(e->eflags & NPC_OPTION2) { // Vertical
		if(e->eflags & NPC_OPTION1) {
			if(player.x > e->x) tsc_call_event(e->event); // Right
		} else {
			if(player.x < e->x) tsc_call_event(e->event); // Left
		}
	} else { // Horizontal
		if(e->eflags & NPC_OPTION1) {
			if(player.y > e->y) tsc_call_event(e->event); // Bottom
		} else {
			if(player.y < e->y) tsc_call_event(e->event); // Top
		}
	}
}

void ai_genericproj(Entity *e) {
	e->nflags ^= NPC_SHOOTABLE;
	ANIMATE(e, 4, 0,1);
	if(++e->timer > TIME(250) ||
			stage_get_block_type(sub_to_block(e->x), sub_to_block(e->y)) == 0x41) {
		effect_create_smoke(e->x >> CSF, e->y >> CSF);
		e->state = STATE_DELETE;
	} else {
		e->x += e->x_speed;
		e->y += e->y_speed;
	}
}

void ondeath_default(Entity *e) {
	e->state = STATE_DESTROY;
}

void ondeath_nodrop(Entity *e) {
	sound_play(e->deathSound, 5);
	effect_create_smoke(e->x >> CSF, e->y >> CSF);
	if(e->eflags & NPC_EVENTONDEATH) tsc_call_event(e->event);
	if(e->eflags & NPC_DISABLEONFLAG) system_set_flag(e->id, TRUE);
	e->state = STATE_DELETE;
}

void onspawn_teleIn(Entity *e) {
	e->x += pixel_to_sub(16);
	e->y += pixel_to_sub(8);
	e->hit_box = (bounding_box) { 8, 8, 8, 8 };
	e->display_box = (bounding_box) { 8, 8, 8, 8 };
}

void ai_teleIn(Entity *e) {
	switch(e->state) {
		case 0: // Appear
		{
			sound_play(SND_TELEPORT, 5);
			e->timer = 5*15;
			e->state++;
			e->grounded = TRUE;
		}
		case 1:
		{
			e->timer--;
			e->frame = e->timer / 5;
			e->x += (e->timer & 1) ? 0x200 : -0x200;
			if(!e->timer) {
				e->state++;
				e->grounded = FALSE;
			}
		}
		break;
		case 2: // Drop
		{
			e->x_next = e->x;
			e->y_next = e->y + e->y_speed;
			if((e->grounded = collide_stage_floor(e))) {
				e->state++;
				e->y_speed = 0;
			} else {
				e->y_speed += SPEED(0x40);
			}
			e->y = e->y_next;
		}
		break;
		case 3: break;
	}
}

void onspawn_teleOut(Entity *e) {
	e->y -= 32 << CSF;
	SNAP_TO_GROUND(e);
	// PAL was jumping too high here
	if(IS_PALSYSTEM) e->y_speed = -0x360;
	else e->y_speed = -SPEED(0x3E0);
}

void ai_teleOut(Entity *e) {
	switch(e->state) {
		case 0: // Hopping up
		{
			if(++e->timer >= TIME(20)) {
				e->state++;
				e->timer = 0;
				e->y_speed = 0;
				Entity *light = entity_find_by_type(OBJ_TELEPORTER_LIGHTS);
				if(light) light->state = 1;
				sound_play(SND_TELEPORT, 5);
			} else {
				e->y_speed += SPEED(0x43);
				e->y += e->y_speed;
			}
		}
		break;
		case 1: // Show teleport animation
		{
			e->timer++;
			e->frame = e->timer / 5;
			e->x += (e->timer & 1) ? 0x200 : -0x200;
			if(e->timer >= 5*15) {
				e->state++;
				e->hidden = TRUE;
				Entity *light = entity_find_by_type(OBJ_TELEPORTER_LIGHTS);
				if(light) light->state = 0;
			}
		}
		break;
		case 2: break;
	}
}

void onspawn_teleLight(Entity *e) {
	e->hidden = TRUE;
	e->x += pixel_to_sub(4);
	e->y += pixel_to_sub(4);
}

void ai_teleLight(Entity *e) {
	if(e->state) e->hidden = ++e->timer & 1;
	else e->hidden = TRUE;
}

void ai_player(Entity *e) {
	uint8_t collide = TRUE;
	if(!e->grounded) e->y_speed += SPEED(0x40);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		case 1:
		{
			e->x_speed = 0;
			e->frame = 0;
		}
		break;
		case 2:		// looking up
		case 3:
		{
			e->frame = 2;
		}
		break;
		case 10:	// he gets flattened
		{
			sound_play(SND_LITTLE_CRASH, 5);
			for(uint8_t i = 0; i < 4; i++) {
				effect_create_smoke(sub_to_pixel(e->x) - 16 + (random() % 32), 
					sub_to_pixel(e->y) - 16 + (random() % 32));
			}
			e->state++;
		}
		/* no break */
		case 11:
		{
			e->frame = 9;
		}
		break;
		case 20:	// he teleports away
		{
			e->timer = 0;
			sound_play(SND_TELEPORT, 5);
			e->state++;
			//Entity *new = entity_create(e->x, e->y, 0x6F, 0);
			//new->id = e->id;
			//new->event = e->event;
			//e->state = STATE_DELETE;
		}
		case 21:
		{
			e->hidden ^= 1;
			if(++e->timer > TIME(50)) {
				e->state = STATE_DELETE;
			}
		}
		break;
		case 50:	// walking
		{
			MOVE_X(SPEED(0x200));
			e->frame = 1;
		}
		break;
		// falling, upside-down (from good ending; Fall stage)
		case 60:
		{
			e->frame = 0;
			sprite_vflip(e->sprite[0], 1);
			e->grounded = FALSE;
		}
		break;
		case 80:	// face away
		{
			e->frame = 4;
		}
		break;
		// walking in place during credits
		case 99:
		case 100:
		case 101:
		case 102:
		{
			collide = FALSE;
			e->grounded = FALSE;
			if(++e->timer > TIME(25)) e->state = 103;
		}
		break;
		case 103:
		{
			collide = FALSE;
			e->grounded = TRUE;
			e->y_speed = 0;
			ANIMATE(e, 10, 0,1,0,2);
		}
		break;
	}
	if(collide) entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
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

void ai_sprinkler(Entity *e) {
	if (e->eflags & NPC_OPTION2) return;
	
	//if (++e->animtime & 1) e->frame ^= 1;
	// Make sure this is an odd number so half the drops will show at once
	if (++e->timer == 7) { 
		Entity *drop = entity_create(e->x, e->y, OBJ_WATER_DROPLET, 0);
		drop->x_speed = -SPEED(2 << CSF) + random() % SPEED(4 << CSF);
		drop->y_speed = -SPEED(3 << CSF) + random() % SPEED(1 << CSF);
		e->timer = 0;
	}
}

void ai_water_droplet(Entity *e) {
	e->y_speed += SPEED(0x20);
	if (e->y_speed > SPEED(0x5ff)) e->y_speed = SPEED(0x5ff);
	e->x += e->x_speed;
	e->y += e->y_speed;
	e->hidden ^= 1;
	if (++e->timer > 10) {
		uint8_t block = stage_get_block_type(sub_to_block(e->x), sub_to_block(e->y));
		if(block & BLOCK_WATER || (block & BLOCK_SOLID && !(block & BLOCK_SLOPE)))
			e->state = STATE_DELETE;
	}
}

void ai_chinfish(Entity *e) {
	switch(e->state) {
		case 0:
			e->state = 1;
			e->x_mark = e->x;
			e->y_mark = e->y;
			e->y_speed = 0x88;
		case 1:
			e->y_speed += (e->y > e->y_mark) ? -8:8;
			LIMIT_Y(0x100);
			e->y += e->y_speed;
			if (e->damage_time) e->frame = 1;
			else e->frame = 0;
		break;
	}
}


void ai_fireplace(Entity *e) {
	switch(e->state) {
		case 0:		// burn
			e->frame = 0;
			e->state = 1;
			e->hidden = 0;
		case 1:
			ANIMATE(e, 8, 0,1,2);
		break;
		
		case 10:	// extinguished by Jellyfish Juice
			e->state = 11;
			//effect(e->CenterX(), e->CenterY(), EFFECT_BOOMFLASH);
			SMOKE_AREA((e->x>>CSF) - 8, (e->y>>CSF) - 8, 16, 16, 2);
		case 11:
			e->hidden = 1;
		break;
	}
}

void ai_gunsmith(Entity *e) {
	if (e->eflags & NPC_OPTION2) {
		// Animate Zzz effect above head
		if(!e->timer) effect_create_misc(EFF_ZZZ, (e->x >> CSF) + 8, (e->y >> CSF) - 8);
		if(++e->timer > TIME(100)) e->timer = 0;
	} else {
		e->frame = 1;
		RANDBLINK(e, 2, 200);
	}
}

void ai_lifeup(Entity *e) {
	ANIMATE(e, 4, 0,1);
}

void ai_chest(Entity *e) {
	ANIMATE(e, 8, 0,1,2);
	ai_grav(e);
}

void ai_sparkle(Entity *e) {
	ANIMATE(e, 8, 0,1,2);
}

void ai_forcefield(Entity *e) {
	ANIMATE(e, 2, 0,1,2,3);
}

void onspawn_segalogo(Entity *e) {
	e->alwaysActive = TRUE;
	e->display_box = (bounding_box) { 48, 16, 48, 16 };
}

void onspawn_balrog_splash(Entity *e) {
	e->alwaysActive = TRUE;
	e->display_box = (bounding_box) { 20, 12, 20, 12 };
}

void ai_segalogo(Entity *e) {
	switch(e->state) {
		case 0: // Normal
		{
			if(++e->timer > 5) {
				e->timer = 0;
				if(++e->frame >= 10) {
					e->frame = 0;
				}
			}
		}
		break;
		case 1: // Being crushed
		{
			e->type++;
			e->oframe = 255;
			e->frame = 0;
			e->timer = 0;
			e->state++;
		}
		case 2:
		{
			if(++e->timer > 2) {
				e->timer = 0;
				if(e->frame == 7) {
					e->hidden = TRUE;
					e->state++;
				} else {
					e->frame++;
				}
			}
		}
		break;
		case 3: // Invisible
		break;
	}
}

void ai_balrog_splash(Entity *e) {
	enum Frame {
		STAND, GASP, DUCK, ARMSUP, BLINK, PAINED, SMILE, WORRY, 
		WALK1, WALK2, AWAY1, AWAY2, FLY1, FLY2
	};
	
	switch(e->state) {
		case 0: // Waiting above screen
		{
			if(++e->timer > TIME(50)) {
				e->timer = 0;
				e->state++;
				e->frame = ARMSUP;
			}
		}
		break;
		case 1: // Falling
		{
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5FF));
			e->y += e->y_speed;
			if(e->y >= (SCREEN_HALF_H - 24) << CSF) {
				e->y_speed = 0x320;
				e->state++;
				e->linkedEntity->state++;
				sound_play(SND_ENEMY_HURT, 5);
			}
		}
		break;
		case 2: // Crushing logo
		{
			e->y += e->y_speed;
			if(e->y >= (SCREEN_HALF_H + 4) << CSF) {
				e->state++;
				e->frame = DUCK;
			}
		}
		break;
		case 3: // Hit ground
		{
			if(++e->timer > TIME(25)) {
				e->timer = 0;
				e->state++;
				e->frame = STAND;
			}
		}
		break;
		case 4: // Standing
		{
			if(++e->timer > TIME(50)) {
				e->state++;
				e->frame = SMILE;
			}
		}
		break;
		case 5: // Smiling
		break;
	}
}

// this object is used in a few places, such as during the Red Demon fight (last cave),
// and during some of the end sequences. It seems to be primarily involved with providing
// "extra" map scrolling modes. Generally you'll <FON on it, then set the mode you desire.
void ai_scroll_ctrl(Entity *e) {
	switch(e->state) {
		// stay above player's head. This is used during the "mad run" Balcony2 stage;
		// you'll notice there is not normal scrolling during this part.
		case 10:
		{
			e->x = player.x;
			e->y = player.y - (32 << CSF);
		}
		break;
		// pan in the specified direction. used when you get the good ending
		// to pan over all the scenes from the island just before it crashes.
		case 20:
		{
			switch(e->dir) {
				case LEFT:	e->x -= (2 << CSF); break;
				case UP:	e->y -= (2 << CSF); break;
				case RIGHT: e->x += (2 << CSF); break;
				case DOWN:	e->y += (2 << CSF); break;
			}
			
			// player is invisible during this part. dragging him along is
			// what makes all the monsters, falling spikes etc react.
			player.x = e->x;
			player.y = e->y;
		}
		break;
		
		// stay below player.
		case 30:
		{
			e->x = player.x;
			e->y = player.y + (80 << CSF);
		}
		break;
		
		// stay mid-way between player and the specified object.
		// used during the Red Demon fight in Last Cave (hidden).
		case 100:
		{
			e->state = 101;
			if (e->eflags & NPC_OPTION2) {
				e->linkedEntity = entity_find_by_event(e->id);
			} else {
				e->linkedEntity = bossEntity;
			}
			//if (!e->linkedEntity) e->state = STATE_DELETE;
		}
		case 101:
		{
			if(e->linkedEntity) {
				if(e->linkedEntity->state == STATE_DELETE) {
					e->linkedEntity = NULL;
				} else {
					e->x = (player.x + e->linkedEntity->x) >> 1;
					e->y = (player.y + e->linkedEntity->y) >> 1;
				}
			} else { // Fallback to follow player in case something went wrong
				e->x = player.x;
				e->y = player.y;
			}
		}
		break;
	}
}

// Makes the screen constantly shake
void ai_quake(Entity *e) {
	(void)(e); // So we don't trip unused parameter warning
	if(!cfg_hellquake && (stageID == 0x50 || stageID == 0x51 || stageID == 0x52
						  || stageID == 0x58)) {
		return;
	}
	camera_shake(10);
}

void ai_xp_capsule(Entity *e) {
	ANIMATE(e, 4, 0,1);
	
	if (e->health < 100) {
		Entity *exp = entity_create(e->x, e->y, OBJ_XP,
				e->id > 6 ? NPC_OPTION2 : 0);
		exp->experience = e->id;
		effect_create_smoke(e->x, e->y);
		sound_play(SND_FUNNY_EXPLODE, 5);
		
		e->state = STATE_DELETE;
	}
}

void onspawn_lightning(Entity *e) {
	e->alwaysActive = TRUE;
	e->hit_box = (bounding_box) { 8, 8, 8, 8 };
	e->display_box = (bounding_box) { 8, 23*8, 8, 8 };
	e->x -= 32 << CSF;
	if(stageID == 0x1C) e->x -= 40 << CSF; // Balrog is a bit farther away
	SNAP_TO_GROUND(e);
}

void ai_lightning(Entity *e) {
	e->animtime++;
	if(e->animtime > 5) {
		SMOKE_AREA((e->x >> CSF) - 16, (e->y >> CSF), 32, 16, 2);
		e->animtime = 0;
		e->frame++;
		if(e->frame > 3) e->state = STATE_DELETE;
	}
}

void onspawn_lvlupdn(Entity *e) {
	e->alwaysActive = TRUE;
	e->timer = TIME(100);
	e->y_speed = -0x300;
	e->display_box.left = 28;
}

void ai_lvlupdn(Entity *e) {
	if(!--e->timer) {
		e->state = STATE_DELETE;
	} else {
		if((e->timer & 3) == 0) e->frame ^= 1;
		if(e->y_speed < 0) e->y_speed += 0x10;
		e->y += e->y_speed;
	}
}
