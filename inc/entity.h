#ifndef INC_ENTITY_H_
#define INC_ENTITY_H_

#include <genesis.h>
#include "common.h"

enum { FILTER_NONE, FILTER_ID, FILTER_EVENT, FILTER_TYPE, FILTER_ALL };

enum { 
	BOSS_NONE, BOSS_OMEGA, BOSS_BALFROG, BOSS_MONSTERX, BOSS_CORE,
	BOSS_IRONHEAD, BOSS_SISTERS, BOSS_UNDEADCORE, BOSS_HEAVYPRESS, BOSS_BALLOS
};

typedef struct Entity Entity;

typedef void (*activateFunc)(Entity*);
typedef void (*updateFunc)(Entity*);
typedef bool (*stateFunc)(Entity*, u16);
typedef void (*hurtFunc)(Entity*);

// Temporarily making these global until I refactor entity and behavior together
extern s16 maxFallSpeed, maxFallSpeedWater,
	gravity, gravityWater,
	gravityJump, gravityJumpWater,
	jumpSpeed, jumpSpeedWater,
	maxWalkSpeed, maxWalkSpeedWater,
	walkAccel, walkAccelWater,
	airControl, airControlWater,
	friction, frictionWater;

struct Entity {
	// We linked list now
	Entity *next, *prev;
	u16 id; // Entity ID (from the stage PXE, or when created by a TSC script)
	u16 event; // Event # to run when triggered
	u16 type; // NPC type - index of npc_info in tables.c (npc.tbl in original game)
	//u16 flags; // NPC flags - see tables.h for what the flags are
	u16 nflags;
	u16 eflags;
	// This is assumed to be an array of type u16[2], or at least next to each other
	// in memory. Index 0 is current joy state, 1 is previous frame's state
	u16 *controller;
	u8 direction; // Direction entity is facing, 0=left, 1=right
	// Behavior properties
	bool alwaysActive;
	activateFunc activate;
	updateFunc update;
	stateFunc set_state;
	hurtFunc hurt;
	// Combat
	u16 health; // If this is an enemy it will die when health reaches 0
	u8 attack; // Damage inflicted on player when colliding
	u8 experience; // How much weapon energy/exp is dropped when killed
	u8 hurtSound; // Sound ID that plays when the entity is hurt
	u8 deathSound; // Sound ID that plays when the entity dies
	u8 deathSmoke;
	// For physics - All this stuff is measured in sub-pixel units
	s32 x; s32 y;
	s32 x_next; s32 y_next; // What x and y will be next frame
	s16 x_speed; s16 y_speed;
	// True when on the ground, enables jumping and on-ground collision is a bit
	// different (have to stick to downward slopes, etc)
	bool grounded, underwater;
	// When pressing the jump button, this is set to the "max jump time" and
	// decrements each frame until either it reaches zero, or the button is released
	// It gives control over how high the player jumps
	u8 jump_time;
	// Collidable area, for both physics and combat
	// This is measured in pixels
	bounding_box hit_box;
	// Sprite ID assigned to this entity, or SPRITE_NONE
	Sprite *sprite;
	// Area where sprite is displayed relative to the center
	// Like hit_box this is also pixels
	bounding_box display_box;
	u8 anim; // Current animation of the sprite being displayed
	//u8 frame, frameTime; // Frame and frame time for sprite animation
	u16 state; // Script state / ANP
	u16 state_time;
	// Used to generate damage strings
	s16 damage_value;
	s8 damage_time;
};

// First element of the "active" list and the "inactive" list
extern Entity *entityList, *inactiveList;
// References whichever entity is a boss otherwise it is NULL
extern Entity *bossEntity;

// Deletes entities based on a criteria
void entities_clear();
void entities_clear_by_event(u16 event);
void entities_clear_by_type(u16 type);

// Counts the number of entities
u16 entities_count();
u16 entities_count_active();
u16 entities_count_inactive();

// Per frame update for active entities
void entities_update();
// Reactivate entities when they come back on screen
// This one is called by the camera when it moves
void entities_update_inactive();

// Replaces existing (active) entities matching criteria to one of another type
// Called by CNP and INP commands
void entities_replace(u16 event, u16 type, u8 direction, u16 flags);
// Sets the state of (active) entities matching criteria using set_state
// Called by ANP command
void entities_set_state(u16 event, u16 state, u8 direction);
void entities_move(u16 event, u16 x, u16 y, u8 direction);
// Called when a flag changes
void entities_handle_flag(u16 flag);

// Deletes an entity and returns the next one
Entity *entity_delete(Entity *e);
// Same as delete entity but does the following first:
// Plays death sound, drops power ups, and creates smoke
Entity *entity_destroy(Entity *e);

// Creates an entity and makes it head of active or inactive list
// Called internally everywhere and by SNP command
Entity *entity_create(u16 x, u16 y, u16 id, u16 event, u16 type, u16 flags);
Entity *entity_create_boss(u16 x, u16 y, u8 bossid, u16 event);

// Finds entity matching an ID and returns it
Entity *entity_find_by_id(u16 id);
Entity *entity_find_by_event(u16 event);
//Entity *entity_find_by_type(u16 type);

// Returns true if an entity of given type exists
bool entity_exists(u16 type);
bool entity_disabled(Entity *e);

// Physics
void entity_update_movement(Entity *e);
void entity_update_walk(Entity *e);
void entity_update_jump(Entity *e);
void entity_update_float(Entity *e);
void entity_update_collision(Entity *e);
bool entity_overlapping(Entity *a, Entity *b);
bounding_box entity_react_to_collision(Entity *a, Entity *b);

#endif // INC_ENTITY_H_
