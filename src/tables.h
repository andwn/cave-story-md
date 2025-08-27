/*
 * This is a database of most of the game's content. Wherever possible, the indexes match
 * the IDs used by the original game.
 * Any time you see a const pointer, that is pointing to ROM data
 */

#ifndef CSMD_TABLES_H
#define CSMD_TABLES_H

#include "common.h"

// Information about each stage, indexed by stageID
#define STAGE_COUNT 95
typedef struct {
	const uint8_t *PXM; // PXM is the layout, each byte is an index of the current tileset
	const uint16_t *PXE; // PXE is the entity list
	// Which palette to load for PAL3. Most use PAL_Regu but some differ
	const uint16_t *npcPalette;
	uint16_t tileset; // Which tileset in tileset_info to use
	uint16_t background; // Which background in background_info to use
	//char name[24]; // The name of the map, as displayed to the player
} stage_info_def;
extern const stage_info_def stage_info[STAGE_COUNT];

// Information about each tileset, indexed by stageTileset
#define TILESET_COUNT 29
typedef struct {
    const uint16_t size; // Number of 16x16 blocks in the tileset
    const uint16_t *pat; // The graphical tile data to load
	const uint16_t *palette; // The palette, which will be loaded to PAL2
	const uint8_t *PXA; // Tile options, how objects interact with different tiles
} tileset_info_def;
extern const tileset_info_def tileset_info[];

// Information about each background, indexed by stageBackground
// The IDs deviate from the original game. I do not know the "correct" order
#define BACKGROUND_COUNT 17
typedef struct {
	const uint16_t *tileset; // Graphical tile data to load
	// Backgrounds do not use their own palette and instead "share" with any other of the
	// 4 already loaded. This value specifies which, like PAL0, PAL1, etc
	uint16_t palette;
	// The "type" is which behavior/algorithm to use when loading/scrolling the background
	// 0 - Draw a plain tiled image into VDP_PLANE_B
	// 1 - Moon/Fog - Draw from a specified pattern into VDP_PLANE_B and scroll the clouds
	// 2 - Clear VDP_PLANE_B to black, no background
	// 3 - Scroll tiled image automatically (Ironhead boss)
	uint8_t type;
	uint8_t width, height; // Size of tiled image for type 0
    uint8_t padding; // Make it even
} background_info_def;
extern const background_info_def background_info[];

// Information about each music track for song_play()
#define SONG_COUNT 43
typedef struct {
	const uint8_t *song; // Location of XGM to be loaded
	//char name[24]; // Name of the track (sound test)
} bgm_info_def;
extern const bgm_info_def bgm_info[];

// Information about each sound effect for sound_play()
#define SOUND_COUNT 118
typedef struct {
    const uint8_t *sound; // Location of PCM data to playback
    const uint8_t *end; // Pointer to the end, to calculate length
} sfx_info_def;
extern const sfx_info_def sfx_info[];

// Information about each character face image
#define FACE_COUNT 30
typedef struct {
	const uint16_t *tiles; // Graphical tile data
	// Like backgrounds, faces share palettes. This is which of the 4 currently loaded to use
	uint16_t palette;
} face_info_def;
extern const face_info_def face_info[];

// Information about each NPC type, indexes are the same as npc.tbl
typedef struct {
	// SpriteDef containing tile data, ignored if a sheet is used
	const SpriteDef *sprite;
	uint8_t sheet; // Sheet ID or NOSHEET
    uint8_t sprite_count; // Size of sprite[0], must be known beforehand
	uint16_t palette; // Any of the 4 loaded palettes to use for the sprite
	EntityMethod onSpawn, onFrame, onDeath; // AI
    char comment[12]; // Really padding to make the element length a power of 2
} npc_info_def;
extern const npc_info_def npc_info[];

// Information about each weapon, indexes match <AM+ and ArmsImage
typedef struct {
	const SpriteDef *sprite; // Sprite to display beneath player
	uint16_t palette;
	uint8_t experience[3]; // Amount of exp required to level up the weapon
    char padding[7];
} weapon_info_def;
extern const weapon_info_def weapon_info[];

// Instructions on how to display text/icons in the credits
typedef struct {
	uint16_t cmd; 
	union {
		struct { 
			//char string[36];
			uint16_t jstring;
		} text;
		struct  { 
			uint16_t id;
			uint16_t pal;
		} icon;
		struct { 
			uint16_t ticks;
		} wait;
		struct { 
			uint16_t pos;
		} move;
		struct { 
			uint16_t id;
		} song;
		struct {
			uint16_t flag;
			uint16_t label;
		} fjump;
		struct {
			uint16_t label;
		} jump;
		struct {
			uint16_t value;
		} label;
		struct { 
			uint16_t id;
			const uint16_t *data;
		} palette;
		struct {
			uint16_t value;
		} textmode;
	};
} credits_info_def;
extern const credits_info_def credits_info[];

// Credits illustrations
typedef struct {
	//const TileSet *tileset;
	const uint32_t *pat;
	const uint16_t pat_size;
	const uint16_t *map;
	const uint16_t *palette;
} illustration_info_def;
extern const illustration_info_def illustration_info[];

#endif //CSMD_TABLES_H
