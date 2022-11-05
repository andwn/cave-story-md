#include "md/types.h"

#ifdef PROFILE
#define PF_BGCOLOR(c) ({ \
	*((volatile uint32_t*) 0xC00004) = 0xC0000000; \
    *((volatile uint16_t*) 0xC00000) = c; \
})
#else
#define PF_BGCOLOR(c) ({})
#endif

// Screen size
#define SCREEN_WIDTH 320
#define SCREEN_HALF_W 160

// On PAL the screen height is 16 pixels more, so these can't be constants
extern uint8_t SCREEN_HEIGHT;
extern uint8_t SCREEN_HALF_H;
extern uint8_t FPS;

// The original Cave Story is 50 FPS, and an MD can either run at 50 or 60 FPS
// depending on region. To try and keep the speed of the game (mostly) the same,
// a table for time and speed are used. On PAL, the values just match the index,
// and on NTSC they are roughly index*5/6 for speed and index*6/5 for time respectively.
extern const uint16_t *time_tab;
extern const int16_t *speed_tab;

extern const uint16_t time_tab_ntsc[0x400];
extern const int16_t speed_tab_ntsc[0x400];
extern const uint16_t time_tab_pal[0x400];
extern const int16_t speed_tab_pal[0x400];

// Default is a bit slow due to branching, but compensates in case x is too large
// Negative values are invalid. Always use -SPEED(x) instead of SPEED(-x)
#define TIME(x) (((x) < 0x400) ? (time_tab[x]) : (time_tab[(x) >> 2] << 2))
#define SPEED(x) (((x) < 0x400) ? (speed_tab[x]) : (speed_tab[(x) >> 2] << 2))

// These are like the above without the branching, when you know what the
// range of possible values of X will be
// 0x000-0x3FF
#define TIME_8(x) (time_tab[x])
#define SPEED_8(x) (speed_tab[x])
#define TIME_10(x) TIME_8(x)
#define SPEED_10(x) SPEED_8(x)
// 0x000-0xFFF, 4 frames/units of inaccuracy
#define TIME_12(x) (time_tab[(x) >> 2] << 2)
#define SPEED_12(x) (speed_tab[(x) >> 2] << 2)

// Direction
enum CSDIR { DIR_LEFT, DIR_UP, DIR_RIGHT, DIR_DOWN, DIR_CENTER };
enum MDDIR { LEFT, RIGHT, UP, DOWN, CENTER };

static inline uint8_t mddir(uint8_t dir) {
	switch(dir) {
		case DIR_LEFT: 		return LEFT;
		case DIR_UP: 		return UP;
		case DIR_RIGHT: 	return RIGHT;
		case DIR_DOWN: 		return DOWN;
		case DIR_CENTER: 	return CENTER;
		default: 			return LEFT;
	}
}

// Unit conversions
// Bit shifting "CSF" is how NXEngine converts units. I kind of like it better than my way
#define CSF 9

// sub - fixed point unit (1/512x1/512)
// pixel - single dot on screen (1x1)
// tile - genesis VDP tile (8x8)
// block - Cave Story tile (16x16)
#define sub_to_pixel(x)   ((x)>>9)
#define sub_to_tile(x)    ((x)>>12)
#define sub_to_block(x)   ((x)>>13)

#define pixel_to_sub(x)   (((int32_t)(x))<<9)
#define pixel_to_tile(x)  ((x)>>3)
#define pixel_to_block(x) ((x)>>4)

#define tile_to_sub(x)    (((int32_t)(x))<<12)
#define tile_to_pixel(x)  ((x)<<3)
#define tile_to_block(x)  ((x)>>1)

#define block_to_sub(x)   (((int32_t)(x))<<13)
#define block_to_pixel(x) ((x)<<4)
#define block_to_tile(x)  ((x)<<1)

// Get tiles from SpriteDefinition
#define SPR_TILES(spr, a, f) ((spr)->animations[a]->frames[f]->tileset->tiles)

// Bounding box used for collision and relative area to display sprites
typedef struct {
	uint8_t left;
	uint8_t top;
	uint8_t right;
	uint8_t bottom;
} bounding_box;
// Used for player bullets to reduce cpu load
typedef struct {
	uint16_t x1;
	uint16_t y1;
	uint16_t x2;
	uint16_t y2;
} extent_box;

typedef struct Entity Entity;
typedef struct Weapon Weapon;
typedef struct Bullet Bullet;

typedef void (*EntityMethod)(Entity*);
typedef void (*WeaponFunc)(Weapon*);
typedef void (*BulletFunc)(Bullet*);
typedef void (*ActionFunc)(uint8_t page);


// Prevents incomplete sprite list from being sent to VDP (flickering)
extern volatile uint8_t ready;

void aftervsync();
