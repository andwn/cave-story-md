#define VDP_PLAN_W              0xB000U
#define VDP_PLAN_A              0xC000U
#define VDP_PLAN_B              0xE000U
#define VDP_SPRITE_TABLE        0xF800U
#define VDP_HSCROLL_TABLE       0xFC00U

#define PLAN_WIDTH              64
#define PLAN_HEIGHT             32
#define PLAN_WIDTH_SFT          6
#define PLAN_HEIGHT_SFT         5

#define HSCROLL_PLANE           0
#define HSCROLL_TILE            2
#define HSCROLL_LINE            3
#define VSCROLL_PLANE           0
#define VSCROLL_2TILE           1

#define PAL0                    0
#define PAL1                    1
#define PAL2                    2
#define PAL3                    3

#define FADE_NONE               0
#define FADE_INPROGRESS         1
#define FADE_LASTFRAME          2

#define TILE_SIZE               32
#define TILE_INDEX_MASK         0x7FF

#define TILE_SYSTEMINDEX        0x0000U
#define TILE_USERINDEX          0x0010U
#define TILE_FONTINDEX          ((VDP_PLAN_W >> 5) - 96)
#define TILE_EXTRA1INDEX        (0xD000U >> 5) // 128 tiles after PLAN_A
#define TILE_EXTRA2INDEX        (0xF000U >> 5) // 64 tiles after PLAN_B

// Tileset width/height
#define TS_WIDTH    32
#define TS_HEIGHT   16
// Stage tileset is first in USERINDEX
#define TILE_TSINDEX        TILE_USERINDEX
#define TILE_TSSIZE         (TS_WIDTH * TS_HEIGHT)
// Face graphics
#define TILE_FACEINDEX      (TILE_TSINDEX + TILE_TSSIZE)
#define TILE_FACESIZE       36
// 16 tiles for the map name display
#define TILE_NAMEINDEX      (TILE_FACEINDEX + TILE_FACESIZE)
#define TILE_NAMESIZE       16
// Space for shared sprite sheets
#define TILE_SHEETINDEX     (TILE_NAMEINDEX + TILE_NAMESIZE)
#define TILE_SHEETSIZE      (TILE_FONTINDEX - TILE_SHEETINDEX)
// Space for prompt/item display at the end of the sprite tiles
#define TILE_PROMPTINDEX    (TILE_SHEETINDEX + TILE_SHEETSIZE - 28)
#define TILE_AIRTANKINDEX   (TILE_PROMPTINDEX - 9)
// Allocation of EXTRA1 (128 tiles) - background & HUD
#define TILE_BACKINDEX      TILE_EXTRA1INDEX
#define TILE_BACKSIZE       96
#define TILE_HUDINDEX       (TILE_BACKINDEX + TILE_BACKSIZE)
#define TILE_HUDSIZE        32
// Allocation of EXTRA2 (64 tiles) - Effects, window, misc
#define TILE_NUMBERINDEX    TILE_EXTRA2INDEX
#define TILE_NUMBERSIZE     16
#define TILE_SMOKEINDEX     (TILE_NUMBERINDEX + TILE_NUMBERSIZE)
#define TILE_SMOKESIZE      28
#define TILE_WINDOWINDEX    (TILE_SMOKEINDEX + TILE_SMOKESIZE)
#define TILE_WINDOWSIZE     9
#define TILE_AIRINDEX       (TILE_WINDOWINDEX + TILE_WINDOWSIZE)
#define TILE_AIRSIZE        8
#define TILE_QMARKINDEX     (TILE_AIRINDEX + TILE_AIRSIZE)
#define TILE_QMARKSIZE      1
#define TILE_WHIMINDEX      (TILE_QMARKINDEX + TILE_QMARKSIZE)
#define TILE_WHIMSIZE       2
// 12 tiles at the end for nemesis vertical frames
#define TILE_NEMINDEX       (0xFE80U >> 5)
// 8 tiles after window plane for blade L3
#define TILE_SLASHINDEX     ((0xC000U >> 5) - 8)
// Unused palette color tiles area
#define TILE_PLAYERINDEX    (TILE_SYSTEMINDEX + 2)
#define TILE_PLAYERSIZE     4
#define TILE_WEAPONINDEX    (TILE_PLAYERINDEX + TILE_PLAYERSIZE)
#define TILE_WEAPONSIZE     6

#define TILE_CLOUDINDEX     (TILE_TSINDEX + 64)
#define TILE_CLOUD2INDEX    (TILE_CLOUDINDEX + (16*12))
#define TILE_CLOUD3INDEX    (TILE_CLOUD2INDEX + (16*3))
#define TILE_CLOUD4INDEX    (TILE_CLOUD3INDEX + (9*3))

#define TILE_ATTR(pal, prio, flipV, flipH, index)                               \
    ((((uint16_t)flipH) << 11) | (((uint16_t)flipV) << 12) |                    \
    (((uint16_t)pal) << 13) | (((uint16_t)prio) << 15) | ((uint16_t)index))

#define SPRITE_SIZE(w, h)   ((((w) - 1) << 2) | ((h) - 1))

#define sprite_pos(s, px, py) { (s).x = 0x80 + (px); (s).y = 0x80 + (py); }
#define sprite_size(s, w, h)  { (s).size = ((((w) - 1) << 2) | ((h) - 1)); }
#define sprite_pri(s, pri)    { (s).attr &= ~(1<<15); (s).attr |= ((pri)&1) << 15; }
#define sprite_pal(s, pal)    { (s).attr &= ~(3<<13); (s).attr |= ((pal)&3) << 13; }
#define sprite_vflip(s, flp)  { (s).attr &= ~(1<<12); (s).attr |= ((flp)&1) << 12; }
#define sprite_hflip(s, flp)  { (s).attr &= ~(1<<11); (s).attr |= ((flp)&1) << 11; }
#define sprite_index(s, ind)  { (s).attr &= ~0x7FF;   (s).attr |= (ind)&0x7FF; }

// 32 bytes of zero, can be sent to VDP to clear any tile
extern const uint32_t TILE_BLANK[8];
// FadeOut is almost completely black, except index 15 which is white
// This allows text to still be displayed after the screen fades to black
extern const uint16_t PAL_FadeOut[64];
extern const uint16_t PAL_FadeOutBlue[64];
// FullWhite is used for a TSC instruction that flashes the screen white
extern const uint16_t PAL_FullWhite[64];
// Remember the pal mode flag, so we don't have to read the control port every time
extern uint8_t pal_mode;

// Set defaults, clear everything
void vdp_init();

// Wait until next vblank
void vdp_vsync();

// Register stuff
void vdp_set_display(uint8_t enabled);

void vdp_set_autoinc(uint8_t val);

void vdp_set_scrollmode(uint8_t hoz, uint8_t vert);

void vdp_set_highlight(uint8_t enabled);

void vdp_set_backcolor(uint8_t index);

void vdp_set_window(uint8_t x, uint8_t y);

// Status
uint16_t vdp_get_palmode();

uint16_t vdp_get_vblank();

// Tile patterns
void vdp_tiles_load(/*volatile*/ const uint32_t *data, uint16_t index, uint16_t num);

void vdp_tiles_load_from_rom(/*volatile*/ const uint32_t *data, uint16_t index, uint16_t num);

// Tile maps
void vdp_map_xy(uint16_t plan, uint16_t tile, uint16_t x, uint16_t y);

void vdp_map_hline(uint16_t plan, const uint16_t *tiles, uint16_t x, uint16_t y, uint16_t len);

void vdp_map_vline(uint16_t plan, const uint16_t *tiles, uint16_t x, uint16_t y, uint16_t len);

void vdp_map_fill_rect(uint16_t plan, uint16_t index, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t inc);

void vdp_map_clear(uint16_t plan);

// Palettes
void vdp_colors(uint16_t index, const uint16_t *values, uint16_t count);

void vdp_color(uint16_t index, uint16_t color);

void vdp_colors_next(uint16_t index, const uint16_t *values, uint16_t count);

void vdp_color_next(uint16_t index, uint16_t color);

uint16_t vdp_fade_step_calc();

void vdp_fade_step_dma();

void vdp_fade(const uint16_t *src, const uint16_t *dst, uint16_t speed, uint8_t async);

// Scrolling
void vdp_hscroll(uint16_t plan, int16_t hscroll);

void vdp_hscroll_tile(uint16_t plan, int16_t *hscroll);

void vdp_vscroll(uint16_t plan, int16_t vscroll);

// Sprites
void vdp_sprite_add(const VDPSprite *spr);

void vdp_sprites_add(const VDPSprite *spr, uint16_t num);

void vdp_sprites_clear();

void vdp_sprites_update();

// Text
void vdp_font_load(const uint32_t *tiles);

void vdp_font_pal(uint16_t pal);

void vdp_puts(uint16_t plan, const char *str, uint16_t x, uint16_t y);

void vdp_text_clear(uint16_t plan, uint16_t x, uint16_t y, uint16_t len);
