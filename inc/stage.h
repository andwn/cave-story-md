#ifndef INC_STAGE_H_
#define INC_STAGE_H_

#include <genesis.h>
#include "common.h"

/*
 * "Stage" refers to a level map. SGDK has a "Map" structure that is not used here,
 * so stage was chosen to avoid conflict
 * "Block" refers to 16x16 in-game tiles. It is named this way to differentiate
 * it from the 8x8 tiles used to store graphical data in VDP
 */

/*
 * Cave Story file types related to stage data
 *
 * PXM - Level layout
 * 0x00 "PXM"
 * 0x03 Unknown byte
 * 0x04 Width
 * 0x06 Height
 * From 0x08 on each byte is 1 tile placement
 * Left to right, top to bottom
 *
 * PXE - Entity data
 * Header is 8 bytes
 * 0x00: "PXE" then nullchar
 * 0x04: Number of entities
 * 0x06: Unknown
 * Then for each entity (12 bytes each)
 * 0x08 + E*12: x position (blocks)
 * 0x0A + E*12: y position
 * 0x0C + E*12: ID
 * 0x0E + E*12: Event # (in the TSC)
 * 0x10 + E*12: NPC type (index in npc.tbl)
 * 0x12 + E*12: Entity flags (see tables.h)
 */

// Tile options (PXA)
// Low 4 bits are distinct values
#define BLOCK_SOLID 0x1
#define BLOCK_DAMAGE 0x2
#define BLOCK_SPECIAL 0x3
#define BLOCK_NPCSOLID 0x4
#define BLOCK_BULLETPASS 0x5
#define BLOCK_PLAYERSOLID 0x6
// High 4 bits are flags that can be used in combination
#define BLOCK_SLOPE 0x10
#define BLOCK_WATER 0x20
#define BLOCK_FOREGROUND 0x40
#define BLOCK_WIND 0x80

// Helper macros
// This will get the block (in the tileset) that is at a specific location in the stage grid
#define stage_get_block(x, y) (stageBlocks[stageTable[y] + (x)])
// Like above, but will return the "tile options" (solid, water, damage, etc)
#define stage_get_block_type(x, y) (tileset_info[stageTileset].PXA[stage_get_block(x, y)])

// Index of current stage in db/stage.c
u16 stageID;
// Size of the stage - how many blocks wide/high
u16 stageWidth, stageHeight;
// A multiplication lookup table for each row of stageBlocks
// Removes all mulu.w and __mulsci3 instructions in entity stage collision
extern u16 *stageTable;
// Index of background in db/back.c and the effect type
u8 stageBackground, stageBackgroundType;
// Copy of level layout data loaded into RAM
// This takes up extra space, but there are times where scripts make modifications to the
// level layout (allowing player to reach some areas) so it is necessary to do this
extern u8 *stageBlocks;
// Which tileset (db/tileset.c) is used by the current stage
u8 stageTileset;
// Used for debug mainly - counts the number of entities created by stage_load()
// It is NOT the total number of entities that exist at any time, use entity_count() instead
u8 stageEntityCount;
// Prepares to draw off-screen tiles when stage_update() is later called
// Camera calls this each time it scrolls past 1 block length (16 pixels)
s8 morphingRow, morphingColumn;

// Clears previous stage and switches to one with the given ID
void stage_load(u16 id);

bool stage_get_block_solid(u16 x, u16 y, bool checkNpcSolid);
// Called by TSC, replaces one block with another and creates smoke
void stage_replace_block(u16 bx, u16 by, u8 index);
// Updates scrolling for the stage and draws blocks as they get near the screen
// It is ideal to call this during vblank
void stage_update();
// Immediately draws a rectangular area of the stage (slow)
void stage_draw_area(u16 _x, u16 _y, u8 _w, u8 _h);

#endif
