#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include <types.h>

//#define KDEBUG
#ifdef KDEBUG
#include <kdebug.h>
// Enable/disable logger for specific modules
#define KDB_SYS		// Save data and flags
#define KDB_TSC		// Log TSC commands as they are executed
#define KDB_STAGE	// Stage loading
#define KDB_AI		// AI routines
#define KDB_SHEET	// Sprite sheets
#define puts(x) KDebug_Alert(x)
#define printf(...) {                                                                          \
	char str[80];                                                                              \
	sprintf(str, __VA_ARGS__);                                                                 \
	KDebug_Alert(str);                                                                         \
}
#else
#define puts(x) /**/
#define printf(...) /**/
#endif

// The original cave story is 50 FPS, so in NTSC mode TIME() and SPEED() are used
// to make the game play at (almost) the same speed as if it were 50 FPS
// Try to only use constant values instead of variables, otherwise the compiler
// will not be able to optimize out the DIV and MUL operations
//#define PAL
#ifdef PAL
#define FPS 50
#define TIME(x)		(x)
#define SPEED(x)	(x)
#else
#define FPS 60
#define TIME(x)		((x) * 60 / 50)
#define SPEED(x)	((x) * 50 / 60)
#endif

// Screen size
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224
#define SCREEN_HALF_W 160
#define SCREEN_HALF_H 112

// Direction
enum { DIR_LEFT, DIR_UP, DIR_RIGHT, DIR_DOWN, DIR_CENTER };

// Tileset width/height
#define TS_WIDTH 32
#define TS_HEIGHT 16

// Stage tileset is first in USERINDEX
#define TILE_TSINDEX TILE_USERINDEX
#define TILE_TSSIZE (TS_WIDTH * TS_HEIGHT)
// Face graphics
#define TILE_FACEINDEX (TILE_TSINDEX + TILE_TSSIZE)
#define TILE_FACESIZE 36
// 16 tiles for the map name display
#define TILE_NAMEINDEX (TILE_FACEINDEX + TILE_FACESIZE)
#define TILE_NAMESIZE 16
// Space for shared sprite sheets
#define TILE_SHEETINDEX (TILE_NAMEINDEX + TILE_NAMESIZE)
#define TILE_SHEETSIZE (TILE_FONTINDEX - TILE_SHEETINDEX)
// Space for prompt/item display at the end of the sprite tiles
#define TILE_PROMPTINDEX (TILE_SHEETINDEX + TILE_SHEETSIZE - 28)
// PLAN_A and PLAN_B are resized to 64x32 instead of 64x64, sprite list + hscroll table is
// also moved to the end as to not overlap the window plane (0xF800)
// These index the 2 unused areas between for some extra tile space
#define TILE_EXTRA1INDEX (0xD000 >> 5)
#define TILE_EXTRA2INDEX (0xF000 >> 5)
// Allocation of EXTRA1 (128 tiles) - background & HUD
#define TILE_BACKINDEX TILE_EXTRA1INDEX
#define TILE_BACKSIZE 96
#define TILE_HUDINDEX (TILE_BACKINDEX + TILE_BACKSIZE)
#define TILE_HUDSIZE 32
// Allocation of EXTRA2 (64 tiles) - Effects, window, misc
#define TILE_NUMBERINDEX TILE_EXTRA2INDEX
#define TILE_NUMBERSIZE 16
#define TILE_SMOKEINDEX (TILE_NUMBERINDEX + TILE_NUMBERSIZE)
#define TILE_SMOKESIZE 28
#define TILE_WINDOWINDEX (TILE_SMOKEINDEX + TILE_SMOKESIZE)
#define TILE_WINDOWSIZE 9
#define TILE_AIRINDEX (TILE_WINDOWINDEX + TILE_WINDOWSIZE)
#define TILE_AIRSIZE 7
// Unused palette color tiles area
#define TILE_PLAYERINDEX (TILE_SYSTEMINDEX + 2)
#define TILE_PLAYERSIZE 4
#define TILE_WEAPONINDEX (TILE_PLAYERINDEX + TILE_PLAYERSIZE)
#define TILE_WEAPONSIZE 6

// Unit conversions
// sub - fixed point unit (1/512x1/512)
// pixel - single dot on screen (1x1)
// tile - genesis VDP tile (8x8)
// block - Cave Story tile (16x16)
#define CSF 9

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

#define floor(x) ((x)&~0x1FF)
#define round(x) (((x)+0x100)&~0x1FF)
#define ceil(x)  (((x)+0x1FF)&~0x1FF)

// Get tileset from SpriteDefinition
#define SPR_TILES(spr, a, f) ((spr)->animations[a]->frames[f]->tileset->tiles)

// Bounding box used for collision and relative area to display sprites
typedef struct {
	u8 left;
	u8 top;
	u8 right;
	u8 bottom;
} bounding_box;

#endif // INC_COMMON_H_
