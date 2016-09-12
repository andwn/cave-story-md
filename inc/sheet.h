#ifndef INC_SHEET_H_
#define INC_SHEET_H_

#include <genesis.h>
#include "common.h"

/*
 * Handles pre-loaded shared sprite sheets and individually owned sprite tile allocations
 */

#define MAX_SHEETS	16
#define MAX_TILOCS	64

#define NOSHEET 255
#define NOTILOC 255
 
// Reduces the copy paste mess of VDP_loadTileData calls
#define SHEET_ADD(sheetid, frames, width, height) {                                            \
	if(sheet_num < MAX_SHEETS) {                                                               \
		u16 index = sheet_num ? sheets[sheet_num-1].index + sheets[sheet_num-1].size           \
							  : TILE_SHEETINDEX;                                               \
		sheets[sheet_num] = (Sheet) { sheetid, frames*width*height, index, width, height };    \
		tiloc_index = sheets[sheet_num].index + sheets[sheet_num].size;                        \
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
	myindex = NOTILOC;                                                                         \
	u8 freeCount = 0;                                                                          \
	for(u8 i = 0; i < MAX_TILOCS; i++) {                                                       \
		if(tilocs[i]) {                                                                        \
			freeCount = 0;                                                                     \
			continue;                                                                          \
		}                                                                                      \
		if(++freeCount * 4 >= (framesize)) {                                                   \
			myindex = i;                                                                       \
			break;                                                                             \
		}                                                                                      \
	}                                                                                          \
	if(myindex != NOTILOC) {                                                                   \
		myindex -= freeCount-1;                                                                \
		while(freeCount--) tilocs[myindex+freeCount] = TRUE;                                   \
	}                                                                                          \
}
#define TILOC_FREE(myindex, framesize) {                                                       \
	u8 freeCount = framesize / 4 + (framesize & 3 ? 1 : 0);                                    \
	while(freeCount--) tilocs[myindex+freeCount] = FALSE;                                      \
}
#define TILES_QUEUE(tiles, index, count) {                                                     \
	DMA_queueDma(DMA_VRAM, (u32)(tiles), (index) * TILE_SIZE, (count) * 16, 2);                \
}

enum { 
	SHEET_NONE, SHEET_PSTAR, SHEET_MGUN, SHEET_FBALL, SHEET_HEART, SHEET_MISSILE, 
	SHEET_ENERGY, SHEET_ENERGYL, SHEET_BAT, SHEET_CRITTER, SHEET_PIGNON, SHEET_JELLY, 
	SHEET_BALFROG, SHEET_CROW, SHEET_GAUDI, SHEET_FUZZ, SHEET_SPIKE, SHEET_BEETLE,
	SHEET_BEHEM, SHEET_TELE, SHEET_BASU, SHEET_BASIL, SHEET_TRAP, SHEET_MANNAN,
	SHEET_PUCHI, SHEET_SKULLH, SHEET_IGORSHOT, SHEET_REDSHOT, SHEET_LABSHOT, SHEET_GAUDID,
	SHEET_PCRITTER, SHEET_FAN, SHEET_BARMIMI, SHEET_DARK, SHEET_DARKBUB, SHEET_POWERS,
	SHEET_POWERF, SHEET_FLOWER, SHEET_BASUSHOT, SHEET_POLISH, SHEET_BABY, SHEET_CROC,
	SHEET_TERM, SHEET_FFIELD,
};

u8 sheet_num;
typedef struct {
	u8 id; // One of the values in the enum above - so an entity can find its sheet
	u8 size; // Total number of tiles used by the complete sheet
	u16 index; // VDP tile index
	u8 w, h; // Size of each frame
} Sheet;
Sheet sheets[MAX_SHEETS];

u16 tiloc_index;
u8 tilocs[MAX_TILOCS];

void sheets_init();

void sheets_refresh_polarstar(u8 level);
void sheets_refresh_machinegun(u8 level);
void sheets_refresh_fireball(u8 level);

void sheets_load_stage(u16 sid);

#endif
