#ifndef MD_VDP_H
#define MD_VDP_H

#include "types.h"

#define VDP_PLANE_W             0xB000U
#define VDP_PLANE_A             0xC000U
#define VDP_PLANE_B             0xE000U
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

#define TILE_FONTINDEX          ((VDP_PLANE_W >> 5) - 96)

#define TILE_ATTR(pal, prio, flipV, flipH, index)                               \
    ((((uint16_t)flipH) << 11) | (((uint16_t)flipV) << 12) |                    \
    (((uint16_t)pal) << 13) | (((uint16_t)prio) << 15) | ((uint16_t)index))

#define SPRITE_SIZE(w, h)   ((((w) - 1) << 2) | ((h) - 1))

// FadeOut is almost completely black, except index 15 which is white
// This allows text to still be displayed after the screen fades to black
extern const uint16_t PAL_FadeOut[64];
extern const uint16_t PAL_FadeOutBlue[64];
// FullWhite is used for a TSC instruction that flashes the screen white
extern const uint16_t PAL_FullWhite[64];

// Screen size
#define ScreenWidth 320
#define ScreenHalfW 160
extern uint16_t ScreenHeight;
extern uint16_t ScreenHalfH;
// Remember the pal mode flag, so we don't have to read the control port every time
extern uint8_t pal_mode;
extern uint8_t FPS;

static inline void sprite_pos(VDPSprite *s, int16_t px, int16_t py) {
    s->x = px + 0x80;
    s->y = py + 0x80;
}
static inline void sprite_size(VDPSprite *s, uint8_t w, uint8_t h) {
    s->size = SPRITE_SIZE(w, h);
}
static inline void sprite_pri(VDPSprite *s, uint16_t pri) {
    s->attr &= ~(1<<15);
    s->attr |= pri << 15;
}
static inline void sprite_pal(VDPSprite *s, uint16_t pal) {
    s->attr &= ~(3<<13);
    s->attr |= pal << 13;
}
static inline void sprite_vflip(VDPSprite *s, uint16_t flp) {
    s->attr &= ~(1<<12);
    s->attr |= flp << 12;
}
static inline void sprite_hflip(VDPSprite *s, uint16_t flp) {
    s->attr &= ~(1<<11);
    s->attr |= flp << 11;
}
static inline void sprite_index(VDPSprite *s, uint16_t ind) {
    s->attr &= ~0x7FF;
    s->attr |= ind;
}

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
void vdp_tiles_load(const uint32_t *data, uint16_t index, uint16_t num);

void vdp_tiles_load_uftc(const void *uftc_data, uint16_t index, uint16_t uftc_index, uint16_t num);

//void vdp_tiles_load_from_rom(const uint32_t *data, uint16_t index, uint16_t num);

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

#endif
