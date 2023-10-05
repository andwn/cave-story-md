#include "ai_common.h"

#define CAI_INIT			20			// ANP'd to this by the entry script in Lab M
#define CAI_START			21			// ANP'd to this by Almond script
#define CAI_RUNNING			22
#define CAI_KNOCKEDOUT		40			// knocked out at beginning of Almond battle
#define CAI_ACTIVE			99

enum { STAND, WALK1, WALK2, LOOKUP, UPWALK1, UPWALK2, LOOKDN, JUMPDN };

uint8_t curly_mgun = 0;
uint8_t curly_watershield = 0;
uint8_t curly_impjump = 0;
uint8_t curly_reachptimer = 0;
uint8_t curly_look = 0;
uint16_t curly_tryjumptime = 0;

uint16_t curly_target_time;
int32_t curly_target_x, curly_target_y;

static void CaiJUMP(Entity *e) {
	if (e->grounded) {
		moveMeToFront = TRUE;
		e->y_speed = -SPEED(0x300) - (rand() & 0x2FF);
		e->y_next -= SPEED(0x300);
		e->grounded = FALSE;
		e->frame = WALK2;
		sound_play(SND_PLAYER_JUMP, 5);
	}
}

// curly that fights beside you
void ai_curly_ai(Entity *e) {
	int32_t xdist, ydist;
	uint8_t reached_p;
	uint16_t otiley;
	uint8_t seeking_player = 0;
	uint8_t wantdir;

	// Spawn air tank if it doesn't exist yet
	if (!curly_watershield) {
		Entity *shield = entity_create(e->x, e->y, OBJ_CAI_WATERSHIELD, 0);
		shield->alwaysActive = TRUE;
		shield->linkedEntity = e;
		curly_watershield = 1;
	}
	
	switch(e->state) {
		case 0:
		{
			e->alwaysActive = TRUE;
			e->x_speed = 0;
			e->y_speed += SPEED(0x1F);
		}
		break;
		case CAI_INIT:			// set to this by an ANP in Maze M
		{
			e->alwaysActive = TRUE;
			e->flags |= NPC_IGNORE44;
			e->x = player.x;
			e->y = player.y;
		}
		/* fallthrough */
		case CAI_START:			// set here after she stops being knocked out in Almond
		{
			e->alwaysActive = TRUE;
			e->x_mark = e->x;
			e->y_mark = e->y;
			e->dir = player.dir;
			e->state = CAI_ACTIVE;
			e->timer = 0;
			// If we traded Curly for machine gun she uses the polar star
			curly_mgun = !player_has_weapon(WEAPON_MACHINEGUN);
			// spawn her gun
			Entity *gun = entity_create(e->x, e->y, OBJ_CAI_GUN + curly_mgun, 0);
			gun->alwaysActive = TRUE;
			gun->linkedEntity = e;
            gun->display_box = (bounding_box) {{ 12, 4, 12, 4 }};
			moveMeToFront = TRUE;
		}
		break;
		case CAI_KNOCKEDOUT:
		{
			e->timer = 0;
			e->state = CAI_KNOCKEDOUT+1;
			e->frame = 9;
		}
		/* fallthrough */
		case CAI_KNOCKEDOUT+1:
		{
			if (++e->timer > TIME(1000)) {	// start fighting
				e->state = CAI_START;
			} else if (e->timer > TIME(750)) {	// stand up
				e->flags &= ~NPC_INTERACTIVE;
				e->frame = 0;
			}
		}
		break;
	}
	// Check collision up front and remember the result
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(e->y_speed < 0) collide_stage_ceiling(e);
	if(e->grounded) e->grounded = collide_stage_floor_grounded(e);
	else e->grounded = collide_stage_floor(e);
	
	uint8_t blockl = collide_stage_leftwall(e);
	uint8_t blockr = collide_stage_rightwall(e);
	
	// Handle underwater
	if((stage_get_block_type(sub_to_block(e->x), sub_to_block(e->y)) & BLOCK_WATER) ||
			(water_entity && e->y > water_entity->y)) {
		e->underwater = TRUE;
	} else {
		e->underwater = FALSE;
	}
	// Inactive, just apply basic physics and gtfo
	if (e->state != CAI_ACTIVE) {
		e->x = e->x_next;
		e->y = e->y_next;
		return;
	}
	
	// first figure out where our target is
	
	// hack in case player REALLY leaves her behind. this works because of the way
	// the level is in a Z shape. first we check to see if the player is on the level below ours.
	if (player.y - e->y_next > pixel_to_sub(160) || e->state==999) {
		// if we're on the top section, head all the way to right, else if we're on the
		// middle section, head for the trap door that was destroyed by the button
		otiley = sub_to_block(e->y_next);
		
		curly_target_time = 0;
		
		if (otiley < 22) {
			e->x_mark = pixel_to_sub((126 * 16) + 8);		// center of big chute on right top
		} else if (otiley > 36 && otiley < 47) {	
			// fell down chute in center of middle section
			// continue down chute, don't get hung up on sides
			e->x_mark = pixel_to_sub(26 * 16);
		} else if (otiley >= 47) {	
			// bottom section - head for exit door
			// (this shouldn't ever execute, though, because player can't be lower than this)
			e->x_mark = pixel_to_sub(81 * 16);
			seeking_player = 1;		// stop when reach exit door
		} else {	
			// on middle section
			e->x_mark = pixel_to_sub((7 * 16) + 8);		// trap door which was destroyed by switch
		}
		e->y_mark = e->y_next;
	} else {
		// if we get real far away from the player leave the enemies alone and come find him
		if (!PLAYER_DIST_X(e, pixel_to_sub(160))) curly_target_time = 0;
		
		// if we're attacking an enemy head towards the enemy else return to the player
		if (curly_target_time) {
			e->x_mark = curly_target_x;
			e->y_mark = curly_target_y;
			
			curly_target_time--;
			if (curly_target_time==60 && !(rand() & 1)) CaiJUMP(e);
		} else {
			e->x_mark = player.x;
			e->y_mark = player.y;
			seeking_player = 1;
		}
	}
	
	// do not fall off the middle railing in Almond
	if (stageID == 0x2F) {
		#define END_OF_RAILING		pixel_to_sub(((72*16)-8))
		if (e->x_mark > END_OF_RAILING) {
			e->x_mark = END_OF_RAILING;
		}
	}
	
	// calculate distance to target
	xdist = abs(e->x_next - e->x_mark);
	ydist = abs(e->y_next - e->y_mark);
	
	// face target. I used two seperate IF statements so she doesn't freak out at start point
	// when her x == xmark.
	if (e->x_next < e->x_mark) wantdir = 1;
	if (e->x_next > e->x_mark) wantdir = 0;
	if (wantdir != e->dir) {
		if (++e->timer2 > 4) {
			e->timer2 = 0;
			e->dir = wantdir;
		}
	} else e->timer2 = 0;
	
	// if trying to return to the player then go into a rest state when we've reached him
	reached_p = 0;
	if (seeking_player && xdist < pixel_to_sub(32) && ydist < pixel_to_sub(64)) {
		if (++curly_reachptimer > 80) {
			e->x_speed *= 7;
			e->x_speed /= 8;
			e->frame = 0;
			reached_p = 1;
		}
	}
	else curly_reachptimer = 0;
	
	if (!reached_p)	{	// if not at rest walk towards target
		// walk towards target
		if (e->x_next > e->x_mark) e->x_speed -= SPEED(0x20);
		if (e->x_next < e->x_mark) e->x_speed += SPEED(0x20);
		
		// jump if we hit a wall, jump higher if off camera
		if (e->grounded && (blockr || blockl)) {
			CaiJUMP(e);
			curly_impjump = abs(e->x - camera.x) > pixel_to_sub(168);
		}
		// if we're below the target try jumping around randomly
		if (e->y_next > e->y_mark && (e->y_next - e->y_mark) > (16<<CSF)) {
			if (++curly_tryjumptime > 20) {
				curly_tryjumptime = 0;
				if (rand() & 1) CaiJUMP(e);
			}
		}
		else curly_tryjumptime = 0;
	}
	
	// the improbable jump - when AI gets confused, just cheat!
	// jump REALLY high by reducing gravity until we clear the wall
	if (curly_impjump > 0) {
		e->y_speed += SPEED(0x10);
		// deactivate Improbable Jump once we clear the wall or hit the ground
		if (e->grounded || (!blockl && !blockr)) curly_impjump = 0;
		else curly_impjump = abs(e->x - camera.x) > pixel_to_sub(168);
	}
	else e->y_speed += SPEED(0x33);
	
	// look up/down at target
	curly_look = 0;
	if (!reached_p || abs(e->y_next - player.y) > pixel_to_sub(48)) {
		if (e->y_next > e->y_mark && ydist >= pixel_to_sub(12) && 
				(!seeking_player || ydist >= pixel_to_sub(80))) curly_look = DIR_UP;
		else if (e->y_next < e->y_mark && !e->grounded && ydist >= pixel_to_sub(80)) 
				curly_look = DIR_DOWN;
	}
	
	// Sprite Animation
	if(e->grounded) {
		if(curly_look == DIR_UP) {
			if(e->x_speed != 0) {
				ANIMATE(e, 8, UPWALK1,LOOKUP,UPWALK2,LOOKUP);
			} else {
				e->frame = LOOKUP;
			}
		} else if(e->x_speed != 0) {
			ANIMATE(e, 8, WALK1,STAND,WALK2,STAND);
		} else if(curly_look == DIR_DOWN) {
			e->frame = LOOKDN;
		} else {
			e->frame = STAND;
		}
	} else {
		if(curly_look == DIR_UP) {
			e->frame = UPWALK2;
		} else if(curly_look == DIR_DOWN) {
			e->frame = JUMPDN;
		} else {
			e->frame = WALK2;
		}
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	LIMIT_X(SPEED(0x300));
	LIMIT_Y(SPEED(0x5ff));
}

static void fire_mgun(int32_t x, int32_t y, uint8_t dir) {
	// Curly shares the bullet array with the player so don't use up the whole thing
	// Use slots 2-4 and 7-8 backwards
	// This leaves room in the worst case for 1 missile, 2 polar star, and 2 fireball
	Bullet *b = NULL;
	for(uint8_t i = 8; i > 6; i--) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(!b) {
		for(uint8_t i = 4; i > 1; i--) {
			if(playerBullet[i].ttl > 0) continue;
			b = &playerBullet[i];
			break;
		}
		if(!b) return;
	}
	sound_play(SND_POLAR_STAR_L3, 5);
	b->type = WEAPON_MACHINEGUN;
	b->level = 3;
	// Need to set the position immediately or else the sprite will blink in upper left
	b->sprite = (Sprite) {
		.x = (x >> CSF) - (camera.x >> CSF) + ScreenHalfW - 8,
		.y = (y >> CSF) - (camera.y >> CSF) + ScreenHalfH - 8,
		.size = SPRITE_SIZE(2, 2),
	};
	SHEET_FIND(b->sheet, SHEET_MGUN);
	b->damage = 6;
	b->ttl = 90;
	b->hit_box = (bounding_box) {{ 4, 4, 4, 4 }};
	b->x = x;
	b->y = y;
	if(dir == DIR_UP) {
		b->sprite.attr = TILE_ATTR(PAL0,0,0,0,sheets[b->sheet].index+8);
		b->x_speed = 0;
		b->y_speed = -pixel_to_sub(4);
	} else if(dir == DIR_DOWN) {
		b->sprite.attr = TILE_ATTR(PAL0,0,1,0,sheets[b->sheet].index+8);
		b->x_speed = 0;
		b->y_speed = pixel_to_sub(4);
	} else {
		b->sprite.attr = TILE_ATTR(PAL0,0,0,0,sheets[b->sheet].index);
		b->x_speed = (dir > 0 ? pixel_to_sub(4) : -pixel_to_sub(4));
		b->y_speed = 0;
	}
}

static void fire_pstar(int32_t x, int32_t y, uint8_t dir) {
	// Use slot 7 and 8, this messes with player's missiles slightly
	Bullet *b = NULL;
	for(uint8_t i = 7; i < 9; i++) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(b == NULL) return;
	sound_play(SND_POLAR_STAR_L3, 5);
	b->type = WEAPON_POLARSTAR;
	b->level = 3;
	b->sprite = (Sprite) {
		.x = (x >> CSF) - (camera.x >> CSF) + ScreenHalfW - 8,
		.y = (y >> CSF) - (camera.y >> CSF) + ScreenHalfH - 8,
		.size = SPRITE_SIZE(2, 2),
	};
	SHEET_FIND(b->sheet, SHEET_PSTAR);
	b->damage = 4;
	b->ttl = 35;
	b->hit_box = (bounding_box) {{ 4, 4, 4, 4 }};
	b->x = x;
	b->y = y;
	if(dir == DIR_UP) {
		b->sprite.attr = TILE_ATTR(PAL0,0,0,0,sheets[b->sheet].index+4);
		b->x_speed = 0;
		b->y_speed = -pixel_to_sub(4);
	} else if(dir == DIR_DOWN) {
		b->sprite.attr = TILE_ATTR(PAL0,0,0,0,sheets[b->sheet].index+4);
		b->x_speed = 0;
		b->y_speed = pixel_to_sub(4);
	} else {
		b->sprite.attr = TILE_ATTR(PAL0,0,0,0,sheets[b->sheet].index);
		b->x_speed = (dir > 0 ? pixel_to_sub(4) : -pixel_to_sub(4));
		b->y_speed = 0;
	}
}

#define SMALLDIST		pixel_to_sub(32)
#define BIGDIST			pixel_to_sub(160)

void ai_cai_gun(Entity *e) {
	Entity *curly = e->linkedEntity;
	if (!curly) { e->state = STATE_DELETE; return; }

    // Stick to curly
	e->dir = curly->dir;
    e->x = curly->x;
    e->y = curly->y + 0x600;
	
	const SpriteDefinition *sd = curly_mgun ? &SPR_MGun : &SPR_Polar;
	
	if (curly_look) {
		if(!e->animtime) {
			sprite_vflip(&e->sprite[0], curly_look == DIR_DOWN ? 1 : 0);
			e->sprite[0].size = SPRITE_SIZE(1, 3);
			e->display_box = curly_mgun ? (bounding_box){{ 4, 14, 4, 10 }} : (bounding_box){{ 4, 18, 4, 6 }};
			e->animtime = TRUE;
			
			dma_queue(DmaVRAM, (uint32_t) (SPR_TILES(sd,0,0)+(96>>2)), e->vramindex << 5, 48, 2);
			
		}
	} else if(e->animtime) {
		sprite_vflip(&e->sprite[0], 0);
		e->sprite[0].size = SPRITE_SIZE(3, 1);
		e->display_box = (bounding_box) {{ 12, 4, 12, 4 }};
		e->animtime = FALSE;
		
		dma_queue(DmaVRAM, (uint32_t) SPR_TILES(sd,0,0), e->vramindex << 5, 48, 2);
		
	}
	
	if (curly_target_time) {
		// fire when we get close to the target
		uint8_t fire;
		if (!curly_look) {
			// firing LR-- fire when lined up vertically and close by horizontally
			fire = ((abs(curly->x - curly_target_x) <= BIGDIST) && (abs(curly->y - curly_target_y) <= SMALLDIST));
		} else {
			// firing vertically-- fire when lined up horizontally and close by vertically
			fire = ((abs(curly->x - curly_target_x) <= SMALLDIST) && (abs(curly->y - curly_target_y) <= BIGDIST));
		}
		if (fire) {
			// Get point where bullet will be created based on direction / looking
			if(!curly_look) {
				e->x_mark = curly->x + (curly->dir ? 8 << CSF : -(8 << CSF));
				e->y_mark = curly->y + (2 << CSF);
			} else if(curly_look == DIR_UP) {
				e->x_mark = curly->x;
				e->y_mark = curly->y - (8 << CSF);
			} else if(curly_look == DIR_DOWN) {
				e->x_mark = curly->x;
				e->y_mark = curly->y + (8 << CSF);
			}
			if (curly_mgun) {	// she has the Machine Gun
				if (!e->timer2) {
					e->timer2 = 2 + (rand() & 3);		// no. shots to fire
					e->timer = 36 + (rand() & 15);
				}
				if (e->timer) {
					e->timer--;
				} else {
					// create the MGun blast
					// curly_look is forward (0), DIR_UP (1), or DIR_DOWN (3)
					// curly->dir is either 0 or 1, where 1 means right but DIR_RIGHT is 2
					// So we do this weird thing to fix it
					fire_mgun(e->x_mark, e->y_mark, curly_look ? curly_look : curly->dir << 1);
					e->timer = 36 + (rand() & 15);
					e->timer2--;
				}
			} else {	// she has the Polar Star
				if (!e->timer) {
					e->timer = 4 + (rand() & 15);
					if ((rand() & 7) == 0) e->timer += 20 + (rand() & 15);
					// create the shot
					fire_pstar(e->x_mark, e->y_mark, curly_look ? curly_look : curly->dir << 1);
				} else {
					e->timer--;
				}
			}
		}
	}
}

// curly's air bubble when she goes underwater
void ai_cai_watershield(Entity *e) {
	Entity *curly = e->linkedEntity;
	if (!curly) { e->state = STATE_DELETE; return; }
	
	if(curly->underwater) {
		e->hidden = FALSE;
		e->x = curly->x;
		e->y = curly->y;
	} else {
		e->hidden = TRUE;
	}
}
