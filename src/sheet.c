#include "sheet.h"

#include "player.h"
#include "resources.h"
#include "stage.h"
#include "tables.h"

#ifndef KDB_SHEET
#define puts(x) /**/
#define printf(...) /**/
#endif

void sheets_init() {
	// Bullets
	sheets[0] = (Sheet){ SHEET_PSTAR, 2*4, TILE_SHEETINDEX, 2, 2 };
	sheets[1] = (Sheet){ SHEET_MGUN, 2*4, sheets[0].index + sheets[0].size, 2, 2 };
	sheets[2] = (Sheet){ SHEET_FBALL, 3*4, sheets[1].index + sheets[1].size, 2, 2 };
	// Power ups
	sheets[3] = (Sheet){ SHEET_HEART, 4*4, sheets[2].index + sheets[2].size, 2, 2 };
	sheets[4] = (Sheet){ SHEET_MISSILE, 4*4, sheets[3].index + sheets[3].size, 2, 2 };
	sheets[5] = (Sheet){ SHEET_ENERGY, 6*1, sheets[4].index + sheets[4].size, 1, 1 };
	sheets[6] = (Sheet){ SHEET_ENERGYL, 6*4, sheets[5].index + sheets[5].size, 2, 2 };
	// Actually load the tiles - assume the VDP is disabled
	sheets_refresh_weapons();
	// Power ups
	SHEET_LOAD(&SPR_Heart,  4,4, sheets[3].index, 1, 0,0, 0,1, 1,0, 1,1);
	SHEET_LOAD(&SPR_MisslP, 4,4, sheets[4].index, 1, 0,0, 0,1, 1,0, 1,1);
	SHEET_LOAD(&SPR_EnergyS,6,1, sheets[5].index, 1, 0,0, 0,1, 0,2, 0,3, 0,4, 0,5);
	SHEET_LOAD(&SPR_EnergyL,6,4, sheets[6].index, 1, 0,0, 0,1, 0,2, 0,3, 0,4, 0,5);
	// Precalculated VRAM offset
	for(u8 s = 0; s < 7; s++) {
		for(u8 i = 0; i < 16; i++) {
			frameOffset[s][i] = sheets[s].w * sheets[s].h * i;
		}
	}
}

void sheets_refresh_polarstar(u8 level) {
	const SpriteDefinition *def;
	if(level == 1) def = &SPR_PolarB1;
	else if(level == 2) def = &SPR_PolarB2;
	else if(level == 3) def = &SPR_PolarB3;
	else {
		printf("Polar Star has no level %hu", level);
		return;
	}
	VDP_loadTileData(SPR_TILES(def, 0, 0), sheets[0].index, 4, 1);
	VDP_loadTileData(SPR_TILES(def, 1, 0), sheets[0].index + 4, 4, 1);
}

void sheets_refresh_machinegun(u8 level) {
	const SpriteDefinition *def;
	if(level == 1) def = &SPR_MGunB1;
	else if(level == 2) def = &SPR_MGunB2;
	else if(level == 3) def = &SPR_MGunB3;
	else {
		printf("Machine Gun has no level %hu", level);
		return;
	}
	VDP_loadTileData(SPR_TILES(def, 0, 0), sheets[1].index, 4, 1);
	VDP_loadTileData(SPR_TILES(def, 1, 0), sheets[1].index + 4, 4, 1);
}

void sheets_refresh_fireball(u8 level) {
	const SpriteDefinition *def;
	if(level == 1) def = &SPR_FirebB1;
	else if(level == 2) def = &SPR_FirebB1;
	else if(level == 3) def = &SPR_FirebB3;
	else {
		printf("Fireball has no level %hu", level);
		return;
	}
	VDP_loadTileData(SPR_TILES(def, 0, 0), sheets[2].index, 4, 1);
	VDP_loadTileData(SPR_TILES(def, 0, 1), sheets[2].index + 4, 4, 1);
	VDP_loadTileData(SPR_TILES(def, 0, 2), sheets[2].index + 8, 4, 1);
}

void sheets_refresh_weapons() {
	Weapon *pstar = player_find_weapon(WEAPON_POLARSTAR);
	Weapon *mgun =  player_find_weapon(WEAPON_MACHINEGUN);
	Weapon *fball = player_find_weapon(WEAPON_FIREBALL);
	sheets_refresh_polarstar(pstar ? pstar->level : 1);
	sheets_refresh_machinegun(mgun ? mgun->level : 1);
	sheets_refresh_fireball(fball ? fball->level : 1);
}

void sheets_load_stage(u16 sid) {
	// Reset values
	sheet_num = 7;
	memset(tilocs, 0, MAX_TILOCS);
	memset(&sheets[7], 0, sizeof(Sheet) * (MAX_SHEETS - 7));
	switch(sid) {
		case 0x0C: // First Cave
		{	SHEET_ADD(SHEET_BAT, &SPR_Bat, 6,2,2, 0,0, 0,1, 0,2, 1,0, 2,0, 3,0);
			SHEET_ADD(SHEET_CRITTER, &SPR_CritHB, 3,2,2, 0,0, 1,0, 2,0);
		} break;
		case 0x10: // Graveyard
		{	SHEET_ADD(SHEET_PIGNON, &SPR_Pignon, 5,2,2, 0,0, 1,0, 1,2, 2,0, 3,0);
			SHEET_ADD(SHEET_PIGNONB, &SPR_PignonB, 5,3,3, 0,0, 1,0, 1,2, 2,0, 3,0);
		} break;
		case 0x11: // Yamashita Farm
		{	SHEET_ADD(SHEET_FLOWER, &SPR_Flower, 6,2,2, 0,0, 0,1, 0,2, 0,3, 0,4, 0,5);
			SHEET_ADD(SHEET_DROP, &SPR_Drop, 1,1,1, 0,0);
		} break;
		case 0x01: // Arthur's House
		{	SHEET_ADD(SHEET_TELE, &SPR_TeleMenu, 10,4,2, 
					0,1, 1,1, 0,2, 1,2, 0,3, 1,3, 0,4, 1,4, 0,5, 1,5);
		} break;
		case 0x02: // Egg Corridor
		{	SHEET_ADD(SHEET_CRITTER, &SPR_CritHG, 3,2,2, 0,0, 1,0, 2,0);
			SHEET_ADD(SHEET_BEETLE, &SPR_BtlHG, 2,2,2, 0,0, 1,0);
			SHEET_ADD(SHEET_BEHEM, &SPR_Behem, 3,4,3, 0,0, 0,1, 0,3);
			SHEET_ADD(SHEET_BASIL, &SPR_Basil, 3,4,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_IGORSHOT, &SPR_EnergyShot, 2,2,2, 0,0, 0,1);
			SHEET_ADD(SHEET_FFIELD, &SPR_ForceField, 4,2,2, 0,0, 0,1, 0,2, 0,3);
		} break;
		case 0x05: // Egg Observatory
		case 0x17: // Egg no. 1
		{	SHEET_ADD(SHEET_TERM, &SPR_Term, 4,2,3, 0,0, 0,1, 0,2, 0,3);
		} break;
		case 0x06: // Grasstown
		{	SHEET_ADD(SHEET_FAN, &SPR_Fan, 6,2,2, 3,0, 3,1, 3,2, 2,0, 2,1, 2,2);
			SHEET_ADD(SHEET_MANNAN, &SPR_Mannan, 3,3,4, 0,0, 1,0, 2,0);
			SHEET_ADD(SHEET_PCRITTER, &SPR_CritBig, 6,3,3, 0,0, 1,0, 2,0, 3,0, 3,1, 3,2);
			SHEET_ADD(SHEET_FROG, &SPR_Frog, 3,3,4, 0,0, 0,1, 1,0);
		} /* Below is for both Grasstown and Waterway */
		case 0x30: // Waterway
		{	SHEET_ADD(SHEET_JELLY, &SPR_Jelly, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
			SHEET_ADD(SHEET_BAT, &SPR_Bat, 6,2,2, 0,0, 0,1, 0,2, 1,0, 2,0, 3,0);
			SHEET_ADD(SHEET_CRITTER, &SPR_CritHG, 6,2,2, 0,0, 1,0, 2,0, 3,0, 3,1, 3,2);
		} /* And a death trap */
		case 0x07: // Santa's House
		{	SHEET_ADD(SHEET_TRAP, &SPR_Trap, 1,4,3, 0,0);
		} break;
		case 0x1C: // Gum
		{	SHEET_ADD(SHEET_REDSHOT, &SPR_FrogShot, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_FROG, &SPR_Frog, 3,3,4, 0,0, 0,1, 1,0);
			SHEET_ADD(SHEET_PUCHI, &SPR_FrogSm, 2,2,2, 0,0, 1,0);
			SHEET_ADD(SHEET_FAN, &SPR_Fan, 3,2,2, 3,0, 3,1, 3,2);
		} break;
		case 0x0A: // Sand Zone
		case 0x25:
		{	SHEET_ADD(SHEET_CROW, &SPR_Crow, 3,4,4, 0,0, 0,1, 2,0);
			SHEET_ADD(SHEET_TRAP, &SPR_Trap, 1,4,3, 0,0);
			// Skullhead
			// Omega Projectile
		} break;
		case 0x09: // Labyrinth I
		case 0x26: // Labyrinth H
		case 0x27: // Labyrinth W
		{	SHEET_ADD(SHEET_CRITTER, &SPR_CritterP, 6,2,2, 0,0, 1,0, 2,0, 3,0, 3,1, 3,2);
			// Gaudi (Walking / Flying, but not death frames)
			// Monster X Wheels
		} break;
		case 0x29: // Clinic Ruins
		{	// Pooh Black's Bubbles
		} break;
		case 0x2D: // Labyrinth M
		{	SHEET_ADD(SHEET_CRITTER, &SPR_CritterP, 6,2,2, 0,0, 1,0, 2,0, 3,0, 3,1, 3,2);
			// Armored Gaudi (no death frames)
			// Fuzz
		} break;
		case 0x2F: // Core
		{	// Terminal
			// Core back
			// Minicore front
			// Minicore back
			// Small projectile
			// Wisp projectile
		} break;
		case 0x31: // Egg Corridor?
		{	// Critter
			// Baby dragon
			// Falling spike
		} break;
		case 0x35: // Outer Wall
		{	// Projectile
		} break;
		case 0x38: // Plantation
		{	// Bat
			// Sideways Press
		} break;
		case 0x3F: // Last Cave
		case 0x43:
		{	// Orange Bat
			// Orange Critter
			// Proximity Press?
		} break;
		case 0x40: // Throne Room
		{	// Bat
			// Black Orb
			// Lightning
		} break;
		case 0x41: // King's Table
		{
			// Red Bat
			// Projectile
			// Cage
		} break;
		case 0x44: // Black Space
		{
			// Orange Bat
			// Orange Critter
			// Projectile
		} break;
		default: printf("Stage %hu has no sheet set", sid);
	}
	// Consider the item menu clobbers sheets and do not use tile allocs in that area
	tiloc_index = max(sheets[sheet_num-1].index + sheets[sheet_num-1].size, 
			TILE_SHEETINDEX + 24*6);
}
