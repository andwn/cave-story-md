#include "entity.h"

#include <genesis.h>
#include "input.h"
#include "stage.h"
#include "resources.h"
#include "camera.h"
#include "system.h"
#include "sprite.h"
#include "tables.h"
#include "player.h"
#include "effect.h"
#include "audio.h"
#include "tsc.h"
#include "npc.h"

// Heightmaps for slopes
const u8 heightmap[4][16] = {
	{ 0x0,0x0,0x1,0x1,0x2,0x2,0x3,0x3,0x4,0x4,0x5,0x5,0x6,0x6,0x7,0x7 },
	{ 0x8,0x8,0x9,0x9,0xA,0xA,0xB,0xB,0xC,0xC,0xD,0xD,0xE,0xE,0xF,0xF },
	{ 0xF,0xF,0xE,0xE,0xD,0xD,0xC,0xC,0xB,0xB,0xA,0xA,0x9,0x9,0x8,0x8 },
	{ 0x7,0x7,0x6,0x6,0x5,0x5,0x4,0x4,0x3,0x3,0x2,0x2,0x1,0x1,0x0,0x0 },
};

// These are the values used by the original game halved,
// because a pixel here is 256 units instead of 512
s16 gravity = 0x50 >> 1, waterGravity = 0x28 >> 1,
	jumpGravity = 0x20 >> 1, waterJumpGravity = 0x10 >> 1;

void entity_update(Entity *e);
bool collide_stage_leftwall(Entity *e);
bool collide_stage_rightwall(Entity *e);
bool collide_stage_floor(Entity *e);
bool collide_stage_floor_grounded(Entity *e);
bool collide_stage_cieling(Entity *e);

void entities_clear() {
	for(u8 i = 0; i < MAX_ENTITIES; i++) {
		entities[i].active = false;
		//if(entities[i].sprite != SPRITE_NONE) sprite_delete(entities[i].sprite);
	}
}

void entities_clear_id(u16 id) {
	for(u8 i = 0; i < MAX_ENTITIES; i++) {
		if(entities[i].active && entities[i].id == id) {
			entities[i].active = false;
			if(entities[i].sprite != SPRITE_NONE) sprite_delete(entities[i].sprite);
		}
	}
}

void entities_clear_event(u16 event) {
	for(u8 i = 0; i < MAX_ENTITIES; i++) {
		if(entities[i].active && entities[i].event == event) {
			entities[i].active = false;
			if(entities[i].sprite != SPRITE_NONE) sprite_delete(entities[i].sprite);
			if(entities[i].flags&NPC_DISABLEONFLAG)
				system_set_flag(entities[i].id, true);
		}
	}
}

void entities_clear_type(u16 type) {
	for(u8 i = 0; i < MAX_ENTITIES; i++) {
		if(entities[i].active && entities[i].type == type) {
			entities[i].active = false;
			if(entities[i].sprite != SPRITE_NONE) sprite_delete(entities[i].sprite);
		}
	}
}

u8 entities_count() {
	u8 count = 0;
	for(u8 i = 0; i < MAX_ENTITIES; i++) {
		if(entities[i].active) count++;
	}
	return count;
}

void entities_update() {
	for(u8 i = 0; i < MAX_ENTITIES; i++) {
		if(!entities[i].active) continue;
		entity_update(&entities[i]);
	}
}

void entity_update_movement(Entity *e) {
	if (e->flags&NPC_IGNORESOLID) {
		entity_update_float(e);
	} else {
		entity_update_walk(e);
		entity_update_jump(e);
	}
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
}

void entity_update_walk(Entity *e) {
	s16 acc = e->accel,
		friction = 0x33 >> 1;
	if (!e->grounded) {
		acc = jumpGravity;
	}
	if (e->controller[0] & BUTTON_LEFT) {
		e->x_speed -= acc;
		if (e->x_speed < -e->max_speed) {
			e->x_speed = -e->max_speed;
		}
	} else if (e->controller[0] & BUTTON_RIGHT) {
		e->x_speed += acc;
		if (e->x_speed > e->max_speed) {
			e->x_speed = e->max_speed;
		}
	} else {
		if (e->x_speed < friction && e->x_speed > -friction) {
			e->x_speed = 0;
		} else if (e->x_speed < 0) {
			e->x_speed += friction;
		} else if (e->x_speed > 0) {
			e->x_speed -= friction;
		}
	}
}

void entity_update_jump(Entity *e) {
	const u8 maxJumpTime = 20;
	if(e->controller[0] & BUTTON_Z) e->jump_time = 1;
	if (e->jump_time > 0) {
		if (e->controller[0] & BUTTON_C) {
			e->y_speed = -0x280;
			e->jump_time--;
		} else {
			e->jump_time = 0;
		}
	}
	if (e->jump_time > 0) return;
	if (e->grounded) {
		if ((e->controller[0] & BUTTON_C) && !(e->controller[1] & BUTTON_C)) {
			e->grounded = false;
			e->y_speed = -0x280;
			e->jump_time = maxJumpTime;
		}
	} else {
		if ((e->controller[0] & BUTTON_C) && e->y_speed > 0) {
				e->y_speed += jumpGravity;
		} else {
			e->y_speed += gravity;
		}
		if (e->y_speed > 0x2FF) {
			e->y_speed = 0x2FF;
		}
	}
}

void entity_update_float(Entity *e)
{
	
}

void entity_update_collision(Entity *e) {
	if(e->x_speed < 0) {
		collide_stage_leftwall(e);
	} else if (e->x_speed > 0) {
		collide_stage_rightwall(e);
	}
	if(e->grounded /*&& e->x_speed != 0*/) {
		e->grounded = collide_stage_floor_grounded(e);
	} else if(e->y_speed > 0) {
		e->grounded = collide_stage_floor(e);
		return;
	}
	if(e->y_speed < 0) {
		collide_stage_cieling(e);
	}
}

bool collide_stage_leftwall(Entity *e) {
	u16 block_x, block_y1, block_y2;
	u8 pxa1, pxa2;
	block_x = pixel_to_block(sub_to_pixel(e->x_next) - e->hit_box.left);
	block_y1 = pixel_to_block(sub_to_pixel(e->y_next) - e->hit_box.top + 3);
	block_y2 = pixel_to_block(sub_to_pixel(e->y_next) + e->hit_box.bottom - 3);
	pxa1 = stage_get_block_type(block_x, block_y1);
	pxa2 = stage_get_block_type(block_x, block_y2);
	if(pxa1 == 0x41 || pxa2 == 0x41 ||
			(!(e->flags&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
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
	block_y1 = pixel_to_block(sub_to_pixel(e->y_next) - e->hit_box.top + 3);
	block_y2 = pixel_to_block(sub_to_pixel(e->y_next) + e->hit_box.bottom - 3);
	pxa1 = stage_get_block_type(block_x, block_y1);
	pxa2 = stage_get_block_type(block_x, block_y2);
	if(pxa1 == 0x41 || pxa2 == 0x41 ||
			(!(e->flags&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
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
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 2;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 2;
	pixel_y = sub_to_pixel(e->y_next) + e->hit_box.bottom;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	if(pxa1 == 0x41 || pxa2 == 0x41 ||
			(!(e->flags&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		e->y_speed = 0;
		e->y_next = pixel_to_sub(
				pixel_y - e->hit_box.bottom);
		return true;
	}
	bool result = false;
	if((pxa1&0x30) && (pxa1&0xF) >= 4 && (pxa1&0xF) < 6 &&
			pixel_y%16 >= heightmap[pxa1%2][pixel_x1%16]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa1%2][pixel_x1%16] - e->hit_box.bottom);
		e->y_speed = 0;
		result = true;
	}
	if((pxa2&0x30) && (pxa2&0xF) >= 6 && (pxa2&0xF) < 8 &&
			pixel_y%16 >= 0xF - heightmap[pxa2%2][pixel_x2%16]) {
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 0xF + 1 -
				heightmap[pxa2%2][pixel_x2%16] - e->hit_box.bottom);
		e->y_speed = 0;
		result = true;
	}
	return result;
}

bool collide_stage_floor_grounded(Entity *e) {
	u16 pixel_x1, pixel_x2, pixel_y1, pixel_y2;
	u8 pxa1, pxa2;//, slope;
	//bool result = false;
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 2;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 2;
	pixel_y1 = sub_to_pixel(e->y_next) + e->hit_box.bottom - 2;
	pixel_y2 = sub_to_pixel(e->y_next) + e->hit_box.bottom + 2;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y1));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y1));
	// Check for rising slopes
	if((pxa1&0x30) && (pxa1&0xF) >= 4 && (pxa1&0xF) < 8) {
		// Walking up left slope
		s32 y_new = pixel_to_sub((pixel_y1&0xFFF0) + 1 +
				heightmap[pxa1%4][pixel_x1%16] - e->hit_box.bottom);
		if(y_new <= e->y_next) {
			e->y_next = y_new;
			e->y_speed = 0;
			return true;
		}
	}
	if((pxa2&0x30) && (pxa2&0xF) >= 4 && (pxa2&0xF) < 8) {
		// Walking up right slope
		s32 y_new = pixel_to_sub((pixel_y1&0xFFF0) + 1 +
				heightmap[pxa2%4][pixel_x2%16] - e->hit_box.bottom);
		if(y_new <= e->y_next) {
			e->y_next = y_new;
			e->y_speed = 0;
			return true;
		}
	}
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y2));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y2));
	// Check for solid blocks
	if(pxa1 == 0x41 || pxa2 == 0x41 ||
		(!(e->flags&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		return true;
	}
	// Check for lowering slopes
	//slope = (pxa1 >= 0x54 && pxa1 < 0x58) + ((pxa1 >= 0x54 && pxa1 < 0x58)<<1);
	//if(slope & 1) {
	//	obj->y_next += pixel_to_sub(collide_slope_sticky(pxa1,
	//			sub_to_pixel(obj->x_next % 16), sub_to_pixel(pixel_y2 % 16)));
	//	result = true;
	//}
	//if(slope & 2) {
	//	obj->y_next += pixel_to_sub(collide_slope_sticky(pxa2,
	//			sub_to_pixel(obj->x_next % 16), sub_to_pixel(pixel_y2 % 16)));
	//	result = true;
	//}
	//if(result) return true;
	return false;
}

bool collide_stage_cieling(Entity *e) {
	u16 block_x1, block_x2, block_y;
	u8 pxa1, pxa2;
	block_x1 = pixel_to_block(sub_to_pixel(e->x_next) - e->hit_box.left + 2);
	block_x2 = pixel_to_block(sub_to_pixel(e->x_next) + e->hit_box.right - 2);
	block_y = pixel_to_block(sub_to_pixel(e->y_next) - e->hit_box.top);
	pxa1 = stage_get_block_type(block_x1, block_y);
	pxa2 = stage_get_block_type(block_x2, block_y);
	if(pxa1 == 0x41 || pxa2 == 0x41 ||
			(!(e->flags&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		e->y_speed = 0;
		e->y_next = pixel_to_sub(
				block_to_pixel(block_y) + block_to_pixel(1) + e->hit_box.top);
		e->jump_time = 0;
		return true;
	}
	return false;
}

Entity *entity_find_by_id(u16 id) {
	for(u8 i = 0; i < MAX_ENTITIES; i++) {
		if(entities[i].active && entities[i].id == id) return &entities[i];
	}
	return NULL;
}

u8 first_inactive() {
	for(u8 i = 0; i < MAX_ENTITIES; i++) if(!entities[i].active) return i;
	return MAX_ENTITIES;
}

bool entity_on_screen(Entity *obj) {
	return obj->x > camera.x - pixel_to_sub(SCREEN_HALF_W + 32) &&
			obj->x < camera.x + pixel_to_sub(SCREEN_HALF_W + 32) &&
			obj->y > camera.y - pixel_to_sub(SCREEN_HALF_H + 32) &&
			obj->y < camera.y + pixel_to_sub(SCREEN_HALF_H + 32);
}

void entity_update(Entity *e) {
	if(e->onScreen) {
		if(!entity_on_screen(e)) {
			e->onScreen = false;
			if(e->sprite != SPRITE_NONE) {
				sprite_delete(e->sprite);
				e->sprite = SPRITE_NONE;
			}
			return;
		}
	} else if(entity_on_screen(e)) {
		e->onScreen = true;
		if(npc_info[e->type].sprite != NULL)
			e->sprite = sprite_create(npc_info[e->type].sprite, npc_info[e->type].palette, false);
	} else {
		return;
	}
	entity_update_movement(e);
	if(!(e->flags & NPC_IGNORESOLID)) entity_update_collision(e);
	if((e->flags & NPC_SHOOTABLE)) {
		for(u8 i = 0; i < 3; i++) {
			if(playerBullet[i].ttl > 0) {
				Bullet *b = &playerBullet[i];
				s16 bx = sub_to_pixel(b->x), by = sub_to_pixel(b->y);
				if(bx-4 < sub_to_pixel(e->x)+e->hit_box.right &&
						bx+4 > sub_to_pixel(e->x)-e->hit_box.left &&
						by-4 < sub_to_pixel(e->y)+e->hit_box.bottom &&
						by+4 > sub_to_pixel(e->y)-e->hit_box.top) {
					b->ttl = 0;
					sprite_delete(b->sprite);
					if(e->flags & NPC_SHOWDAMAGE)
						effect_create_damage_string(-b->damage, bx, by, 60);
					if(e->health <= b->damage) {
						// Killed enemy
						e->health = 0;
						//soundmgr_play(entities[j].deathSound, 5);
						// entity_create(e->x, e->y, 0, 0, (exp), 0);
						// effect_create_smoke(e->x, e->y);
						if(e->flags & NPC_EVENTONDEATH) tsc_call_event(e->event);
						if(e->flags & NPC_DISABLEONFLAG) system_set_flag(e->id, true);
						sprite_delete(e->sprite);
						e->sprite = SPRITE_NONE;
						e->active = false;
						return;
					}
					e->health -= b->damage;
					sound_play(e->hurtSound, 5);
				}
			}
		}
	}
	if(e->sprite == SPRITE_NONE) return;
	sprite_set_position(e->sprite,
		sub_to_pixel(e->x) - sub_to_pixel(camera.x) - (e->display_box.left-8) + SCREEN_HALF_W,
		sub_to_pixel(e->y) - sub_to_pixel(camera.y) - (e->display_box.top-8) + SCREEN_HALF_H);
}

Entity *entity_create(u16 x, u16 y, u16 id, u16 event, u16 type, u16 flags) {
	u8 index = first_inactive();
	if(index == MAX_ENTITIES) SYS_die("Too many objects!");
	Entity *e = &entities[index];
	e->x = block_to_sub(x);
	e->y = block_to_sub(y);
	e->id = id;
	e->event = event;
	e->type = type;
	// Apply NPC flags in addition to entity flags
	e->flags = flags | npc_flags(type); //npc_info[type].flags;
	e->x_speed = 0;
	e->y_speed = 0;
	e->health = npc_health(type); //npc_info[type].health;
	e->attack = npc_attack(type); //npc_info[type].damage;
	e->experience = npc_experience(type); //npc_info[type].experience;
	e->hurtSound = npc_hurtSound(type); //npc_info[type].hurtSound;
	e->deathSound = npc_deathSound(type); //npc_info[type].deathSound;
	e->onScreen = entity_on_screen(e);
	if(npc_info[type].sprite != NULL && e->onScreen) {
		e->sprite = sprite_create(npc_info[type].sprite, npc_info[type].palette, false);
	} else {
		e->sprite = SPRITE_NONE;
	}
	e->hit_box = npc_hitBox(type); //npc_info[type].hitBox;
	e->display_box = npc_displayBox(type); //npc_info[type].displayBox;
	e->active = true;
	return e;
}
