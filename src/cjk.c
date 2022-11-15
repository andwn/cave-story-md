#include "common.h"

#include "res/local.h"
#include "md/dma.h"
#include "gamemode.h"
#include "md/stdlib.h"
#include "resources.h"
#include "md/comp.h"
#include "md/sys.h"
#include "md/vdp.h"
#include "md/xgm.h"
#include "window.h"

#include "cjk.h"

uint16_t cjkVramMode;
uint16_t cjkVramIndex;

uint16_t cjkTileBuf[6][16];
uint16_t cjkShiftChar;

uint16_t cjkMapBuf[3][2][36];
int16_t cjkMapRow;

// = CJK_TITLE ALLOCATION =
// Start at VRAM index 2, should be plenty of space

// = CJK_MESSAGE ALLOCATION =
// Need 36 x 6 = 216 tiles
// TILE_FONTINDEX (+96 tiles)
// VDP_PLANE_W edge (+28 tiles) = 124
// TILE_HUDINDEX (+32 tiles) = 156
// TILE_NUMBERINDEX (+16 tiles) = 172
// TILE_NAMEINDEX (+8 (of 16) tiles) = 180
// TILE_FACEINDEX (+36 tiles) = 216

// = CJK_CREDITS ALLOCATION =
// 320 tiles of space in this region:
#define TILE_KANJISTART	(TILE_SHEETINDEX + 144 + 228 + 135)
// TILE_HUDINDEX (+32 tiles) = 352
// TILE_FACE_INDEX (+36 tiles) = 388
static uint16_t CjkNextTile() {
    uint16_t index;
    switch(cjkVramMode) {
        case CJK_TITLE:
            index = cjkVramIndex + 2;
            cjkVramIndex++;
            if(cjkVramIndex >= 256) cjkVramIndex = 0;
            break;
        case CJK_MESSAGE:
            if(cjkVramIndex < 96) {
                index = cjkVramIndex + TILE_FONTINDEX;
            } else if(cjkVramIndex < 124) {
                index = ((cjkVramIndex - 96) << 2) + (VDP_PLANE_W >> 5) + 3;
            } else if(cjkVramIndex < 156) {
                index = (cjkVramIndex - 124) + TILE_HUDINDEX;
            } else if(cjkVramIndex < 172) {
                index = (cjkVramIndex - 156) + TILE_NUMBERINDEX;
            } else if(cjkVramIndex < 180) {
                index = (cjkVramIndex - 172) + TILE_NAMEINDEX;
            } else {
                index = (cjkVramIndex - 180) + TILE_FACEINDEX;
            }
            cjkVramIndex++;
            if(cjkVramIndex >= 216 || (showingFace && cjkVramIndex >= 216-36)) cjkVramIndex = 0;
            break;
        case CJK_CREDITS:
            if(cjkVramIndex < 320) {
                index = cjkVramIndex + TILE_KANJISTART;
            } else if(cjkVramIndex < 352) {
                index =(cjkVramIndex - 320) + TILE_HUDINDEX;
            } else {
                index =(cjkVramIndex - 352) + TILE_FACEINDEX;
            }
            cjkVramIndex++;
            if(cjkVramIndex >= 388) cjkVramIndex = 0;
            break;
    }
    return index;
}

void cjk_reset(uint16_t vramMode) {
    cjkVramMode = vramMode;
    cjkVramIndex = cjkShiftChar = cjkMapRow = 0;
    uint16_t attr = TILE_ATTR(PAL0,1,0,0,TILE_WINDOWINDEX+4);
    for(uint16_t i=0;i<3;i++) for(uint16_t j=0;j<2;j++) for(uint16_t k=0;k<36;k++) cjkMapBuf[i][j][k] = attr;
}

void cjk_newline() {
    cjkShiftChar = 0;
    if(++cjkMapRow > 2) cjkMapRow = 0;
    // Clear new row
    uint16_t attr = TILE_ATTR(PAL0,1,0,0,TILE_WINDOWINDEX+4);
    for(uint16_t j=0;j<2;j++) for(uint16_t k=0;k<36;k++) cjkMapBuf[cjkMapRow][j][k] = attr;
}

void cjk_winscroll(uint16_t win_x, uint16_t win_y) {
    int16_t row1 = cjkMapRow - 2;
    int16_t row2 = cjkMapRow - 1;
    if(row1 < 0) row1 += 3;
    if(row2 < 0) row2 += 3;
    //disable_ints();
    //z80_pause_fast();
    vdp_map_hline(VDP_PLANE_W, cjkMapBuf[row1][0], win_x, win_y, 38 - win_x);
    vdp_map_hline(VDP_PLANE_W, cjkMapBuf[row1][1], win_x, win_y + 1, 38 - win_x);
    vdp_map_hline(VDP_PLANE_W, cjkMapBuf[row2][0], win_x, win_y + 2, 38 - win_x);
    vdp_map_hline(VDP_PLANE_W, cjkMapBuf[row2][1], win_x, win_y + 3, 38 - win_x);
    //z80_resume();
    //enable_ints();
}

void cjk_draw(uint16_t plan, uint16_t chr, uint16_t x, uint16_t y, uint16_t backCol, uint16_t shadow) {
    // Locate char in BMP
    const uint16_t *bmp;
    if(chr > 0xFF) {
        chr -= 0x100;
        bmp = ((const uint16_t*)BMP_KANJI) + chr * 16;
    } else {
        chr -= 0x20;
        bmp = ((const uint16_t*)BMP_ASCII) + chr * 16;
    }
    // Glyph widths are 12pt, so every other char has to be shifted 4px and overlap the last one
    // |11|10|00| <- First char drawn non-shifted to first 2 columns
    // |00|01|11| <- Second char drawn shifted to second and third column
    uint16_t shift = 0;
    uint16_t bufIndex = 0;
    if(cjkShiftChar) {
        shift = 4;
        bufIndex = 2;
        cjkVramIndex -= 2;
    } else {
        // Clear tile data
        uint16_t data = (backCol << 12) | (backCol << 8) | (backCol << 4) | backCol;
        for(uint16_t i = 0; i < 6; i++) for(uint16_t j = 0; j < 16; j++) cjkTileBuf[i][j] = data;
    }
    // Convert 16x16 1bpp bmp to 8x8 4bpp tiles
    for(uint16_t yy = 0; yy < 16; yy++) {
        uint16_t row = bmp[yy];
        uint16_t column = 12;
        uint16_t last = 0;
        do {
            column--;
            // Palette indices: 15 is white, 2 is texbox blue, 1 is black, 0 is transparency
            uint16_t pixel = row & (1 << (column + 4));
            uint16_t color = pixel ? 15 : ((last && shadow) ? 1 : backCol);
            last = pixel;
            uint16_t toff = ((11 - column + shift) & 8 ? 2 : 0) + (yy & 8 ? 1 : 0);
            uint16_t xoff = (11 - column + shift) & 4 ? 1 : 0;
            cjkTileBuf[bufIndex + toff][((yy & 7) * 2) + xoff] |= color << ((column & 3) * 4);
        } while(column > 0);
    }
    // Skip tile/map upload for sprite (plan = 0)
    if(plan) {
        // Tiles are drawn top to bottom first, so the same code can be used for tilemaps and sprites
        for (uint16_t i = 0; i < 4; i++) {
            uint16_t index = CjkNextTile();
            //disable_ints();
            //z80_pause_fast();
            dma_now(DmaVRAM, (uint32_t) cjkTileBuf[bufIndex + i], index << 5, 16, 2);
            //z80_resume();
            //enable_ints();
            uint16_t attr = TILE_ATTR(PAL0, 1, 0, 0, index);
            uint16_t xx = x + ((i & 2) >> 1);
            vdp_map_xy(plan, attr, xx, (y + (i & 1)) & 31);
            if(plan == VDP_PLANE_W) {
                cjkMapBuf[cjkMapRow][i & 1][xx - 2 - (showingFace ? 7 : 0)] = attr;
            }
        }
    }
    cjkShiftChar = !cjkShiftChar;
}

void cjk_drawsprite(uint16_t offset, uint16_t chr1, uint16_t chr2) {
    cjkShiftChar = 0;
    showingFace = FALSE;
    uint16_t index = TILE_FACEINDEX + offset;
    cjk_draw(0, chr1, 0, 0, 0, 1);
    if(chr2) cjk_draw(0, chr2, 0, 0, 0, 1);
    // Upload tiles immediately
    //disable_ints();
    //z80_pause_fast();
    vdp_tiles_load((uint32_t*)cjkTileBuf, index, 6);
    //z80_resume();
    //enable_ints();
}
