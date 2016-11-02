#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"
#include "sprite.h"

void ai_null(Entity *e) {}

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
		e->frame = 1;
	} else if(stage_get_block_type(x+1, y) == 0x41) { // Solid on right
		e->frame = 1;
		e->dir = 1;
	}
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

// Triggers will activate on player's collision
// OPTION2 off: horizontal, on: vertical
void ai_trigger(Entity *e) {
	if(tscState) return;
	if(!e->state) {
		e->alwaysActive = TRUE;
		e->state = 1;
		if(e->eflags&NPC_OPTION2) { // Vertical
			e->hit_box.left = 2; e->hit_box.right = 2;
			for(; e->hit_box.top <= 240; e->hit_box.top += 16) {
				if(stage_get_block_type((e->x>>CSF)/16, 
						((e->y>>CSF)-e->hit_box.top)/16) == 0x41) break;
			}
			for(; e->hit_box.bottom <= 240; e->hit_box.bottom += 16) {
				if(stage_get_block_type((e->x>>CSF)/16, 
						((e->y>>CSF)+e->hit_box.bottom)/16) == 0x41) break;
			}
		} else { // Horizontal
			e->hit_box.top = 4; e->hit_box.bottom = 0;
			for(; e->hit_box.left <= 240; e->hit_box.left += 16) {
				if(stage_get_block_type(((e->x>>CSF)-e->hit_box.left)/16, 
						(e->y>>CSF)/16) == 0x41) break;
			}
			for(; e->hit_box.right <= 240; e->hit_box.right += 16) {
				if(stage_get_block_type(((e->x>>CSF)+e->hit_box.right)/16, 
						(e->y>>CSF)/16) == 0x41) break;
			}
		}
	}
	// Prevent getting stuck in the eggs in egg corridor
	if((e->eflags & NPC_OPTION2) || player.y_speed < 0 || stageID != 0x02) {
		if(entity_overlapping(&player, e)) tsc_call_event(e->event);
	}
}

void ai_genericproj(Entity *e) {
	ANIMATE(e, 4, 0,1);
	Bullet *b = bullet_colliding(e);
	if(b) {
		b->ttl = 0;
		effect_create_smoke(e->x >> CSF, e->y >> CSF);
		e->state = STATE_DELETE;
	} else if(++e->timer > TIME(250) ||
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

void onspawn_teleIn(Entity *e) {
	e->x += pixel_to_sub(16);
	e->y += pixel_to_sub(8);
}

void ai_teleIn(Entity *e) {
	switch(e->state) {
		case 0: // Appear
		{
			sound_play(SND_TELEPORT, 5);
			e->timer = 5*15;
			e->state++;
		}
		case 1:
		{
			e->timer--;
			e->frame = e->timer / 5;
			e->x += e->timer & 1 ? 0x200 : -0x200;
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
	e->y -= pixel_to_sub(32);
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
			e->x += e->timer & 1 ? 0x200 : -0x200;
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
			for(u8 i = 0; i < 4; i++) {
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
			Entity *new = entity_create(e->x, e->y, 0x6F, 0);
			new->id = e->id;
			new->event = e->event;
			e->state = STATE_DELETE;
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
			ANIMATE(e, 16, 0,1,0,2);
		}
		break;
	}
	entity_update_collision(e);
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
		u8 block = stage_get_block_type(sub_to_block(e->x), sub_to_block(e->y));
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
