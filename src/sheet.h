#ifndef SHEET_H
#define SHEET_H

/*
 * Handles preloaded shared sprite sheets and individually owned sprite tile allocations.
 * SHEET refers to a preloaded sprite sheet in VRAM.
 * TILOC is short for "Tile Allocation".
 *
 * When an NPC has a "num_sprites" of one or more, the value of "sheet" is first checked.
 *
 * If valid (<MAX_SHEETS), the NPC's sprite will be set up to point to the VRAM index stored
 * in the sheets array, indexed by the "sheet" field.
 * Composite sprites (>1 VDP sprite) can NOT be sheets, only the first will be drawn.
 *
 * If "sheet" is NOSHEET or otherwise too large (>=NUM_SHEETS), a segment of VRAM will
 * be "dynamically" allocated for use by the current NPC only. Rather than the VRAM index
 * being changed when the NPC's animation frame is updated, tiles for the new frame will be
 * queued for DMA next vblank, and uploaded to the same unchanging VRAM index.
 *
 * Most NPCs (those not flagged "persistent") will be banished to the "inactive list" when
 * leaving the screen area. When this happens to an NPC with a Tiloc, it will be
 * unallocated, and the "tiloc" field will be set to TILOC_INVALID. When this same NPC
 * is back on-screen (and therefore, pulled back out of the "inactive list") the Tiloc
 * will be allocated again.
 */

#define MAX_SHEETS	24
#define MAX_TILOCS	96

#define NOSHEET 255
#define NOTILOC 255
 
// Reduces the copy paste mess of vdp_tiles_load calls
#define SHEET_ADD(sheetid, sdef, frames, width, height, ...) {                                 \
	if(sheet_num < MAX_SHEETS) {                                                               \
		uint16_t index = sheet_num ? sheets[sheet_num-1].index + sheets[sheet_num-1].size      \
							  : TILE_SHEETINDEX;                                               \
		sheets[sheet_num] = (Sheet) {                                                          \
			sheetid, (frames)*(width)*(height), index, width, height                           \
		};                                                                                     \
		tiloc_index = sheets[sheet_num].index + sheets[sheet_num].size;                        \
		SHEET_LOAD(sdef, frames, (width)*(height), sheets[sheet_num].index, 1, __VA_ARGS__);   \
		for(uint8_t i = 0; i < 16; i++) frameOffset[sheet_num][i] = (width)*(height)*i;        \
		sheet_num++;                                                                           \
	}                                                                                          \
}

#define SHEET_ADD_NEW(sheetid, sdef, frames) {                                            	   \
	if(sheet_num < MAX_SHEETS) {                                                               \
		uint16_t index = sheet_num ? sheets[sheet_num-1].index + sheets[sheet_num-1].size      \
							  : TILE_SHEETINDEX;                                               \
        uint16_t fsize = sdef->width * sdef->height;                                           \
		sheets[sheet_num] = (Sheet) {                                                          \
			sheetid, (frames) * fsize, index, sdef->width, sdef->height                        \
		};                                                                                     \
		tiloc_index = sheets[sheet_num].index + sheets[sheet_num].size;                        \
		vdp_tiles_load(sdef->tiles, index, sheets[sheet_num].size);                            \
		for(uint8_t i = 0; i < 16; i++) frameOffset[sheet_num][i] = fsize*i;                   \
		sheet_num++;                                                                           \
	}                                                                                          \
}

// Replaces the tiles in a previously loaded sprite sheet
#define SHEET_MOD(sheetid, sdef, frames, width, height, ...) {                                 \
	uint8_t sindex = NOSHEET;                                                                  \
	SHEET_FIND(sindex, sheetid);                                                               \
	if(sindex != NOSHEET) {                                                                    \
		SHEET_LOAD(sdef, frames, (width)*(height), sheets[sindex].index, 1, __VA_ARGS__);      \
	}                                                                                          \
}

// The end params are frame indices from the sprite definition
#define SHEET_LOAD(sdef, frames, fsize, index, dma, ...) {                                     \
	static const uint8_t fa[frames] = { __VA_ARGS__ };                                         \
	for(uint16_t i = 0; i < frames; i++) {                                                     \
		vdp_tiles_load(SPR_TILES(sdef,fa[i]),(index)+i*(fsize),fsize);						   \
	}                                                                                          \
}

#define SHEET_FIND(index, sid) {                                                               \
	for(uint8_t ii = MAX_SHEETS; ii--; ) {                                                     \
		if(sheets[ii].id == sid) {                                                             \
			index = ii;                                                                        \
			break;                                                                             \
		}                                                                                      \
	}                                                                                          \
}

#define TILES_QUEUE(tiles, index, count) {                                                     \
	dma_queue_rom(DmaVRAM, (uint32_t)(tiles), (index) << 5, (count) << 4, 2);                  \
}

enum { 
	SHEET_NONE,    SHEET_PSTAR,  SHEET_MGUN,    SHEET_FBALL,  SHEET_HEART,  SHEET_MISSILE, 
	SHEET_ENERGY,  SHEET_ENERGYL,SHEET_BAT,     SHEET_CRITTER,SHEET_PIGNON, SHEET_FROGFEET, 
	SHEET_BALFROG, SHEET_CROW,   SHEET_GAUDI,   SHEET_FUZZ,   SHEET_SPIKE,  SHEET_BEETLE,
	SHEET_BEHEM,   SHEET_TELE,   SHEET_BASU,    SHEET_BASIL,  SHEET_TRAP,   SHEET_MANNAN,
	SHEET_PUCHI,   SHEET_SKULLH, SHEET_IGORSHOT,SHEET_REDSHOT,SHEET_LABSHOT,SHEET_IRONHBLK,
	SHEET_PCRITTER,SHEET_FAN,    SHEET_BARMIMI, SHEET_DARK,   SHEET_DARKBUB,SHEET_BLOWFISH,
	SHEET_POWERF,  SHEET_FLOWER, SHEET_BASUSHOT,SHEET_POLISH, SHEET_BABY,   SHEET_FIREWSHOT,
	SHEET_TERM,    SHEET_FFIELD, SHEET_FROG,    SHEET_DROP,   SHEET_PIGNONB,SHEET_OMGSHOT,
	SHEET_CURLYB,  SHEET_OMGLEG, SHEET_MISSL,   SHEET_XTARGET,SHEET_XFISHY, SHEET_XTREAD,
	SHEET_BLADE,   SHEET_FUZZC,  SHEET_XBODY,   SHEET_SPUR,   SHEET_CROC,   SHEET_GAUDISHOT,
	SHEET_SNAKE,   SHEET_BUBB,   SHEET_NEMES,   SHEET_CGUN,   SHEET_JELLY,  SHEET_GAUDIEGG,
	SHEET_ZZZ,	   SHEET_GAUDID, SHEET_IKACHAN, SHEET_POWERS, SHEET_SMSTAL, SHEET_LGSTAL,
	SHEET_SISHEAD, SHEET_BUYOB,  SHEET_BUYO,	SHEET_HOPPY,  SHEET_ACID,   SHEET_NIGHTSHOT,
	SHEET_GUNFSHOT,SHEET_MIDO,   SHEET_PRESS,	SHEET_STUMPY, SHEET_CORES1, SHEET_CORES3,
	SHEET_CORES4,  SHEET_REDDOT, SHEET_MIMI,	SHEET_DOCSHOT,SHEET_RING,	SHEET_SHOCK,
	SHEET_FIRE,    SHEET_MAPI,   SHEET_MUCORE,  SHEET_MUCORE2,SHEET_MUCORE3,SHEET_CAGE,
	SHEET_BLADES,  SHEET_NEMESV, SHEET_BLOCK,   SHEET_BLOCKM, SHEET_ROCK,	SHEET_CRYSTAL,
	SHEET_BUTE,    SHEET_BUTEARW,SHEET_ROLLING, SHEET_DELEET, SHEET_BONE,   SHEET_DEVIL,
	SHEET_BUTEDIE, SHEET_PLATF,  SHEET_ROT,     SHEET_TARGET, SHEET_BLGMISL,SHEET_HPLIT,
	SHEET_AHCHOO,  SHEET_RAVIL,	 SHEET_GAUDIF,  SHEET_BSHUTTR,SHEET_SHUTTER,SHEET_LIFT,
	SHEET_FIREW,   SHEET_WSTAR,
};

extern uint8_t sheet_num;
typedef struct {
	uint8_t id; // One of the values in the enum above - so an entity can find its sheet
	uint8_t size; // Total number of tiles used by the complete sheet
	uint16_t index; // VDP tile index
	uint8_t w, h; // Size of each frame
} Sheet;
extern Sheet sheets[MAX_SHEETS];

// Avoids MULU in entity update
extern uint8_t frameOffset[MAX_SHEETS][16];

extern uint16_t tiloc_index;
extern uint8_t tilocs[MAX_TILOCS];

// Get the first available space in VRAM and allocate it
static uint8_t tiloc_add(uint8_t framesize) {
	framesize = (framesize + 3) >> 2;
	uint8_t myindex = NOTILOC;
	uint8_t freeCount = 0;
	for(uint8_t i = 0; i < MAX_TILOCS; i++) {
		if(tilocs[i]) {
			freeCount = 0;
		} else {
			freeCount++;
			if(freeCount >= framesize) {
				myindex = i;
				break;
			}
		}
	}
	if(myindex != NOTILOC) {
		myindex -= freeCount-1;
		while(freeCount--) tilocs[myindex+freeCount] = TRUE;
	}
	return myindex;
}
static void tiloc_free(const uint8_t myindex, const uint8_t framesize) {
	uint8_t freeCount = ((framesize) >> 2) + (((framesize) & 3) ? 1 : 0);
	while(freeCount--) tilocs[(myindex)+freeCount] = FALSE;
}

void sheets_load_weapon(Weapon *w);
void sheets_refresh_weapon(Weapon *w);
void sheets_load_stage(uint16_t sid, uint8_t init_base, uint8_t init_tiloc);
void sheets_load_intro(void);

#endif
