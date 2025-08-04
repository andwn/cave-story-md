#include "dma.h"
#include "sys.h"
#include "vdp.h"
#include "comp.h"
#include "math.h"
#include "res/system.h"

const uint16_t PAL_FadeOut[64] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xEEE,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
const uint16_t PAL_FadeOutBlue[64] = {
        0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0xEEE,
        0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200,
        0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200,
        0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200
};
const uint16_t PAL_FullWhite[64] = {
        0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE,
        0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE,
        0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE,
        0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE, 0xEEE
};

static volatile uint16_t *const vdp_data_port = (uint16_t *) 0xC00000;
static volatile uint16_t *const vdp_ctrl_port = (uint16_t *) 0xC00004;
static volatile uint32_t *const vdp_ctrl_wide = (uint32_t *) 0xC00004;

// Palette vars
static uint16_t pal_current[64];
static uint16_t pal_next[64];
static uint8_t pal_fading;
static uint8_t pal_fadespeed;
static uint8_t pal_fadecnt;

// Sprite vars
static uint16_t sprite_count;
static Sprite sprite_table[80];
static uint16_t sprite_ymax;

// Font vars
static uint16_t font_pal;

// Screen size
//uint16_t ScreenHeight;
//uint16_t ScreenHalfH;
//uint8_t pal_mode;
uint8_t FPS;

void vdp_init(void) {
    // Store pal_mode and adjust some stuff based on it
    pal_mode = *vdp_ctrl_port & 1;
    ScreenHeight = pal_mode ? 240 : 224;
    ScreenHalfH = ScreenHeight >> 1;
    sprite_ymax = ScreenHeight + 32;
    FPS = pal_mode ? 50 : 60;
    // Set the registers
#ifdef SLOW_MODE
    *vdp_ctrl_port = 0x8000;
#else
    *vdp_ctrl_port = 0x8004;
#endif
    *vdp_ctrl_port = 0x8174 | (pal_mode ? 8 : 0); // Enable display
    *vdp_ctrl_port = 0x8200 | (VDP_PLANE_A >> 10); // Plane A address
    *vdp_ctrl_port = 0x8300 | (VDP_PLANE_W >> 10); // Window address
    *vdp_ctrl_port = 0x8400 | (VDP_PLANE_B >> 13); // Plane B address
    *vdp_ctrl_port = 0x8500 | (VDP_SPRITE_TABLE >> 9); // Sprite list address
    *vdp_ctrl_port = 0x8600;
    *vdp_ctrl_port = 0x8700; // Background color palette index
    *vdp_ctrl_port = 0x8800;
    *vdp_ctrl_port = 0x8900;
    *vdp_ctrl_port = 0x8A01; // Horizontal interrupt timer
    *vdp_ctrl_port = 0x8B00 | (VSCROLL_PLANE << 2) | HSCROLL_PLANE; // Scroll mode
    *vdp_ctrl_port = 0x8C81; // No interlace or shadow/highlight
    *vdp_ctrl_port = 0x8D00 | (VDP_HSCROLL_TABLE >> 10); // HScroll table address
    *vdp_ctrl_port = 0x8E00;
    *vdp_ctrl_port = 0x8F02; // Auto increment
    *vdp_ctrl_port = 0x9001; // Map size (64x32)
    *vdp_ctrl_port = 0x9100; // Window X
    *vdp_ctrl_port = 0x9200; // Window Y
    // Reset DMA queue
    dma_clear();
    // Reset the tilemaps
    vdp_map_clear(VDP_PLANE_A);
    vdp_hscroll(VDP_PLANE_A, 0);
    vdp_vscroll(VDP_PLANE_A, 0);
    vdp_map_clear(VDP_PLANE_B);
    vdp_hscroll(VDP_PLANE_B, 0);
    vdp_vscroll(VDP_PLANE_B, 0);
    // Reset sprites
    vdp_sprites_clear();
    vdp_sprites_update();
    // (Re)load the font
    vdp_font_load(UFTC_SysFont);
    vdp_colors(0, PAL_FadeOut, 64);
    // Put blank tile in index 0
    vdp_tiles_load(BlankData, 0, 1);
}

//void vdp_vsync(void) {
//    vblank = 0;
//    while (!vblank);
//    vblank = 0;
//}

// Register stuff

void vdp_set_display(uint8_t enabled) {
    *vdp_ctrl_port = 0x8134 | (enabled ? 0x40 : 0) | (pal_mode ? 0x08 : 0);
}

void vdp_set_autoinc(uint8_t val) {
    *vdp_ctrl_port = 0x8F00 | val;
}

void vdp_set_scrollmode(uint8_t hoz, uint8_t vert) {
    *vdp_ctrl_port = 0x8B00 | (vert << 2) | hoz;
}

void vdp_set_highlight(uint8_t enabled) {
    *vdp_ctrl_port = 0x8C81 | (enabled << 3);
}

void vdp_set_backcolor(uint8_t index) {
    *vdp_ctrl_port = 0x8700 | index;
}

void vdp_set_window(uint8_t x, uint8_t y) {
    *vdp_ctrl_port = 0x9100 | x;
    *vdp_ctrl_port = 0x9200 | y;
}

// Tile patterns

void vdp_tiles_load(const uint32_t *data, uint16_t index, uint16_t num) {
    //dma_now(DmaVRAM, (uint32_t) data, index << 5, num << 4, 2);
    uint32_t from = (uint32_t) data;
    uint16_t to = index << 5;
    uint16_t len1 = num << 4;
    uint32_t end = ((from >> 1) & 0xFFFF) + len1;
    if(end > 0x10000) {
        // Unaligned
        uint16_t len2 = end & 0xFFFF;
        len1 -= len2;
        dma_now(DmaVRAM, from, to, len1, 2);
        __asm__("": : :"memory");
        from += len1 << 1;
        to += len1 << 1;
        dma_now(DmaVRAM, from, to, len2, 2);
    } else {
        dma_now(DmaVRAM, from, to, len1, 2);
    }
}

void vdp_tiles_load_uftc(const void *uftc_data, uint16_t index, uint16_t uftc_index, uint16_t num) {
    extern const uint32_t *__compbuf_start;
    static const uint16_t maxlen = 0x1000 / TILE_SIZE;

    while(num) {
        uint16_t now = min(num, maxlen);
        uftc_unpack(uftc_data, &__compbuf_start, uftc_index, now);
        __asm__("": : :"memory");
        dma_now(DmaVRAM, (uint32_t) &__compbuf_start, index << 5, now << 4, 2);
        num -= now;
        index += now;
        uftc_index += now;
    }
}

// Tile maps

void vdp_map_xy(uint16_t plan, uint16_t tile, uint16_t x, uint16_t y) {
    uint32_t addr = plan + ((x + (y << PLAN_WIDTH_SFT)) << 1);
    *vdp_ctrl_wide = ((0x4000 + ((addr) & 0x3FFF)) << 16) + (((addr) >> 14) | 0x00);
    *vdp_data_port = tile;
}

void vdp_map_hline(uint16_t plan, const uint16_t *tiles, uint16_t x, uint16_t y, uint16_t len) {
    dma_now(DmaVRAM, (uint32_t) tiles, plan + ((x + (y << PLAN_WIDTH_SFT)) << 1), len, 2);
}

void vdp_map_vline(uint16_t plan, const uint16_t *tiles, uint16_t x, uint16_t y, uint16_t len) {
    dma_now(DmaVRAM, (uint32_t) tiles, plan + ((x + (y << PLAN_WIDTH_SFT)) << 1), len, 128);
}

void vdp_map_fill_rect(uint16_t plan, uint16_t index, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t inc) {
    uint16_t tiles[64];
    for (uint16_t yy = 0; yy < h; yy++) {
        for (uint16_t xx = 0; xx < w; xx++) {
            tiles[xx] = index;
            index += inc;
        }
        __asm__("": : :"memory");
        dma_now(DmaVRAM, (uint32_t) tiles, plan + ((x + ((y + yy) << PLAN_WIDTH_SFT)) << 1), w, 2);
    }
}

void vdp_map_clear(uint16_t plan) {
    uint16_t addr = plan;
    while (addr < plan + 0x1000) {
        __asm__("": : :"memory");
        dma_now(DmaVRAM, (uint32_t) BlankData, addr, 0x80, 2);
        addr += 0x100;
    }
}

// Palettes

void vdp_colors(uint16_t index, const uint16_t *values, uint16_t count) {
    dma_now(DmaCRAM, (uint32_t) values, index << 1, count, 2);
    for (uint16_t i = count; i--;) pal_current[index + i] = values[i];
}

void vdp_color(uint16_t index, uint16_t color) {
    uint16_t ind = index << 1;
    *vdp_ctrl_wide = ((0xC000 + (((uint32_t) ind) & 0x3FFF)) << 16) + ((((uint32_t) ind) >> 14) | 0x00);
    *vdp_data_port = color;
    pal_current[index] = color;
}

void vdp_colors_next(uint16_t index, const uint16_t *values, uint16_t count) {
    for (uint16_t i = count; i--;) pal_next[index + i] = values[i];
}

void vdp_color_next(uint16_t index, uint16_t color) {
    pal_next[index] = color;
}

uint16_t vdp_fade_step_calc(void) {
    if (pal_fading != FADE_INPROGRESS) return 0;
    if (++pal_fadecnt >= pal_fadespeed) {
        pal_fadecnt = 0;
        uint16_t colors_changed = 0;
        for (uint16_t i = 64; i--;) {
            uint16_t cR = pal_current[i] & 0x00E;
            uint16_t nR = pal_next[i] & 0x00E;
            uint16_t cG = pal_current[i] & 0x0E0;
            uint16_t nG = pal_next[i] & 0x0E0;
            uint16_t cB = pal_current[i] & 0xE00;
            uint16_t nB = pal_next[i] & 0xE00;
            if (cR != nR) {
                pal_current[i] += cR < nR ? 0x002 : -0x002;
                colors_changed++;
            }
            if (cG != nG) {
                pal_current[i] += cG < nG ? 0x020 : -0x020;
                colors_changed++;
            }
            if (cB != nB) {
                pal_current[i] += cB < nB ? 0x200 : -0x200;
                colors_changed++;
            }
        }
        if (!colors_changed) {
            pal_fading = FADE_LASTFRAME;
            return 2;
        }
    }
    return 1;
}

void vdp_fade_step_dma(void) {
    if (pal_fading != FADE_NONE) {
        dma_queue(DmaCRAM, (uint32_t) pal_current, 0, 64, 2);
        if (pal_fading == FADE_LASTFRAME) pal_fading = FADE_NONE;
    }
}

void vdp_fade(const uint16_t *src, const uint16_t *dst, uint16_t speed, uint8_t async) {
    if (src) vdp_colors(0, src, 64);
    if (dst) vdp_colors_next(0, dst, 64);
    pal_fading = FADE_INPROGRESS;
    pal_fadespeed = speed;
    pal_fadecnt = 0;
    if (!async) {
        while (vdp_fade_step_calc()) {
            sys_wait_vblank();
            vdp_fade_step_dma();
            dma_flush();
        }
    }
}

// Scroll

void vdp_hscroll(uint16_t plan, int16_t hscroll) {
    uint32_t addr = (plan == VDP_PLANE_A) ? VDP_HSCROLL_TABLE : VDP_HSCROLL_TABLE + 2;
    *vdp_ctrl_wide = ((0x4000 + ((addr) & 0x3FFF)) << 16) + (((addr) >> 14) | 0x00);
    *vdp_data_port = hscroll;
}

void vdp_hscroll_tile(uint16_t plan, int16_t *hscroll) {
    dma_now(DmaVRAM, (uint32_t) hscroll, VDP_HSCROLL_TABLE + (plan == VDP_PLANE_A ? 0 : 2), 30, 32);
}

void vdp_vscroll(uint16_t plan, int16_t vscroll) {
    uint32_t addr = (plan == VDP_PLANE_A) ? 0 : 2;
    *vdp_ctrl_wide = ((0x4000 + ((addr) & 0x3FFF)) << 16) + (((addr) >> 14) | 0x10);
    *vdp_data_port = vscroll;
}

// Sprites

void vdp_sprite_add(const Sprite *spr) {
    // Exceeded max number of sprites
    if (sprite_count >= 80) return;
    // Prevent drawing off-screen sprites
    if ((unsigned) (spr->x - 96) < 352 && (unsigned) (spr->y - 96) < sprite_ymax) {
        sprite_table[sprite_count] = *spr;
        sprite_table[sprite_count].link = sprite_count + 1;
        sprite_count++;
    }
}

void vdp_sprites_add(const Sprite *spr, uint16_t num) {
    for (uint16_t i = num; i--;) vdp_sprite_add(&spr[i]);
}

void vdp_sprites_clear(void) {
    static const Sprite NULL_SPRITE = {.x = 0x80, .y = 0x80};
    sprite_count = 0;
    vdp_sprites_add(&NULL_SPRITE, 1);
}

void vdp_sprites_update(void) {
    if (!sprite_count) return;
    sprite_table[sprite_count - 1].link = 0; // Mark end of sprite list
    dma_queue(DmaVRAM, (uint32_t) sprite_table, VDP_SPRITE_TABLE, sprite_count << 2, 2);
    sprite_count = 0;
}

// Font / Text

void vdp_font_load(const uint32_t *tiles) {
    font_pal = 0;
    // ASCII 32-127
    vdp_tiles_load_uftc(tiles, TILE_FONTINDEX, 0, 0x60);
    // Extended charset
    uint16_t index = (VDP_PLANE_W >> 5) + 3;
    for (uint16_t i = 0; i < 30; i++) {
        vdp_tiles_load_uftc(tiles, index, 0x60 + i, 1);
        index += 4;
    }
}

void vdp_font_pal(uint16_t pal) {
    font_pal = pal;
}

void vdp_nputs(uint16_t plan, const char *str, uint16_t x, uint16_t y, uint16_t maxlen) {
    uint32_t addr = plan + ((x + (y << PLAN_WIDTH_SFT)) << 1);
    *vdp_ctrl_wide = ((0x4000 + ((addr) & 0x3FFF)) << 16) + (((addr) >> 14) | 0x00);
    for (uint16_t i = 0; i < maxlen && *str; ++i) {
        // Wrap around the plane, don't fall to next line
        if (i + x == 64) {
            addr -= x << 1;
            *vdp_ctrl_wide = ((0x4000 + ((addr) & 0x3FFF)) << 16) + (((addr) >> 14) | 0x00);
        }
		uint16_t c = (unsigned) *str++;
		if (c == 1) { // Accent chars
			c = (VDP_PLANE_W >> 5) - 1 + ((*str++) << 2);
		} else {
			c = TILE_FONTINDEX + c - 0x20;
		}
        *vdp_data_port = TILE_ATTR(font_pal, 1, 0, 0, c);
    }
}

void vdp_text_clear(uint16_t plan, uint16_t x, uint16_t y, uint16_t len) {
    uint32_t addr = plan + ((x + (y << PLAN_WIDTH_SFT)) << 1);
    *vdp_ctrl_wide = ((0x4000 + ((addr) & 0x3FFF)) << 16) + (((addr) >> 14) | 0x00);
    while (len--) *vdp_data_port = 0;
}
