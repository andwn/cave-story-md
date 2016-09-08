#include <genesis.h>
#include "common.h"

/*
 * Module to organize sprite sheets for manual tile allocation
 * 
 * SGDK will often duplicate tiles uploads for sprites especially after they are deleted
 * and there are a lot of the same sprite existing at the same time. 
 * I work around this by manually picking out specific sprites and adding them here. 
 * The objects which use this are responsible for knowing their sprite sheet's 
 * frame width, height, number and animation
 */

#define MAX_SHEETS	12
 
// Reduces the copy paste mess of VDP_loadTileData calls
// The end params are anim,frame value couples from the sprite definition
#define SHEET_LOAD(sdef, frames, fsize, index, dma, ...) {                                     \
	const u8 fa[frames*2] = { __VA_ARGS__ };                                                   \
	for(u8 i = 0; i < frames; i++) {                                                           \
		VDP_loadTileData(SPR_TILESET(sdef,fa[i*2],fa[i*2+1])->tiles,index+i*fsize,fsize,dma);  \
	}                                                                                          \
}

#define TILES_QUEUE(tiles,index,count) {                                                       \
	DMA_queueDma(DMA_VRAM,(u32)tiles,(index)*TILE_SIZE,(count)*16,2);                          \
}

#define FIND_SHEET(index, sid) {                                                               \
	for(u8 i = MAX_SHEETS; i--; ) {                                                            \
		if(sheets[i].id == sid) {                                                              \
			index = i;                                                                         \
			break;                                                                             \
		}                                                                                      \
	}                                                                                          \
}

enum { 
	SHEET_NONE, SHEET_PSTAR, SHEET_MGUN, SHEET_FBALL, SHEET_HEART, SHEET_MISSILE, 
	SHEET_ENERGY, SHEET_ENERGYL, SHEET_BAT, SHEET_CRITTER, SHEET_PIGNON, SHEET_JELLY, 
	SHEET_BALFROG, SHEET_CROW, SHEET_GAUDI, SHEET_FUZZ, SHEET_SPIKE,
};

// Frame count/height/time not included, the object will assume their own values
typedef struct {
	u8 id; // One of the values in the enum above
	u8 size; // Number of tiles
	u16 index; // VDP tile index
	u8 w, h;
} Sheet;

Sheet sheets[MAX_SHEETS];

void sheets_init();

void sheets_refresh_polarstar(u8 level);
void sheets_refresh_machinegun(u8 level);
void sheets_refresh_fireball(u8 level);

void sheets_load_stage(u16 sid);
