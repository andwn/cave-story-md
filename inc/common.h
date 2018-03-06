#include <stdint.h>
#ifdef KDEBUG
#include <kdebug.h>
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

//#define PROFILE
#ifdef PROFILE
char pf_text[38];
uint16_t pf_count[32];
uint16_t pf_frame;
uint16_t pf_avg;
uint16_t pf_peak;
#define PF_INIT() ({ \
    pf_frame = 0; \
})
#define PF_START_FRAME() ({ \
    pf_count[pf_frame] = 0; \
})
#define PF_TICK() ({ \
    pf_count[pf_frame]++; \
})
#define PF_END_FRAME() ({ \
    if(pf_count[pf_frame] > pf_peak) pf_peak = pf_count[pf_frame]; \
    if(++pf_frame == 32) { \
        pf_frame = pf_avg = 0; \
        for(uint16_t i = 0; i < 32; i++) pf_avg += pf_count[i]; \
        pf_avg >>= 5; \
        sprintf(pf_text, "OBJ:%03hu+%03hu AVG:%04hu PEAK:%04hu", \
                entities_count_active(), entities_count_inactive(), pf_avg, pf_peak); \
        pf_peak = 0; \
    } \
})
#define PF_DRAW() ({ \
    if(pf_frame == 0) { \
        VDP_drawTextWindow(pf_text, 4, 0); \
    } \
})
#else
#define PF_INIT() /**/
#define PF_START_FRAME() /**/
#define PF_TICK() /**/
#define PF_END_FRAME() /**/
#define PF_DRAW() /**/
#endif

//#define PROFILE_BG
#ifdef PROFILE_BG
#define PF_BGCOLOR(c) ({ VDP_setPaletteColor(0, c); })
#else
#define PF_BGCOLOR(c) /**/
#endif

// Screen size
#define SCREEN_WIDTH 320
#define SCREEN_HALF_W 160

// On PAL the screen height is 16 pixels more, so these can't be constants
uint8_t SCREEN_HEIGHT;
uint8_t SCREEN_HALF_H;
uint8_t FPS;

// The original Cave Story is 50 FPS, and an MD can either run at 50 or 60 FPS
// depending on region. To try and keep the speed of the game (mostly) the same,
// a table for time and speed adjustments are loaded in memory. On PAL, the values
// just match the index, and on NTSC they are roughly index*5/6 for speed and 
// index*6/5 for time respectively.

// Default is a bit slow due to branching, but compensates in case x is too large
// Negative values are invalid. Always use -SPEED(x) instead of SPEED(-x)
#define TIME(x) (((x) < 0x100) ? (time_tab[x]) : (time_tab[(x) >> 4] << 4))
#define SPEED(x) (((x) < 0x100) ? (speed_tab[x]) : (speed_tab[(x) >> 4] << 4))

// These are like the above without the branching, when you know what the
// range of possible values of X will be
// 0x000-0x0FF
#define TIME_8(x) (time_tab[x])
#define SPEED_8(x) (speed_tab[x])
// 0x000-0x3FF, 4 frames/units of inaccuracy
#define TIME_10(x) (time_tab[(x) >> 2] << 2)
#define SPEED_10(x) (speed_tab[(x) >> 2] << 2)
// 0x000-0xFFF, 16 frames/units of inaccuracy
#define TIME_12(x) (time_tab[(x) >> 4] << 4)
#define SPEED_12(x) (speed_tab[(x) >> 4] << 4)

uint16_t time_tab[0x100];
uint16_t speed_tab[0x100];

// Div/mod tables to help math when displaying digits
const uint8_t div10[100];
const uint8_t mod10[100];

// Direction
enum CSDIR { DIR_LEFT, DIR_UP, DIR_RIGHT, DIR_DOWN, DIR_CENTER };
enum MDDIR { LEFT, RIGHT, UP, DOWN, CENTER };

// Angles
#define A_RIGHT	0x00
#define A_DOWN	0x40
#define A_LEFT	0x80
#define A_UP	0xC0

// Sine & cosine lookup tables
const int16_t sin[0x100];
const int16_t cos[0x100];
// Above tables but every value multiplied by 1.5, quick reference:
// <<1 == *3, <<2 == *6, <<3 == *12, <<4 == *24, <<5 == *48
const int16_t sin2[0x100];
const int16_t cos2[0x100];

// Unit conversions
// Bit shifting "CSF" is how NXEngine converts units. I kind of like it better than my way
#define CSF 9

// sub - fixed point unit (1/512x1/512)
// pixel - single dot on screen (1x1)
// tile - genesis VDP tile (8x8)
// block - Cave Story tile (16x16)
#define sub_to_pixel(x)   ((x)>>CSF)
#define sub_to_tile(x)    ((x)>>12)
#define sub_to_block(x)   ((x)>>13)

#define pixel_to_sub(x)   ((x)<<CSF)
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

#define min(X, Y)   (((X) < (Y))?(X):(Y))
#define max(X, Y)   (((X) > (Y))?(X):(Y))
#define abs(X)      (((X) < 0)?-(X):(X))

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

// SGDK / Rescomp Types
typedef struct {
    uint16_t value;
} VDPPlan;

typedef struct {
    uint16_t compression;
    uint16_t numTile;
    uint32_t *tiles;
} TileSet;

typedef struct {
    uint16_t index;
    uint16_t length;
    uint16_t *data;
} Palette;

typedef struct {
    int16_t y;
    union {
        struct {
            uint8_t size;
            uint8_t link;
        };
        uint16_t size_link;
    };
    uint16_t attribut;
    int16_t x;
} VDPSprite;

typedef struct {
    int16_t y;          // respect VDP sprite field order
    uint16_t size;
    int16_t x;
    uint16_t numTile;
} VDPSpriteInf;

typedef struct {
    uint16_t numSprite;
    VDPSpriteInf **vdpSpritesInf;
    uint32_t UNUSED_collision;
    TileSet *tileset;
    int16_t w;
    int16_t h;
    uint16_t timer;
} AnimationFrame;

typedef struct {
    uint16_t numFrame;
    AnimationFrame **frames;
    uint16_t length;
    uint8_t *sequence;
    int16_t loop;
} Animation;

typedef struct {
    Palette *palette;
    uint16_t numAnimation;
    Animation **animations;
    uint16_t maxNumTile;
    uint16_t maxNumSprite;
} SpriteDefinition;

// VBlank stuff
extern volatile uint8_t vblank;

// Prevents incomplete sprite list from being sent to VDP (flickering)
volatile uint8_t ready;

void vsync();
void aftervsync();
