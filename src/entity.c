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

// List functions
void list_clear(Entity *first);
Entity *list_get_prev(Entity *e, Entity *list);

Entity *entity_update(Entity *e);
Entity *entity_update_inactive(Entity *e);
bool collide_stage_leftwall(Entity *e);
bool collide_stage_rightwall(Entity *e);
bool collide_stage_floor(Entity *e);
bool collide_stage_floor_grounded(Entity *e);
bool collide_stage_cieling(Entity *e);

void list_clear(Entity *first) {
	Entity *e;
	while(first != NULL) {
		e = first->next;
		MEM_free(first);
		first = e;
	}
}

Entity *list_get_prev(Entity *e, Entity *list) {
	while(list->next != NULL) {
		if(list->next == e) return list;
		list = list->next;
	}
	return NULL;
}

void entities_clear() {
	list_clear(entityList);
	list_clear(inactiveList);
	// Might not be needed
	entityList = NULL;
	inactiveList = NULL;
}

void entity_deactivate(Entity *e) {
	Entity *next = e->next;
	Entity *prev = list_get_prev(e, entityList);
	e->next = inactiveList;
	inactiveList = e;
	if(prev == NULL) {
		entityList = next;
	} else {
		prev->next = next;
	}
}

void entity_reactivate(Entity *e) {
	Entity *next = e->next;
	Entity *prev = list_get_prev(e, inactiveList);
	e->next = entityList;
	entityList = e;
	if(prev == NULL) {
		inactiveList = next;
	} else {
		prev->next = next;
	}
}

void entities_clear_id(u16 id) {
	Entity *e, *temp;
	// First active element
	while(entityList->id == id) {
		temp = entityList->next;
		MEM_free(entityList);
		entityList = temp;
	}
	// Other active elements
	e = entityList;
	while(e->next != NULL) {
		if(e->next->id == id) {
			if(e->next->sprite != SPRITE_NONE) sprite_delete(e->next->sprite);
			temp = e->next->next;
			MEM_free(e->next);
			e->next = temp;
		}
		else e = e->next;
	}

	// First inactive element
	while(inactiveList->id == id) {
		temp = inactiveList->next;
		MEM_free(inactiveList);
		inactiveList = temp;
	}
	// Other inactive elements
	e = inactiveList;
	while(e->next != NULL) {
		if(e->next->id == id) {
			temp = e->next->next;
			MEM_free(e->next);
			e->next = temp;
		}
		else e = e->next;
	}
}

void entities_clear_event(u16 event) {
	Entity *e, *temp;
	// First active element
	while(entityList->event == event) {
		temp = entityList->next;
		MEM_free(entityList);
		entityList = temp;
	}
	// Other active elements
	e = entityList;
	while(e->next != NULL) {
		if(e->next->event == event) {
			if(e->next->flags & NPC_DISABLEONFLAG) system_set_flag(e->next->id, true);
			if(e->next->sprite != SPRITE_NONE) sprite_delete(e->next->sprite);
			temp = e->next->next;
			MEM_free(e->next);
			e->next = temp;
		}
		else e = e->next;
	}

	// First inactive element
	while(inactiveList->event == event) {
		temp = inactiveList->next;
		MEM_free(inactiveList);
		inactiveList = temp;
	}
	// Other inactive elements
	e = inactiveList;
	while(e->next != NULL) {
		if(e->next->event == event) {
			temp = e->next->next;
			MEM_free(e->next);
			e->next = temp;
		}
		else e = e->next;
	}
}

void entities_clear_type(u16 type) {
	Entity *e, *temp;
	// First active element
	while(entityList->type == type) {
		temp = entityList->next;
		MEM_free(entityList);
		entityList = temp;
	}
	// Other active elements
	e = entityList;
	while(e->next != NULL) {
		if(e->next->type == type) {
			if(e->next->sprite != SPRITE_NONE) sprite_delete(e->next->sprite);
			temp = e->next->next;
			MEM_free(e->next);
			e->next = temp;
		}
		else e = e->next;
	}
	// First inactive element
	while(inactiveList->type == type) {
		temp = inactiveList->next;
		MEM_free(inactiveList);
		inactiveList = temp;
	}
	// Other inactive elements
	e = inactiveList;
	while(e->next != NULL) {
		if(e->next->type == type) {
			temp = e->next->next;
			MEM_free(e->next);
			e->next = temp;
		}
		else e = e->next;
	}
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
	//if(inactiveList != NULL) {
	//	Entity *e = inactiveList;
	//	while(e != NULL) {
	//		e = entity_update_inactive(e);
	//	}
	//}
	if(entityList != NULL) {
		Entity *e = entityList;
		while(e != NULL) {
			e = entity_update(e);
		}
	}
}

void entities_update_inactive() {
	if(inactiveList != NULL) {
		Entity *e = inactiveList;
		while(e != NULL) {
			e = entity_update_inactive(e);
		}
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
	if(e->grounded) {
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

bool entity_overlapping(Entity *a, Entity *b) {
	s16 ax1 = sub_to_pixel(a->x) - a->hit_box.left,
		ax2 = sub_to_pixel(a->x) + a->hit_box.right,
		ay1 = sub_to_pixel(a->y) - a->hit_box.top,
		ay2 = sub_to_pixel(a->y) + a->hit_box.bottom,
		bx1 = sub_to_pixel(b->x) - b->hit_box.left,
		bx2 = sub_to_pixel(b->x) + b->hit_box.right,
		by1 = sub_to_pixel(b->y) - b->hit_box.top,
		by2 = sub_to_pixel(b->y) + b->hit_box.bottom;
	return (ax1 < bx2 && ax2 > bx1 && ay1 < by2 && ay2 > by1);
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

bool entity_on_screen(Entity *obj) {
	return obj->x > camera.x - pixel_to_sub(SCREEN_HALF_W + 32) &&
			obj->x < camera.x + pixel_to_sub(SCREEN_HALF_W + 32) &&
			obj->y > camera.y - pixel_to_sub(SCREEN_HALF_H + 32) &&
			obj->y < camera.y + pixel_to_sub(SCREEN_HALF_H + 32);
}

Entity *entity_update_inactive(Entity *e) {
	if(entity_on_screen(e)) {
		if(npc_info[e->type].sprite != NULL)
			e->sprite = sprite_create(npc_info[e->type].sprite, npc_info[e->type].palette, false);
		//Entity *activated = e;
		//e = e->next;
		//entity_cut(activated);
		//entity_insert_after(activated, lastActiveEntity);
		Entity *next = e->next;
		entity_reactivate(e);
		return next;
	}
	return e->next;
}

Entity *entity_update(Entity *e) {
	if(!entity_on_screen(e)) {
		if(e->sprite != SPRITE_NONE) {
			sprite_delete(e->sprite);
			e->sprite = SPRITE_NONE;
		}
		Entity *next = e->next;
		entity_deactivate(e);
		return next;
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
						//e->sprite = SPRITE_NONE;
						//e->active = false;
						Entity *next = e->next;
						Entity *prev = list_get_prev(e, entityList);
						prev->next = next;
						MEM_free(e);
						return next;
					}
					e->health -= b->damage;
					sound_play(e->hurtSound, 5);
				}
			}
		}
	}
	if(e->sprite != SPRITE_NONE) {
		sprite_set_position(e->sprite,
			sub_to_pixel(e->x) - sub_to_pixel(camera.x) - (e->display_box.left-8) + SCREEN_HALF_W,
			sub_to_pixel(e->y) - sub_to_pixel(camera.y) - (e->display_box.top-8) + SCREEN_HALF_H);
	}
	return e->next;
}

Entity *entity_create(u16 x, u16 y, u16 id, u16 event, u16 type, u16 flags) {
	Entity *e = MEM_alloc(sizeof(Entity));
	e->x = block_to_sub(x);
	e->y = block_to_sub(y);
	e->id = id;
	e->event = event;
	e->type = type;
	// Apply NPC flags in addition to entity flags
	e->flags = flags | npc_flags(type);
	e->x_speed = 0;
	e->y_speed = 0;
	e->health = npc_health(type);
	e->attack = npc_attack(type);
	e->experience = npc_experience(type);
	e->hurtSound = npc_hurtSound(type);
	e->deathSound = npc_deathSound(type);
	if(entity_on_screen(e)) {
		e->next = entityList;
		entityList = e;
		if(npc_info[type].sprite != NULL) {
			e->sprite = sprite_create(npc_info[type].sprite, npc_info[type].palette, false);
		} else e->sprite = SPRITE_NONE;
	} else {
		e->next = inactiveList;
		inactiveList = e;
		e->sprite = SPRITE_NONE;

	}
	e->hit_box = npc_hitBox(type);
	e->display_box = npc_displayBox(type);
	return e;
}
