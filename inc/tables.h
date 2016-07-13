#ifndef INC_TABLES_H_
#define INC_TABLES_H_

#include <genesis.h>
#include "common.h"
#include "npc.h"
#include "ai.h"
#include "weapon.h"

/*
 * This is a database of most of the game's content. Wherever possible, the indexes match
 * the IDs used by the original game.
 * Any time you see a const pointer, that is pointing to ROM data
 */

// Information about each stage, indexed by stageID
#define STAGE_COUNT 95
typedef struct {
	const u8 *PXM; // PXM is the layout, each byte is an index of the current tileset
	const u8 *PXE; // PXE is the entity list
	const u8 *TSC; // TSC is the script
	// Which palette to load for PAL3. Most use PAL_Regu but some differ
	const Palette *npcPalette;
	u8 tileset; // Which tileset in tileset_info to use
	u8 background; // Which background in background_info to use
	u8 name[24]; // The name of the map, as displayed to the player
} stage_info_def;
extern const stage_info_def stage_info[STAGE_COUNT];

// Information about each tileset, indexed by stageTileset
#define TILESET_COUNT 26
typedef struct {
	const TileSet *tileset; // The graphical tile data to load
	const Palette *palette; // The palette, which will be loaded to PAL2
	const u8 *PXA; // Tile options, how objects interact with different tiles
} tileset_info_def;

// Information about each background, indexed by stageBackground
// The IDs deviate from the original game. I do not know the "correct" order
#define BACKGROUND_COUNT 13
typedef struct {
	const TileSet *tileset; // Graphical tile data to load
	// Backgrounds do not use their own palette and instead "share" with any other of the
	// 4 already loaded. This value specifies which, like PAL0, PAL1, etc
	u16 palette;
	// The "type" is which behavior/algorithm to use when loading/scrolling the background
	// 0 - Draw a plain tiled image into PLAN_B
	// 1 - Moon/Fog - Draw from a specified pattern into PLAN_B and scroll the clouds
	// 2 - Clear PLAN_B to black, no background
	u8 type;
	u8 width, height; // Size of tiled image for type 0
} background_info_def;

// Information about each music track for song_play()
#define SONG_COUNT 42
typedef struct {
	const u8 *song; // Location of XGM to be loaded
	char name[24]; // Name of the track (sound test)
} song_info_def;

// Information about each sound effect for sound_play()
#define SOUND_COUNT 118
typedef struct {
	const u8 *sound; // Location of PCM data to playback
	u32 length; // Number of frames (sound frames, not screen frames)
} sound_info_def;

// Information about each character face image
#define FACE_COUNT 30
typedef struct {
	const TileSet *tiles; // Graphical tile data
	// Like backgrounds, faces share palettes. This is which of the 4 currently loaded to use
	u16 palette;
} face_info_def;

// Information about each NPC type, indexes are the same as npc.tbl
// It has 8 extra spots for major bosses though, as they do not exist in npc.tbl
typedef struct {
	const SpriteDefinition *sprite; // Sprite to load
	u16 palette; // Any of the 4 loaded palettes to use for the sprite
	u8 zorder;
	char name[26]; // Name of the NPC (NPC Test)
	// The "methods" for AI/behavior. Storing them in this table frees up 16 bytes per entity
	EntityMethod onCreate, onUpdate, onState, onHurt;
} npc_info_def;

// Information about each weapon, indexes match <AM+ and ArmsImage
typedef struct {
	const SpriteDefinition *sprite; // Sprite to display beneath player
	u8 experience[3]; // Amount of exp required to level up the weapon
	u8 name[16];
} weapon_info_def;

// Oh... I was going to do something with this
typedef struct {
	u8 *function;
	u8 paramCount;
} command_info_def;

extern const tileset_info_def tileset_info[TILESET_COUNT];

extern const background_info_def background_info[BACKGROUND_COUNT];

extern const song_info_def song_info[SONG_COUNT];

extern const sound_info_def sound_info[SOUND_COUNT];

extern const npc_info_def npc_info[NPC_COUNT + 8];

extern const weapon_info_def weapon_info[WEAPON_COUNT];

extern const face_info_def face_info[FACE_COUNT];

#endif /* INC_TABLES_H_ */
