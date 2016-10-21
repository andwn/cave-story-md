#ifndef INC_AI_H_
#define INC_AI_H_

#include "common.h"
#include "entity.h"

/*
 * This module contains behavior or AI for entities specific to NPC type
 * There are 3 "methods" which may be indexed in the npc_info table for an NPC
 * * onSpawn: when first created or replaced
 * * onFrame: each frame while it is active
 * * onDeath: when it's killed (by a collision with a bullet object ONLY)
 */

#define ENTITY_ONSPAWN(e) npc_info[e->type].onSpawn(e)
#define ENTITY_ONFRAME(e) npc_info[e->type].onFrame(e)
#define ENTITY_ONDEATH(e) npc_info[e->type].onDeath(e)

typedef void (*EntityMethod)(Entity*);

// Special states
// Destroy signals the engine to make the entity explode and drop powerups
#define STATE_DESTROY 998
// Signal to delete, but don't explode
#define STATE_DELETE 999

// states to control the water-level object
#define WL_CALM				10	// calm and slow at set point
#define WL_CYCLE			20	// cycles between set point and top of screen
#define WL_DOWN				21	// in cycle--currently down
#define WL_UP				22	// in cycle--currently up
#define WL_STAY_UP			30	// goes to top of screen and doesn't come back down

// These were for "water screen level" and will go away soon
//#define WATER_TOP			0
//#define WATER_DISABLE		255

/* Helper Macros */

#define SNAP_TO_GROUND(e); {                                                                   \
	u16 bx = sub_to_block(e->x);                                                               \
	u16 by = sub_to_block(e->y + ((e->hit_box.bottom+1)<<CSF));                                \
	if(stage_get_block_type(bx, by) != 0x41) {                                                 \
		e->y += 16 << CSF;                                                                     \
	} else {                                                                                   \
		by = sub_to_block(e->y + ((e->hit_box.bottom-1)<<CSF));                                \
		if(stage_get_block_type(bx, by) == 0x41) e->y -= 8 << CSF;                             \
	}                                                                                          \
}

#define FACE_PLAYER(e) (e->dir = e->x < player.x)
#define TURN_AROUND(e) (e->dir ^= 1)

#define PLAYER_DIST_X(dist) (player.x > e->x - (dist) && player.x < e->x + (dist))
#define PLAYER_DIST_Y(dist) (player.y > e->y - (dist) && player.y < e->y + (dist))
#define PLAYER_DIST_X2(dist1, dist2) (player.x > e->x - (dist1) && player.x < e->x + (dist2))
#define PLAYER_DIST_Y2(dist1, dist2) (player.y > e->y - (dist1) && player.y < e->y + (dist2))

#define LIMIT_X(v) {                                                                           \
	if(e->x_speed > (v)) e->x_speed = (v);                                                     \
	if(e->x_speed < -(v)) e->x_speed = -(v);                                                   \
}
#define LIMIT_Y(v) {                                                                           \
	if(e->y_speed > (v)) e->y_speed = (v);                                                     \
	if(e->y_speed < -(v)) e->y_speed = -(v);                                                   \
}

#define MOVE_X(v) (e->x_speed = e->dir ? (v) : -(v))
#define ACCEL_X(v) (e->x_speed += e->dir ? (v) : -(v))

#define CURLY_TARGET_HERE(e) {                                                                 \
	curly_target_time = 120;                                                                   \
	curly_target_x = e->x;                                                                     \
	curly_target_y = e->y;                                                                     \
}

// The shifts here would be >>1 and >>CSF, but that will cause the speed value to be
// truncated per-pixel. The new values give a bit more leeway
#define FIRE_ANGLED_SHOT(type, xx, yy, angle, speed) {                                         \
	Entity *shot = entity_create(xx, yy, (type), 0);                                           \
	shot->x_speed = (sintab32[(angle) % 1024] >> 3) * ((speed) >> 7);                          \
	shot->y_speed = (sintab32[((angle) + 256) % 1024] >> 3) * ((speed) >> 7);                  \
}

#define SMOKE_AREA(x, y, w, h, count) {                                                        \
	for(u8 i = 0; i < (count); i++) {                                                          \
		effect_create_smoke((x) + (random() % (w)),                                            \
							(y) + (random() % (h)));                                           \
	}                                                                                          \
}

#define ANIMATE(e, spd, ...) {                                                                 \
	const u8 anim[] = { __VA_ARGS__ };                                                         \
	if(!((e)->animtime % spd)) (e)->frame = anim[(e)->animtime / spd];                         \
	if(++(e)->animtime >= spd * sizeof(anim)) (e)->animtime = 0;                               \
}

#define RANDBLINK(e, blinkframe, prob) {                                                       \
	if(e->animtime) {                                                                          \
		e->animtime--;                                                                         \
		e->frame = blinkframe;                                                                 \
	} else if(!(random() % (prob))) {                                                          \
		e->frame = blinkframe;                                                                 \
		e->animtime = 8;                                                                       \
	}                                                                                          \
}

/* Shared Variables */

Entity *water_entity;
//u8 water_screenlevel;

// These get aliased for other uses when curly isn't around
u16 curly_target_time;
s32 curly_target_x, curly_target_y;

Entity *pieces[10]; // Most bosses use this

void generic_npc_states(Entity *e);

#endif /* INC_AI_H_ */
