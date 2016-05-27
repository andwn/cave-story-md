#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include <types.h>

// Screen size
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224
#define SCREEN_HALF_W 160
#define SCREEN_HALF_H 112

// Direction
enum { DIR_LEFT, DIR_UP, DIR_RIGHT, DIR_DOWN, DIR_CENTER };

// Tile sizes
#define TS_WIDTH 32
#define TS_HEIGHT 20
#define TS_SIZE (TS_WIDTH * TS_HEIGHT)
#define BACK_SIZE 0x60
#define HUD_SIZE 0x20
#define FACE_SIZE 0x24

// Tile indexes
#define TILE_TSINDEX TILE_USERINDEX
#define TILE_BACKINDEX (TILE_TSINDEX + TS_SIZE)
#define TILE_HUDINDEX (TILE_BACKINDEX + BACK_SIZE)
#define TILE_FACEINDEX (TILE_HUDINDEX + HUD_SIZE)
#define TILE_SPRITEINDEX (TILE_FACEINDEX + FACE_SIZE)
// Stick window border in the empty space after SGDK font
#define TILE_WINDOWINDEX (TILE_FONTINDEX + 0x60)
// Extra space for tiles between planes
#define TILE_EXTRA1INDEX (0xD000 >> 5)
#define TILE_EXTRA2INDEX (0xF000 >> 5)

// Specific locations and sizes for sprite tiles
#define TILE_PLAYERINDEX TILE_SPRITEINDEX
#define TILE_PLAYERSIZE 4
#define TILE_WEAPONINDEX (TILE_PLAYERINDEX + TILE_PLAYERSIZE)
#define TILE_WEAPONSIZE 2
#define TILE_BULLETINDEX (TILE_WEAPONINDEX + TILE_WEAPONSIZE)
#define TILE_BULLETSIZE 8
#define TILE_NPCINDEX (TILE_BULLETINDEX + TILE_BULLETSIZE)
#define TILE_NPCSIZE ((0xB000 >> 5) - TILE_NPCINDEX)
// Use the extra 128 tiles for big bosses
#define TILE_BOSSINDEX TILE_EXTRA1INDEX
// Use the extra 64 tiles for smoke and damage numbers
#define TILE_NUMBERINDEX TILE_EXTRA2INDEX
#define TILE_NUMBERSIZE 16
#define TILE_SMOKEINDEX (TILE_NUMBERINDEX + TILE_NUMBERSIZE)
#define TILE_SMOKESIZE 28

// Unit conversions
// sub - fixed point unit (1/512x1/512)
// pixel - single dot on screen (1x1)
// tile - genesis VDP tile (8x8)
// block - Cave Story tile (16x16)
#define sub_to_pixel(x)   ((x)>>9)
#define sub_to_tile(x)    ((x)>>12)
#define sub_to_block(x)   ((x)>>13)

#define pixel_to_sub(x)   ((x)<<9)
#define pixel_to_tile(x)  ((x)>>3)
#define pixel_to_block(x) ((x)>>4)

#define tile_to_sub(x)    ((x)<<12)
#define tile_to_pixel(x)  ((x)<<3)
#define tile_to_block(x)  ((x)>>1)

#define block_to_sub(x)   ((x)<<13)
#define block_to_pixel(x) ((x)<<4)
#define block_to_tile(x)  ((x)<<1)

#define floor(x) ((x)&~0xFF)
#define round(x) ((x+0x80)&~0xFF)
#define ceil(x)  ((x+0x100)&~0xFF)

// Booleans
typedef unsigned char bool;
enum {false, true};

// Generic function pointer
typedef void (*func)();

// Bounding box
typedef struct {
	u8 left;
	u8 top;
	u8 right;
	u8 bottom;
} bounding_box;

#endif // INC_COMMON_H_
