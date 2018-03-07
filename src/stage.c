#include "common.h"

#include "ai.h"
#include "camera.h"
#include "dma.h"
#include "effect.h"
#include "entity.h"
#include "error.h"
#include "hud.h"
#include "input.h"
#include "joy.h"
#include "memory.h"
#include "npc.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "sprite.h"
#include "string.h"
#include "system.h"
#include "tables.h"
#include "tools.h"
#include "tsc.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_pal.h"
#include "vdp_tile.h"
#include "vdp_ext.h"
#include "xgm.h"

#include "stage.h"

// Could fit under the Oside map (192 tile gap)
#define TILE_MOONINDEX (TILE_TSINDEX + 32*8)
// Another tile gap, fits under both Almond and Cave
#define TILE_WATERINDEX (TILE_TSINDEX + 384)

// Index of background in db/back.c and the effect type
uint8_t stageBackground = 0xFF;

int16_t backScrollTable[32];
//uint8_t *PXM = NULL;
uint8_t *stageBlocks = NULL;
uint16_t *stageTable = NULL;

typedef struct {
	uint8_t index;
	uint8_t dir;
} Current;
Current currents[4];
uint8_t currentsCount = 0;
uint8_t currentsTimer = 0;

void stage_load_tileset();
void stage_load_blocks();
void stage_load_entities();

void stage_draw_block(uint16_t x, uint16_t y);
void stage_draw_screen();
void stage_draw_screen_credits();
void stage_draw_background();
void stage_draw_moonback();

void stage_load(uint16_t id) {
	VDP_setEnable(FALSE);
	// Prevents an issue where a column of the previous map would get drawn over the new one
	DMA_clearQueue();
	input_update(); // Prevent menu from opening after loading save
	stageID = id;
	// Clear out or deactivate stuff from the old stage
	effects_clear();
	entities_clear();
	if(stageBlocks) {
		MEM_free(stageBlocks);
		stageBlocks = NULL;
	}
	//if(PXM) {
	//	MEM_free(PXM);
	//	PXM = NULL;
	//	stageBlocks = NULL;
	//}
	if(stageTable) {
		MEM_free(stageTable);
		stageTable = NULL;
	}
	sprites_clear();
	water_entity = NULL;
	bossEntity = NULL;
	
	MUSIC_TICK();
	// Load the tileset
	if(stageTileset != stage_info[id].tileset) {
		stageTileset = stage_info[id].tileset;
		stage_load_tileset();
		MUSIC_TICK();
	}
	// Load sprite sheets
	sheets_load_stage(id, FALSE, TRUE);
	MUSIC_TICK();
	// Stage palette and shared NPC palette
	if(stageID == 0x30) {
		VDP_setCachedPalette(PAL2, PAL_RiverAlt.data); // For Waterway green background
	} else {
		VDP_setCachedPalette(PAL2, tileset_info[stageTileset].palette->data);
	}
	VDP_setCachedPalette(PAL3, stage_info[id].npcPalette->data);
	// Load backgrounds
	if(background_info[stage_info[id].background].type == 4 || 
			stageBackground != stage_info[id].background) {
		stageBackground = stage_info[id].background;
		stageBackgroundType = background_info[stageBackground].type;
		
		VDP_setBackgroundColor(0); // Color index 0 for everything except fog
		if(stageBackgroundType == 0) { // Tiled image
			VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
			VDP_loadTileSet(background_info[stageBackground].tileset, TILE_BACKINDEX, TRUE);
			stage_draw_background();
		} else if(stageBackgroundType == 1) { // Moon
			VDP_setScrollingMode(HSCROLL_TILE, VSCROLL_PLANE);
			stage_draw_moonback();
		} else if(stageBackgroundType == 2) { // Solid Color
			VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
			VDP_clearPlan(PLAN_B, TRUE);
		} else if(stageBackgroundType == 3) { // Tiled image, auto scroll
			VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
			VDP_loadTileSet(background_info[stageBackground].tileset, TILE_BACKINDEX, TRUE);
			stage_draw_background();
		} else if(stageBackgroundType == 4) { // Almond Water
			VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
			VDP_clearPlan(PLAN_B, TRUE);
			// Your guess is as good as mine as to why these numbers specifically
			// fix the stupid water background drawing over the wrong rows.
			// Yeah there's even a fucking PAL specific number nice awesomenfweroqvnerbn
			if(stageID == STAGE_DARK_PLACE) {
				backScrollTable[0] = -13;
			} else {
				backScrollTable[0] = pal_mode ? -20 : -21;
			}
			VDP_loadTileSet(&BG_Water, TILE_WATERINDEX, TRUE);
		} else if(stageBackgroundType == 5) { // Fog
			VDP_setScrollingMode(HSCROLL_TILE, VSCROLL_PLANE);
			// Use background color from tileset
			VDP_setBackgroundColor(32);
			stage_draw_moonback();
		}
		MUSIC_TICK();
	}
	// Load stage PXM into RAM
	stage_load_blocks();
	MUSIC_TICK();
	// Move camera to player's new position
	camera_set_position(player.x, player.y - (stageBackgroundType == 3 ? 8<<CSF : 0));
	camera.target = &player;
	camera.x_offset = 0;
	camera.y_offset = 0;
	MUSIC_TICK();
	stage_draw_screen(); // Draw 64x32 foreground PXM area at camera's position
	MUSIC_TICK();
	stage_load_entities(); // Create entities defined in the stage's PXE
	MUSIC_TICK();
	// For rooms where the boss is always loaded
	if(stageBackgroundType == 3) {
		bossEntity = entity_create(0, 0, 360 + BOSS_IRONHEAD, 0);
	} else if(stageBackgroundType == 4) {
		backScrollTable[0] = (SCREEN_HEIGHT >> 3) + 1;
	}
	if(stageID == STAGE_BLACK_SPACE) {
		bossEntity = entity_create(0, 0, 360 + BOSS_UNDEADCORE, 0);
	}
	if(stageID == STAGE_HELL_B3 || stageID == STAGE_HELL_PASSAGEWAY_2) {
		bossEntity = entity_create(0, 0, 360 + BOSS_HEAVYPRESS, 0);
	}
	DMA_flushQueue();
	MUSIC_TICK();
	if((playerEquipment & EQUIP_CLOCK) || stageID == STAGE_HELL_B1) system_draw_counter();
	tsc_load_stage(id);
	MUSIC_TICK();
	XGM_set68KBUSProtection(FALSE);
	VDP_setEnable(TRUE);
}

void stage_load_credits(uint8_t id) {
	stageID = id;
	
	entities_clear();
	sprites_clear();
	if(stageBlocks) {
		MEM_free(stageBlocks);
		stageBlocks = NULL;
	}
	//if(PXM) {
	//	MEM_free(PXM);
	//	PXM = NULL;
	//	stageBlocks = NULL;
	//}
	if(stageTable) {
		MEM_free(stageTable);
		stageTable = NULL;
	}
	
	VDP_setEnable(FALSE);
	MUSIC_TICK();
	stageTileset = stage_info[id].tileset;
	stage_load_tileset();
	MUSIC_TICK();
	sheets_load_stage(id, FALSE, TRUE);
	MUSIC_TICK();
	VDP_setCachedPalette(PAL2, tileset_info[stageTileset].palette->data);
	stage_load_blocks();
	MUSIC_TICK();
	stage_draw_screen_credits();
	MUSIC_TICK();
	stage_load_entities();
	MUSIC_TICK();
	DMA_flushQueue();
	MUSIC_TICK();
	tsc_load_stage(id);
	MUSIC_TICK();
	XGM_set68KBUSProtection(FALSE);
	VDP_setEnable(TRUE);
}

void stage_load_tileset() {
	VDP_loadTileSet(tileset_info[stageTileset].tileset, TILE_TSINDEX, TRUE);
	// Inject the breakable block sprite into the tileset
	const uint8_t *PXA = tileset_info[stageTileset].PXA;
	for(uint16_t i = 0; i < 160; i++) {
		if(PXA[i] == 0x43) {
			uint32_t addr1 = ((i * 2) / TS_WIDTH * TS_WIDTH * 2) + ((i * 2) % TS_WIDTH),
			addr2 = ((i * 2) / TS_WIDTH * TS_WIDTH * 2) + ((i * 2) % TS_WIDTH) + TS_WIDTH;
			VDP_loadTileData(TS_Break.tiles, TILE_TSINDEX + addr1, 2, TRUE);
			VDP_loadTileData(TS_Break.tiles + 16, TILE_TSINDEX + addr2, 2, TRUE);
		}
	}
	// Search for any "wind" tiles and note their index to animate later
	currentsCount = 0;
	for(uint16_t i = 0; i < 160; i++) {
		if(!(PXA[i] & 0x80)) continue;
		currents[currentsCount] = (Current) { .index = i, .dir = PXA[i] & 0x3 };
		if(++currentsCount == 4) break;
	}
}

void stage_load_blocks() {
	const uint8_t *PXM = stage_info[stageID].PXM;
	stageWidth = PXM[4] | (PXM[5] << 8);
	stageHeight = PXM[6] | (PXM[7] << 8);
	PXM += 8;
	stageBlocks = MEM_alloc(stageWidth * stageHeight);
	if(!stageBlocks) error_oom();
	memcpy(stageBlocks, PXM, stageWidth * stageHeight);

	//PXM = decompress_slz(stage_info[stageID].PXM);
	//if(!PXM) error_oom();
	//stageWidth = PXM[4] | (PXM[5] << 8);
	//stageHeight = PXM[6] | (PXM[7] << 8);
	//stageBlocks = &PXM[8];

	//uint16_t size = stage_info[stageID].PXM[0] << 8 | stage_info[stageID].PXM[1];
	//PXM = MEM_alloc(size);
	//if(!PXM) error_oom();
	//__asm__("movem.l %%d5-%%d7/%%a5-%%a6,-(%%sp)\n\t"
	//		"move.l %0,%%a5\n\t"
	//		"move.l %1,%%a6\n\t"
	//		"jsr (DecompressSlz)\n\t"
	//		"movem.l (%%sp)+,%%d5-%%d7/%%a5-%%a6" 
	//		: : "g" (stage_info[stageID].PXM), "g" (PXM));
	//stageWidth = PXM[4] | (PXM[5] << 8);
	//stageHeight = PXM[6] | (PXM[7] << 8);
	//stageBlocks = &PXM[8];

	// Multiplication table for stage rows
	stageTable = MEM_alloc(stageHeight * 2);
	if(!stageTable) error_oom();
	uint16_t blockTotal = 0;
	for(uint16_t y = 0; y < stageHeight; y++) {
		stageTable[y] = blockTotal;
		blockTotal += stageWidth;
	}
}

void stage_load_entities() {
	const uint8_t *PXE = stage_info[stageID].PXE;
	// PXE[4] is the number of entities to load. It's word length but never more than 255
	for(uint8_t i = 0; i < PXE[4]; i++) {
		uint16_t x, y, id, event, type, flags;
		// Like all of cave story's data files PXEs are little endian
		x     = PXE[8  + i * 12] + (PXE[9  + i * 12]<<8);
		y     = PXE[10 + i * 12] + (PXE[11 + i * 12]<<8);
		id    = PXE[12 + i * 12] + (PXE[13 + i * 12]<<8);
		event = PXE[14 + i * 12] + (PXE[15 + i * 12]<<8);
		type  = PXE[16 + i * 12] + (PXE[17 + i * 12]<<8);
		flags = PXE[18 + i * 12] + (PXE[19 + i * 12]<<8);
		// There are some unused entities that have all these values as 0, as well as
		// entities that should only exist when specific flags are on/off
		// Loading these would be a waste of memory, just skip them
		if(!id && !event && !type && !flags) continue;
		if((flags&NPC_DISABLEONFLAG) && system_get_flag(id)) continue;
		if((flags&NPC_ENABLEONFLAG) && !system_get_flag(id)) continue;
		// Special case to not load save points if SRAM is not found
		if(type == OBJ_SAVE_POINT && system_get_flag(FLAG_DISABLESAVE)) continue;
		// I'll probably need this code block again in the future.
		// When an NPC is assigned the improper number of sprites for their metasprite
		// loading it will crash BlastEm and possibly hardware too. This steps through
		// each entity as it is loaded so the problematic NPC can be found
		/*
		if(joy_down(BUTTON_A)) {
			VDP_setEnable(TRUE);
			VDP_setPaletteColor(15, 0xEEE);
			char str[2][36];
			sprintf(str[0], "X:%hu Y:%hu I:%hu", x, y, id);
			sprintf(str[1], "E:%hu T:%hu F:%hu", event, type, flags);
			VDP_drawText(str[0], 2, 2);
			VDP_drawText(str[1], 2, 4);
			
			input_update();
			while(!joy_pressed(BUTTON_C)) {
				input_update();
				vsync();
				aftervsync();
				VDP_setHorizontalScroll(PLAN_A, 0);
				VDP_setVerticalScroll(PLAN_A, 0);
			}
			VDP_setEnable(FALSE);
		}
		*/
		entity_create_ext(block_to_sub(x) + 0x1000, block_to_sub(y) + 0x1000, type, flags, id, event);
	}
}

// Replaces a block with another (for <CMP, <SMP, and breakable blocks)
void stage_replace_block(uint16_t bx, uint16_t by, uint8_t index) {
	stageBlocks[stageTable[by] + bx] = index;
	int16_t cx = sub_to_block(camera.x), cy = sub_to_block(camera.y);
	if(cx - 16 > bx || cx + 16 < bx || cy - 8 > by || cy + 8 < by) return;
	// Only redraw if change was made onscreen
	stage_draw_block(bx, by);
}

// Stage vblank drawing routine
void stage_update() {
	// Background Scrolling
	// Type 2 is not included here, that's blank backgrounds which are not scrolled
	if(stageBackgroundType == 0) {
		VDP_setHorizontalScroll(PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		VDP_setHorizontalScroll(PLAN_B, -sub_to_pixel(camera.x) / 4 + SCREEN_HALF_W);
		VDP_setVerticalScroll(PLAN_B, sub_to_pixel(camera.y) / 4 - SCREEN_HALF_H);
	} else if(stageBackgroundType == 1 || stageBackgroundType == 5) {
		// PLAN_A Tile scroll
		int16_t off[32];
		off[0] = -sub_to_pixel(camera.x) + SCREEN_HALF_W;
		for(uint8_t i = 1; i < 32; i++) {
			off[i] = off[0];
		}
		VDP_setHorizontalScrollTile(PLAN_A, 0, off, 32, TRUE);
		VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		// Moon background has different spots scrolling horizontally at different speeds
		backScrollTimer--;
		
		if(pal_mode) {
			uint8_t y = 28;
			for(;y >= 22; --y) backScrollTable[y] = backScrollTimer << 1;
			for(;y >= 18; --y) backScrollTable[y] = backScrollTimer;
			for(;y >= 15; --y) backScrollTable[y] = backScrollTimer >> 1;
			for(;y >= 11; --y) backScrollTable[y] = backScrollTimer >> 2;
			VDP_setHorizontalScrollTile(PLAN_B, 0, backScrollTable, 32, TRUE);
			//VDP_setVerticalScroll(PLAN_B, -8);
		} else {
			uint8_t y = 27;
			for(;y >= 21; --y) backScrollTable[y] = backScrollTimer << 1;
			for(;y >= 17; --y) backScrollTable[y] = backScrollTimer;
			for(;y >= 14; --y) backScrollTable[y] = backScrollTimer >> 1;
			for(;y >= 10; --y) backScrollTable[y] = backScrollTimer >> 2;
			VDP_setHorizontalScrollTile(PLAN_B, 0, backScrollTable, 32, TRUE);
			//VDP_setVerticalScroll(PLAN_B, 0);
		}
	} else if(stageBackgroundType == 3) {
		// Lock camera at specific spot
		camera.target = NULL;
		// Ironhead boss background auto scrolls leftward
		backScrollTable[0] -= 2;
		VDP_setHorizontalScroll(PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		VDP_setHorizontalScroll(PLAN_B, backScrollTable[0]);
	} else if(stageBackgroundType == 4) {
		int16_t rowc = SCREEN_HEIGHT >> 3;
		int16_t rowgap = 31 - rowc;
		// Water surface relative to top of screen
		int16_t scroll = (water_entity->y >> CSF) - ((camera.y >> CSF) - SCREEN_HALF_H);
		int16_t row = scroll >> 3;
		int16_t oldrow = backScrollTable[0];
		while(row < oldrow) { // Water is rising (Y decreasing)
			oldrow--;
			uint8_t rowup = 31 - ((oldrow + rowgap) & 31);// Row that will be updated
			if(oldrow > rowc) { // Below Screen
				uint16_t mapBuffer[64] = {};
				DMA_doDma(DMA_VRAM, (uint32_t)mapBuffer, VDP_PLAN_B + (rowup << 7), 64, 2);
			} else { // On screen or above
				uint16_t mapBuffer[64];
				for(uint16_t x = 0; x < 64; x++) {
					mapBuffer[x] = TILE_ATTR_FULL(PAL0,1,0,0,
							TILE_WATERINDEX + (oldrow == rowc ? x&3 : 4 + (random()&15)));
				}
				DMA_doDma(DMA_VRAM, (uint32_t)mapBuffer, VDP_PLAN_B + (rowup << 7), 64, 2);
			}
		}
		while(row > oldrow) { // Water is lowering (Y increasing)
			oldrow++;
			uint8_t rowup = 31 - (oldrow & 31); // Row that will be updated
			if(oldrow <= 0) { // Above screen
				uint16_t mapBuffer[64];
				for(uint16_t x = 0; x < 64; x++) {
					mapBuffer[x] = TILE_ATTR_FULL(PAL0,1,0,0,
							TILE_WATERINDEX + (oldrow == 0 ? x&3 : 4 + (random()&15)));
				}
				DMA_doDma(DMA_VRAM, (uint32_t)mapBuffer, VDP_PLAN_B + (rowup << 7), 64, 2);
			} else { // On screen or below
				uint16_t mapBuffer[64] = {};
				DMA_doDma(DMA_VRAM, (uint32_t)mapBuffer, VDP_PLAN_B + (rowup << 7), 64, 2);
			}
		}
		
		VDP_setHorizontalScroll(PLAN_B, -sub_to_pixel(camera.x) + SCREEN_HALF_W - backScrollTimer);
		VDP_setVerticalScroll(PLAN_B, -scroll);
		VDP_setHorizontalScroll(PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		backScrollTable[0] = row;
	} else {
		// Only scroll foreground
		VDP_setHorizontalScroll(PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
	}
	if(currentsCount) { // Waterway currents
		currentsTimer = (currentsTimer + 1) & 0x1F;
		uint8_t t = currentsTimer & 3;
		if(t < currentsCount) {
			uint16_t from_index = 0;
			uint8_t *from_ts = NULL;
			uint16_t to_index = TILE_TSINDEX + ((currents[t].index & 15) << 1) + ((currents[t].index >> 4) << 6);
			switch(currents[t].dir) {
				case 0: // Left
					from_ts = (uint8_t*) TS_WindH.tiles;
					from_index += (currentsTimer >> 1) & ~1;
				break;
				case 1: // Up
					from_ts = (uint8_t*) TS_WindV.tiles;
					from_index += (currentsTimer >> 1) & ~1;
				break;
				case 2: // Right
					from_ts = (uint8_t*) TS_WindH.tiles;
					from_index += 14 - ((currentsTimer >> 1) & ~1);
				break;
				case 3: // Down
					from_ts = (uint8_t*) TS_WindV.tiles;
					from_index += 14 - ((currentsTimer >> 1) & ~1);
				break;
				default: return;
			}
			// Replace the tile in the tileset
			DMA_doDma(DMA_VRAM, (uint32_t) (from_ts + (from_index << 5)), to_index << 5, 32, 2);
			from_index += 16;
			to_index += 32;
			DMA_doDma(DMA_VRAM, (uint32_t) (from_ts + (from_index << 5)), to_index << 5, 32, 2);
		}
	}
}

void stage_draw_screen() {
	uint16_t maprow[64];
	int16_t y = sub_to_tile(camera.y) - 16;
	for(uint16_t i = 32; i--; ) {
		if(vblank) aftervsync(); // So we don't lag the music
		vblank = 0;
		
		if(y >= 0 && y < stageHeight << 1) {
			int16_t x = sub_to_tile(camera.x) - 32;
			for(uint16_t j = 64; j--; ) {
				if(x >= stageWidth << 1) break;
				if(x >= 0) {
					uint16_t b = stage_get_block(x>>1, y>>1);
					uint16_t t = ((b&15) << 1) + ((b>>4) << 6);
					uint16_t ta = stage_get_block_type(x>>1, y>>1);
					uint16_t pal = (ta == 0x43 || ta & 0x80) ? PAL1 : PAL2;
					maprow[x&63] = TILE_ATTR_FULL(pal, (ta&0x40) > 0, 
							0, 0, TILE_TSINDEX + t + (x&1) + ((y&1)<<5));
				}
				x++;
			}
			DMA_doDma(DMA_VRAM, (uint32_t)maprow, VDP_PLAN_A + ((y&31)<<7), 64, 2);
		}
		y++;
	}
}

void stage_draw_screen_credits() {
	uint16_t maprow[20];
	for(uint8_t y = 0; y < 30; y++) {
		for(uint8_t x = 20; x < 40; x++) {
			uint8_t b = stage_get_block(x/2, y/2);
			uint16_t t = (b%16) * 2 + (b/16) * 64;
			maprow[x-20] = TILE_ATTR_FULL(PAL2,0,0,0, TILE_TSINDEX + t + (x&1) + ((y&1)*32));
		}
		DMA_doDma(DMA_VRAM, (uint32_t)maprow, VDP_PLAN_A + y*0x80 + 40, 20, 2);
	}
}

// Draws just one block
void stage_draw_block(uint16_t x, uint16_t y) {
	if(x >= stageWidth || y >= stageHeight) return;
	uint16_t t, b, xx, yy; uint8_t p;
	p = (stage_get_block_type(x, y) & 0x40) > 0;
	t = block_to_tile(stage_get_block(x, y));
	b = TILE_TSINDEX + (t / TS_WIDTH * TS_WIDTH * 2) + (t % TS_WIDTH);
	xx = block_to_tile(x) % 64;
	yy = block_to_tile(y) % 32;
	
	VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(2, p, 0, 0, b), xx, yy);
	VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(2, p, 0, 0, b+1), xx+1, yy);
	VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(2, p, 0, 0, b+TS_WIDTH), xx, yy+1);
	VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(2, p, 0, 0, b+TS_WIDTH+1), xx+1, yy+1);
	
}

// Fills PLAN_B with a tiled background
void stage_draw_background() {
	uint8_t w = background_info[stageBackground].width;
	uint8_t h = background_info[stageBackground].height;
	uint16_t pal = background_info[stageBackground].palette;
	for(uint8_t y = 0; y < 32; y += h) {
		for(uint8_t x = 0; x < 64; x += w) {
			VDP_fillTileMapRectInc(PLAN_B, TILE_ATTR_FULL(pal,0,0,0,TILE_BACKINDEX), x, y, 
					(x+w > 64) ? (64-x) : w, (y+h > 32) ? (32-y) : h);
		}
	}
	
}

void stage_draw_moonback() {
	const uint32_t *topTiles, *btmTiles;
	const uint16_t *topMap, *btmMap;
	if(stageBackgroundType == 1) {
		// Moon
		topTiles = (uint32_t*) PAT_MoonTop;
		btmTiles = (uint32_t*) PAT_MoonBtm;
		topMap = (uint16_t*) MAP_MoonTop;
		btmMap = (uint16_t*) MAP_MoonBtm;
	} else {
		// Fog
		topTiles = (uint32_t*) PAT_FogTop;
		btmTiles = (uint32_t*) PAT_FogBtm;
		topMap = (uint16_t*) MAP_FogTop;
		btmMap = (uint16_t*) MAP_FogBtm;
	}
	// Load the top section in the designated background area
	VDP_loadTileData(topTiles, TILE_BACKINDEX, 12, TRUE);
	// Load the clouds under the map, it just fits
	VDP_loadTileData(btmTiles, TILE_MOONINDEX, 188, TRUE);
	for(uint8_t y = 0; y < 32; y++) backScrollTable[y] = 0;
	VDP_setVerticalScroll(PLAN_B, 0);
	// Top part
	uint16_t index = pal_mode ? 0 : 40;
	for(uint16_t y = 0; y < (pal_mode ? 11 : 10); y++) {
		DMA_doDma(DMA_VRAM, (uint32_t) &topMap[index], VDP_PLAN_B + (y << 7), 40, 2);
		index += 40;
	}
	
	if(vblank) aftervsync(); // So we don't lag the music
	vblank = 0;
	
	// Bottom part
	index = 0;
	for(uint16_t y = (pal_mode ? 11 : 10); y < (pal_mode ? 32 : 28); y++) {
		DMA_doDma(DMA_VRAM, (uint32_t) &btmMap[index], VDP_PLAN_B + (y << 7),             32, 2);
		DMA_doDma(DMA_VRAM, (uint32_t) &btmMap[index], VDP_PLAN_B + (y << 7) + (32 << 1), 32, 2);
		index += 32;
	}
	backScrollTimer = 0;
}
