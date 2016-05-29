#ifndef INC_STAGE_H_
#define INC_STAGE_H_

#include <genesis.h>
#include "common.h"

// "Stage" refers to a level map. SGDK has a "Map" structure that is not used here,
// so stage was chosen to avoid conflict
// "Block" here refers to 16x16 in-game tiles. It is named this way to differentiate
// it from the 8x8 tiles used to store graphical data in VDP

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
 * 0x10 + E*12: NPC type (index in npc.tbl, or in our case npc_info in tables.c)
 * 0x12 + E*12: Entity flags (see tables.h)
 */

#define BLOCK_SOLID 0x1
#define BLOCK_DAMAGE 0x2
#define BLOCK_SPECIAL 0x3
#define BLOCK_NPCSOLID 0x4
#define BLOCK_BULLETPASS 0x5
#define BLOCK_PLAYERSOLID 0x6

#define BLOCK_SLOPE 0x10
#define BLOCK_WATER 0x20
#define BLOCK_FOREGROUND 0x40
#define BLOCK_WIND 0x80

// Helper macros
#define stage_get_block(x, y) (stageBlocks[(y) * stageWidth + (x)])
#define stage_get_block_type(x, y) (stageTileFlags[(x)%32][(y)%32])

u16 stageID; // Index of current stage in stage_info
u16 stageWidth, stageHeight; // Width and height measured in blocks
u8 stageBackgroundType; // Which effect to use to display the background
extern u8 *stageBlocks; // Pointer to level layout data on ROM
u8 stageTileset;
// Cached tile flags in a 512x512 area around the player
// Used to speed up collision detection
u8 stageTileFlags[32][32];
u8 stageEntityCount; // Used for debug mainly

// Clears previous stage and switches to one with the given ID, which is
// indexed in the stage_info table
void stage_load(u16 id);

bool stage_get_block_solid(u16 x, u16 y, bool checkNpcSolid);

void stage_replace_block(u16 bx, u16 by, u8 index);
// Updates scrolling for the stage and draws blocks as they get near the screen
// It is ideal to call this during vblank
void stage_update();
// Updates the stageTileFlags "cache" array and prepares to draw off-screen
// tiles when stage_update() is later called
// Camera uses this
void stage_morph(s16 _x, s16 _y, s8 x_dir, s8 y_dir);
// Immediately draws a rectangular area of the stage
void stage_draw_area(u16 _x, u16 _y, u8 _w, u8 _h);

#endif
