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

// Screen size
#define SCREEN_WIDTH 320
#define SCREEN_HALF_W 160

// The original cave story is 50 FPS, so in NTSC mode TIME() and SPEED() are used
// to make the game play at (almost) the same speed as if it were 50 FPS.
// Try to only use constant values instead of variables, otherwise the compiler
// will not be able to optimize out the DIV and MUL operations
//#define PAL
#ifdef PAL
#define FPS 50
#define SCREEN_HEIGHT 240
#define SCREEN_HALF_H 120
#define TIME(x)		(x)
#define SPEED(x)	(x)
#else
#define FPS 60
#define SCREEN_HEIGHT 224
#define SCREEN_HALF_H 112
#define TIME(x)		((x) * 6 / 5)
#define SPEED(x)	((x) * 5 / 6)
#endif

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
	int32_t x1;
	int32_t y1;
	int32_t x2;
	int32_t y2;
} extent_box;

typedef struct Entity Entity;
typedef struct Weapon Weapon;
typedef struct Bullet Bullet;

typedef void (*EntityMethod)(Entity*);
typedef void (*WeaponFunc)(Weapon*);
typedef void (*BulletFunc)(Bullet*);

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
    uint16_t UNUSED_numCollision;
    uint32_t UNUSED_collisions;
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
void aftervblank();
