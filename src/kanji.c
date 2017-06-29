#define KDEBUG
#include "common.h"

#include "dma.h"
#include "memory.h"
#include "resources.h"
#include "string.h"
#include "tools.h"
#include "vdp.h"
#include "vdp_pal.h"
#include "vdp_tile.h"

#include "kanji.h"

// Allocate kanji tiles to:
// TILE_FONTINDEX (96 tiles)
// Unused end of window plane mapping (28 tiles - 1 per row)
// TILE_NUMBERINDEX (16 tiles)
// (Need to find someplace else for 4 more)

#define BMPCHAR_BYTES	32
#define TILECHAR_LONGS	32

void kanji_draw(VDPPlan plan, uint16_t vramIndex, uint16_t chr, uint16_t x, uint16_t y, uint16_t backCol) {
	// Read the 1bpp bitmap to create a glyph, note bitmaps are upside down
	const uint8_t *bmp;
	printf("CHR: 0x%04hx", chr);
	if(chr > 0xFF) {
		chr -= 0x100;
		bmp = BMP_Kanji + sizeof(BMP_Kanji) - chr * BMPCHAR_BYTES;
	} else {
		chr -= 0x20;
		bmp = BMP_Ascii + sizeof(BMP_Ascii) - chr * BMPCHAR_BYTES;
	}
	// Reorder bmp data (16x16 -> 8x8) into chunks array
	uint8_t chunks[BMPCHAR_BYTES];
	static const uint8_t order[] = {
		0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15,
		16,24,17,25,18,26,19,27,20,28,21,29,22,30,23,31
	};
	for(uint8_t i = 0; i < BMPCHAR_BYTES; i++) chunks[order[i]] = *(--bmp);
	// Convert chunks to 4bpp tiles, using a set foreground and background color
	uint32_t tiles[TILECHAR_LONGS] = {};
	for(uint8_t i = 0; i < TILECHAR_LONGS; i++) {
		for(uint8_t column = 0; column < 8; column++) {
			// Palette indeces: 15 is white, 2 is texbox blue, 0 is transparency
			uint32_t color = chunks[i] & (1 << column) ? 15 : backCol;
			tiles[i] |= color << (column * 4);
		}
	}
	if(vramIndex >= 0xB000 >> 5 && vramIndex < 0xC000 >> 5) {
		// Between gaps in the window mapping, can't load in sequence
		VDP_loadTileData(&tiles[0],  vramIndex,   1, TRUE);
		VDP_loadTileData(&tiles[8],  vramIndex+4, 1, TRUE);
		VDP_loadTileData(&tiles[16], vramIndex+8, 1, TRUE);
		VDP_loadTileData(&tiles[24], vramIndex+12,1, TRUE);
		VDP_setTileMapXY(plan, TILE_ATTR_FULL(PAL0,1,0,0,vramIndex),   x,  y);
		VDP_setTileMapXY(plan, TILE_ATTR_FULL(PAL0,1,0,0,vramIndex+4), x+1,y);
		VDP_setTileMapXY(plan, TILE_ATTR_FULL(PAL0,1,0,0,vramIndex+8), x,  y+1);
		VDP_setTileMapXY(plan, TILE_ATTR_FULL(PAL0,1,0,0,vramIndex+12),x+1,y+1);
	} else {
		VDP_loadTileData(tiles, vramIndex, 4, TRUE);
		VDP_fillTileMapRectInc(plan, TILE_ATTR_FULL(PAL0,1,0,0,vramIndex), x, y, 2, 2);
	}
}
