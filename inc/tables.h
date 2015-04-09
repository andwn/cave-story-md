#ifndef INC_TABLES_H_
#define INC_TABLES_H_

#include <genesis.h>
#include "common.h"
#include "npc.h"

#define STAGE_COUNT 95
#define TILESET_COUNT 23
#define BACKGROUND_COUNT 13
#define SONG_COUNT 42
#define SOUND_COUNT 118
#define WEAPON_COUNT 14
#define FACE_COUNT 30

// NPC Flags
// Player can't pass through the NPC
#define NPC_SOLID			0x0001
// Ignore tile type 0x44. Normally NPCs cannot pass them
#define NPC_IGNORE44		0x0002
// Can't be damaged and makes a *clink* sound
#define NPC_INVINCIBLE		0x0004
// Goes through walls, used on flying enemies
#define NPC_IGNORESOLID		0x0008
// I think only the Behemoth enemy uses this
#define NPC_BOUNCYTOP		0x0010
// Can be damaged with player bullets
#define NPC_SHOOTABLE		0x0020
// Not sure what "special solid" is
#define NPC_SPECIALSOLID	0x0040
// Touching the bottom, top, and back does not hurt
#define NPC_FRONTATKONLY	0x0080
// Custom, depends on the NPC type
#define NPC_OPTION1			0x0100
// Trigger event [Entity.event] on death
#define NPC_EVENTONDEATH	0x0200
// Drops health/energy/missiles
#define NPC_DROPPOWERUP		0x0400
// Only appears if the flag [Entity.id] is set
#define NPC_ENABLEONFLAG	0x0800
// Custom, depends on the NPC type
#define NPC_OPTION2			0x1000
// Calls event [Entity.event] when player presses down
#define NPC_INTERACTIVE		0x2000
// Only appears if the flag [Entity.id] is unset
#define NPC_DISABLEONFLAG	0x4000
// Pops up red numbers when damaged
#define NPC_SHOWDAMAGE		0x8000

typedef struct {
	const u8 *PXM;
	const u8 *PXE;
	const u8 *TSC;
	const Palette *npcPalette;
	u8 tileset;
	u8 background;
	u8 name[24];
} stage_info_def;

typedef struct {
	const TileSet *tileset;
	const Palette *palette;
	const u8 *PXA;
} tileset_info_def;

typedef struct {
	const TileSet *tileset;
	u16 palette;
	u8 type;
	u8 width;
	u8 height;
} background_info_def;

typedef struct {
	const u8 *song;
	u8 name[24];
} song_info_def;

typedef struct {
	const u8 *sound;
	u32 length;
} sound_info_def;

typedef struct {
	const TileSet *tiles;
	u16 palette;
} face_info_def;

typedef struct {
	const SpriteDefinition *sprite;
	u16 palette;
} npc_info_def;

typedef struct {
	const SpriteDefinition *sprite;
	const SpriteDefinition *bulletSprite[3];
	u8 shootSound[3];
	u8 experience[3];
	u8 damage[3];
} weapon_info_def;

extern const stage_info_def stage_info[STAGE_COUNT];

extern const tileset_info_def tileset_info[TILESET_COUNT];

extern const background_info_def background_info[BACKGROUND_COUNT];

extern const song_info_def song_info[SONG_COUNT];

extern const sound_info_def sound_info[SOUND_COUNT];

extern const npc_info_def npc_info[NPC_COUNT];

extern const weapon_info_def weapon_info[WEAPON_COUNT];

extern const face_info_def face_info[FACE_COUNT];

#endif /* INC_TABLES_H_ */
