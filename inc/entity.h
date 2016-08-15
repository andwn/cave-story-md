#ifndef INC_ENTITY_H_
#define INC_ENTITY_H_

#include <genesis.h>
#include "common.h"

/*
 * Apart from effects and player bullets, all game objects are entities.
 */

// Used in Entity.spriteAnim, disables loading sprite when reactivating
#define SPRITE_DISABLE 0xFF       

enum { 
	BOSS_NONE, BOSS_OMEGA, BOSS_BALFROG, BOSS_MONSTERX, BOSS_CORE,
	BOSS_IRONHEAD, BOSS_SISTERS, BOSS_UNDEADCORE, BOSS_HEAVYPRESS, BOSS_BALLOS
};

// The original game runs at 50 Hz. The PAL values are copied from it
// NTSC values calculated with: value * (50.0 / 60.0)
// All measured in subpixel, 0x200 (512) units is one pixel
#ifdef PAL
#define MAX_FALL_SPEED 0x5FF
#define GRAVITY 0x50
#define GRAVITY_JUMP 0x20
#define JUMP_SPEED 0x500
#define MAX_WALK_SPEED 0x32C
#define WALK_ACCEL 0x55
#define AIR_CONTROL 0x20
#define FRICTION 0x33
#define MAX_JUMP_TIME 0x0F
#else
#define MAX_FALL_SPEED 0x4FF
#define GRAVITY 0x43
#define GRAVITY_JUMP 0x1B
#define JUMP_SPEED 0x430
#define MAX_WALK_SPEED 0x2A5
#define WALK_ACCEL 0x47
#define AIR_CONTROL 0x1B
#define FRICTION 0x2A // (This one is 42.499 and I went for 42 here)
#define MAX_JUMP_TIME 0x11
#endif

// This slightly redundant typedef is required for Entity to refer to itself
typedef struct Entity Entity;

struct Entity {
	// Next and previous linked list nodes. NULL at the end/beginning of the list
	Entity *next, *prev;
	/* NPC Attributes */
	u16 health; // If this is an enemy it will die when health reaches 0
	u8 attack; // Damage inflicted on player when colliding
	u8 experience; // How much weapon energy/exp is dropped when killed
	u8 hurtSound; // Sound ID that plays when the entity is hurt
	u8 deathSound; // Sound ID that plays when the entity dies
	u8 deathSmoke; // Type of smoke effect when destroyed
	u16 nflags; // NPC Flags from the npc.tbl that apply to every instance of this entity
	/* PXE Attributes */
	u16 id; // Entity ID
	u16 event; // Event # to run when triggered
	u16 type; // NPC type - index of both npc.tbl and npc_info
	u16 eflags; // PXE Flags are per entity, and are added with NPC flags via bitwise OR
	/* AI / Behavior */
	bool alwaysActive; // Guaranteed to never deactivate when true
	u16 state; // Script state / ANP
	u16 state_time;
	u16 state_time2;
	// This is assumed to be an array of type u16[2], or at least next to each other
	// in memory (input.h). Index 0 is current joy state, 1 is previous frame's state
	u16 *controller;
	/* Physics */
	s32 x, y; // Current position
	s32 x_next, y_next; // What position will be changed to next frame
	s32 x_mark, y_mark; // Marker value so the AI can remember a position later
	s16 x_speed, y_speed; // Velocity
	bool direction, // Direction entity is facing, 0=left, 1=right
		grounded, // True when on the ground, enables jumping
		underwater, // True when entity is within a water tile
		enableSlopes; // Check collision with slopes when enabled
	u8 jump_time; // Time until jump button no longer increases jump height
	bounding_box hit_box; // Collidable area, for both physics and combat
	/* Display */
	Sprite *sprite; // Sprite assigned to this entity, or NULL
	// These 4 sprite variables remember the sprite state when an entity deactivates
	// Once reactivated, they are used to restore the sprite attributes
	u8 spriteAnim, spriteFrame, spriteVFlip;
	bounding_box display_box; // Area where sprite is displayed relative to the center
	// Used to generate damage strings
	s16 damage_value;
	s8 damage_time;
};

// First element of the "active" list. Entities in this list are updated fully
extern Entity *entityList;
// To save memory and CPU (indeed the game will run 15 FPS without this), off screen
// entities are moved into the "inactive" list. This list is not iterated by
// entities_update(), but only in entities_update_inactive() which is not called
// every frame but instead each time the camera moves the length of 1 block
extern Entity *inactiveList;
// References whichever entity is a boss otherwise it is NULL
extern Entity *bossEntity;
// Heightmaps for slope tiles
extern const u8 heightmap[4][16];

// Deletes entities based on a criteria, scripts sometimes call the bottom 2
void entities_clear();
void entities_clear_by_event(u16 event);
void entities_clear_by_type(u16 type);

// Counts the number of entities currently loaded, for debugging
u16 entities_count();
u16 entities_count_active();
u16 entities_count_inactive();

// Per frame update for active entities
void entities_update();
// Reactivate entities when they come back on screen
void entities_update_inactive();

// Replaces existing entities of the specified event to one of another type
// Called by CNP and INP commands
void entities_replace(u16 event, u16 type, u8 direction, u16 flags);
// Sets the entity states of the specified event using ENTITY_SET_STATE
// Called by ANP command
void entities_set_state(u16 event, u16 state, u8 direction);
// Moves the entity of the specified event to another location
void entities_move(u16 event, u16 x, u16 y, u8 direction);

void entities_pause();
void entities_unpause();

// Deletes an entity and returns the next one
Entity *entity_delete(Entity *e);
// Same as delete entity but does the following first:
// Plays death sound, drops power ups, and creates smoke
Entity *entity_destroy(Entity *e);

// Creates an entity and makes it head of active or inactive list
// Called internally everywhere and by SNP command
Entity *entity_create(u16 x, u16 y, u16 id, u16 event, u16 type, u16 flags, u8 direction);
Entity *entity_create_boss(u16 x, u16 y, u8 bossid, u16 event);

// Finds entity matching an ID or event and returns it
Entity *entity_find_by_id(u16 id);
Entity *entity_find_by_event(u16 event);
Entity *entity_find_by_type(u16 event);

// Returns true if an entity of given type exists
bool entity_exists(u16 type);
// Returns true if the entity should be disabled (checks flags)
bool entity_disabled(Entity *e);

void entity_drop_powerup(Entity *e);
bool entity_on_screen(Entity *e);

void entity_sprite_create(Entity *e);

// Handles movement physics based on controller input, will set the values x_next and y_next
void entity_update_movement(Entity *e);
// These 3 are individual parts of update_movement()
void entity_update_walk(Entity *e);
void entity_update_jump(Entity *e);
void entity_update_float(Entity *e);
// Handles collision with the loaded stage, pushes x_next and y_next out of solid areas
void entity_update_collision(Entity *e);

bool collide_stage_leftwall(Entity *e);
bool collide_stage_rightwall(Entity *e);
bool collide_stage_floor(Entity *e);
bool collide_stage_floor_grounded(Entity *e);
bool collide_stage_ceiling(Entity *e);

// Returns true if entity a's hitbox is overlapping b's
bool entity_overlapping(Entity *a, Entity *b);
// Pushes entity a outside of entity b's hitbox
bounding_box entity_react_to_collision(Entity *a, Entity *b, bool realXY);

void entity_default(Entity *e, u16 type, u16 flags);

#endif // INC_ENTITY_H_
