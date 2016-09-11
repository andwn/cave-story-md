#ifndef INC_ENTITY_H_
#define INC_ENTITY_H_

#include <genesis.h>
#include "common.h"

/*
 * Apart from effects and player bullets, all game objects are entities.
 */

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

#define entity_on_screen(e) ((unsigned)((e)->x - camera_xmin) < camera_xsize && \
							(unsigned)((e)->y - camera_ymin) < camera_ysize)

//#define entity_disabled(e) ((e->eflags&NPC_ENABLEONFLAG) && !system_get_flag(e->id))

// This slightly redundant typedef is required for Entity to refer to itself
typedef struct Entity Entity;

struct Entity {
	// Next and previous linked list nodes. NULL at the end/beginning of the list
	Entity *next, *prev;
	Entity *linkedEntity; // Arbitrary entity used by AI
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
	u16 eflags; // PXE Flags are per entity, and are added with NPC flags via binary OR
	/* AI / Behavior */
	bool alwaysActive; // Guaranteed to never deactivate when true
	u16 state, timer, timer2; // AI script state and timers
	/* Physics */
	s32 x, y; // Current position
	s32 x_next, y_next; // What position will be changed to next frame
	s32 x_mark, y_mark; // Marker value so the AI can remember a position later
	s16 x_speed, y_speed; // Velocity
	bool dir, odir, // Direction entity is facing, 0=left, 1=right
		grounded, // True when on the ground, enables jumping
		underwater, // True when entity is within a water tile
		enableSlopes; // Check collision with slopes when enabled
	u8 jump_time; // Time until jump button no longer increases jump height
	bounding_box hit_box; // Collidable area, for both physics and combat
	bounding_box display_box; // Area where sprite is displayed relative to the center
	// Used to generate damage strings
	s16 damage_value; // Cumulative damage to be displayed
	s8 damage_time; // Amount of time before effect is created
	/* Sprite Stuff */
	bool hidden,
		foreground; // Draw sprite before player
	u8 sprite_count; // Number of (hardware) sprites
	u8 frame, oframe; // Sprite frame index being displayed, remember old one to detect changes
	u8 animtime; // Animation timer used by AI and ANIMATE() macro
	u8 sheet, tiloc;
	u16 vramindex; // Sheet or tiles index
	u8 framesize; // Number of tiles per frame
	VDPSprite sprite[0]; // Raw sprite(s) to copy into sprite list
};

// List of "active" entities. Updated and drawn every frame
extern Entity *entityList;
// List of "inactive" entities. Most which go offscreen end up this list and are not
// updated again until they are back on screen
extern Entity *inactiveList;
// References whichever entity is a boss otherwise it is NULL
extern Entity *bossEntity;
// Heightmaps for slope tiles
extern const u8 heightmap[4][16];

// Deletes entities based on a criteria, scripts sometimes call the bottom 2
void entities_clear();
void entities_clear_by_event(u16 event);
void entities_clear_by_type(u16 type);

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

// Deletes an entity and returns the next one
Entity *entity_delete(Entity *e);
// Same as delete entity but does the following first:
// Plays death sound, drops power ups, and creates smoke
Entity *entity_destroy(Entity *e);

// Creates an entity and makes it head of active or inactive list
// Called internally everywhere and by SNP command
Entity *entity_create(s32 x, s32 y, u16 type, u16 flags);

// Finds entity matching an ID or event and returns it
Entity *entity_find_by_id(u16 id);
Entity *entity_find_by_event(u16 event);
Entity *entity_find_by_type(u16 type);

void entity_drop_powerup(Entity *e);

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

// Drawing
void entities_draw_fore();
void entities_draw_back();

#endif // INC_ENTITY_H_
