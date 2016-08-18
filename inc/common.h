#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include <types.h>

//#define PAL
#ifdef PAL
#define FPS 50
#else
#define FPS 60
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
#define TILE_FACEINDEX (TILE_TSINDEX + TILE_TSSIZE)
#define TILE_FACESIZE 36
// VRAM reserved for SGDK sprite engine
#define TILE_SPRITEINDEX (TILE_FACEINDEX + TILE_FACESIZE)
#define TILE_SPRITESIZE (TILE_FONTINDEX - TILE_SPRITEINDEX)
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
#define SPR_TILESET(spr, a, f) (spr.animations[a]->frames[f]->tileset)

// "Safe" wrappers for sprite functions will only execute if given a non-null sprite
#define SPR_SAFEADD(s, def, x, y, attr, z) ({ \
	SPR_SAFERELEASE(s); \
	s = SPR_addSprite(def, x, y, attr); \
	if(s != NULL) { \
		s->data = z; \
	} \
})
#define SPR_SAFERELEASE(s); ({ if(s != NULL) { SPR_releaseSprite(s); s = NULL; } })
#define SPR_SAFEVFLIP(s ,f); ({ if(s != NULL) { SPR_setVFlip(s, f); } })
#define SPR_SAFEHFLIP(s, f); ({ if(s != NULL) { SPR_setHFlip(s, f); } })
#define SPR_SAFEMOVE(s, x, y); ({ if(s != NULL) { SPR_setPosition(s, x, y); } })
#define SPR_SAFEANIM(s, a); ({ if(s != NULL) { SPR_setAnim(s, a); } })
#define SPR_SAFEFRAME(s, f); ({ if(s != NULL) { SPR_setFrame(s, f); } })
#define SPR_SAFEANIMFRAME(s, a, f); ({ if(s != NULL) { SPR_setAnimAndFrame(s, a, f); } })
#define SPR_SAFEVISIBILITY(s, v); ({ if(s != NULL) { SPR_setVisibility(s, v); } })
#define SPR_SAFETILEINDEX(s, i); ({ if(s != NULL) { SPR_setVRAMTileIndex(s, i); } })

// Booleans
typedef unsigned char bool;
enum {false, true};

// Generic function pointer
typedef void (*func)();

// Bounding box used for collision and relative area to display sprites
typedef struct {
	u8 left;
	u8 top;
	u8 right;
	u8 bottom;
} bounding_box;

#endif // INC_COMMON_H_
