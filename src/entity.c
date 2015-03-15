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

// The original game runs at 50 Hz. The PAL values are copied from it
// NTSC values calculated with: value * (50.0 / 60.0)

#define MAX_FALL_SPEED_NTSC 0x4FF
#define MAX_FALL_SPEED_PAL 0x5FF
#define MAX_FALL_SPEED_WATER_NTSC 0x27F
#define MAX_FALL_SPEED_WATER_PAL 0x2FF

#define GRAVITY_NTSC 0x43
#define GRAVITY_PAL 0x50
#define GRAVITY_WATER_NTSC 0x21
#define GRAVITY_WATER_PAL 0x28

#define GRAVITY_JUMP_NTSC 0x1B
#define GRAVITY_JUMP_PAL 0x20
#define GRAVITY_JUMP_WATER_NTSC 0x0D
#define GRAVITY_JUMP_WATER_PAL 0x10

#define JUMP_SPEED_NTSC 0x430
#define JUMP_SPEED_PAL 0x500
#define JUMP_SPEED_WATER_NTSC 0x210
#define JUMP_SPEED_WATER_PAL 0x280

#define MAX_WALK_SPEED_NTSC 0x2A5
#define MAX_WALK_SPEED_PAL 0x32C
#define MAX_WALK_SPEED_WATER_NTSC 0x151
#define MAX_WALK_SPEED_WATER_PAL 0x196

#define WALK_ACCEL_NTSC 0x47
#define WALK_ACCEL_PAL 0x55
#define WALK_ACCEL_WATER_NTSC 0x23
#define WALK_ACCEL_WATER_PAL 0x2A

#define AIR_CONTROL_NTSC 0x1B
#define AIR_CONTROL_PAL 0x20
#define AIR_CONTROL_WATER_NTSC 0x0D
#define AIR_CONTROL_WATER_PAL 0x10

#define FRICTION_NTSC 0x2A // (This one is 42.499 and I went for 42 here)
#define FRICTION_PAL 0x33
#define FRICTION_WATER_NTSC 0x15
#define FRICTION_WATER_PAL 0x19

// Heightmaps for slopes
const u8 heightmap[4][16] = {
	{ 0x0,0x0,0x1,0x1,0x2,0x2,0x3,0x3,0x4,0x4,0x5,0x5,0x6,0x6,0x7,0x7 },
	{ 0x8,0x8,0x9,0x9,0xA,0xA,0xB,0xB,0xC,0xC,0xD,0xD,0xE,0xE,0xF,0xF },
	{ 0xF,0xF,0xE,0xE,0xD,0xD,0xC,0xC,0xB,0xB,0xA,0xA,0x9,0x9,0x8,0x8 },
	{ 0x7,0x7,0x6,0x6,0x5,0x5,0x4,0x4,0x3,0x3,0x2,0x2,0x1,0x1,0x0,0x0 },
};

// Not halved anymore, using 512 per pixel units now
s16 maxFallSpeed = MAX_FALL_SPEED_NTSC, maxFallSpeedWater = MAX_FALL_SPEED_WATER_NTSC,
	gravity = GRAVITY_NTSC, gravityWater = GRAVITY_WATER_NTSC,
	gravityJump = GRAVITY_JUMP_NTSC, gravityJumpWater = GRAVITY_JUMP_WATER_NTSC,
	jumpSpeed = JUMP_SPEED_NTSC, jumpSpeedWater = JUMP_SPEED_WATER_NTSC,
	maxWalkSpeed = MAX_WALK_SPEED_NTSC, maxWalkSpeedWater = MAX_WALK_SPEED_WATER_NTSC,
	walkAccel = WALK_ACCEL_NTSC, walkAccelWater = WALK_ACCEL_WATER_NTSC,
	airControl = AIR_CONTROL_NTSC, airControlWater = AIR_CONTROL_WATER_NTSC,
	friction = FRICTION_NTSC, frictionWater = FRICTION_WATER_NTSC;

// List functions
Entity *list_clear(Entity *list, u8 criteria, u16 value);
Entity *list_get_prev(Entity *e, Entity *list);

Entity *entity_update(Entity *e);
Entity *entity_update_inactive(Entity *e);
bool collide_stage_leftwall(Entity *e);
bool collide_stage_rightwall(Entity *e);
bool collide_stage_floor(Entity *e);
bool collide_stage_floor_grounded(Entity *e);
bool collide_stage_ceiling(Entity *e);

// Big switch statement for type specific stuff
void entity_create_special(Entity *e);

Entity *list_get_prev(Entity *e, Entity *list) {
	while(list->next != NULL) {
		if(list->next == e) return list;
		list = list->next;
	}
	return NULL;
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

bool entity_matches_criteria(Entity *e, u8 criteria, u16 value) {
	switch(criteria) {
	case FILTER_ID:
		return (e->id == value);
	case FILTER_EVENT:
		return (e->event == value);
	case FILTER_TYPE:
		return (e->type == value);
	case FILTER_ALL:
		return true;
	default:
		return false;
	}
}

// Clears a single linked list of entities, returns first element
Entity *list_clear(Entity *list, u8 criteria, u16 value) {
	Entity *e, *temp;
	// First element
	while(list != NULL) {
		if(entity_matches_criteria(list, criteria, value)) {
			if(list->flags&NPC_DISABLEONFLAG) system_set_flag(list->id, true);
			temp = list->next;
			MEM_free(entityList);
			list = temp;
		} else {
			break;
		}
	}
	if(list == NULL) return NULL; // If we cleared every entity
	// Other elements
	e = list;
	while(e->next != NULL) {
		if(entity_matches_criteria(e->next, criteria, value)) {
			if(e->next->sprite != SPRITE_NONE) sprite_delete(e->next->sprite);
			if(e->next->flags&NPC_DISABLEONFLAG) system_set_flag(e->next->id, true);
			temp = e->next->next;
			MEM_free(e->next);
			e->next = temp;
		} else {
			e = e->next;
		}
	}
	return list;
}

void entities_clear(u8 criteria, u16 value) {
	entityList = list_clear(entityList, criteria, value);
	inactiveList = list_clear(inactiveList, criteria, value);
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

// TODO: Water for all these functions
// TODO: If "friction" isn't supposed to be used in the air, what value is?
void entity_update_walk(Entity *e) {
	s16 acc = walkAccel,
		fric = friction,
		max_speed = maxWalkSpeed;
	if (!e->grounded) acc = airControl;
	if (e->controller[0] & BUTTON_LEFT) {
		e->x_speed -= acc;
		if (e->x_speed < -max_speed) {
			e->x_speed = -max_speed;
		}
	} else if (e->controller[0] & BUTTON_RIGHT) {
		e->x_speed += acc;
		if (e->x_speed > max_speed) {
			e->x_speed = max_speed;
		}
	} else {
		if (e->x_speed < fric && e->x_speed > -fric) {
			e->x_speed = 0;
		} else if (e->x_speed < 0) {
			e->x_speed += fric;
		} else if (e->x_speed > 0) {
			e->x_speed -= fric;
		}
	}
}

// TODO: Real value for max jump time
void entity_update_jump(Entity *e) {
	const u8 maxJumpTime = 18;
	if(e->controller[0] & BUTTON_Z) e->jump_time = 1;
	if (e->jump_time > 0) {
		if (e->controller[0] & BUTTON_C) {
			e->y_speed = -jumpSpeed;
			e->jump_time--;
		} else {
			e->jump_time = 0;
		}
	}
	if (e->jump_time > 0) return;
	if (e->grounded) {
		if ((e->controller[0] & BUTTON_C) && !(e->controller[1] & BUTTON_C)) {
			e->grounded = false;
			e->y_speed = -jumpSpeed;
			e->jump_time = maxJumpTime;
		}
	} else {
		if ((e->controller[0] & BUTTON_C) && e->y_speed >= 0) {
				e->y_speed += gravityJump;
		} else {
			e->y_speed += gravity;
		}
		if (e->y_speed > maxFallSpeed) {
			e->y_speed = maxFallSpeed;
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
	if(e->y_speed < 0) collide_stage_ceiling(e);

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
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 1;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 1;
	pixel_y = sub_to_pixel(e->y_next) + e->hit_box.bottom;
	pxa1 = stage_get_block_type(pixel_to_block(pixel_x1), pixel_to_block(pixel_y));
	pxa2 = stage_get_block_type(pixel_to_block(pixel_x2), pixel_to_block(pixel_y));
	if(pxa1 == 0x41 || pxa2 == 0x41 ||
			(!(e->flags&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SOUND_THUD, 2);
		e->y_speed = 0;
		e->y_next = pixel_to_sub((pixel_y&~0xF) - e->hit_box.bottom);
		return true;
	}
	bool result = false;
	if((pxa1&0x30) && (pxa1&0xF) >= 4 && (pxa1&0xF) < 6 &&
			pixel_y%16 >= heightmap[pxa1%2][pixel_x1%16]) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SOUND_THUD, 2);
		e->y_next = pixel_to_sub((pixel_y&0xFFF0) + 1 +
				heightmap[pxa1%2][pixel_x1%16] - e->hit_box.bottom);
		e->y_speed = 0;
		result = true;
	}
	if((pxa2&0x30) && (pxa2&0xF) >= 6 && (pxa2&0xF) < 8 &&
			pixel_y%16 >= 0xF - heightmap[pxa2%2][pixel_x2%16]) {
		if(e == &player && e->y_speed > 0xFF) sound_play(SOUND_THUD, 2);
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
	pixel_x1 = sub_to_pixel(e->x_next) - e->hit_box.left + 1;
	pixel_x2 = sub_to_pixel(e->x_next) + e->hit_box.right - 1;
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

bool collide_stage_ceiling(Entity *e) {
	u16 block_x1, block_x2, block_y;
	u8 pxa1, pxa2;
	block_x1 = pixel_to_block(sub_to_pixel(e->x_next) - e->hit_box.left + 2);
	block_x2 = pixel_to_block(sub_to_pixel(e->x_next) + e->hit_box.right - 2);
	block_y = pixel_to_block(sub_to_pixel(e->y_next) - e->hit_box.top);
	pxa1 = stage_get_block_type(block_x1, block_y);
	pxa2 = stage_get_block_type(block_x2, block_y);
	if(pxa1 == 0x41 || pxa2 == 0x41 ||
			(!(e->flags&NPC_IGNORE44) && (pxa1 == 0x44 || pxa2 == 0x44))) {
		if(e == &player && e->y_speed < -0xFF) sound_play(SOUND_HEADBONK, 2);
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
					if(e->health <= b->damage) {
						if(e->flags & NPC_SHOWDAMAGE)
							effect_create_damage_string(e->damage_value - b->damage,
									sub_to_pixel(e->x), sub_to_pixel(e->y), 60);
						// Killed enemy
						e->health = 0;
						sound_play(e->deathSound, 5);
						if(e->flags&NPC_DROPPOWERUP) {
							// entity_create(e->x, e->y, 0, 0, (exp), 0);
						}
						effect_create_smoke(e->deathSmoke,
								sub_to_pixel(e->x), sub_to_pixel(e->y));
						if(e->flags & NPC_EVENTONDEATH) tsc_call_event(e->event);
						if(e->flags & NPC_DISABLEONFLAG) system_set_flag(e->id, true);
						sprite_delete(e->sprite);
						Entity *next = e->next;
						Entity *prev = list_get_prev(e, entityList);
						prev->next = next;
						MEM_free(e);
						return next;
					}
					if(e->flags & NPC_SHOWDAMAGE) {
						e->damage_value -= b->damage;
						e->damage_time = 30;
					}
					e->health -= b->damage;
					sound_play(e->hurtSound, 5);
				}
			}
		}
	}
	if((e->flags & NPC_SHOWDAMAGE) && e->damage_value != 0) {
		e->damage_time--;
		if(e->damage_time <= 0) {
			effect_create_damage_string(e->damage_value,
					sub_to_pixel(e->x), sub_to_pixel(e->y), 60);
			e->damage_value = 0;
		}
	}
	if(e->sprite != SPRITE_NONE) {
		sprite_set_position(e->sprite,
			sub_to_pixel(e->x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - e->display_box.left,
			sub_to_pixel(e->y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - e->display_box.top);
	}
	return e->next;
}

Entity *entity_create(u16 x, u16 y, u16 id, u16 event, u16 type, u16 flags) {
	u16 eflags = flags | npc_flags(type);
	if((eflags&NPC_DISABLEONFLAG) && system_get_flag(id)) return NULL;
	if((eflags&NPC_ENABLEONFLAG) && !system_get_flag(id)) return NULL;
	Entity *e = MEM_alloc(sizeof(Entity));
	e->x = block_to_sub(x) + pixel_to_sub(8);
	e->y = block_to_sub(y) + pixel_to_sub(8);
	e->id = id;
	e->event = event;
	e->type = type;
	// Apply NPC flags in addition to entity flags
	e->flags = eflags;
	e->x_speed = 0;
	e->y_speed = 0;
	e->health = npc_health(type);
	e->attack = npc_attack(type);
	e->experience = npc_experience(type);
	e->hurtSound = npc_hurtSound(type);
	e->deathSound = npc_deathSound(type);
	e->deathSmoke = npc_deathSmoke(type);
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
	e->damage_value = 0;
	e->damage_time = 0;
	entity_create_special(e);
	return e;
}

void entity_create_special(Entity *e) {
	u16 x = sub_to_block(e->x), y = sub_to_block(e->y);
	switch(e->type) {
	case 211: // Spikes
		if(stage_get_block(x, y+1) == 0x41) break;
		if(stage_get_block(x, y-1) == 0x41) {
			sprite_set_attr(e->sprite, TILE_ATTR(PAL1, false, true, false));
			break;
		}
		//if(stage_get_block(x-1, y) == 0x41) break;
		//if(stage_get_block(x+1, y) == 0x41) break;
		break;
	default:
		break;
	}
}
