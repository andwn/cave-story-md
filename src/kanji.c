#include "common.h"

#include "dma.h"
#include "gamemode.h"
#include "memory.h"
#include "resources.h"
#include "tools.h"
#include "vdp.h"

#include "kanji.h"

// Allocate kanji tiles to:
// TILE_FONTINDEX (96 tiles)
// Unused end of window plane mapping (28 tiles - 1 per row)
// TILE_NAMEINDEX (16 tiles)
// TILE_NUMBERINDEX (16 tiles)

#define TILECHAR_LONGS	32

void kanji_draw(uint16_t plan, uint16_t vramIndex, uint16_t chr, uint16_t x, uint16_t y, uint16_t backCol, uint8_t shadow) {
	const uint8_t *bmp;
	if(chr > 0xFF) {
		chr -= 0x100;
		bmp = BMP_Kanji + chr * 32;
	} else {
		chr -= 0x20;
		bmp = BMP_Ascii + chr * 32;
	}
	// Convert chunks to 4bpp tiles, using a set foreground and background color
	volatile uint32_t tiles[TILECHAR_LONGS];
	for(uint16_t i = 0; i < TILECHAR_LONGS; i++) {
		uint8_t row = bmp[i];
		uint8_t column = 8;
		uint8_t last = 0;
		tiles[i] = 0;
		do {
			column--;
			// Palette indeces: 15 is white, 2 is texbox blue, 1 is black, 0 is transparency
			uint8_t pixel = row & (1 << column);
			uint32_t color = pixel ? 15 : ((last && shadow) ? 1 : backCol);
			last = pixel;
			tiles[i] |= color << (column * 4);
		} while(column > 0);
	}
	// Game mode is checked because the credits override the whole window plane, not just the offscreen section
	if(gamemode == GM_GAME && vramIndex >= (0xB000 >> 5) && vramIndex < (0xC000 >> 5)) {
		// Between gaps in the window mapping, can't load in sequence
		vdp_tiles_load(&tiles[0],  vramIndex,   1);
		vdp_tiles_load(&tiles[8],  vramIndex+4, 1);
		vdp_tiles_load(&tiles[16], vramIndex+8, 1);
		vdp_tiles_load(&tiles[24], vramIndex+12,1);
		vdp_map_xy(plan, TILE_ATTR(PAL0,1,0,0,vramIndex),   x,  y);
		vdp_map_xy(plan, TILE_ATTR(PAL0,1,0,0,vramIndex+4), x+1,y);
		vdp_map_xy(plan, TILE_ATTR(PAL0,1,0,0,vramIndex+8), x,  y+1);
		vdp_map_xy(plan, TILE_ATTR(PAL0,1,0,0,vramIndex+12),x+1,y+1);
	} else {
		vdp_tiles_load(tiles, vramIndex, 4);
		vdp_map_xy(plan, TILE_ATTR(PAL0,1,0,0,vramIndex),   x,  y);
		vdp_map_xy(plan, TILE_ATTR(PAL0,1,0,0,vramIndex+1), x+1,y);
		vdp_map_xy(plan, TILE_ATTR(PAL0,1,0,0,vramIndex+2), x,  y+1);
		vdp_map_xy(plan, TILE_ATTR(PAL0,1,0,0,vramIndex+3), x+1,y+1);
	}
}

void kanji_loadtilesforsprite(uint16_t vramIndex, uint16_t chr1, uint16_t chr2) {
	uint16_t c[2] = { chr1, chr2 };
	const uint8_t *bmp[2];
	for(uint16_t i = 0; i < 2; i++ ) {
		if(c[i] == 0) return;
		if(c[i] > 0xFF) {
			c[i] -= 0x100;
			bmp[i] = BMP_Kanji + (c[i] << 5);
		} else {
			c[i] -= 0x20;
			bmp[i] = BMP_Ascii + (c[i] << 5);
		}
		// Convert chunks to 4bpp tiles, using a set foreground and background color
		uint32_t tiles[TILECHAR_LONGS];
		// Sprite tiles are up->down before left->right so fix the order
		static const uint8_t order[32] = {
			0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23,
			8,9,10,11,12,13,14,15,24,25,26,27,28,29,30,31
		};
		for(uint16_t k = 0; k < TILECHAR_LONGS; k++) {
            uint8_t row = bmp[i][order[k]];
            uint8_t column = 8;
            uint8_t last = 0;
			tiles[k] = 0;
            do {
				column--;
                // Palette indeces: 15 is white, 2 is texbox blue, 1 is black, 0 is transparency
                uint8_t pixel = row & (1 << column);
                uint32_t color = pixel ? 15 : (last ? 1 : 0);
                last = pixel;
                tiles[k] |= color << (column << 2);
            } while(column > 0);
        }
		vdp_tiles_load(tiles, vramIndex + (i << 2), 4);
	}
}
