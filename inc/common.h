#ifndef INC_COMMON_H_
#define INC_COMMON_H_

// Screen size
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224
#define SCREEN_HALF_W 160
#define SCREEN_HALF_H 112

// Direction
#define DIRECTION_LEFT 0
#define DIRECTION_UP 1
#define DIRECTION_RIGHT 2
#define DIRECTION_DOWN 3
#define DIRECTION_CENTER 4

// Tile sizes, indexes
#define TS_WIDTH 32
#define TS_HEIGHT 20
#define BACK_SIZE 16 * 6
#define FACE_SIZE 6 * 6
#define TILE_BACKINDEX (TILE_USERINDEX + (TS_WIDTH * TS_HEIGHT))
#define TILE_WINDOWINDEX (TILE_FONTINDEX + 0x60)
#define TILE_FACEINDEX (TILE_FONTINDEX - 6 * 6)
#define TILE_HUDINDEX (TILE_BACKINDEX + BACK_SIZE)

// Unit conversions
// subpixel/unit (1/512x1/512)
// pixel (1x1)
// tile (8x8)
// block (16x16)
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

#ifndef _GENESIS_H_
// This is so types like u8/u16 can be used in headers without including genesis.h
#define s8      char
#define s16     short
#define s32     long
#define u8      unsigned char
#define u16     unsigned short
#define u32     unsigned long

#define NULL 0
#define TRUE 1
#define FALSE 0

#endif // _GENESIS_H_

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
