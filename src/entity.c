#include "entity.h"

#include "input.h"
#include "stage.h"
#include "resources.h"
#include "camera.h"
#include "system.h"
#include "tables.h"
#include "player.h"
#include "effect.h"
#include "audio.h"
#include "tsc.h"
#include "npc.h"
#include "ai.h"
#include "sprite.h"
#include "sheet.h"

/* Linked List Macros */

#define LIST_PUSH(list, obj) ({                                                                \
	obj->next = list;                                                                          \
	obj->prev = NULL;                                                                          \
	list->prev = obj;                                                                          \
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
		MEM_free(temp);                                                                        \
	}                                                                                          \
})

#define LIST_CLEAR_BY_FILTER(list, var, match) ({                                              \
	Entity *obj = list, *temp;                                                                 \
	while(obj) {                                                                               \
		temp = obj->next;                                                                      \
		if(obj->var == match) {                                                                \
			LIST_REMOVE(list, obj);                                                            \
			if((obj->eflags&NPC_DISABLEONFLAG)) system_set_flag(obj->id, TRUE);                \
			MEM_free(obj);                                                                     \
		}                                                                                      \
		obj = temp;                                                                            \
	}                                                                                          \
})

// Heightmaps for slopes
const u8 heightmap[4][16] = {
	{ 0x0,0x0,0x1,0x1,0x2,0x2,0x3,0x3,0x4,0x4,0x5,0x5,0x6,0x6,0x7,0x7 },
	{ 0x8,0x8,0x9,0x9,0xA,0xA,0xB,0xB,0xC,0xC,0xD,0xD,0xE,0xE,0xF,0xF },
	{ 0xF,0xF,0xE,0xE,0xD,0xD,0xC,0xC,0xB,0xB,0xA,0xA,0x9,0x9,0x8,0x8 },
	{ 0x7,0x7,0x6,0x6,0x5,0x5,0x4,0x4,0x3,0x3,0x2,0x2,0x1,0x1,0x0,0x0 },
};

Entity *entityList = NULL, *inactiveList = NULL, *bossEntity = NULL;

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
	//if(e->type == OBJ_DOOR) { // In back
	//	LIST_MOVE_TAIL(inactiveList, entityListTail, e);
	//}
	LIST_MOVE(inactiveList, entityList, e);
	
	if(e->sheet == NOSHEET && npc_info[e->type].sprite) {
		// Try to allocate some VRAM
		TILOC_ADD(e->tiloc, e->framesize);
		if(e->tiloc != NOTILOC) {
			e->vramindex = tiloc_index + e->tiloc * 4;
			sprite_index(e->sprite[0], e->vramindex);
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
	MEM_free(e);
	return next;
}

Entity *entity_destroy(Entity *e) {
	sound_play(e->deathSound, 5);
	entity_drop_powerup(e);
	effect_create_smoke(e->x >> CSF, e->y >> CSF);
	if(e->eflags & NPC_EVENTONDEATH) tsc_call_event(e->event);
	if(e->eflags & NPC_DISABLEONFLAG) system_set_flag(e->id, TRUE);
	Entity *next = e->next;
	LIST_REMOVE(entityList, e);
	// If we had tile allocation release it for future generations to use
	if(e->tiloc != NOTILOC) {
		TILOC_FREE(e->tiloc, e->framesize);
		e->tiloc = NOTILOC;
	}
	MEM_free(e);
	return next;
}

void entities_clear() {
	LIST_CLEAR(entityList);
	LIST_CLEAR(inactiveList);
}

u16 entities_count_active() {
	u16 count = 0;
	Entity *e = entityList;
	while(e) {
		count++;
		e = e->next;
	}
	return count;
}

u16 entities_count_inactive() {
	u16 count = 0;
	Entity *e = inactiveList;
	while(e) {
		count++;
		e = e->next;
	}
	return count;
}

u16 entities_count() {
	return entities_count_active() + entities_count_inactive();
}

void entities_update() {
	Entity *e = entityList;
	while(e) {
		if(!e->alwaysActive && !entity_on_screen(e)) {
			Entity *next = e->next;
			entity_deactivate(e);
			e = next;
			continue;
		}
		// AI onUpdate method - may set STATE_DELETE
		ENTITY_ONFRAME(e);
		if(e->state == STATE_DELETE) {
			e = entity_delete(e);
			continue;
		} else if(e->state == STATE_DESTROY) {
			e = entity_destroy(e);
			continue;
		}
		// Handle Shootable flag - check for collision with player's bullets
		if((e->nflags | e->eflags) & NPC_SHOOTABLE) {
			Bullet *b = bullet_colliding(e);
			if(b) {
				// Destroy the bullet, or if it is a missile make it explode
				if(b->type == WEAPON_MISSILE || b->type == WEAPON_SUPERMISSILE) {
					if(b->x_speed != 0 || b->y_speed != 0) {
						bullet_missile_explode(b);
						if(e->eflags & NPC_INVINCIBLE) {
							sound_play(SND_TINK, 5);
						} else {
							if(b->damage < e->health) sound_play(e->hurtSound, 5);
						}
					}
				} else {
					b->ttl = 0;
					if(e->eflags & NPC_INVINCIBLE) {
						sound_play(SND_TINK, 5);
					} else {
						if(b->damage < e->health) sound_play(e->hurtSound, 5);
					}
				}
				if(!(e->eflags & NPC_INVINCIBLE)) {
					if(e->health <= b->damage) {
						if((e->nflags | e->eflags) & NPC_SHOWDAMAGE)
							effect_create_damage(e->damage_value - b->damage,
									sub_to_pixel(e->x), sub_to_pixel(e->y));
						// Killed enemy
						e->health = 0;
						ENTITY_ONDEATH(e);
						if(e->state == STATE_DESTROY) {
							e = entity_destroy(e);
						} else {
							e = e->next;
						}
						continue;
					}
					if((e->nflags | e->eflags) & NPC_SHOWDAMAGE) {
						e->damage_value -= b->damage;
						e->damage_time = 30;
					}
					e->health -= b->damage;
				}
			}
		}
		// Solid Entities
		bounding_box collision = { 0, 0, 0, 0 };
		if((e->eflags|e->nflags) & (NPC_SPECIALSOLID)) {
			collision = entity_react_to_collision(&player, e, TRUE);
			if(collision.bottom) {
				if((e->eflags|e->nflags) & NPC_BOUNCYTOP) {
					player.y_speed = pixel_to_sub(-1);
					player.grounded = FALSE;
				} else {
					playerPlatform = e;
				}
				player.y_next = player.y;
				collide_stage_ceiling(&player);
				player.y = player.y_next;
			// Double check stage collision to avoid clipping through walls
			} else if(collision.top) {
				player.y_next = player.y;
				collide_stage_floor(&player);
				player.y = player.y_next;
			} else if(collision.left) {
				player.x_next = player.x;
				collide_stage_rightwall(&player);
				player.x = player.x_next;
			} else if(collision.right) {
				player.x_next = player.x;
				collide_stage_leftwall(&player);
				player.x = player.x_next;
			}
		} // "Smushy" Solid Entities
		else if((e->eflags|e->nflags) & (NPC_SOLID)) {
			collision = entity_react_to_collision(&player, e, TRUE);
			if(collision.bottom) {
				player.y += (collision.bottom - 1);
				if((e->eflags|e->nflags) & NPC_BOUNCYTOP) {
					player.y_speed = pixel_to_sub(-1);
					player.grounded = FALSE;
				} else {
					playerPlatform = e;
				}
				player.y_next = player.y;
				collide_stage_ceiling(&player);
				player.y = player.y_next;
			// Double check stage collision to avoid clipping through walls
			} else if(collision.top) {
				player.y -= (collision.top - 1);
				player.y_next = player.y;
				collide_stage_floor(&player);
				player.y = player.y_next;
			} else if(collision.left) {
				player.x += (collision.left - 1);
				player.x_next = player.x;
				collide_stage_rightwall(&player);
				player.x = player.x_next;
			} else if(collision.right) {
				player.x -= (collision.right - 1);
				player.x_next = player.x;
				collide_stage_leftwall(&player);
				player.x = player.x_next;
			}
		}
		// Can damage player if we have an attack stat and no script is running
		if(e->attack > 0 && playerIFrames == 0 && !tscState) {
			u32 collided = *(u32*)&collision; // I do what I want
			if((collided > 0 || entity_overlapping(&player, e))) {
				// If the enemy has NPC_FRONTATKONLY, and the player is not colliding
				// with the front of the enemy, the player shouldn't get hurt
				if((e->eflags|e->nflags)&NPC_FRONTATKONLY) {
					if((e->dir == 0 && collision.right == 0) ||
							(e->dir > 0 && collision.left == 0)) {
						e = e->next;
						continue;
					}
				}
				if(player_inflict_damage(e->attack)) {
					// Player died, don't update entities anymore
					return;
				}
			}
		}
		// Display damage
		if(((e->eflags|e->nflags) & NPC_SHOWDAMAGE) && e->damage_value) {
			e->damage_time--;
			if(e->damage_time <= 0) {
				effect_create_damage(e->damage_value,
						sub_to_pixel(e->x), sub_to_pixel(e->y));
				e->damage_value = 0;
			}
		}
		// Handle sprite movement/changes
		if(!e->hidden) {
			if(e->sheet != NOSHEET) {
				sprite_pos(e->sprite[0],
						(e->x>>CSF) - (camera.x>>CSF) + SCREEN_HALF_W - e->display_box.left,
						(e->y>>CSF) - (camera.y>>CSF) + SCREEN_HALF_H - e->display_box.top);
				if(e->frame != e->oframe) {
					e->oframe = e->frame;
					sprite_index(e->sprite[0], e->vramindex + e->frame * e->framesize);
				}
				if(e->dir != e->odir) {
					e->odir = e->dir;
					sprite_hflip(e->sprite[0], e->dir);
				}
			} else if(e->tiloc != NOTILOC) {
				const AnimationFrame *f = npc_info[e->type].sprite->animations[0]->frames[e->frame];
				if(e->frame != e->oframe) {
					e->oframe = e->frame;
					TILES_QUEUE(f->tileset->tiles, e->vramindex, e->framesize);
				}
				// We can't just flip the vdpsprites, gotta draw them in backwards order too
				if(e->dir) {
					s16 bx = (e->x>>CSF) - (camera.x>>CSF) + SCREEN_HALF_W + e->display_box.left, 
						by = (e->y>>CSF) - (camera.y>>CSF) + SCREEN_HALF_H - e->display_box.top;
					u8 x = min(f->w, 32), y = 0;
					for(u8 i = 0; i < e->sprite_count; i++) {
						sprite_pos(e->sprite[i], bx - x, by + y);
						sprite_hflip(e->sprite[i], 1);
						x += min(f->w - x, 32);
						if(x >= f->w + 32) {
							x = 32;
							y += 32;
						}
					}
				} else {
					s16 bx = (e->x>>CSF) - (camera.x>>CSF) + SCREEN_HALF_W - e->display_box.left, 
						by = (e->y>>CSF) - (camera.y>>CSF) + SCREEN_HALF_H - e->display_box.top;
					u8 x = 0, y = 0;
					for(u8 i = 0; i < e->sprite_count; i++) {
						sprite_pos(e->sprite[i], bx + x, by + y);
						sprite_hflip(e->sprite[i], 0);
						x += 32;
						if(x >= f->w) {
							x = 0;
							y += 32;
						}
					}
				}
			}
		}
		e = e->next;
	}
}

void entities_update_inactive() {
	Entity *e = inactiveList;
	while(e != NULL) {
		if(e->alwaysActive || entity_on_screen(e)) {
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

u8 collide_stage_leftwall(Entity *e) {
	u16 block_x, block_y1, block_y2;
	u8 pxa1, pxa2;
	block_x = pixel_to_block(sub_to_pixel(e->x_next) - 
			(e->dir ? e->hit_box.right : e->hit_box.left));
	block_y1 = pixel_to_block(sub_to_pixel(e->y_next) - e->hit_box.top + 4);
	block_y2 = pixel_to_block(sub_to_pixel(e->y_next) + e->hit_box.bottom - 3);
	pxa1 = stage_get_block_type(block_x, block_y1);
	pxa2 = stage_get_block_type(block_x, block_y2);
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->eflags|e->nflags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		e->x_speed = 0;
		e->x_next = pixel_to_sub(
			block_to_pixel(block_x) + block_to_pixel(1) + e->hit_box.left);
		return TRUE;
	}
	return FALSE;
}

u8 collide_stage_rightwall(Entity *e) {
	u16 block_x, block_y1, block_y2;
	u8 pxa1, pxa2;
	block_x = pixel_to_block(sub_to_pixel(e->x_next) + 
			(e->dir ? e->hit_box.left : e->hit_box.right));
	block_y1 = pixel_to_block(sub_to_pixel(e->y_next) - e->hit_box.top + 4);
	block_y2 = pixel_to_block(sub_to_pixel(e->y_next) + e->hit_box.bottom - 3);
	pxa1 = stage_get_block_type(block_x, block_y1);
	pxa2 = stage_get_block_type(block_x, block_y2);
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->eflags|e->nflags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		e->x_speed = 0;
		e->x_next = pixel_to_sub(
				block_to_pixel(block_x) - e->hit_box.right);
		return TRUE;
	}
	return FALSE;
}

u8 collide_stage_floor(Entity *e) {
	u16 pixel_x1, pixel_x2, pixel_x3, pixel_y;
	u8 pxa1, pxa2, pxa3;
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 1;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 1;
	pixel_x3 = sub_to_pixel(e->x_next);
	pixel_y = sub_to_pixel(e->y_next) + e->hit_box.bottom;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	pxa3 = stage_get_block_type(pixel_to_block(pixel_x3), pixel_to_block(pixel_y + 2));
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->eflags|e->nflags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SND_THUD, 2);
		e->y_speed = 0;
		e->y_next = pixel_to_sub((pixel_y&~0xF) - e->hit_box.bottom);
		return TRUE;
	}
	if(!e->enableSlopes) return FALSE;
	u8 result = FALSE;
	if((pxa1&0x10) && (pxa1&0xF) >= 4 && (pxa1&0xF) < 6 &&
			pixel_y%16 >= heightmap[pxa1%2][pixel_x1%16]) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SND_THUD, 2);
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa1%2][pixel_x1%16] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	if((pxa2&0x10) && (pxa2&0xF) >= 6 && (pxa2&0xF) < 8 &&
			pixel_y%16 >= 0xF - heightmap[pxa2%2][pixel_x2%16]) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SND_THUD, 2);
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 0xF + 1 -
				heightmap[pxa2%2][pixel_x2%16] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	// Extra check in the center
	if(!result && (pxa3 & 0x10)) {
		if((pxa3 & 0xF) >= 4 && (pixel_y + 2) % 16 >= heightmap[pxa3%4][pixel_x3%16]) {
			if(e == &player && e->y_speed > 0xFF) sound_play(SND_THUD, 2);
			e->y_next = e->y;
			e->y_speed = 0;
			result = TRUE;
		}
	}
	return result;
}

u8 collide_stage_slope_grounded(Entity *e) {
	u16 pixel_x1, pixel_x2, pixel_x3, pixel_y;
	u8 pxa1, pxa2, pxa3;
	u8 result = FALSE;
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 1;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 1;
	pixel_x3 = sub_to_pixel(e->x_next);
	// If we are on flat ground and run up to a slope
	pixel_y = sub_to_pixel(e->y_next) + e->hit_box.bottom - 1;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	if((pxa1&0x10) && (pxa1&0xF) >= 4 && (pxa1&0xF) < 6 &&
			pixel_y%16 >= heightmap[pxa1%4][pixel_x1%16]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa1%4][pixel_x1%16] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	if((pxa2&0x10) && (pxa2&0xF) >= 6 && (pxa2&0xF) < 8 &&
			pixel_y%16 >= heightmap[pxa2%4][pixel_x2%16]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa2%4][pixel_x2%16] - e->hit_box.bottom);
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
			pixel_y%16 >= heightmap[pxa1%4][pixel_x1%16]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa1%4][pixel_x1%16] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	if((pxa2&0x10) && (pxa2&0xF) >= 6 && (pxa2&0xF) < 8 &&
			pixel_y%16 >= heightmap[pxa2%4][pixel_x2%16]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa2%4][pixel_x2%16] - e->hit_box.bottom);
		e->y_speed = 0;
		result = TRUE;
	}
	// Extra check in the center
	if(!result && (pxa3 & 0x10)) {
		if((pxa3 & 0xF) >= 4 && (pixel_y + 2) % 16 >= heightmap[pxa3%4][pixel_x3%16]) {
			e->y_next = e->y;
			e->y_speed = 0;
			result = TRUE;
		}
	}
	return result;
}

u8 collide_stage_floor_grounded(Entity *e) {
	u8 result = FALSE;
	// First see if we are still standing on a flat block
	u8 pxa1 = stage_get_block_type(pixel_to_block(sub_to_pixel(e->x_next) - e->hit_box.left),
			pixel_to_block(sub_to_pixel(e->y_next) + e->hit_box.bottom + 1));
	u8 pxa2 = stage_get_block_type(pixel_to_block(sub_to_pixel(e->x_next) + e->hit_box.right),
			pixel_to_block(sub_to_pixel(e->y_next) + e->hit_box.bottom + 1));
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
		(!((e->eflags|e->nflags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		// After going up a slope and returning to flat land, we are one or
		// two pixels too low. This causes the player to ignore new upward slopes
		// which is bad, so this is a dumb hack to push us back up if we are
		// a bit too low
		if((sub_to_pixel(e->y_next) + e->hit_box.bottom) % 16 < 4) {
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

u8 collide_stage_ceiling(Entity *e) {
	u16 pixel_x1, pixel_x2, pixel_y;
	u8 pxa1, pxa2;
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 2;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 2;
	pixel_y = sub_to_pixel(e->y_next) - e->hit_box.top + 2;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->eflags|e->nflags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		if(e == &player && e->y_speed < -0xFF) sound_play(SND_BONK_HEAD, 2);
		e->y_speed = 0;
		e->y_next = pixel_to_sub((pixel_y&~0xF) + e->hit_box.top) + block_to_sub(1);
		e->jump_time = 0;
		return TRUE;
	}
	u8 result = FALSE;
	if((pxa1&0x10) && (pxa1&0xF) >= 0 && (pxa1&0xF) < 2 &&
			pixel_y%16 <= 0xF - heightmap[pxa1%2][pixel_x1%16]) {
		if(e == &player && e->y_speed < -0xFF) sound_play(SND_BONK_HEAD, 2);
		e->y_next = pixel_to_sub((pixel_y&~0xF) + 0xF -
				heightmap[pxa1%2][pixel_x1%16] + e->hit_box.top);
		e->y_speed = 0;
		result = TRUE;
	}
	if((pxa2&0x10) && (pxa2&0xF) >= 2 && (pxa2&0xF) < 4 &&
			pixel_y%16 <= heightmap[pxa2%2][pixel_x2%16]) {
		if(e == &player && e->y_speed < -0xFF) sound_play(SND_BONK_HEAD, 2);
		e->y_next = pixel_to_sub((pixel_y&~0xF) +
				heightmap[pxa2%2][pixel_x2%16] + e->hit_box.top);
		e->y_speed = 0;
		result = TRUE;
	}
	return result;
}

u8 entity_overlapping(Entity *a, Entity *b) {
	s16 ax1 = sub_to_pixel(a->x) - (a->dir ? a->hit_box.right : a->hit_box.left),
		ax2 = sub_to_pixel(a->x) + (a->dir ? a->hit_box.left : a->hit_box.right),
		ay1 = sub_to_pixel(a->y) - a->hit_box.top,
		ay2 = sub_to_pixel(a->y) + a->hit_box.bottom,
		bx1 = sub_to_pixel(b->x) - (b->dir ? b->hit_box.right : b->hit_box.left),
		bx2 = sub_to_pixel(b->x) + (b->dir ? b->hit_box.left : b->hit_box.right),
		by1 = sub_to_pixel(b->y) - b->hit_box.top,
		by2 = sub_to_pixel(b->y) + b->hit_box.bottom;
	return (ax1 < bx2 && ax2 > bx1 && ay1 < by2 && ay2 > by1);
}

bounding_box entity_react_to_collision(Entity *a, Entity *b, u8 realXY) {
	bounding_box result = { 0, 0, 0, 0 };
	s16 ax1 = sub_to_pixel(a->x_next) - (a->dir ? a->hit_box.right : a->hit_box.left),
		ax2 = sub_to_pixel(a->x_next) + (a->dir ? a->hit_box.left : a->hit_box.right),
		ay1 = sub_to_pixel(a->y_next) - a->hit_box.top,
		ay2 = sub_to_pixel(a->y_next) + a->hit_box.bottom,
		bx1 = sub_to_pixel(b->x) - (b->dir ? b->hit_box.right : b->hit_box.left),
		bx2 = sub_to_pixel(b->x) + (b->dir ? b->hit_box.left : b->hit_box.right),
		by1 = sub_to_pixel(b->y) - b->hit_box.top,
		by2 = sub_to_pixel(b->y) + b->hit_box.bottom;
	if(!(ax1 < bx2 && ax2 > bx1 && ay1 < by2 && ay2 > by1)) return result;
	// Wall reaction
	ax1 = sub_to_pixel(a->x_next) - a->hit_box.left + 1;
	ax2 = sub_to_pixel(a->x_next) + a->hit_box.right - 1;
	ay1 = sub_to_pixel(a->y_next) - a->hit_box.top + 2;
	ay2 = sub_to_pixel(a->y_next) + a->hit_box.bottom - 3;
	if(ay1 < by2 && ay2 > by1) {
		s16 move1 = pixel_to_sub(bx2 - ax1);
		s16 move2 = pixel_to_sub(bx1 - ax2);
		if(abs(move1) < abs(move2)) {
			result.left = 1;
			if(realXY) {
				a->x += move1;
			} else {
				a->x_next += move1;
			}
			if(a->x_speed < 0) a->x_speed = 0;
		} else {
			result.right = 1;
			if(realXY) {
				a->x += move2;
			} else {
				a->x_next += move2;
			}
			if(a->x_speed > 0) a->x_speed = 0;
		}
	}
	// Floor reaction
	ax1 = sub_to_pixel(a->x_next) - a->hit_box.left + 2;
	ax2 = sub_to_pixel(a->x_next) + a->hit_box.right - 2;
	ay1 = sub_to_pixel(a->y_next) - a->hit_box.top;
	ay2 = sub_to_pixel(a->y_next) + a->hit_box.bottom;
	if(ax1 < bx2 && ax2 > bx1) {
		s16 move1 = pixel_to_sub(by2 - ay1);
		s16 move2 = pixel_to_sub(by1 - ay2) + pixel_to_sub(1);
		if(abs(move1) < abs(move2)) {
			result.top = 1;
			if(realXY) {
				a->y += move1;
			} else {
				a->y_next += move1;
			}
			if(a->y_speed < 0) a->y_speed = 0;
		} else {
			result.bottom = 1;
			if(realXY) {
				a->y += move2;
			} else {
				a->y_next += move2;
			}
			if(a->y_speed > 0) a->y_speed = 0;
			a->grounded = TRUE;
		}
	}
	return result;
}

Entity *entity_find_by_id(u16 id) {
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

Entity *entity_find_by_event(u16 event) {
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

Entity *entity_find_by_type(u16 type) {
	Entity *e = entityList;
	while(e) {
		if(e->type == type) return e;
		else e = e->next;
	}
	return NULL;
}

void entities_clear_by_event(u16 event) {
	LIST_CLEAR_BY_FILTER(entityList, event, event);
	LIST_CLEAR_BY_FILTER(inactiveList, event, event);
}

void entities_clear_by_type(u16 type) {
	LIST_CLEAR_BY_FILTER(entityList, type, type);
	LIST_CLEAR_BY_FILTER(inactiveList, type, type);
}

void entity_drop_powerup(Entity *e) {
	u8 chance = random() % 5;
	if(chance >= 3) { // Weapon Energy
		if(e->experience > 0) {
			Entity *exp = entity_create(e->x, e->y, OBJ_XP,
					e->experience > 6 ? NPC_OPTION2 : 0);
			exp->experience = e->experience;
		}
	} else if(chance == 2 && (player_has_weapon(WEAPON_MISSILE) || 
		player_has_weapon(WEAPON_SUPERMISSILE))) { // Missiles
		if(e->experience > 6) {
			entity_create(e->x, e->y, 86, NPC_OPTION1 | NPC_OPTION2);
		} else {
			entity_create(e->x, e->y, 86, NPC_OPTION1);
		}
	} else { // Heart
		if(e->experience > 6) {
			Entity *heart = entity_create(e->x, e->y, 87, NPC_OPTION1 | NPC_OPTION2);
			heart->health = 5;
		} else {
			Entity *heart = entity_create(e->x, e->y, 87, NPC_OPTION1);
			heart->health = 2;
		}
	}
}

void entity_default(Entity *e, u16 type, u16 flags) {
	// Depending on the NPC type, apply default values
	e->type = type;
	e->eflags |= flags;
	e->enableSlopes = TRUE;
	e->tiloc = NOTILOC;
	e->sheet = NOSHEET;
	if(type < NPC_COUNT) {
		e->nflags = npc_flags(type);
		e->health = npc_hp(type);
		e->attack = npc_attack(type);
		e->experience = npc_xp(type);
		e->deathSound = npc_diesfx(type);
		e->hurtSound = npc_hurtsfx(type);
		e->hit_box = npc_hitbox(type);
		e->display_box = npc_displaybox(type);
	} else {
		e->health = 1;
		e->hit_box = (bounding_box) { 8, 8, 8, 8 };
		e->display_box = (bounding_box) { 8, 8, 8, 8 };
	}
}

Entity *entity_create(s32 x, s32 y, u16 type, u16 flags) {
	// Allocate memory and start applying values
	u8 sprite_count = npc_info[type].sprite_count;
	Entity *e = MEM_alloc(sizeof(Entity) + sizeof(VDPSprite) * sprite_count);
	memset(e, 0, sizeof(Entity) + sizeof(VDPSprite) * sprite_count);
	e->x = x;
	e->y = y;
	e->sprite_count = sprite_count;
	entity_default(e, type, flags);
	if(sprite_count) {
		if(npc_info[type].sheet != NOSHEET) { // Sheet
			SHEET_FIND(e->sheet, npc_info[type].sheet);
			e->vramindex = sheets[e->sheet].index;
			e->framesize = sheets[e->sheet].w * sheets[e->sheet].h;
			e->sprite[0] = (VDPSprite) {
				.size = SPRITE_SIZE(sheets[e->sheet].w, sheets[e->sheet].h),
				.attribut = TILE_ATTR_FULL(npc_info[type].palette,0,0,0,e->vramindex)
			};
			e->oframe = 255;
		} else if(npc_info[type].sprite) { // Use our own tiles
			const AnimationFrame *f = npc_info[e->type].sprite->animations[0]->frames[0];
			e->framesize = f->tileset->numTile;
			TILOC_ADD(e->tiloc, e->framesize);
			if(e->tiloc != NOTILOC) {
				e->vramindex = tiloc_index + e->tiloc * 4;
				u16 tile_offset = 0;
				for(u8 i = 0; i < e->sprite_count; i++) {
					e->sprite[i] = (VDPSprite) {
						.size = f->vdpSpritesInf[i]->size,
						.attribut = TILE_ATTR_FULL(npc_info[type].palette,0,0,0,
								e->vramindex + tile_offset)
					};
					tile_offset += f->vdpSpritesInf[i]->numTile;
				}
				e->oframe = 255;
			}
		}
	}
	ENTITY_ONSPAWN(e);
	if(e->alwaysActive || entity_on_screen(e)) {
		LIST_PUSH(entityList, e);
	} else {
		LIST_PUSH(inactiveList, e);
	}
	return e;
}

void entities_replace(u16 event, u16 type, u8 direction, u16 flags) {
	Entity *e = entityList;
	while(e) {
		if(e->event == event) {
			// Need to re-create the structure, the replaced entity may have a different
			// number of sprites
			Entity *new = entity_create(e->x, e->y, type, e->eflags | flags);
			new->dir = direction;
			new->id = e->id;
			new->event = event;
			if(!new->state) new->state = e->state;
			e = entity_delete(e);
		} else e = e->next;
	}
	e = inactiveList;
	while(e) {
		if(e->event == event) {
			Entity *new = entity_create(e->x, e->y, type, e->eflags | flags);
			new->dir = direction;
			new->id = e->id;
			new->event = event;
			if(!new->state) new->state = e->state;
			e = entity_delete(e);
		} else e = e->next;
	}
}

void entities_set_state(u16 event, u16 state, u8 direction) {
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

void entities_move(u16 event, u16 x, u16 y, u8 direction) {
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

u8 entity_exists(u16 type) {
	Entity *e = entityList;
	while(e) {
		if(e->type == type) return TRUE;
		e = e->next;
	}
	return FALSE;
}

void entities_draw() {
	const Entity *e = entityList;
	while(e) {
		if(!e->hidden) {
			sprite_addq(e->sprite, e->sprite_count);
		}
		e = e->next;
	}
}
