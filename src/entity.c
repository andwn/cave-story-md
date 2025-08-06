#include "common.h"
#include "audio.h"
#include "res/stage.h"
#include "camera.h"
#include "md/dma.h"
#include "effect.h"
#include "md/joy.h"
#include "math.h"
#include "md/stdlib.h"
#include "npc.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "system.h"
#include "tables.h"
#include "md/comp.h"
#include "tsc.h"
#include "md/vdp.h"
#include "weapon.h"
#include "md/sys.h"

#include "entity.h"
#include "ai.h"
#include "res/local.h"

/* Linked List Macros */

#define LIST_PUSH(list, obj) ({                                                                \
	obj->next = list;                                                                          \
	obj->prev = NULL;                                                                          \
	if(list) list->prev = obj;                                                                 \
	list = obj;                                                                                \
})

#define LIST_REMOVE(list, obj) ({                                                              \
	if(obj->next) obj->next->prev = obj->prev;                                                 \
	if(obj->prev) obj->prev->next = obj->next;                                                 \
	else list = obj->next;                                                                     \
})

#define LIST_MOVE(fromList, toList, obj) ({                                                    \
	LIST_REMOVE(fromList, obj);                                                                \
	LIST_PUSH(toList, obj);                                                                    \
})

#define LIST_CLEAR(list) ({                                                                    \
	Entity *temp;                                                                              \
	while(list) {                                                                              \
		temp = list;                                                                           \
		LIST_REMOVE(list, list);                                                               \
		free(temp);                                                                            \
	}                                                                                          \
})

// Heightmaps for slopes
const uint8_t heightmap[4][16] = {
	{ 0x0,0x0,0x1,0x1,0x2,0x2,0x3,0x3,0x4,0x4,0x5,0x5,0x6,0x6,0x7,0x7 },
	{ 0x8,0x8,0x9,0x9,0xA,0xA,0xB,0xB,0xC,0xC,0xD,0xD,0xE,0xE,0xF,0xF },
	{ 0xF,0xF,0xE,0xE,0xD,0xD,0xC,0xC,0xB,0xB,0xA,0xA,0x9,0x9,0x8,0x8 },
	{ 0x7,0x7,0x6,0x6,0x5,0x5,0x4,0x4,0x3,0x3,0x2,0x2,0x1,0x1,0x0,0x0 },
};

uint8_t moveMeToFront;

Entity *entityList, *inactiveList, *bossEntity;
Entity *pieces[10]; // List for bosses to keep track of parts
Entity *water_entity;

uint16_t entity_active_count;

static const LocSprite* find_locsprite(uint16_t type) {
	switch(type) {
		case OBJ_HEY:		return SPR_HEY;
		case OBJ_LEVELUP:   return SPR_LEVELUP;
		case OBJ_LEVELDOWN: return SPR_LEVELDOWN;
		case OBJ_EMPTY:     return SPR_EMPTY;
		default: 			return NULL;
	}
}

uint8_t entity_on_screen(Entity *e) {
	uint32_t x = e->x, y = e->y;
	return x - camera_xmin < camera_xsize && y - camera_ymin < camera_ysize;
}

// Move to inactive list, delete sprite
void entity_deactivate(Entity *e) {
	LIST_MOVE(entityList, inactiveList, e);
	// If we had tile allocation release it for future generations to use
	if(e->tiloc != NOTILOC) {
		TILOC_FREE(e->tiloc, e->framesize);
		e->tiloc = NOTILOC;
	}
}

// Move into active list, recreate sprite
void entity_reactivate(Entity *e) {
	LIST_MOVE(inactiveList, entityList, e);
	if(e->sheet == NOSHEET && npc_info[e->type].sprite) {
		// Try to allocate some VRAM
		TILOC_ADD(e->tiloc, e->framesize);
		if(e->tiloc != NOTILOC) {
			const SpriteDef *f = npc_info[e->type].sprite;
			e->vramindex = tiloc_index + (e->tiloc << 2);
			uint16_t tile_offset = 0;
			for(uint8_t i = 0; i < e->sprite_count; i++) {
				sprite_index(&e->sprite[i], e->vramindex + tile_offset);
				tile_offset += f->sprites[i]->numTile;
			}
			e->oframe = 255;
		}
	}
}

Entity *entity_delete(Entity *e) {
	Entity *next = e->next;
	LIST_REMOVE(entityList, e);
	// If we had tile allocation release it for future generations to use
	if(e->tiloc != NOTILOC) {
		TILOC_FREE(e->tiloc, e->framesize);
		e->tiloc = NOTILOC;
	}
	free(e);
	return next;
}

Entity *entity_delete_inactive(Entity *e) {
	Entity *next = e->next;
	LIST_REMOVE(inactiveList, e);
	free(e);
	return next;
}

Entity *entity_destroy(Entity *e) {
	sound_play(e->deathSound, 5);
	entity_drop_powerup(e);
	effect_create_smoke(e->x >> CSF, e->y >> CSF);
	effect_create_smoke(e->x >> CSF, e->y >> CSF);
	if(e->flags & NPC_EVENTONDEATH) tsc_call_event(e->event);
	if(e->flags & NPC_DISABLEONFLAG) system_set_flag(e->id, TRUE);
	return entity_delete(e);
}

void entities_clear(void) {
	LIST_CLEAR(entityList);
	LIST_CLEAR(inactiveList);
}

uint16_t entities_count_active(void) {
	uint16_t count = 0;
	Entity *e = entityList;
	while(e) {
		count++;
		e = e->next;
	}
	return count;
}

uint16_t entities_count_inactive(void) {
	uint16_t count = 0;
	Entity *e = inactiveList;
	while(e) {
		count++;
		e = e->next;
	}
	return count;
}

uint16_t entities_count(void) {
	return entities_count_active() + entities_count_inactive();
}

void entities_update(uint8_t draw) {
	uint16_t new_active_count = 0;
	Entity *e = entityList;
	while(e) {
		if(!e->alwaysActive && !entity_on_screen(e)) {
			Entity *next = e->next;
			entity_deactivate(e);
			e = next;
			continue;
		}
		new_active_count++;
		// AI onUpdate method - may set STATE_DELETE
		//ENTITY_ONFRAME(e);
        e->onFrame(e);
		if(e->state == STATE_DELETE) {
			e = entity_delete(e);
			continue;
		} else if(e->state == STATE_DESTROY) {
			e = entity_destroy(e);
			continue;
		}
		// Handle Shootable flag - check for collision with player's bullets
		if(e->flags & NPC_SHOOTABLE) {
            // TODO: Only generate this when the entity actually moves,
            // and use it in more places like player & stage collision
			extent_box ee = (extent_box) {
				.x1 = (e->x >> CSF) - (e->hit_box.left),
				.y1 = (e->y >> CSF) - (e->hit_box.top),
				.x2 = (e->x >> CSF) + (e->hit_box.right),
				.y2 = (e->y >> CSF) + (e->hit_box.bottom),
			};
			uint8_t cont = FALSE;
            // This code is run 10 times for every shootable entity. It has to be fast
            // Using a pointer instead of indexing the array removes a lot of redundant lookups in the asm
            Bullet *pb = playerBullet;
			for(uint16_t i = 0; i < MAX_BULLETS; i++) {
                // The ttl check added a whopping 4 asm instructions lmao
                // I've opted to instead set a dead bullet's extent.x1 to 0xFFFF
                // That way the first check will always fail
				if(/*playerBullet[i].ttl &&*/
                    pb->extent.x1 <= ee.x2 && pb->extent.x2 >= ee.x1 &&
                    pb->extent.y2 >= ee.y1 && pb->extent.y1 <= ee.y2)
				{	// Collided
					entity_handle_bullet(e, pb);
					if(e->state == STATE_DESTROY) {
						e = entity_destroy(e);
						cont = TRUE;
						break;
					} else if(e->state == STATE_DELETE) {
						e = entity_delete(e);
						cont = TRUE;
						break;
					}
				}
                pb++;
			}
			if(cont) continue;
			// Whimsical Star collision
			if(!(e->flags & NPC_INVINCIBLE) && wstarCollideIndex < playerStarNum) {
				WStarBullet *star = &wstarBullet[wstarCollideIndex];
				if(star->x_px <= ee.x2 && star->x_px >= ee.x1 && star->y_px >= ee.y1 && star->y_px <= ee.y2) {
					//k_str("wstar_hit");
					if(e->health < 2) {
						if(e->flags & NPC_SHOWDAMAGE) {
							effect_create_damage(e->damage_value - 1, NULL, e->x >> CSF, e->y >> CSF);
							e->damage_time = e->damage_value = 0;
						}
						// Killed enemy
						e->health = 0;
						ENTITY_ONDEATH(e);
						if(e->state == STATE_DESTROY) {
							e = entity_destroy(e);
							continue;
						} else if(e->state == STATE_DELETE) {
							e = entity_delete(e);
							continue;
						}
					} else if((e->flags & NPC_SHOWDAMAGE) || e->shakeWhenHit) {
						e->damage_value -= 1;
						e->damage_time = 30;
					}
					e->health -= 1;
				}
			}
		}
		// Hard Solids
		uint16_t collided = FALSE;
		if(e->flags & NPC_SPECIALSOLID) {
			// Apply x_next/y_next so player is completely outside us
			bounding_box collision = entity_react_to_collision(&player, e);
			collided = collision.full; //*((uint32_t*) &collision) > 0;
			player.x = player.x_next;
			player.y = player.y_next;
			if(collided && player.health > 0 && (e->type == OBJ_BLOCK_MOVEH || e->type == OBJ_BLOCK_MOVEV)) {
				if(blk(player.x, 0, player.y, 0) == 0x41) {
					// Player got crushed
					if(player_inflict_damage(100)) return;
				}
			}
			if(collision.bottom) {
				if(e->flags & NPC_BOUNCYTOP) {
					player.y_speed = -(1 << CSF);
					player.grounded = FALSE;
				} else {
					playerPlatform = e;
					playerPlatformTime = 0;
				}
			}
		} // Soft solids
		else if(e->flags & NPC_SOLID) {
			bounding_box collision = entity_react_to_collision(&player, e);
			collided = collision.full; //*((uint32_t*) &collision) > 0;
			// Don't apply x_next/y_next, push outward 1 pixel at a time
			if(collision.bottom && e->y > player.y) {
				player.y -= 1<<CSF;
				if(e->flags & NPC_BOUNCYTOP) {
					player.y_speed = -(1 << CSF);
					player.grounded = FALSE;
				} else {
					playerPlatform = e;
					playerPlatformTime = 0;
				}
			} else if(collision.top && e->y < player.y) {
				player.y += 1<<CSF;
			} else if(collision.left && e->x < player.x) {
				player.x += 1<<CSF;
			} else if(collision.right && e->x > player.x) {
				player.x -= 1<<CSF;
			}
		}
		// Can damage player if we have an attack stat and no script is running
		if(e->attack && !playerIFrames && !tscState) {
			if(!(e->flags & (NPC_SOLID | NPC_SPECIALSOLID))) {
				// Smaller hitbox if they are pass-through
				player.hit_box = PLAYER_SOFT_HIT_BOX;
				collided = entity_overlapping(&player, e);
				player.hit_box = PLAYER_HARD_HIT_BOX;
			}
			if(collided) {
				// If the enemy has NPC_FRONTATKONLY, and the player is not colliding
				// with the front of the enemy, the player shouldn't get hurt
				if(e->flags & NPC_FRONTATKONLY) {
					if(!PLAYER_DIST_Y(e, pixel_to_sub(e->hit_box.top + 3))) {
						collided = FALSE;
					} else {
						if(e->dir) {
							if(player.x < e->x) collided = FALSE;
						} else {
							if(player.x > e->x) collided = FALSE;
						}
					}
				}
				if(collided && player_inflict_damage(e->attack)) return;
			}
		}
		// Damage timer and shaking
		if(e->damage_value) {
			e->damage_time--;
			if(e->shakeWhenHit) {
				e->xoff = (e->damage_time & 3) - 1;
			}
			if(!e->damage_time) {
				if(e->flags & NPC_SHOWDAMAGE) {
					effect_create_damage(e->damage_value, e, 0, 0);
				}
				e->damage_value = 0;
				e->xoff = 0;
			}
		}
		// Handle sprite movement/changes
		if(draw && !e->hidden) {
			if(e->sheet != NOSHEET) {
				// Preallocated sheet
				sprite_pos(&e->sprite[0],
						(e->x>>CSF) - camera.x_shifted - e->display_box.left + e->xoff,
						(e->y>>CSF) - camera.y_shifted - e->display_box.top);
				sprite_index(&e->sprite[0], e->vramindex + frameOffset[e->sheet][e->frame]);
				sprite_hflip(&e->sprite[0], e->dir);
			} else if(e->tiloc != NOTILOC) {
				int16_t fwidth = 8;
                if(npc_info[e->type].sprite == NULL) {
					// Localized sprite
					const LocSprite *def = find_locsprite(e->type);
					if(def) {
						fwidth = def->width << 3;
						if(e->oframe != e->frame) {
							e->oframe = e->frame;
							TILES_QUEUE(&def->tiles[e->frame * e->framesize * 8], e->vramindex, e->framesize);
						}
					}
				} else {
					// Normal Tiloc Sprite
                    const SpriteDef *def = npc_info[e->type].sprite;
					fwidth = def->w;
                    if(e->frame != e->oframe) {
                        e->oframe = e->frame;
                        TILES_QUEUE(def->tilesets[e->frame]->tiles, e->vramindex, e->framesize);
                    }
				}
				// We can't just flip the vdpsprites, gotta draw them in backwards order too
				if(e->dir) {
					int16_t bx = (e->x>>CSF) - camera.x_shifted + e->display_box.left + e->xoff,
							by = (e->y>>CSF) - camera.y_shifted - e->display_box.top;
					int16_t x = min(fwidth, 32);
					for(uint16_t i = 0; i < e->sprite_count; i++) {
						sprite_pos(&e->sprite[i], bx - x, by);
						sprite_hflip(&e->sprite[i], 1);
						if(x >= fwidth) {
							x = min(fwidth, 32);
							by += 32;
						} else {
							x += min(fwidth - x, 32);
						}
					}
				} else {
					int16_t bx = (e->x>>CSF) - camera.x_shifted - e->display_box.left + e->xoff,
							by = (e->y>>CSF) - camera.y_shifted - e->display_box.top;
					int16_t x = 0;
					for(uint16_t i = 0; i < e->sprite_count; i++) {
						sprite_pos(&e->sprite[i], bx + x, by);
						sprite_hflip(&e->sprite[i], 0);
						x += 32;
						if(x >= fwidth) {
							x = 0;
							by += 32;
						}
					}
				}
			}
			vdp_sprites_add(e->sprite, e->sprite_count);
		}
		if(moveMeToFront) {
			moveMeToFront = FALSE;
			Entity *next = e->next;
			LIST_REMOVE(entityList, e);
			LIST_PUSH(entityList, e);
			e = next;
		} else e = e->next;
	}
	entity_active_count = new_active_count;
}

void entity_handle_bullet(Entity *e, Bullet *b) {
	// Destroy the bullet, or if it is a missile make it explode
	if(b->type == WEAPON_MISSILE || b->type == WEAPON_SUPERMISSILE) {
		if(!b->state) {
			bullet_missile_explode(b);
			if((e->flags & NPC_INVINCIBLE) || e->type == OBJ_MA_PIGNON) {
				sound_play(SND_TINK, 5);
			} else {
				if(b->damage < e->health) sound_play(e->hurtSound, 5);
			}
		} else if(e->type == OBJ_MA_PIGNON) {
			// Ma Pignon is invulnerable to missiles
			return;
		}
	} else if(b->type == WEAPON_SPUR || b->type == WEAPON_SPUR_TAIL 
			|| (b->type == WEAPON_BLADE && b->level == 3)) {
		// Don't destroy Spur or Blade L3
		b->hits++;
		if(!(e->flags & NPC_INVINCIBLE) && !(e->damage_time) && b->damage < e->health) {
			sound_play(e->hurtSound, 5);
		}
	} else if(b->type == WEAPON_NEMESIS && b->level < 3) {
        if(e->flags & NPC_INVINCIBLE) {
            bullet_deactivate(b);
            sound_play(SND_TINK, 5);
            return;
        }
        if(b->damage < e->health) {
            if (b->last_hit[0] == e || b->last_hit[1] == e) {
                return;
            } else {
                if (b->last_hit[0] == NULL) {
                    b->last_hit[0] = e;
                } else if (b->last_hit[1] == NULL) {
                    b->last_hit[1] = e;
                } else {
                    b->last_hit[0] = e;
                }
                sound_play(e->hurtSound, 5);
            }
        }
	} else {
        bullet_deactivate(b);
		if(e->flags & NPC_INVINCIBLE) {
			sound_play(SND_TINK, 5);
		} else {
			if(b->damage < e->health) sound_play(e->hurtSound, 5);
		}
        if(b->type == WEAPON_POLARSTAR) {
            effect_create_misc(EFF_PSTAR_HIT, b->x >> CSF, b->y >> CSF, FALSE);
        }
	}
	if(!(e->flags & NPC_INVINCIBLE)) {
		if(e->health <= b->damage) {
			if(e->flags & NPC_SHOWDAMAGE) {
				effect_create_damage(e->damage_value - b->damage, NULL, e->x >> CSF, e->y >> CSF);
				e->damage_time = e->damage_value = 0;
			}
			// Killed enemy
			e->health = 0;
			ENTITY_ONDEATH(e);
			if(b->type == WEAPON_SPUR || b->type == WEAPON_SPUR_TAIL) {
				if(--b->damage == 0) {
                    bullet_deactivate(b);
                }
			}
			return;
		} else if((e->flags & NPC_SHOWDAMAGE) || e->shakeWhenHit) {
			e->damage_value -= b->damage;
			e->damage_time = 30;
		}
		e->health -= b->damage;
		if(b->type == WEAPON_SPUR || b->type == WEAPON_SPUR_TAIL) {
			if(--b->damage == 0) bullet_deactivate(b);
		}
	}
}

void entities_update_inactive(void) {
	Entity *e = inactiveList;
	while(e) {
		if(entity_on_screen(e)) {
			Entity *next = e->next;
			entity_reactivate(e);
			e = next;
		} else {
			e = e->next;
		}
	}
}

void entity_update_collision(Entity *e) {
	if(e->x_speed < 0) {
		collide_stage_leftwall(e);
	} else if (e->x_speed > 0) {
		collide_stage_rightwall(e);
	}
	if(e->grounded) {
		e->grounded = collide_stage_floor_grounded(e);
	} else if(e->y_speed > 0) {
		e->grounded = collide_stage_floor(e);
		return;
	}
	if(e->y_speed < 0) collide_stage_ceiling(e);
}

uint8_t collide_stage_leftwall(Entity *e) {
	int16_t xoff = e->dir ? e->hit_box.right : e->hit_box.left;
	uint16_t pixel_x = (e->x_next >> CSF) - xoff;
	uint16_t pixel_y = (e->y_next >> CSF);
	uint16_t block_x = pixel_to_block(pixel_x);
	uint16_t block_y1 = pixel_to_block(pixel_y - e->hit_box.top + 3);
	uint16_t block_y2 = pixel_to_block(pixel_y + e->hit_box.bottom - 3);
	uint8_t pxa1 = stage_get_block_type(block_x, block_y1);
	uint8_t pxa2 = stage_get_block_type(block_x, block_y2);
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->flags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		e->x_speed = 0;
		e->x_next &= ~0x1FF; // Align to pixel
		e->x_next += pixel_to_sub(min((pixel_x & ~0xF) + 16 - pixel_x, 3));
		return TRUE;
	}
	return FALSE;
}

uint8_t collide_stage_rightwall(Entity *e) {
	int16_t xoff = e->dir ? e->hit_box.left : e->hit_box.right;
	uint16_t pixel_x = (e->x_next >> CSF) + xoff;
	uint16_t pixel_y = (e->y_next >> CSF);
	uint16_t block_x = pixel_to_block(pixel_x);
	uint16_t block_y1 = pixel_to_block(pixel_y - e->hit_box.top + 3);
	uint16_t block_y2 = pixel_to_block(pixel_y + e->hit_box.bottom - 3);
	uint8_t pxa1 = stage_get_block_type(block_x, block_y1);
	uint8_t pxa2 = stage_get_block_type(block_x, block_y2);
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->flags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		e->x_speed = 0;
		e->x_next &= ~0x1FF;
		e->x_next -= pixel_to_sub(min(pixel_x - (pixel_x & ~0xF), 3));
		return TRUE;
	}
	return FALSE;
}

uint8_t collide_stage_floor(Entity *e) {
	uint16_t pixel_x1, pixel_x2, pixel_x3, pixel_y;
	uint8_t pxa1, pxa2, pxa3;
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 1;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 1;
	pixel_x3 = sub_to_pixel(e->x_next);
	pixel_y = sub_to_pixel(e->y_next) + e->hit_box.bottom;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	pxa3 = stage_get_block_type(pixel_to_block(pixel_x3), pixel_to_block(pixel_y + 2));
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->flags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SND_THUD, 2);
		e->y_speed = 0;
		e->y_next = pixel_to_sub((pixel_y&~0xF) - e->hit_box.bottom);
		return TRUE;
	}
	if(!e->enableSlopes) return FALSE;
	uint8_t result = FALSE;
	if((pxa1&0x10) && (pxa1&0xF) >= 4 && (pxa1&0xF) < 6 &&
			(pixel_y&15) >= heightmap[pxa1&1][pixel_x1&15]) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SND_THUD, 2);
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa1&1][pixel_x1&15] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	if((pxa2&0x10) && (pxa2&0xF) >= 6 && (pxa2&0xF) < 8 &&
			(pixel_y&15) >= (uint16_t)(0xF - heightmap[pxa2&1][pixel_x2&15])) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SND_THUD, 2);
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 0xF + 1 -
				heightmap[pxa2&1][pixel_x2&15] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	// Extra check in the center
	if(!result && (pxa3 & 0x10)) {
		if((pxa3 & 0xF) >= 4 && ((pixel_y + 2) & 15) >= heightmap[pxa3&3][pixel_x3&15]) {
			if(e == &player && e->y_speed > 0xFF) sound_play(SND_THUD, 2);
			e->y_next = e->y;
			e->y_speed = 0;
			result = TRUE;
		}
	}
	return result;
}

uint8_t collide_stage_slope_grounded(Entity *e) {
	uint16_t pixel_x1, pixel_x2, pixel_x3, pixel_y;
	uint8_t pxa1, pxa2, pxa3;
	uint8_t result = FALSE;
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 1;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 1;
	pixel_x3 = sub_to_pixel(e->x_next);
	// If we are on flat ground and run up to a slope
	pixel_y = sub_to_pixel(e->y_next) + e->hit_box.bottom - 1;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	if((pxa1&0x10) && (pxa1&0xF) >= 4 && (pxa1&0xF) < 6 &&
			(pixel_y&15) >= heightmap[pxa1&3][pixel_x1&15]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa1&3][pixel_x1&15] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	if((pxa2&0x10) && (pxa2&0xF) >= 6 && (pxa2&0xF) < 8 &&
			(pixel_y&15) >= heightmap[pxa2&3][pixel_x2&15]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa2&3][pixel_x2&15] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	if(result) return TRUE;
	// If we're already on a slope
	pixel_y = sub_to_pixel(e->y_next) + e->hit_box.bottom + 1;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	pxa3 = stage_get_block_type(pixel_to_block(pixel_x3), pixel_to_block(pixel_y + 2));
	if((pxa1&0x10) && (pxa1&0xF) >= 4 && (pxa1&0xF) < 6 &&
			(pixel_y&15) >= heightmap[pxa1&3][pixel_x1&15]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa1&3][pixel_x1&15] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	if((pxa2&0x10) && (pxa2&0xF) >= 6 && (pxa2&0xF) < 8 &&
			(pixel_y&15) >= heightmap[pxa2&3][pixel_x2&15]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa2&3][pixel_x2&15] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	// Extra check in the center
	if(!result && (pxa3 & 0x10)) {
		if((pxa3 & 0xF) >= 4 && ((pixel_y + 2) & 15) >= heightmap[pxa3&3][pixel_x3&15]) {
			e->y_next = e->y;
			e->y_speed = 0;
			result = TRUE;
		}
	}
	return result;
}

uint8_t collide_stage_floor_grounded(Entity *e) {
	uint8_t result = FALSE;
	// First see if we are still standing on a flat block
	uint8_t pxa1 = stage_get_block_type(pixel_to_block(sub_to_pixel(e->x_next) - e->hit_box.left),
			pixel_to_block(sub_to_pixel(e->y_next) + e->hit_box.bottom + 1));
	uint8_t pxa2 = stage_get_block_type(pixel_to_block(sub_to_pixel(e->x_next) + e->hit_box.right),
			pixel_to_block(sub_to_pixel(e->y_next) + e->hit_box.bottom + 1));
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
		(!((e->flags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		// After going up a slope and returning to flat land, we are one or
		// two pixels too low. This causes the player to ignore new upward slopes
		// which is bad, so this is a dumb hack to push us back up if we are
		// a bit too low
		if(((sub_to_pixel(e->y_next) + e->hit_box.bottom) & 15) < 4) {
			e->y_next = pixel_to_sub(((sub_to_pixel(e->y_next) + e->hit_box.bottom)&~0xF) -
				e->hit_box.bottom);
		}
		result = TRUE;
	}
	if(e->enableSlopes && collide_stage_slope_grounded(e)) {
		result = TRUE;
	}
	return result;
}

uint8_t collide_stage_ceiling(Entity *e) {
	uint16_t pixel_x1, pixel_x2, pixel_y;
	uint8_t pxa1, pxa2;
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 2;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 2;
	// Without the +1 here, quote will clip to the left/right of ceiling tiles
	pixel_y = sub_to_pixel(e->y_next) - e->hit_box.top + 1;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	uint8_t result = FALSE;
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->flags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		e->y_next = pixel_to_sub((pixel_y&~0xF) + e->hit_box.top + 15) + 0x100;
		result = TRUE;
	} else {
		if((pxa1&0x10) && (pxa1&0xF) >= 0 && (pxa1&0xF) < 2 &&
				(pixel_y&15) <= (uint16_t)(0xF - heightmap[pxa1&1][pixel_x1&15])) {
			e->y_next = pixel_to_sub((pixel_y&~0xF) + 0xF -
					heightmap[pxa1&1][pixel_x1&15] + (e->hit_box.top - 1)) + 0x100;
			result = TRUE;
		}
		if((pxa2&0x10) && (pxa2&0xF) >= 2 && (pxa2&0xF) < 4 &&
				(pixel_y&15) <= heightmap[pxa2&1][pixel_x2&15]) {
			e->y_next = pixel_to_sub((pixel_y&~0xF) +
					heightmap[pxa2&1][pixel_x2&15] + (e->hit_box.top - 1)) + 0x100;
			result = TRUE;
		}
	}
	if(result) {
		if(e == &player) {
			e->jump_time = 0;
			if(!playerNoBump && e->y_speed < -SPEED_10(0x200)) {
				sound_play(SND_BONK_HEAD, 2);
				effect_create_misc(EFF_BONKL, (e->x >> CSF) - 4, (e->y >> CSF) - 6, FALSE);
				effect_create_misc(EFF_BONKR, (e->x >> CSF) + 4, (e->y >> CSF) - 6, FALSE);
				if(shoot_cooldown) {
					playerNoBump = TRUE;
				} else {
					e->y_speed = min(-e->y_speed >> 1, e->underwater ? SPEED_8(0x80) : SPEED_8(0xFF));
				}
			} else if(!shoot_cooldown || !joy_down(JOY_DOWN)) {
				e->y_speed = 0;
			}
		} else if(e->y_speed < -SPEED_10(0x200)) {
			e->y_speed = min(-e->y_speed >> 1, e->underwater ? SPEED_8(0x80) : SPEED_8(0xFF));
		} else {
			e->y_speed = 0;
		}
	} else if(e == &player) {
		playerNoBump = FALSE;
	}
	return result;
}

uint8_t entity_overlapping(Entity *a, Entity *b) {
	int16_t ax1 = sub_to_pixel(a->x) - (a->dir ? a->hit_box.right : a->hit_box.left),
		ax2 = sub_to_pixel(a->x) + (a->dir ? a->hit_box.left : a->hit_box.right),
		ay1 = sub_to_pixel(a->y) - a->hit_box.top,
		ay2 = sub_to_pixel(a->y) + a->hit_box.bottom,
		bx1 = sub_to_pixel(b->x) - (b->dir ? b->hit_box.right : b->hit_box.left),
		bx2 = sub_to_pixel(b->x) + (b->dir ? b->hit_box.left : b->hit_box.right),
		by1 = sub_to_pixel(b->y) - b->hit_box.top,
		by2 = sub_to_pixel(b->y) + b->hit_box.bottom;
	return (ax1 < bx2 && ax2 > bx1 && ay1 < by2 && ay2 > by1);
}

bounding_box entity_react_to_collision(Entity *a, Entity *b) {
	bounding_box result = {0};
	int16_t ax1 = sub_to_pixel(a->x_next) - (a->dir ? a->hit_box.right : a->hit_box.left),
		ax2 = sub_to_pixel(a->x_next) + (a->dir ? a->hit_box.left : a->hit_box.right),
		ay1 = sub_to_pixel(a->y_next) - a->hit_box.top,
		ay2 = sub_to_pixel(a->y_next) + a->hit_box.bottom,
		bx1 = sub_to_pixel(b->x) - (b->dir ? b->hit_box.right : b->hit_box.left),
		bx2 = sub_to_pixel(b->x) + (b->dir ? b->hit_box.left : b->hit_box.right),
		by1 = sub_to_pixel(b->y) - b->hit_box.top,
		by2 = sub_to_pixel(b->y) + b->hit_box.bottom;
	if(!(ax1 < bx2 && ax2 > bx1 && ay1 < by2 && ay2 > by1)) return result;
	// This is an attempt to fix falling into platforms that are moving up
	if(abs(a->y_speed - b->y_speed) < SPEED_12(0x600)) {
		// Wall reaction
		ax1 = sub_to_pixel(a->x_next) - a->hit_box.left + 1;
		ax2 = sub_to_pixel(a->x_next) + a->hit_box.right - 1;
		ay1 = sub_to_pixel(a->y_next) - a->hit_box.top + 2;
		ay2 = sub_to_pixel(a->y_next) + a->hit_box.bottom - 3;
		if(ay1 < by2 && ay2 > by1) {
			int16_t move1 = pixel_to_sub(bx2 - ax1);
			int16_t move2 = pixel_to_sub(bx1 - ax2);
			if(abs(move1) < abs(move2)) {
				result.left = 1;
				a->x_next += move1;
				if(a->x_speed < 0) a->x_speed = 0;
			} else {
				result.right = 1;
				a->x_next += move2;
				if(a->x_speed > 0) a->x_speed = 0;
			}
		}
		// Floor reaction
		ax1 = sub_to_pixel(a->x_next) - a->hit_box.left + 2;
		ax2 = sub_to_pixel(a->x_next) + a->hit_box.right - 2;
		ay1 = sub_to_pixel(a->y_next) - a->hit_box.top;
		ay2 = sub_to_pixel(a->y_next) + a->hit_box.bottom;
		if(ax1 < bx2 && ax2 > bx1) {
			int16_t move1 = pixel_to_sub(by2 - ay1);
			int16_t move2 = pixel_to_sub(by1 - ay2) + pixel_to_sub(1);
			if(abs(move1) < abs(move2)) {
				result.top = 1;
				a->y_next += move1;
				if(a->y_speed < 0) a->y_speed = 0;
			} else {
				result.bottom = 1;
				a->y_next += move2;
				if(a->y_speed > 0) a->y_speed = 0;
				a->grounded = TRUE;
			}
		}
	} else { // This is 100% copy paste but wall/floor reversed
		// Floor reaction
		ax1 = sub_to_pixel(a->x_next) - a->hit_box.left + 2;
		ax2 = sub_to_pixel(a->x_next) + a->hit_box.right - 2;
		ay1 = sub_to_pixel(a->y_next) - a->hit_box.top;
		ay2 = sub_to_pixel(a->y_next) + a->hit_box.bottom;
		if(ax1 < bx2 && ax2 > bx1) {
			int16_t move1 = pixel_to_sub(by2 - ay1);
			int16_t move2 = pixel_to_sub(by1 - ay2) + pixel_to_sub(1);
			if(abs(move1) < abs(move2)) {
				result.top = 1;
				a->y_next += move1;
				if(a->y_speed < 0) a->y_speed = 0;
			} else {
				result.bottom = 1;
				a->y_next += move2;
				if(a->y_speed > 0) a->y_speed = 0;
				a->grounded = TRUE;
			}
		}
		// Wall reaction
		ax1 = sub_to_pixel(a->x_next) - a->hit_box.left + 1;
		ax2 = sub_to_pixel(a->x_next) + a->hit_box.right - 1;
		ay1 = sub_to_pixel(a->y_next) - a->hit_box.top + 2;
		ay2 = sub_to_pixel(a->y_next) + a->hit_box.bottom - 3;
		if(ay1 < by2 && ay2 > by1) {
			int16_t move1 = pixel_to_sub(bx2 - ax1);
			int16_t move2 = pixel_to_sub(bx1 - ax2);
			if(abs(move1) < abs(move2)) {
				result.left = 1;
				a->x_next += move1;
				if(a->x_speed < 0) a->x_speed = 0;
			} else {
				result.right = 1;
				a->x_next += move2;
				if(a->x_speed > 0) a->x_speed = 0;
			}
		}
	}
	return result;
}

Entity *entity_find_by_id(uint16_t id) {
	Entity *e = entityList;
	while(e) {
		if(e->id == id) return e;
		else e = e->next;
	}
	e = inactiveList;
	while(e) {
		if(e->id == id) return e;
		else e = e->next;
	}
	return NULL;
}

Entity *entity_find_by_event(uint16_t event) {
	Entity *e = entityList;
	while(e) {
		if(e->event == event) return e;
		else e = e->next;
	}
	e = inactiveList;
	while(e) {
		if(e->event == event) return e;
		else e = e->next;
	}
	return NULL;
}

Entity *entity_find_by_type(uint16_t type) {
	Entity *e = entityList;
	while(e) {
		if(e->type == type) return e;
		else e = e->next;
	}
	return NULL;
}

void entities_clear_by_event(uint16_t event) {
    Entity *e = entityList;
    while(e) {
		if(e->event == event) {
            if((e->flags&NPC_DISABLEONFLAG)) system_set_flag(e->id, TRUE);
			e = entity_delete(e);
		} else {
            e = e->next;
        }
	}
    e = inactiveList;
    while(e) {
        if(e->event == event) {
            if((e->flags&NPC_DISABLEONFLAG)) system_set_flag(e->id, TRUE);
            e = entity_delete_inactive(e);
        } else {
            e = e->next;
        }
    }
}

void entities_clear_by_type(uint16_t type) {
    Entity *e = entityList;
    while(e) {
        if(e->type == type) {
            if((e->flags&NPC_DISABLEONFLAG)) system_set_flag(e->id, TRUE);
            e = entity_delete(e);
        } else {
            e = e->next;
        }
    }
    e = inactiveList;
    while(e) {
        if(e->type == type) {
            if((e->flags&NPC_DISABLEONFLAG)) system_set_flag(e->id, TRUE);
            e = entity_delete_inactive(e);
        } else {
            e = e->next;
        }
    }
}

void entity_drop_powerup(Entity *e) {
	uint8_t chance = mod10[rand() & 0x3FF] >> 1;
	if(chance >= 2) { // Weapon Energy
		if(e->experience > 0) {
			Entity *exp = entity_create(e->x, e->y, OBJ_XP,
					e->experience > 6 ? NPC_OPTION2 : 0);
			exp->experience = e->experience;
		}
	} else if(chance == 1 && (player_has_weapon(WEAPON_MISSILE) || 
		player_has_weapon(WEAPON_SUPERMISSILE))) { // Missiles
		if(e->experience > 6) {
            Entity *missile = entity_create(e->x, e->y, 86, NPC_OPTION1 | NPC_OPTION2);
            missile->experience = 3;
		} else {
            Entity *missile = entity_create(e->x, e->y, 86, NPC_OPTION1);
            missile->experience = 1;
		}
	} else { // Heart
		if(e->experience > 6) {
			Entity *heart = entity_create(e->x, e->y, 87, NPC_OPTION1 | NPC_OPTION2);
			heart->experience = 6;
		} else {
			Entity *heart = entity_create(e->x, e->y, 87, NPC_OPTION1);
			heart->experience = 2;
		}
	}
}

void entity_default(Entity *e, uint16_t type, uint16_t flags) {
	// Depending on the NPC type, apply default values
	e->type = type;
    e->onFrame = npc_info[e->type].onFrame;
	e->enableSlopes = TRUE;
	e->shakeWhenHit = TRUE;
	e->tiloc = NOTILOC;
	e->sheet = NOSHEET;
	if(type < NPC_COUNT) {
		e->flags = npc_flags(type);
		e->health = npc_hp(type);
		e->attack = npc_attack(type);
		e->experience = npc_xp(type);
		e->deathSound = npc_diesfx(type);
		e->hurtSound = npc_hurtsfx(type);
		e->hit_box = npc_hitbox(type);
		e->display_box = npc_displaybox(type);
	} else {
		e->health = 1;
		e->hit_box = (bounding_box) {{ 8, 8, 8, 8 }};
		e->display_box = (bounding_box) {{ 8, 8, 8, 8 }};
	}
    e->flags |= flags;
}

Entity *entity_create_ext(int32_t x, int32_t y, uint16_t type, uint16_t flags, uint16_t id, uint16_t event) {
	// Allocate memory and start applying values
	uint8_t sprite_count = npc_info[type].sprite_count;
	Entity *e = malloc(sizeof(Entity) + sizeof(Sprite) * sprite_count);
	if(!e) {
		vdp_set_scrollmode(HSCROLL_PLANE, VSCROLL_PLANE);
		vdp_hscroll(VDP_PLANE_A, 0);
		vdp_vscroll(VDP_PLANE_A, 0);
		vdp_color(0, 0);
		vdp_color(15, 0xEEE);
		vdp_puts(VDP_PLANE_A, "OOM", 4, 4);
		k_str("OOM");
		for(;;);
	}

	//k_str("new entity");
	//k_hex16(type);

	memclr(e, sizeof(Entity) + sizeof(Sprite) * sprite_count);
	e->x = x;
	e->y = y;
	e->id = id;
	e->event = event;
	e->sprite_count = sprite_count;
	entity_default(e, type, flags);
	if(sprite_count) {
        if(npc_info[type].sheet != NOSHEET) { // Sheet
            SHEET_FIND(e->sheet, npc_info[type].sheet);
            e->vramindex = sheets[e->sheet].index;
            e->framesize = sheets[e->sheet].w * sheets[e->sheet].h;
            e->sprite[0] = (Sprite) {
                    .size = SPRITE_SIZE(sheets[e->sheet].w, sheets[e->sheet].h),
                    .attr = TILE_ATTR(npc_info[type].palette,0,0,0,e->vramindex)
            };
            e->oframe = 255;
        } else if(npc_info[type].sprite) { // Use our own tiles
			//k_str("new tiloc");
            const SpriteDef *spr = npc_info[type].sprite;
            e->framesize = spr->tilesets[0]->numTile;
            TILOC_ADD(e->tiloc, e->framesize);
            if(e->tiloc != NOTILOC) {
                e->vramindex = tiloc_index + (e->tiloc << 2);
                uint16_t tile_offset = 0;
                for(uint8_t i = 0; i < sprite_count; i++) {
                    e->sprite[i] = (Sprite) {
                            .size = spr->sprites[i]->size,
                            .attr = TILE_ATTR(npc_info[type].palette,0,0,0,
                                              e->vramindex + tile_offset)
                    };
                    tile_offset += spr->sprites[i]->numTile;
                }
                e->oframe = 255;
            }
        } else {
			const LocSprite *def = find_locsprite(type);
			if(def) {
				e->framesize = def->width * def->height;
				TILOC_ADD(e->tiloc, e->framesize);
				if(e->tiloc != NOTILOC) {
					e->vramindex = tiloc_index + (e->tiloc << 2);
					uint16_t i = 0;
					uint16_t tile_offset = 0;
					for(int16_t yy = def->height; yy > 0; yy -= 4) {
						for(int16_t xx = def->width; xx > 0; xx -= 4) {
							e->sprite[i++] = (Sprite) {
								.size = SPRITE_SIZE(min(4, xx),min(4, yy)),
								.attr = TILE_ATTR(npc_info[type].palette,0,0,0,e->vramindex + tile_offset)
							};
							tile_offset += min(4, xx) * min(4, yy);
						}
					}
					e->oframe = 255;
				}
			}
		}
	}
	ENTITY_ONSPAWN(e);
	if(e->alwaysActive || entity_on_screen(e)) {
		LIST_PUSH(entityList, e);
	} else {
		LIST_PUSH(inactiveList, e);
		if(e->tiloc != NOTILOC) {
			TILOC_FREE(e->tiloc, e->framesize);
			e->tiloc = NOTILOC;
		}
	}
	return e;
}

void entities_replace(uint16_t event, uint16_t type, uint8_t direction, uint16_t flags) {
    static const uint16_t keep = (NPC_INTERACTIVE | NPC_EVENTONDEATH
                                      | NPC_DISABLEONFLAG | NPC_ENABLEONFLAG | NPC_OPTION2);
	Entity *e = entityList;
	while(e) {
		if(e->event == event) {
			// Need to re-create the structure, the replaced entity may have a different
			// number of sprites
			int32_t x = e->x;
			int32_t y = e->y;
			flags |= e->flags & keep;
			uint16_t id = e->id;
            e = entity_delete(e);
			Entity *new = entity_create_ext(x, y, type, flags, id, event);
            new->flags |= flags;
			new->dir = direction;
		} else e = e->next;
	}
	e = inactiveList;
	while(e) {
		if(e->event == event) {
            int32_t x = e->x;
            int32_t y = e->y;
            flags |= e->flags & keep;
            uint16_t id = e->id;
            e = entity_delete_inactive(e); // So Balrog doesn't delete every entity in the room
            Entity *new = entity_create_ext(x, y, type, flags, id, event);
            new->flags |= flags;
            new->dir = direction;
		} else e = e->next;
	}
}

void entities_set_state(uint16_t event, uint16_t state, uint8_t direction) {
	Entity *e = entityList;
	while(e) {
		if(e->event == event) {
			e->dir = direction;
			e->state = state;
		}
		e = e->next;
	}
	e = inactiveList;
	while(e) {
		if(e->event == event) {
			e->dir = direction;
			e->state = state;
		}
		e = e->next;
	}
}

void entities_move(uint16_t event, uint16_t x, uint16_t y, uint8_t direction) {
	Entity *e = entityList;
	while(e) {
		if(e->event == event) {
			e->dir = direction;
			e->x = block_to_sub(x) + pixel_to_sub(8);
			e->y = block_to_sub(y) + pixel_to_sub(8);
			e->grounded = FALSE;
			break;
		}
		e = e->next;
	}
	e = inactiveList;
	while(e) {
		if(e->event == event) {
			e->dir = direction;
			e->x = block_to_sub(x) + pixel_to_sub(8);
			e->y = block_to_sub(y) + pixel_to_sub(8);
			e->grounded = FALSE;
			break;
		}
		e = e->next;
	}
}

uint8_t entity_exists(uint16_t type) {
	Entity *e = entityList;
	while(e) {
		if(e->type == type) return TRUE;
		e = e->next;
	}
	return FALSE;
}

void entities_draw(void) {
	const Entity *e = entityList;
	while(e) {
		if(!e->hidden) {
			vdp_sprites_add(e->sprite, e->sprite_count);
		}
		e = e->next;
	}
}

void generic_npc_states(Entity *e) {
	switch(e->state) {
		case 0:		// stand
		{
			e->frame = 0;
			e->x_speed = 0;
			e->y_speed = 0;
			if(e->type != OBJ_KAZUMA && !tscState) {
                RANDBLINK(e, 3, 200);
			}
		}
		break;
		case 3:		// walking
		case 4:
		{
			static const uint8_t f[] = { 1, 0, 2, 0 };
			if(++e->animtime >= 32) e->animtime = 0;
			e->frame = f[e->animtime >> 3];
			MOVE_X(SPEED_10(0x200));
		}
		break;
		case 5:		// face away
		{
			e->frame = e->type == OBJ_KAZUMA ? 3 : 4;
			e->x_speed = 0;
		}
		break;
		case 8:		// walk (alternate state used by OBJ_NPC_JACK)
		{
			if (e->type == OBJ_JACK) {
				e->state = 4;
				e->frame = 1;
			}
		}
		break;
	}
}
