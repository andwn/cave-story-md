/*
 * Apart from effects and player bullets, all game objects are entities.
 */

enum { 
	BOSS_NONE, BOSS_OMEGA, BOSS_BALFROG, BOSS_MONSTERX, BOSS_CORE,
	BOSS_IRONHEAD, BOSS_SISTERS, BOSS_UNDEADCORE, BOSS_HEAVYPRESS, BOSS_BALLOS
};

uint8_t entity_on_screen(Entity *e);

struct Entity {
    /* +0x00 Pointers */
    Entity *next; // Next linked list element, NULL at the end
    Entity *prev; // Previous linked list element, NULL at start
    Entity *linkedEntity; // Arbitrary entity used by AI
    EntityMethod onFrame; // Unique behavior routine depending on the type of object
    /* +0x10 NPC Attributes */
    uint16_t health; // If this is an enemy it will die when health reaches 0
    uint8_t attack; // Damage inflicted on player when colliding
    uint8_t experience; // How much weapon energy/exp is dropped when killed
    uint8_t hurtSound; // Sound ID that plays when the entity is hurt
    uint8_t deathSound; // Sound ID that plays when the entity dies
    /* +0x16 PXE Attributes */
    uint16_t id; // Entity ID
    uint16_t event; // Event # to run when triggered
    uint16_t type; // NPC type - index of both npc.tbl and npc_info
    uint16_t flags; // NPC Flags from the npc.tbl and PXE
    /* +0x1E AI / Behavior */
    uint16_t state; // Script state
    uint16_t timer; // Timers for various things
    uint16_t timer2;
    uint8_t alwaysActive; // Guaranteed to never deactivate when TRUE
    uint8_t animtime; // Animation timer used by AI and ANIMATE() macro
    /* +0x24 Physics coords */
    int32_t x, y; // Current position
    int32_t x_next, y_next; // What position will be changed to next frame
    int32_t x_mark, y_mark; // Marker value so the AI can remember a position later
    int16_t x_speed, y_speed; // Velocity
    /* +0x40 Collision */
    bounding_box hit_box; // Collidable area, for both physics and combat
    uint8_t grounded; // True when on the ground, enables jumping
    uint8_t jump_time; // Time until jump button no longer increases jump height
    uint8_t underwater; // True when entity is within a water tile
    uint8_t enableSlopes; // Check collision with slopes when enabled
    /* +0x48 Misc */
    int16_t damage_value; // Cumulative damage to be displayed
    uint8_t shakeWhenHit; // Whether to shake entity when taking damage
    int8_t damage_time; // Amount of time before effect is created
    /* +0x4C Sprite */
    bounding_box display_box; // Area where sprite is displayed relative to the center
    uint8_t hidden;
    uint8_t dir; // Direction entity is facing, 0=left, 1=right
    uint8_t odir; // Previous direction (dirty marker for flipping the sprite)
    int8_t  xoff; // Sprite display offset (for enemy shake during damage)
    uint8_t frame; // Sprite frame index being displayed
    uint8_t oframe; // Previous frame index (dirty marker to redraw sprite)
    /* +0x56 There are 2 methods of handling a sprite, sheet and tiloc - see sheet.h */
    uint8_t sheet; // Preloaded sprite sheet index or NOSHEET
    uint8_t tiloc; // Dedicated vram tile allocation index or NOTILOC
    uint8_t framesize; // Number of tiles per frame
    uint8_t sprite_count; // Number of (hardware) sprites
    uint16_t vramindex; // Sheet or tiles index
    /* +0x5C Expandable hardware sprite area */
    Sprite sprite[0]; // Raw sprite(s) to copy into sprite list
    /* 0x5C + sprite_count * 8 */
};

// List of "active" entities. Updated and drawn every frame
extern Entity *entityList;
// List of "inactive" entities. Most which go offscreen end up this list and are not
// updated again until they are back on screen
extern Entity *inactiveList;
// References whichever entity is a boss otherwise it is NULL
extern Entity *bossEntity;

extern Entity *pieces[10]; // List for bosses to keep track of parts

extern Entity *water_entity;
// Heightmaps for slope tiles
extern const uint8_t heightmap[4][16];

extern uint16_t entity_active_count;
extern uint8_t moveMeToFront;

// Deletes entities based on a criteria, scripts sometimes call the bottom 2
void entities_clear();
void entities_clear_by_event(uint16_t event);
void entities_clear_by_type(uint16_t type);

// Per frame update for active entities
void entities_update(uint8_t draw);
// Reactivate entities when they come back on screen
void entities_update_inactive();

void entity_handle_bullet(Entity *e, Bullet *b);

// Replaces existing entities of the specified event to one of another type
// Called by CNP and INP commands
void entities_replace(uint16_t event, uint16_t type, uint8_t direction, uint16_t flags);
// Sets the entity states of the specified event using ENTITY_SET_STATE
// Called by ANP command
void entities_set_state(uint16_t event, uint16_t state, uint8_t direction);
// Moves the entity of the specified event to another location
void entities_move(uint16_t event, uint16_t x, uint16_t y, uint8_t direction);

// Deletes an entity and returns the next one
Entity *entity_delete(Entity *e);
// Same as delete entity but does the following first:
// Plays death sound, drops power ups, and creates smoke
Entity *entity_destroy(Entity *e);

void entity_deactivate(Entity *e);
void entity_reactivate(Entity *e);

// Creates an entity and makes it head of active or inactive list
// Called internally everywhere and by SNP command
#define entity_create(x, y, type, flags) (entity_create_ext(x, y, type, flags, 0, 0))
Entity *entity_create_ext(int32_t x, int32_t y, uint16_t type, uint16_t flags, uint16_t id, uint16_t event);

// Finds entity matching an ID or event and returns it
Entity *entity_find_by_id(uint16_t id);
Entity *entity_find_by_event(uint16_t event);
Entity *entity_find_by_type(uint16_t type);

void entity_drop_powerup(Entity *e);

// Handles collision with the loaded stage, pushes x_next and y_next out of solid areas
void entity_update_collision(Entity *e);

uint8_t collide_stage_leftwall(Entity *e);
uint8_t collide_stage_rightwall(Entity *e);
uint8_t collide_stage_floor(Entity *e);
uint8_t collide_stage_floor_grounded(Entity *e);
uint8_t collide_stage_ceiling(Entity *e);

// Returns TRUE if entity a's hitbox is overlapping b's
uint8_t entity_overlapping(Entity *a, Entity *b);
// Pushes entity a outside of entity b's hitbox
bounding_box entity_react_to_collision(Entity *a, Entity *b);

void entity_default(Entity *e, uint16_t type, uint16_t flags);

// This is called by entities to check if it got hit by any bullets
// It will return the first bullet that is colliding with the given entity, if any
Bullet *bullet_colliding(Entity *e);

// Drawing
void entities_draw();

void generic_npc_states(Entity *e);

uint16_t entities_count_active();
uint16_t entities_count_inactive();
uint16_t entities_count();
