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

/* Helper Macros */

#define SNAP_TO_GROUND(e); {                                                                   \
	uint16_t bx = sub_to_block(e->x);                                                          \
	uint16_t by = sub_to_block(e->y + (pixel_to_sub(e->hit_box.bottom+1)));                    \
	if(stage_get_block_type(bx, by) != 0x41) {                                                 \
		e->y += pixel_to_sub(16);                                                              \
	} else {                                                                                   \
		by = sub_to_block(e->y + (pixel_to_sub(e->hit_box.bottom-1)));                         \
		if(stage_get_block_type(bx, by) == 0x41) e->y -= pixel_to_sub(8);                      \
	}                                                                                          \
}

static inline void FACE_PLAYER(Entity *e) { e->dir = e->x < player.x; }
static inline void TURN_AROUND(Entity *e) { e->dir ^= 1; }

static inline uint8_t PLAYER_DIST_X(Entity *e, int32_t dist) {
	return player.x > e->x - dist && player.x < e->x + dist;
}
static inline uint8_t PLAYER_DIST_Y(Entity *e, int32_t dist) {
	return player.y > e->y - dist && player.y < e->y + dist;
}
static inline uint8_t PLAYER_DIST_X2(Entity *e, int32_t dist1, int32_t dist2) {
	return player.x > e->x - dist1 && player.x < e->x + dist2;
}
static inline uint8_t PLAYER_DIST_Y2(Entity *e, int32_t dist1, int32_t dist2) {
	return player.y > e->y - dist1 && player.y < e->y + dist2;
}

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

#define FIRE_ANGLED_SHOT(type, xx, yy, angle, speed) {                                         \
	Entity *shot = entity_create(xx, yy, (type), 0);                                           \
	shot->x_speed = ((int32_t)cos[(uint8_t)angle] * speed) >> CSF;                            \
	shot->y_speed = ((int32_t)sin[(uint8_t)angle] * speed) >> CSF;                            \
}

#define THROW_AT_TARGET(shot, tgtx, tgty, speed) {                                             \
	uint8_t angle = get_angle(shot->x, shot->y, tgtx, tgty);                                   \
	shot->x_speed = ((int32_t)cos[(uint8_t)angle] * speed) >> CSF;                            \
	shot->y_speed = ((int32_t)sin[(uint8_t)angle] * speed) >> CSF;                            \
}

#define SMOKE_AREA(x, y, w, h, count) {                                                        \
	for(uint8_t i = 0; i < (count); i++) {                                                     \
		effect_create_smoke((x) + (random() % (w)),                                            \
							(y) + (random() % (h)));                                           \
	}                                                                                          \
}

#define ANIMATE(e, spd, ...) {                                                                 \
	static const uint8_t anim[] = { __VA_ARGS__ };                                             \
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

#define SCREEN_FLASH(numframes) vdp_fade(PAL_FullWhite, NULL, numframes, TRUE);

/* Shared Variables */

// These get aliased for other uses when curly isn't around
extern uint16_t curly_target_time;
extern int32_t curly_target_x, curly_target_y;

enum CrystalStates {
	CRYSTAL_INFRONT,
	CRYSTAL_INBACK,
	CRYSTAL_TOFRONT,
	CRYSTAL_TOBACK
};
#define crystal_entity	water_entity
#define crystal_state	curly_target_time
#define crystal_xmark	curly_target_x
#define crystal_ymark	curly_target_y

uint8_t get_angle(int32_t curx, int32_t cury, int32_t tgtx, int32_t tgty);
