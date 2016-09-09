#include <genesis.h>
#include "common.h"

/*
 * Handles pre-loaded shared sprite sheets and individually owned sprite tile allocations
 */

#define MAX_SHEETS	16
#define MAX_TILOCS	32
 
// Reduces the copy paste mess of VDP_loadTileData calls
#define SHEET_ADD(sheetid, frames, width, height) {                                            \
	if(sheet_num < MAX_SHEETS) {                                                               \
		u16 index = sheet_num ? sheets[sheet_num-1].index + sheets[sheet_num-1].size           \
							  : TILE_SHEETINDEX;                                               \
		sheets[sheet_num] = (Sheet) { sheetid, frames*width*height, index, width, height };    \
		sheet_num++;                                                                           \
	}                                                                                          \
}
// The end params are anim,frame value couples from the sprite definition
#define SHEET_LOAD(sdef, frames, fsize, index, dma, ...) {                                     \
	const u8 fa[frames*2] = { __VA_ARGS__ };                                                   \
	for(u8 i = 0; i < frames; i++) {                                                           \
		VDP_loadTileData(SPR_TILES(sdef,fa[i*2],fa[i*2+1]),index+i*fsize,fsize,dma);           \
	}                                                                                          \
}
#define SHEET_FIND(index, sid) {                                                               \
	for(u8 i = MAX_SHEETS; i--; ) {                                                            \
		if(sheets[i].id == sid) {                                                              \
			index = i;                                                                         \
			break;                                                                             \
		}                                                                                      \
	}                                                                                          \
}

// Get the first available space in VRAM and allocate it
#define TILOC_ADD(myindex, framesize) {                                                        \
	myindex = tiloc_num ? tilocs[tiloc_num-1].index + tilocs[tiloc_num-1].size                 \
			: sheet_num ? sheets[sheet_num-1].index + sheets[sheet_num-1].size                 \
			: TILE_SHEETINDEX;                                                                 \
	tilocs[tiloc_num] = (Tiloc) { framesize, myindex };                                        \
	tiloc_num++;                                                                               \
}
#define TILES_QUEUE(tiles, index, count) {                                                     \
	DMA_queueDma(DMA_VRAM, (u32)tiles, (index) * TILE_SIZE, (count) * 16, 2);                  \
}

enum { 
	SHEET_NONE, SHEET_PSTAR, SHEET_MGUN, SHEET_FBALL, SHEET_HEART, SHEET_MISSILE, 
	SHEET_ENERGY, SHEET_ENERGYL, SHEET_BAT, SHEET_CRITTER, SHEET_PIGNON, SHEET_JELLY, 
	SHEET_BALFROG, SHEET_CROW, SHEET_GAUDI, SHEET_FUZZ, SHEET_SPIKE,
};

u8 sheet_num;
typedef struct {
	u8 id; // One of the values in the enum above - so an entity can find its sheet
	u8 size; // Total number of tiles used by the complete sheet
	u16 index; // VDP tile index
	u8 w, h; // Size of each frame
} Sheet;
Sheet sheets[MAX_SHEETS];

u8 tiloc_num;
typedef struct {
	u8 size; // Number of tiles
	u16 index; // VDP tile index
} Tiloc;
Tiloc tilocs[MAX_TILOCS];

void sheets_init();

void sheets_refresh_polarstar(u8 level);
void sheets_refresh_machinegun(u8 level);
void sheets_refresh_fireball(u8 level);

void sheets_load_stage(u16 sid);
