#include "ai_common.h"

void ai_null(Entity *e) { (void)(e); }

void onspawn_snap(Entity *e) {
	SNAP_TO_GROUND(e);
}

void onspawn_op2flip(Entity *e) {
	if(e->flags & NPC_OPTION2) e->dir = 1;
}

void onspawn_snapflip(Entity *e) {
	if(e->flags & NPC_OPTION2) e->dir = 1;
	SNAP_TO_GROUND(e);
}

void onspawn_op2frame(Entity *e) {
	if(e->flags & NPC_OPTION2) e->frame = 1;
}

void onspawn_op2snap(Entity *e) {
	if(e->flags & NPC_OPTION2) {
		SNAP_TO_GROUND(e);
	}
}

void onspawn_pushup(Entity *e) {
	e->y -= pixel_to_sub(16);
}

void onspawn_blackboard(Entity *e) {
	e->y -= 16<<CSF;
	if(e->flags & NPC_OPTION2) e->frame = 1;
}

void onspawn_persistent(Entity *e) {
	e->alwaysActive = TRUE;
}

// Needed for save point after sisters fight
void onspawn_interactive(Entity *e) {
	if(e->type == OBJ_SAVE_POINT && system_get_flag(FLAG_DISABLESAVE)) {
		e->state = STATE_DELETE;
	} else {
		e->flags |= NPC_INTERACTIVE;
	}
}

// Spikes use a second frame for 90 degree rotation
// In the actual game, option 1 & 2 are used for this, but whatever
void onspawn_spike(Entity *e) {
	if(g_stage.id == STAGE_LABYRINTH_M) {
		// Disable sprite in Labyrinth M
		// The map has a brown version overlapping us so it's pointless
		e->hidden = TRUE;
		return;
	}
	
	uint16_t x = sub_to_block(e->x), y = sub_to_block(e->y);
	if(stage_get_block_type(x, y+1) == 0x41) { // Solid on bottom
	} else if(stage_get_block_type(x, y-1) == 0x41) { // Solid on top
		sprite_vflip(&e->sprite[0], TRUE);
	} else if(stage_get_block_type(x-1, y) == 0x41) { // Solid on left
		e->frame = 1;
	} else if(stage_get_block_type(x+1, y) == 0x41) { // Solid on right
		e->frame = 1;
		e->dir = 1;
	}
}

// The big spike, collision box is too high
void onspawn_trap(Entity *e) {
	e->hit_box.top -= 2;
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
		e->y_speed += SPEED_8(0x40);
		e->grounded = collide_stage_floor(e);
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

// Type 0, used for many invisible things that are sometimes useful.
// Outer Wall and Balcony use it as a kill plane when you fall into the void.
// Hell B3 also apparently had a hidden trigger to warp to the boss too.
void ai_nothing(Entity *e) {
	if(g_stage.id == STAGE_HELL_B3 && e->event == 302) {
		e->state = STATE_DELETE;
		return;
	}
	if(!e->state) {
		e->state = 1;
		if(e->flags & NPC_OPTION1) {
			e->type = OBJ_HVTRIGGER;
            e->onFrame = npc_info[OBJ_HVTRIGGER].onFrame;
			e->flags &= ~(NPC_OPTION1 | NPC_OPTION2);
			e->hit_box.top = 4; e->hit_box.bottom = 0;
			e->hit_box.left = 128; e->hit_box.right = 128;
		}
	}
}

void onspawn_trigger(Entity *e) {
	e->alwaysActive = TRUE;
	if(e->flags & NPC_OPTION2) { // Vertical
		// First test if the trigger is placed outside of the map, if so change the type
		// to OOB trigger which will activate based on player position instead of collision
		uint16_t ex = sub_to_block(e->x);
		if(ex <= 0) { // Left OOB
			e->type = OBJ_TRIGGER_SPECIAL;
            e->onFrame = npc_info[OBJ_TRIGGER_SPECIAL].onFrame;
			e->flags &= ~NPC_OPTION1;
		} else if(ex >= g_stage.pxm.width - 1) { // Right OOB
			e->type = OBJ_TRIGGER_SPECIAL;
            e->onFrame = npc_info[OBJ_TRIGGER_SPECIAL].onFrame;
			e->flags |= NPC_OPTION1;
		} else { // Not OOB
			e->hit_box.left = 2; e->hit_box.right = 2;
			// Don't expand immediately
			e->hit_box.top = 1; e->hit_box.bottom = 1;
		}
	} else { // Horizontal
		uint16_t ey = sub_to_block(e->y);
		if(ey <= 0) { // Top OOB
			e->type = OBJ_TRIGGER_SPECIAL;
            e->onFrame = npc_info[OBJ_TRIGGER_SPECIAL].onFrame;
			e->flags &= ~NPC_OPTION1;
		} else if(ey >= g_stage.pxm.height - 1) { // Bottom OOB
			e->type = OBJ_TRIGGER_SPECIAL;
            e->onFrame = npc_info[OBJ_TRIGGER_SPECIAL].onFrame;
			e->flags |= NPC_OPTION1;
		} else {  // Not OOB
			e->hit_box.top = 4; e->hit_box.bottom = 0;
			// Don't expand immediately so Sisters skip works
			e->hit_box.left = 1; e->hit_box.right = 1;
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
		if(e->flags & NPC_OPTION2) {
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
	if(e->flags & NPC_OPTION2) { // Vertical
		if(e->flags & NPC_OPTION1) {
			if(player.x > e->x) tsc_call_event(e->event); // Right
		} else {
			if(player.x < e->x) tsc_call_event(e->event); // Left
		}
	} else { // Horizontal
		if(e->flags & NPC_OPTION1) {
			if(player.y > e->y) tsc_call_event(e->event); // Bottom
		} else {
			if(player.y < e->y) tsc_call_event(e->event); // Top
		}
	}
}

void ai_genericproj(Entity *e) {
	e->flags ^= NPC_SHOOTABLE;
	if((++e->animtime & 3) == 0) e->frame ^= 1;
	if(++e->timer > TIME_8(250) || blk(e->x, 0, e->y, 0) == 0x41) {
		effect_create(EFF_DISSIPATE, e->x >> CSF, e->y >> CSF, FALSE);
		Effect *smk = effect_create(EFF_SMOKE, e->x >> CSF, e->y >> CSF, FALSE);
		if(smk) {
			smk->x_speed = -(e->x_speed >> CSF);
			smk->y_speed = -(e->y_speed >> CSF);
		}
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
	effect_create(EFF_SMOKE, e->x >> CSF, e->y >> CSF, FALSE);
	if(e->flags & NPC_EVENTONDEATH) tsc_call_event(e->event);
	if(e->flags & NPC_DISABLEONFLAG) system_set_flag(e->id, TRUE);
	e->state = STATE_DELETE;
}

void ai_teleIn(Entity *e) {
	uint32_t buf[2*8];
	const uint8_t frame_off = (playerEquipment & EQUIP_MIMIMASK) ? 10 : 0;
	switch(e->state) {
		case 0:
			e->state = 1;
			e->frame = 0 + frame_off;
			e->oframe = e->frame; // Don't draw frame
			e->hidden = TRUE;
			e->timer = 0;
			e->hit_box = (bounding_box) {{ 8, 8, 8, 8 }};
			e->display_box = (bounding_box) {{ 8, 8, 8, 8 }};
			e->x += 16 * 0x200;
			e->y += 8 * 0x200;
			sound_play(29, 5);
			break;
		case 1:
			if(e->timer == 0) {
				// Overwirte frame with full transparency and unhide
				dma_now(DmaVRAM, (uint32_t)BlankData, e->vramindex << 5, 16*4, 2);
				e->hidden = FALSE;
			}
			if(++e->timer == TIME(64)) {
				e->state = 2;
				e->timer = 0;
			} else if(e->timer2 < 16) {
                if(++e->animtime > TIME(3)) {
                    e->animtime = 0;
                    // Pull two tiles from the sprite to overwrite
                    const uint32_t *src = SPR_Quote.tilesets[e->frame]->tiles + (e->timer2 & 8);
                    uint16_t dst_index = e->vramindex + (e->timer2 > 7 ? 1 : 0);
                    // Erase a portion (extends down over time)
                    uint16_t y = 0;
                    for(; y <= (e->timer2 & 7); y++) {
                        buf[y] = src[y];
                        buf[y+8] = src[y+16];
                    }
                    for(; y < 8; y++) {
                        buf[y] = 0;
                        buf[y+8] = 0;
                    }
                    // Inject into top or bottom half of sprite tiles in vram
                    uint16_t vaddr = dst_index << 5;

                    dma_now(DmaVRAM, (uint32_t)&buf[0], vaddr, 16, 2);
                    dma_now(DmaVRAM, (uint32_t)&buf[8], vaddr+64, 16, 2);

                    e->timer2++;
                }
            }
			break;

		case 2:
			if(++e->timer > TIME(20)) {
				e->state = 3;
				e->frame = 1 + frame_off;
				e->hit_box.bottom = 8;
				e->grounded = FALSE;
			}
			break;

		case 3:
			moveMeToFront = TRUE;
			e->y_speed += SPEED(0x40);
			e->y += e->y_speed;
			e->y_next = e->y;
			if(blk(e->x, 0, e->y, 8) == 0x41) {
				e->y = e->y_next;
				e->state = 4;
				e->timer = 0;
				e->frame = 0 + frame_off;
			}
			break;
	}
	// Force a specific direction
	switch(g_stage.id) {
		case STAGE_ARTHURS_HOUSE:
		case STAGE_EGG_CORRIDOR:
		case STAGE_GRASSTOWN:
		case STAGE_LABYRINTH_M:
		case STAGE_EGG_CORRIDOR_2:
			e->dir = 1;
		break;
		case STAGE_GRASSTOWN_SHELTER:
		case STAGE_SAND_ZONE:
		case STAGE_SAND_ZONE_2:
		case STAGE_LABYRINTH_A:
			e->dir = 0;
		break;
	}
}

void ai_teleOut(Entity *e) {
	uint32_t buf[2*8];
	const uint8_t frame_off = (playerEquipment & EQUIP_MIMIMASK) ? 10 : 0;
    switch(e->state) {
		case 0:
			e->state = 1;
			e->dir = player.dir;
			e->frame = 0 + frame_off;
			e->x = player.x;
			e->y = player.y; //-= 16 << CSF;
			e->hit_box = (bounding_box) {{ 8, 8, 8, 8 }};
			e->display_box = (bounding_box) {{ 8, 8, 8, 8 }};
			break;

		case 1: // Wait a bit
			if(++e->timer > TIME(20)) {
				e->timer = 0;
				e->state = 2;
				e->frame = 1 + frame_off;
				e->y_speed = -SPEED(0x300);
				e->grounded = FALSE;
			}
			break;

		case 2: // Jump up to the teleporter
            e->y_speed += SPEED(0x40);
            e->y += e->y_speed;
			if(e->y_speed > 0 && blk(e->x, 0, e->y, 16) == 0x41) {
				e->y -= 0x200;
				e->grounded = TRUE;
				e->y_speed = 0;
				e->state = 3;
				e->timer = 0;
				e->frame = 0 + frame_off;
			}
			break;

		case 3: // Wait before teleporter starts
			if(++e->timer > TIME(40)) {
				e->state = 4;
				e->timer = TIME(65);
                e->timer2 = 0;
                e->animtime = 0;
				sound_play(29, 5);
			}
			break;

		case 4:
			if(--e->timer == 0) {
				e->state = STATE_DELETE;
            } else if(e->timer2 < 16) {
                if(++e->animtime > TIME(3)) {
                    e->animtime = 0;
                    // Pull two tiles from the sprite to overwrite
                    const uint32_t *src = SPR_Quote.tilesets[e->frame]->tiles + (e->timer2 & 8);
                    uint16_t dst_index = e->vramindex + (e->timer2 > 7 ? 1 : 0);
                    // Erase a portion (extends down over time)
                    uint16_t y = 0;
                    for(; y <= (e->timer2 & 7); y++) {
                        buf[y] = 0;
                        buf[y+8] = 0;
                    }
                    for(; y < 8; y++) {
                        buf[y] = src[y];
                        buf[y+8] = src[y+16];
                    }
                    // Inject into top or bottom half of sprite tiles in vram
                    uint16_t vaddr = dst_index << 5;

                    dma_now(DmaVRAM, (uint32_t)&buf[0], vaddr, 16, 2);
                    dma_now(DmaVRAM, (uint32_t)&buf[8], vaddr+64, 16, 2);

                    e->timer2++;
                }
            }
			break;
	}
}

// 022: Teleporter
void ai_teleporter(Entity *e) {
	if(e->state) {
		e->frame ^= 1;
		if(!e->timer++) moveMeToFront = TRUE;
	} else {
		e->frame = 0;
		e->timer = 0;
	}
}

// 023: Teleporter Lights
void ai_teleLight(Entity *e) {
	moveMeToFront = TRUE;
    if((++e->animtime & 1) == 0) {
        if(++e->frame > 7) e->frame = 0;
    }
}

void ai_player(Entity *e) {
	uint8_t collide = TRUE;
	if(!e->grounded) e->y_speed += SPEED_8(0x40);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		{
			if(g_stage.id == STAGE_SEAL_CHAMBER_2) e->dir = 1;
			e->state++;
		} /* fallthrough */
		case 1:
		{
			e->x_speed = 0;
			e->y_speed = 0;
			e->frame = 0;
			e->grounded = TRUE;
			collide = FALSE;
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
				effect_create(EFF_SMOKE, sub_to_pixel(e->x) - 16 + (rand() & 31),
					sub_to_pixel(e->y) - 16 + (rand() & 31), FALSE);
			}
			e->state++;
		}
		/* fallthrough */
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
		} /* fallthrough */
		case 21:
		{
			e->hidden = (++e->timer & 2);
			if(e->timer > TIME_8(50)) {
				e->state = STATE_DELETE;
			}
		}
		break;
		case 50:	// walking
		{
			MOVE_X(SPEED_10(0x200));
			e->frame = 1;
		}
		break;
		// falling, upside-down (from good ending; Fall stage)
		case 60:
		{
			e->frame = 0;
			e->hidden = FALSE;
			sprite_vflip(&e->sprite[0], 1);
			e->grounded = FALSE;
			collide = FALSE;
			if(e->y_speed > SPEED_10(0x1FF)) e->y_speed = SPEED_10(0x1FF);
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
			if(++e->timer > TIME_8(25)) e->state = 103;
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
	if((++e->animtime & 3) == 0) if(++e->frame > 2) e->frame = 1;
}

void ai_savepoint(Entity *e) {
	if((++e->animtime & 3) == 0) if(++e->frame > 7) e->frame = 0;
	ai_grav(e);
}

void ai_refill(Entity *e) {
	e->frame = (++e->animtime & 2) >> 1;
	ai_grav(e);
}

void ai_sprinkler(Entity *e) {
	if (e->flags & NPC_OPTION2) return;
	
	//if (++e->animtime & 1) e->frame ^= 1;
	// Make sure this is an odd number so half the drops will show at once
	if (++e->timer == 7) { 
		Entity *drop = entity_create(e->x, e->y, OBJ_WATER_DROPLET, 0);
		drop->x_speed = -0x3FF + (rand() & 0x7FF);
		drop->y_speed = -0x6FF + (rand() & 0x3FF);
		e->timer = 0;
	}
}

void ai_water_droplet(Entity *e) {
	if(e->x_speed > 0) {
		e->x_speed -= 4;
	} else {
		e->x_speed += 4;
	}
	if(e->y_speed < 0x5E0) e->y_speed += 0x20;
	e->x += e->x_speed;
	e->y += e->y_speed;
	//e->hidden ^= 1;
	e->frame = rand() & 3;
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
			e->y_mark = e->y;
			e->y_speed = SPEED(0x80);
			// Fallthrough
		case 1:
			if(e->y_mark < e->y) {
				e->y_speed -= SPEED(8);
			} else {
				e->y_speed += SPEED(8);
			}
			break;
	}

	if(e->damage_time) {
		e->frame = 2;
	} else {
		if(++e->animtime > 4) {
			e->animtime = 0;
			if(++e->frame > 1) e->frame = 0;
		}
	}

	e->x += e->x_speed;
	e->y += e->y_speed;
}


void ai_fireplace(Entity *e) {
	switch(e->state) {
		case 0:		// burn
			e->frame = 0;
			e->state = 1;
			e->hidden = 0;
			/* fallthrough */
		case 1:
			ANIMATE(e, 4, 0,1,2,3);
		break;
		
		case 10:	// extinguished by Jellyfish Juice
			e->state = 11;
			//effect(e->CenterX(), e->CenterY(), EFFECT_BOOMFLASH);
			SMOKE_AREA((e->x>>CSF) - 8, (e->y>>CSF) - 8, 16, 16, 2);
			/* fallthrough */
		case 11:
			e->hidden = 1;
		break;
	}
}

void ai_gunsmith(Entity *e) {
	if (e->flags & NPC_OPTION2) {
		// Animate Zzz effect above head
		if(!e->timer) effect_create(EFF_ZZZ, (e->x >> CSF) + 8, (e->y >> CSF) - 8, FALSE);
		if(++e->timer > TIME_8(100)) e->timer = 0;
	} else {
		e->frame = 1;
		RANDBLINK(e, 2, 200);
	}
}

void ai_lifeup(Entity *e) {
	ANIMATE(e, 4, 0,1);
}

void ai_chest(Entity *e) {
	e->timer++;
	switch(e->timer) {
		case 80: e->frame = 1; break;
		case 88: e->frame = 2; break;
		case 96: e->frame = 0; e->timer = rand() & 0x3F; break;
	}
	ai_grav(e);
}

void ai_sparkle(Entity *e) {
	ANIMATE(e, 4, 0,1,2,3);
}

void ai_forcefield(Entity *e) {
	ANIMATE(e, 2, 0,1,2,3);
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
			e->y = player.y - pixel_to_sub(32);
		}
		break;
		// pan in the specified direction. used when you get the good ending
		// to pan over all the scenes from the island just before it crashes.
		case 20:
		{
			switch(e->dir) {
				case LEFT:	e->x -= pixel_to_sub(2); break;
				case UP:	e->y -= pixel_to_sub(2); break;
				case RIGHT: e->x += pixel_to_sub(2); break;
				case DOWN:	e->y += pixel_to_sub(2); break;
			}
			
			// player is invisible during this part. dragging him along is
			// what makes all the monsters, falling spikes etc react.
			player.x = e->x;
			player.y = e->y;
			player.y_speed = 0;
		}
		break;
		
		// stay below player.
		case 30:
		{
			e->x = player.x;
			e->y = player.y + pixel_to_sub(80);
		}
		break;
		
		// stay mid-way between player and the specified object.
		// used during the Red Demon fight in Last Cave (hidden).
		// every phase of Ballos also uses this.
		case 100:
		{
			// The real game uses the dir parameter of ANP to tell this object the event # 
			// of the target. It's too late to change how my engine works to really handle
			// that, so here are some case-by-case hacks to fix it
			uint16_t target = e->id;
			if(g_stage.id == STAGE_SEAL_CHAMBER) {
				if(bossEntity) {
					target = 0;
				} else {
					target = 900;
				}
			} else if(g_stage.id == STAGE_LAST_CAVE_2) {
				target = 250;
			}
			if (target) {
				e->linkedEntity = entity_find_by_event(target);
			} else {
				e->linkedEntity = bossEntity;
			}
			e->state++;
		} /* fallthrough */
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
	if(!cfg_hellquake && (g_stage.id == STAGE_HELL_B1 || 
						  g_stage.id == STAGE_HELL_B2 || 
						  g_stage.id == STAGE_HELL_B3 || 
						  g_stage.id == STAGE_SEAL_CHAMBER)) {
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
		effect_create(EFF_SMOKE, e->x, e->y, FALSE);
		effect_create(EFF_DISSIPATE, e->x >> CSF, e->y >> CSF, FALSE);
		sound_play(SND_FUNNY_EXPLODE, 5);
		
		e->state = STATE_DELETE;
	}
}

void onspawn_lightning(Entity *e) {
	e->alwaysActive = TRUE;
	e->hit_box = (bounding_box) {{ 6, 22*8, 6, 8 }};
	e->display_box = (bounding_box) {{ 8, 23*8, 8, 8 }};
	if(g_stage.id != STAGE_SEAL_CHAMBER) e->x -= pixel_to_sub(32);
	if(g_stage.id == STAGE_GRASSTOWN_GUM) e->x -= pixel_to_sub(40); // Balrog is a bit farther away
	SNAP_TO_GROUND(e);
}

void ai_lightning(Entity *e) {
	e->animtime++;
	if(e->animtime > TIME_8(5)) {
		if(e->flags & NPC_OPTION2) e->attack = 10;
		SMOKE_AREA((e->x >> CSF) - 16, (e->y >> CSF), 32, 16, 2);
		e->animtime = 0;
		e->frame++;
		if(e->frame > 3) e->state = STATE_DELETE;
	}
}

void onspawn_lvlupdn(Entity *e) {
	e->alwaysActive = TRUE;
	e->timer = TIME_8(100);
	e->y_speed = -SPEED_10(0x300);
	e->display_box.left = 28;
}

void ai_lvlupdn(Entity *e) {
	if(!--e->timer) {
		e->state = STATE_DELETE;
	} else {
		if((e->timer & 3) == 0) e->frame ^= 1;
		if(e->y_speed < 0) e->y_speed += SPEED_8(0x10);
		e->y += e->y_speed;
	}
}

// misery/balrog in bubble
void ai_intro_kings(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->state++;
			if(!(e->flags & NPC_OPTION2)) {
				e->frame = 1;
				e->timer = TIME_8(25);
				e->y -= 0x640;
			}
		} /* fallthrough */
		case 1:
		{
			if(++e->timer >= TIME_8(50)) {
				e->timer = 0;
				e->timer2 ^= 1;
			}
			if(e->timer2) {
				e->y += SPEED_8(0x40);
			} else {
				e->y -= SPEED_8(0x40);
			}
		}
		break;
	}
}

// demon crown on throne
void ai_intro_crown(Entity *e) {
	switch(e->state) {
		case 0:
		{
			//e->x += pixel_to_sub(8);
			e->y += pixel_to_sub(13-8);
			e->state++;
		} /* fallthrough */
		case 1:
		{
			e->timer++;
			if(e->timer > TIME(55) && (e->timer & 7) == 1) {
				const int8_t offx = -8 + (rand() & 15);
			    Effect *eff = effect_create(EFF_FANU, (e->x>>CSF) + offx, (e->y>>CSF) + 8, FALSE);
				if(eff) eff->y_speed = -2;
			}
		}
		break;
	}
}


void ai_intro_doctor(Entity *e) {
	switch(e->state) {
		case 0:
		{
			//e->x += pixel_to_sub(8);
			e->y -= pixel_to_sub(8);
			e->state++;
		} /* fallthrough */
		case 1:
		{
			e->frame = 0;
		}
		break;
		
		case 10:	// chuckle; facing screen
		{
			e->state = 11;
			e->frame = 0;
			e->animtime = 0;
			e->timer2 = 0;
		} /* fallthrough */
		case 11:
		{
			if(++e->animtime > 7) {
			    e->animtime = 0;
				if(++e->frame > 1) {
					e->frame = 0;
					if(++e->timer2 > 9) e->state = 1;
				}
			}
		}
		break;
		
		case 20:	// walk
		{
			e->state++;
			e->frame = 2;
			e->animtime = 0;
		} /* fallthrough */
		case 21:
		{
			ANIMATE(e, 10, 2,0,3,0);
			e->x += SPEED_8(0xFF);
		}
		break;
		
		case 30:	// face away
		{
			e->frame = 4;
			e->state++;
		}
		break;
		
		case 40:	// chuckle; facing away
		{
			e->state++;
			e->frame = 4;
			e->animtime = 0;
			e->timer2 = 0;
		} /* fallthrough */
		case 41:
		{
			if(++e->animtime > 7) {
                e->animtime = 0;
				if(++e->frame > 5) {
					e->frame = 4;
					if(++e->timer2 > 9) e->state = 30;
				}
			}
		}
		break;
	}
}

void ai_drip_generator(Entity *e) {
    if((rand() & 127) == 0) {
        entity_create(e->x, e->y - 0x1800, OBJ_WATER_DROPLET, 0);
    }
}
