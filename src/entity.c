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

/* Linked List Macros */

#define LIST_PUSH(list, obj) ({                                                                \
	obj->next = list;                                                                          \
	obj->prev = NULL;                                                                          \
	list->prev = obj;                                                                          \
	list = obj;                                                                                \
})

#define LIST_REMOVE(list, obj) ({                                                              \
	if(obj->next != NULL) obj->next->prev = obj->prev;                                         \
	if(obj->prev != NULL) obj->prev->next = obj->next;                                         \
	else list = obj->next;                                                                     \
})

#define LIST_MOVE(fromList, toList, obj) ({                                                    \
	LIST_REMOVE(fromList, obj);                                                                \
	LIST_PUSH(toList, obj);                                                                    \
})

#define LIST_CLEAR(list) ({                                                                    \
	Entity *temp;                                                                              \
	while(list != NULL) {                                                                      \
		temp = list;                                                                           \
		LIST_REMOVE(list, list);                                                               \
		SPR_SAFERELEASE(temp->sprite);                                                         \
		MEM_free(temp);                                                                        \
	}                                                                                          \
})

#define LIST_CLEAR_BY_FILTER(list, var, match) ({                                              \
	Entity *obj = list, *temp;                                                                 \
	while(obj != NULL) {                                                                       \
		temp = obj->next;                                                                      \
		if(obj->var == match) {                                                                \
			LIST_REMOVE(list, obj);                                                            \
			SPR_SAFERELEASE(obj->sprite);                                                      \
			if((obj->eflags&NPC_DISABLEONFLAG)) system_set_flag(obj->id, true);                \
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

// Internal functions
void sprite_create(Entity *e);

void entity_drop_powerup(Entity *e);
bool entity_on_screen(Entity *e);

// Initialize sprite for entity
void sprite_create(Entity *e) {
	if(e->spriteAnim == SPRITE_DISABLE || npc_info[e->type].sprite == NULL) {
		e->sprite = NULL;
		return;
	}
	e->sprite = SPR_addSprite(npc_info[e->type].sprite, 
		sub_to_pixel(e->x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - e->display_box.left, 
		sub_to_pixel(e->y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - e->display_box.top, 
		TILE_ATTR(npc_info[e->type].palette, 0, e->spriteVFlip, e->direction));
	SPR_SAFEANIMFRAME(e->sprite, e->spriteAnim, e->spriteFrame);
	SPR_SAFEVISIBILITY(e->sprite, AUTO_FAST);
}

// Move to inactive list, delete sprite
void entity_deactivate(Entity *e) {
	LIST_MOVE(entityList, inactiveList, e);
	// Remember sprite frame and animation (or if there was no sprite)
	if(e->sprite != NULL) {
		e->spriteAnim = e->sprite->animInd;
		e->spriteFrame = e->sprite->frameInd;
		e->spriteVFlip = (e->sprite->attribut & TILE_ATTR_VFLIP_MASK) > 0;
	}
	SPR_SAFERELEASE(e->sprite);
}

// Move into active list, recreate sprite
void entity_reactivate(Entity *e) {
	LIST_MOVE(inactiveList, entityList, e);
	sprite_create(e);
}

Entity *entity_delete(Entity *e) {
	Entity *next = e->next;
	LIST_REMOVE(entityList, e);
	SPR_SAFERELEASE(e->sprite);
	MEM_free(e);
	return next;
}

Entity *entity_destroy(Entity *e) {
	sound_play(e->deathSound, 5);
	entity_drop_powerup(e);
	effect_create_smoke(e->deathSmoke, sub_to_pixel(e->x), sub_to_pixel(e->y));
	if(e->eflags & NPC_EVENTONDEATH) tsc_call_event(e->event);
	if(e->eflags & NPC_DISABLEONFLAG) system_set_flag(e->id, true);
	Entity *next = e->next;
	LIST_REMOVE(entityList, e);
	SPR_SAFERELEASE(e->sprite);
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
	while(e != NULL) {
		count++;
		e = e->next;
	}
	return count;
}

u16 entities_count_inactive() {
	u16 count = 0;
	Entity *e = inactiveList;
	while(e != NULL) {
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
	while(e != NULL) {
		if(!entity_on_screen(e) && !e->alwaysActive) {
			Entity *next = e->next;
			entity_deactivate(e);
			e = next;
			continue;
		}
		// AI onUpdate method - may set STATE_DELETE
		ENTITY_ONUPDATE(e);
		if(e->state == STATE_DELETE) {
			e = entity_delete(e);
			continue;
		}
		// Handle Shootable flag - check for collision with player's bullets
		if(((e->eflags|e->nflags) & NPC_SHOOTABLE)) {
			Bullet *b = bullet_colliding(e);
			if(b != NULL) {
				// Destroy the bullet, or if it is a missile make it explode
				if(b->type == WEAPON_MISSILE || b->type == WEAPON_SUPERMISSILE) {
					if(b->x_speed != 0 || b->y_speed != 0) {
						bullet_missile_explode(b);
						if(b->damage < e->health) sound_play(e->hurtSound, 5);
					}
				} else {
					b->ttl = 0;
					SPR_SAFERELEASE(b->sprite);
					if(b->damage < e->health) sound_play(e->hurtSound, 5);
				}
				if(e->health <= b->damage) {
					if((e->eflags|e->nflags) & NPC_SHOWDAMAGE)
						effect_create_damage(e->damage_value - b->damage,
								sub_to_pixel(e->x), sub_to_pixel(e->y), 60);
					// Killed enemy
					e->health = 0;
					ENTITY_SET_STATE(e, STATE_DEFEATED, 0);
					if(e->state == STATE_DESTROY) {
						e = entity_destroy(e);
					} else {
						e = e->next;
					}
					continue;
				}
				if((e->eflags|e->nflags) & NPC_SHOWDAMAGE) {
					e->damage_value -= b->damage;
					e->damage_time = 30;
				}
				e->health -= b->damage;
				ENTITY_ONHURT(e);
			}
		}
		// Solid Entities
		bounding_box collision = { 0, 0, 0, 0 };
		if((e->eflags|e->nflags) & (NPC_SOLID | NPC_SPECIALSOLID)) {
			collision = entity_react_to_collision(&player, e);
			if(collision.bottom && ((e->eflags|e->nflags) & NPC_BOUNCYTOP)) {
				player.y_speed = pixel_to_sub(-1);
				player.grounded = false;
			}
		}
		// Can damage player if we have an attack stat and no script is running
		if(e->attack > 0 && !tsc_running()) {
			u32 collided = *(u32*)&collision; // I do what I want
			if((collided > 0 || entity_overlapping(&player, e)) && playerIFrames == 0) {
				// If the enemy has NPC_FRONTATKONLY, and the player is not colliding
				// with the front of the enemy, the player shouldn't get hurt
				if((e->eflags|e->nflags)&NPC_FRONTATKONLY) {
					if((e->direction == 0 && collision.right == 0) ||
							(e->direction > 0 && collision.left == 0)) {
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
		if(((e->eflags|e->nflags) & NPC_SHOWDAMAGE) && e->damage_value != 0) {
			e->damage_time--;
			if(e->damage_time <= 0) {
				effect_create_damage(e->damage_value,
						sub_to_pixel(e->x), sub_to_pixel(e->y), 60);
				e->damage_value = 0;
			}
		}
		SPR_SAFEMOVE(e->sprite,
			sub_to_pixel(e->x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - e->display_box.left,
			sub_to_pixel(e->y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - e->display_box.top);
		e = e->next;
	}
}

void entities_update_inactive() {
	Entity *e = inactiveList;
	while(e != NULL) {
		if(e->alwaysActive || (entity_on_screen(e) && !entity_disabled(e))) {
			Entity *next = e->next;
			entity_reactivate(e);
			e = next;
		} else {
			e = e->next;
		}
	}
}

void entity_update_movement(Entity *e) {
	if((e->eflags|e->nflags)&NPC_IGNORESOLID) {
		entity_update_float(e);
	} else {
		entity_update_walk(e);
		entity_update_jump(e);
	}
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
}

void entity_update_walk(Entity *e) {
	s16 acc;
	s16 fric;
	s16 max_speed = MAX_WALK_SPEED;
	if(e->grounded) {
		acc = WALK_ACCEL;
		fric = FRICTION;
	} else {
		acc = AIR_CONTROL;
		fric = AIR_CONTROL;
	}
	if(stage_get_block_type(sub_to_block(e->x), sub_to_block(e->y)) & BLOCK_WATER) {
		e->underwater = true;
		acc /= 2;
		fric /=2;
		max_speed /= 2;
	} else {
		e->underwater = false;
	}
	if(e->controller[0] & BUTTON_LEFT) {
		e->x_speed -= acc;
		if(e->x_speed < -max_speed) {
			e->x_speed = min(e->x_speed + acc, -max_speed);
		}
	} else if(e->controller[0] & BUTTON_RIGHT) {
		e->x_speed += acc;
		if(e->x_speed > max_speed) {
			e->x_speed = max(e->x_speed - acc, max_speed);
		}
	} else if(e->grounded) {
		if(e->x_speed < fric && e->x_speed > -fric) {
			e->x_speed = 0;
		} else if(e->x_speed < 0) {
			e->x_speed += fric;
		} else if(e->x_speed > 0) {
			e->x_speed -= fric;
		}
	}
}

void entity_update_jump(Entity *e) {
	s16 jumpSpeed = JUMP_SPEED;
	s16 gravity = GRAVITY;
	s16 gravityJump = GRAVITY_JUMP;
	s16 maxFallSpeed = MAX_FALL_SPEED;
	if(e->underwater) {
		jumpSpeed /= 2;
		gravity /= 2;
		gravityJump /= 2;
		maxFallSpeed /= 2;
	}
	if(e->jump_time > 0) {
		if(e->controller[0] & BUTTON_C) {
			e->jump_time--;
		} else {
			e->jump_time = 0;
		}
	}
	if(e->jump_time > 0) return;
	if(e->grounded) {
		if((e->controller[0] & BUTTON_C) && !(e->controller[1] & BUTTON_C)) {
			e->grounded = false;
			e->y_speed = -jumpSpeed;
			e->jump_time = MAX_JUMP_TIME;
		}
	} else {
		if((e->controller[0] & BUTTON_C) && e->y_speed >= 0) {
			e->y_speed += gravityJump;
		} else {
			e->y_speed += gravity;
		}
		if(e->y_speed > maxFallSpeed) {
			e->y_speed = maxFallSpeed;
		}
	}
}

void entity_update_float(Entity *e) {
	s16 acc = WALK_ACCEL;
	s16 fric = FRICTION;
	s16 max_speed = MAX_WALK_SPEED;
	if(e->controller[0] & BUTTON_LEFT) {
		e->x_speed -= acc;
		if (e->x_speed < -max_speed) {
			e->x_speed = -max_speed;
		}
	} else if(e->controller[0] & BUTTON_RIGHT) {
		e->x_speed += acc;
		if (e->x_speed > max_speed) {
			e->x_speed = max_speed;
		}
	} else {
		if(e->x_speed < fric && e->x_speed > -fric) {
			e->x_speed = 0;
		} else if (e->x_speed < 0) {
			e->x_speed += fric;
		} else if (e->x_speed > 0) {
			e->x_speed -= fric;
		}
	}
	if(e->controller[0] & BUTTON_UP) {
		e->y_speed -= acc;
		if (e->y_speed < -max_speed) {
			e->y_speed = -max_speed;
		}
	} else if(e->controller[0] & BUTTON_DOWN) {
		e->y_speed += acc;
		if (e->y_speed > max_speed) {
			e->y_speed = max_speed;
		}
	} else {
		if(e->y_speed < fric && e->y_speed > -fric) {
			e->y_speed = 0;
		} else if(e->y_speed < 0) {
			e->y_speed += fric;
		} else if(e->y_speed > 0) {
			e->y_speed -= fric;
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

bool collide_stage_leftwall(Entity *e) {
	u16 block_x, block_y1, block_y2;
	u8 pxa1, pxa2;
	block_x = pixel_to_block(sub_to_pixel(e->x_next) - e->hit_box.left);
	block_y1 = pixel_to_block(sub_to_pixel(e->y_next) - e->hit_box.top + 4);
	block_y2 = pixel_to_block(sub_to_pixel(e->y_next) + e->hit_box.bottom - 3);
	pxa1 = stage_get_block_type(block_x, block_y1);
	pxa2 = stage_get_block_type(block_x, block_y2);
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->eflags|e->nflags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		e->x_speed = 0;
		e->x_next = pixel_to_sub(
			block_to_pixel(block_x) + block_to_pixel(1) + e->hit_box.left);
		return true;
	}
	return false;
}

bool collide_stage_rightwall(Entity *e) {
	u16 block_x, block_y1, block_y2;
	u8 pxa1, pxa2;
	block_x = pixel_to_block(sub_to_pixel(e->x_next) + e->hit_box.right);
	block_y1 = pixel_to_block(sub_to_pixel(e->y_next) - e->hit_box.top + 4);
	block_y2 = pixel_to_block(sub_to_pixel(e->y_next) + e->hit_box.bottom - 3);
	pxa1 = stage_get_block_type(block_x, block_y1);
	pxa2 = stage_get_block_type(block_x, block_y2);
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->eflags|e->nflags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		e->x_speed = 0;
		e->x_next = pixel_to_sub(
				block_to_pixel(block_x) - e->hit_box.right);
		return true;
	}
	return false;
}

bool collide_stage_floor(Entity *e) {
	u16 pixel_x1, pixel_x2, pixel_y;
	u8 pxa1, pxa2;
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 1;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 1;
	pixel_y = sub_to_pixel(e->y_next) + e->hit_box.bottom;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->eflags|e->nflags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SOUND_THUD, 2);
		e->y_speed = 0;
		e->y_next = pixel_to_sub((pixel_y&~0xF) - e->hit_box.bottom);
		return true;
	}
	bool result = false;
	if((pxa1&0x10) && (pxa1&0xF) >= 4 && (pxa1&0xF) < 6 &&
			pixel_y%16 >= heightmap[pxa1%2][pixel_x1%16]) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SOUND_THUD, 2);
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa1%2][pixel_x1%16] - e->hit_box.bottom);
		e->y_speed = 0;
		result = true;
	}
	if((pxa2&0x10) && (pxa2&0xF) >= 6 && (pxa2&0xF) < 8 &&
			pixel_y%16 >= 0xF - heightmap[pxa2%2][pixel_x2%16]) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SOUND_THUD, 2);
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 0xF + 1 -
				heightmap[pxa2%2][pixel_x2%16] - e->hit_box.bottom);
		e->y_speed = 0;
		result = true;
	}
	return result;
}

u8 read_slope_table(s16 x, s16 y) {
	s16 mx, my;
	u8 t, type;
	mx = pixel_to_block(x);
	my = pixel_to_block(y);
	if (mx < 0 || my < 0 || mx >= stageWidth || my >= stageHeight)
		return 0;
	t = stage_get_block_type(mx, my);
	if(t & BLOCK_SLOPE) {
		type = (t & 0x07) + 1;
		return type;
	}
	return 0;
}

bool collide_stage_slope_grounded(Entity *e) {
	u16 pixel_x1, pixel_x2, pixel_y;
	u8 pxa1, pxa2;
	bool result = false;
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 1;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 1;
	// If we are on flat ground and run up to a slope
	pixel_y = sub_to_pixel(e->y_next) + e->hit_box.bottom - 1;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	if((pxa1&0x10) && (pxa1&0xF) >= 4 && (pxa1&0xF) < 6 &&
			pixel_y%16 >= heightmap[pxa1%4][pixel_x1%16]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa1%4][pixel_x1%16] - e->hit_box.bottom);
		e->y_speed = 0;
		result = true;
	}
	if((pxa2&0x10) && (pxa2&0xF) >= 6 && (pxa2&0xF) < 8 &&
			pixel_y%16 >= heightmap[pxa2%4][pixel_x2%16]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa2%4][pixel_x2%16] - e->hit_box.bottom);
		e->y_speed = 0;
		result = true;
	}
	if(result) return true;
	// If we're already on a slope
	pixel_y = sub_to_pixel(e->y_next) + e->hit_box.bottom + 1;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	if((pxa1&0x10) && (pxa1&0xF) >= 4 && (pxa1&0xF) < 6 &&
			pixel_y%16 >= heightmap[pxa1%4][pixel_x1%16]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa1%4][pixel_x1%16] - e->hit_box.bottom);
		e->y_speed = 0;
		result = true;
	}
	if((pxa2&0x10) && (pxa2&0xF) >= 6 && (pxa2&0xF) < 8 &&
			pixel_y%16 >= heightmap[pxa2%4][pixel_x2%16]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa2%4][pixel_x2%16] - e->hit_box.bottom);
		e->y_speed = 0;
		result = true;
	}
	return result;
}

bool collide_stage_floor_grounded(Entity *e) {
	bool result = false;
	// If we aren't moving we're still on the ground
	// Lolno im floating an sheit
	//if(e->y_speed == 0 && e->x_speed == 0) return true;
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
		result = true;
	}
	if(collide_stage_slope_grounded(e)) {
		result = true;
	}
	return result;
}

bool collide_stage_ceiling(Entity *e) {
	u16 pixel_x1, pixel_x2, pixel_y;
	u8 pxa1, pxa2;
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 2;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 2;
	pixel_y = sub_to_pixel(e->y_next) - e->hit_box.top + 2;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	if(pxa1 == 0x41 || pxa2 == 0x41 || pxa1 == 0x43 || pxa2 == 0x43 ||
			(!((e->eflags|e->nflags)&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		if(e == &player && e->y_speed < -0xFF) sound_play(SOUND_HEADBONK, 2);
		e->y_speed = 0;
		e->y_next = pixel_to_sub((pixel_y&~0xF) + e->hit_box.top) + block_to_sub(1);
		e->jump_time = 0;
		return true;
	}
	bool result = false;
	if((pxa1&0x10) && (pxa1&0xF) >= 0 && (pxa1&0xF) < 2 &&
			pixel_y%16 <= 0xF - heightmap[pxa1%2][pixel_x1%16]) {
		if(e == &player && e->y_speed < -0xFF) sound_play(SOUND_HEADBONK, 2);
		e->y_next = pixel_to_sub((pixel_y&~0xF) + 0xF -
				heightmap[pxa1%2][pixel_x1%16] + e->hit_box.top);
		e->y_speed = 0;
		result = true;
	}
	if((pxa2&0x10) && (pxa2&0xF) >= 2 && (pxa2&0xF) < 4 &&
			pixel_y%16 <= heightmap[pxa2%2][pixel_x2%16]) {
		if(e == &player && e->y_speed < -0xFF) sound_play(SOUND_HEADBONK, 2);
		e->y_next = pixel_to_sub((pixel_y&~0xF) +
				heightmap[pxa2%2][pixel_x2%16] + e->hit_box.top);
		e->y_speed = 0;
		result = true;
	}
	return result;
}

bool entity_overlapping(Entity *a, Entity *b) {
	s16 ax1 = sub_to_pixel(a->x) - (a->direction ? a->hit_box.right : a->hit_box.left),
		ax2 = sub_to_pixel(a->x) + (a->direction ? a->hit_box.left : a->hit_box.right),
		ay1 = sub_to_pixel(a->y) - a->hit_box.top,
		ay2 = sub_to_pixel(a->y) + a->hit_box.bottom,
		bx1 = sub_to_pixel(b->x) - (b->direction ? b->hit_box.right : b->hit_box.left),
		bx2 = sub_to_pixel(b->x) + (b->direction ? b->hit_box.left : b->hit_box.right),
		by1 = sub_to_pixel(b->y) - b->hit_box.top,
		by2 = sub_to_pixel(b->y) + b->hit_box.bottom;
	return (ax1 < bx2 && ax2 > bx1 && ay1 < by2 && ay2 > by1);
}

bounding_box entity_react_to_collision(Entity *a, Entity *b) {
	bounding_box result = { 0, 0, 0, 0 };
	s16 ax1 = sub_to_pixel(a->x_next) - (a->direction ? a->hit_box.right : a->hit_box.left),
		ax2 = sub_to_pixel(a->x_next) + (a->direction ? a->hit_box.left : a->hit_box.right),
		ay1 = sub_to_pixel(a->y_next) - a->hit_box.top,
		ay2 = sub_to_pixel(a->y_next) + a->hit_box.bottom,
		bx1 = sub_to_pixel(b->x) - (b->direction ? b->hit_box.right : b->hit_box.left),
		bx2 = sub_to_pixel(b->x) + (b->direction ? b->hit_box.left : b->hit_box.right),
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
		s16 move1 = pixel_to_sub(by2 - ay1);
		s16 move2 = pixel_to_sub(by1 - ay2) + pixel_to_sub(1);
		if(abs(move1) < abs(move2)) {
			result.top = 1;
			a->y_next += move1;
			if(a->y_speed < 0) a->y_speed = 0;
		} else {
			result.bottom = 1;
			a->y_next += move2;
			if(a->y_speed > 0) a->y_speed = 0;
			a->grounded = true;
		}
	}
	return result;
}

Entity *entity_find_by_id(u16 id) {
	Entity *e = entityList;
	while(e != NULL) {
		if(e->id == id) return e;
		else e = e->next;
	}
	e = inactiveList;
	while(e != NULL) {
		if(e->id == id) return e;
		else e = e->next;
	}
	return NULL;
}

Entity *entity_find_by_event(u16 event) {
	Entity *e = entityList;
	while(e != NULL) {
		if(e->event == event) return e;
		else e = e->next;
	}
	e = inactiveList;
	while(e != NULL) {
		if(e->event == event) return e;
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

bool entity_on_screen(Entity *obj) {
	return obj->x > camera.x - pixel_to_sub(SCREEN_HALF_W + 32) &&
			obj->x < camera.x + pixel_to_sub(SCREEN_HALF_W + 32) &&
			obj->y > camera.y - pixel_to_sub(SCREEN_HALF_H + 32) &&
			obj->y < camera.y + pixel_to_sub(SCREEN_HALF_H + 32);
}


void entity_drop_powerup(Entity *e) {
	u8 chance = random() % 5;
	u16 bx = sub_to_block(e->x), by = sub_to_block(e->y);
	if(chance >= 3) { // Weapon Energy
		s16 i = e->experience;
		for(; i >= 5; i -= 5) { // Big
			Entity *exp = entity_create(bx, by, 0, 0, 1, 0, 0);
			exp->experience = 5;
			SPR_SAFEANIM(e->sprite, 2);
		}
		for(; i >= 3; i -= 3) { // Med
			Entity *exp = entity_create(bx, by, 0, 0, 1, 0, 0);
			exp->experience = 3;
			SPR_SAFEANIM(e->sprite, 1);
		}
		for(; i > 0; i--) { // Small
			Entity *exp = entity_create(bx, by, 0, 0, 1, 0, 0);
			exp->experience = 1;
		}
		
	} else if(chance == 2 && (player_has_weapon(WEAPON_MISSILE) || 
		player_has_weapon(WEAPON_SUPERMISSILE))) { // Missiles
		if(e->experience > 6) {
			Entity *msl = entity_create(bx, by, 0, 0, 86, NPC_OPTION2, 0);
			msl->experience = 3;
		} else {
			Entity *msl = entity_create(bx, by, 0, 0, 86, 0, 0);
			msl->experience = 3;
		}
	} else { // Heart
		if(e->experience > 6) {
			Entity *heart = entity_create(bx, by, 0, 0, 87, NPC_OPTION2, 0);
			heart->health = 5;
		} else {
			Entity *heart = entity_create(bx, by, 0, 0, 87, 0, 0);
			heart->health = 2;
		}
	}
}

void entity_default(Entity *e, u16 type, u16 flags) {
	// Depending on the NPC type, apply default values
	e->type = type;
	if(type <= 360) {
		e->nflags = npc_flags(type);
		e->health = npc_health(type);
		e->attack = npc_attack(type);
		e->experience = npc_experience(type);
		e->hurtSound = npc_hurtSound(type);
		e->deathSound = npc_deathSound(type);
		e->deathSmoke = npc_deathSmoke(type);
		e->hit_box = npc_hitBox(type);
		e->display_box = npc_displayBox(type);
	} else {
		e->nflags = 0;
		e->health = 1;
		e->attack = 0;
		e->experience = 0;
		e->hurtSound = 0;
		e->deathSound = 0;
		e->deathSmoke = 0;
		e->hit_box = (bounding_box) { 8, 8, 8, 8 };
		e->display_box = (bounding_box) { 8, 8, 8, 8 };
	}
	e->eflags |= flags;
	e->x_speed = 0;
	e->y_speed = 0;
	e->direction = 0;
	e->grounded = false;
	e->underwater = false;
	e->damage_value = 0;
	e->damage_time = 0;
	e->sprite = NULL;
	e->spriteFrame = 0;
	e->spriteAnim = 0;
	e->spriteVFlip = 0;
	e->state = 0;
	e->state_time = 0;
}

bool entity_disabled(Entity *e) {
	// Some entities should not be activated until a specific system flag is set,
	// and some no longer appear once one is set
	if((e->eflags&NPC_ENABLEONFLAG) && !system_get_flag(e->id)) return true;
	//if((e->eflags&NPC_DISABLEONFLAG) && system_get_flag(e->id)) return true;
	return false;
}

Entity *entity_create(u16 x, u16 y, u16 id, u16 event, u16 type, u16 flags, u8 direction) {
	if((flags&NPC_DISABLEONFLAG) && system_get_flag(id)) return NULL;
	// Allocate memory and start applying values
	Entity *e = MEM_alloc(sizeof(Entity));
	e->next = NULL; e->prev = NULL;
	e->x = block_to_sub(x) + pixel_to_sub(8);
	e->y = block_to_sub(y) + pixel_to_sub(8);
	e->id = id;
	e->event = event;
	e->eflags = flags;
	if(stageID == 28) {
		e->alwaysActive = true;
	} else {
		e->alwaysActive = false;
	}
	entity_default(e, type, 0);
	e->direction = direction;
	ENTITY_ONCREATE(e);
	if(e->alwaysActive || (entity_on_screen(e) && !entity_disabled(e))) {
		LIST_PUSH(entityList, e);
		sprite_create(e);
	} else {
		LIST_PUSH(inactiveList, e);
		e->sprite = NULL;
	}
	return e;
}

Entity *entity_create_boss(u16 x, u16 y, u8 bossid, u16 event) {
	Entity *e = MEM_alloc(sizeof(Entity));
	e->x = block_to_sub(x) + pixel_to_sub(8);
	e->y = block_to_sub(y) + pixel_to_sub(8);
	e->id = 0;
	e->event = event;
	e->eflags = NPC_SOLID | NPC_SHOOTABLE | NPC_EVENTONDEATH | NPC_SHOWDAMAGE;
	entity_default(e, 360 + bossid, 0);
	e->alwaysActive = true;
	ENTITY_ONCREATE(e);
	LIST_PUSH(entityList, e);
	bossEntity = e;
	sprite_create(e);
	return e;
}

void entities_replace(u16 event, u16 type, u8 direction, u16 flags) {
	Entity *e = entityList;
	while(e != NULL) {
		if(e->event == event) {
			SPR_SAFERELEASE(e->sprite);
			entity_default(e, type, flags);
			e->direction = direction;
			ENTITY_ONCREATE(e);
			sprite_create(e);
		}
		e = e->next;
	}
	e = inactiveList;
	while(e != NULL) {
		if(e->event == event) {
			entity_default(e, type, flags);
			e->direction = direction;
			ENTITY_ONCREATE(e);
			e->sprite = NULL;
		}
		e = e->next;
	}
}

void entities_set_state(u16 event, u16 state, u8 direction) {
	Entity *e = entityList;
	while(e != NULL) {
		if(e->event == event) {
			if(e->direction != direction){
				e->direction = direction;
				SPR_SAFEHFLIP(e->sprite, direction);
			}
			ENTITY_SET_STATE(e, state, 0);
		}
		e = e->next;
	}
	e = inactiveList;
	while(e != NULL) {
		if(e->event == event) {
			e->direction = direction;
			ENTITY_SET_STATE(e, state, 0);
		}
		e = e->next;
	}
}

void entities_move(u16 event, u16 x, u16 y, u8 direction) {
	Entity *e = entityList;
	while(e != NULL) {
		if(e->event == event) {
			if(e->direction != direction){
				e->direction = direction;
				SPR_SAFEHFLIP(e->sprite, direction);
			}
			e->x = block_to_sub(x) + pixel_to_sub(8);
			e->y = block_to_sub(y) + pixel_to_sub(8);
			e->grounded = false;
			break;
		}
		e = e->next;
	}
	e = inactiveList;
	while(e != NULL) {
		if(e->event == event) {
			e->direction = direction;
			e->x = block_to_sub(x) + pixel_to_sub(8);
			e->y = block_to_sub(y) + pixel_to_sub(8);
			e->grounded = false;
			break;
		}
		e = e->next;
	}
}

void entities_pause() {
	while(entityList != NULL) entity_deactivate(entityList);
}

void entities_unpause() {
	entities_update_inactive();
}

bool entity_exists(u16 type) {
	Entity *e = entityList;
	while(e != NULL) {
		if(e->type == type) return true;
		e = e->next;
	}
	return false;
}
