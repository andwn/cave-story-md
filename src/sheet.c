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
	// The rest are blank
	for(u8 i = 7; i < 10; i++) sheets[i] = (Sheet) {};
	// Actually load the tiles - assume the VDP is disabled
	Weapon *pstar = player_find_weapon(WEAPON_POLARSTAR);
	Weapon *mgun =  player_find_weapon(WEAPON_MACHINEGUN);
	Weapon *fball = player_find_weapon(WEAPON_FIREBALL);
	sheets_refresh_polarstar(pstar ? pstar->level : 1);
	sheets_refresh_machinegun(mgun ? mgun->level : 1);
	sheets_refresh_fireball(fball ? fball->level : 1);
	// Power ups
	SHEET_LOAD(&SPR_Heart,  4,4, sheets[3].index, 1, 0,0, 0,1, 1,0, 1,1);
	SHEET_LOAD(&SPR_MisslP, 4,4, sheets[4].index, 1, 0,0, 0,1, 1,0, 1,1);
	SHEET_LOAD(&SPR_EnergyS,6,1, sheets[5].index, 1, 0,0, 0,1, 0,2, 0,3, 0,4, 0,5);
	SHEET_LOAD(&SPR_EnergyL,6,4, sheets[6].index, 1, 0,0, 0,1, 0,2, 0,3, 0,4, 0,5);
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

void sheets_load_stage(u16 sid) {
	// Reset values
	sheet_num = 7;
	memset(tilocs, 0, MAX_TILOCS);
	memset(&sheets[7], 0, sizeof(Sheet) * (MAX_SHEETS - 7));
	switch(sid) {
		case 0x0C: // First Cave
		{	// Bat
			SHEET_ADD(SHEET_BAT, 6, 2, 2);
			SHEET_LOAD(&SPR_Bat, 6,4, sheets[7].index, 1, 0,0, 0,1, 0,2, 1,0, 2,0, 3,0);
			// Cave Critter
			SHEET_ADD(SHEET_CRITTER, 3, 2, 2);
			SHEET_LOAD(&SPR_CritHB, 3,4, sheets[8].index, 1, 0,0, 1,0, 2,0);
		}
		break;
		case 0x10: // Graveyard
		{	// Pignon
			SHEET_ADD(SHEET_PIGNON, 5, 2, 2);
			SHEET_LOAD(&SPR_Pignon, 5,4, sheets[7].index, 1, 0,0, 1,0, 1,2, 2,0, 3,0);
			// Big Pignon
			SHEET_ADD(SHEET_PIGNONB, 5, 3, 3);
			SHEET_LOAD(&SPR_PignonB, 5,9, sheets[8].index, 1, 0,0, 1,0, 1,2, 2,0, 3,0);
		}
		break;
		case 0x11: // Yamashita Farm
		{	// Flowers
			SHEET_ADD(SHEET_FLOWER, 6, 2, 2);
			SHEET_LOAD(&SPR_Flower, 6,4, sheets[7].index, 1, 0,0, 0,1, 0,2, 0,3, 0,4, 0,5);
			// Water drop
			SHEET_ADD(SHEET_DROP, 1, 1, 1);
			SHEET_LOAD(&SPR_Drop, 1,1, sheets[8].index, 1, 0,0);
		}
		break;
		case 0x01: // Arthur's House
		{	// Teleport menu icons
			SHEET_ADD(SHEET_TELE, 10, 4, 2);
			SHEET_LOAD(&SPR_TeleMenu, 10,8, sheets[7].index, 1, 
					0,1, 1,1, 0,2, 1,2, 0,3, 1,3, 0,4, 1,4, 0,5, 1,5);
		}
		break;
		case 0x02: // Egg Corridor
		{	// Green Critter (no hover)
			SHEET_ADD(SHEET_CRITTER, 3, 2, 2);
			SHEET_LOAD(&SPR_CritHG, 3,4, sheets[7].index, 1, 0,0, 1,0, 2,0);
			// Beetle
			SHEET_ADD(SHEET_BEETLE, 2, 2, 2);
			SHEET_LOAD(&SPR_BtlHG, 2,4, sheets[8].index, 1, 0,0, 1,0);
			// Behemoth
			SHEET_ADD(SHEET_BEHEM, 3, 4, 3);
			SHEET_LOAD(&SPR_Behem, 3,12, sheets[9].index, 1, 0,0, 0,1, 0,3);
			// Basil
			SHEET_ADD(SHEET_BASIL, 2, 4, 2);
			SHEET_LOAD(&SPR_Basil, 2,8, sheets[10].index, 1, 0,0, 0,1);
			// Igor Shot
			SHEET_ADD(SHEET_IGORSHOT, 2, 2, 2);
			SHEET_LOAD(&SPR_EnergyShot, 2,4, sheets[11].index, 1, 0,0, 0,1);
		}
		break;
		case 0x05: // Egg Observatory
		case 0x17: // Egg no. 1
		{	// Terminal
			SHEET_ADD(SHEET_TERM, 4, 2, 3);
			SHEET_LOAD(&SPR_Term, 4,6, sheets[8].index, 1, 0,0, 0,1, 0,2, 0,3);
		}
		case 0x06: // Grasstown
		case 0x30: // Waterway
		{	// Jelly
			SHEET_ADD(SHEET_JELLY, 5, 2, 2);
			SHEET_LOAD(&SPR_Jelly, 5,4, sheets[7].index, 1, 0,0, 0,1, 0,2, 0,3, 0,4);
			// Bat
			SHEET_ADD(SHEET_BAT, 6, 2, 2);
			SHEET_LOAD(&SPR_Bat, 6,4, sheets[8].index, 1, 0,0, 0,1, 0,2, 1,0, 2,0, 3,0);
			// Green Critter
			SHEET_ADD(SHEET_CRITTER, 6, 2, 2);
			SHEET_LOAD(&SPR_CritHG, 6,4, sheets[9].index, 1, 0,0, 1,0, 2,0, 3,0, 3,1, 3,2);
			// Death trap
			SHEET_ADD(SHEET_TRAP, 1, 4, 3);
			SHEET_LOAD(&SPR_Trap, 1,12, sheets[10].index, 1, 0,0);
			// Fan
			SHEET_ADD(SHEET_FAN, 8, 2, 2);
			SHEET_LOAD(&SPR_Fan, 8,4, sheets[11].index, 1, 1,0, 1,1, 1,2, 1,3, 
					3,0, 3,1, 3,2, 3,3);
			// Mannan
			SHEET_ADD(SHEET_MANNAN, 3, 2, 4);
			SHEET_LOAD(&SPR_Mannan, 3,8, sheets[12].index, 1, 0,0, 1,0, 2,0);
			// Power Critter
			SHEET_ADD(SHEET_PCRITTER, 6, 3, 3);
			SHEET_LOAD(&SPR_CritBig, 6,9, sheets[13].index, 1, 0,0, 1,0, 2,0, 3,0, 3,1, 3,2);
		}
		break;
		case 0x1C: // Gum
		{	// Red Shot
			SHEET_ADD(SHEET_REDSHOT, 3, 2, 2);
			SHEET_LOAD(&SPR_FrogShot, 3,4, sheets[7].index, 1, 0,0, 0,1, 0,2);
			// Frog
			SHEET_ADD(SHEET_FROG, 3, 3, 4);
			SHEET_LOAD(&SPR_Frog, 3,12, sheets[8].index, 1, 0,0, 0,1, 1,0);
			// Puchi
			SHEET_ADD(SHEET_PUCHI, 2, 2, 2);
			SHEET_LOAD(&SPR_FrogSm, 2,4, sheets[9].index, 1, 0,0, 1,0);
			// Fan (vertical only)
			SHEET_ADD(SHEET_FAN, 4, 2, 2);
			SHEET_LOAD(&SPR_Fan, 4,4, sheets[10].index, 1, 1,0, 1,1, 1,2, 1,3);
		}
		break;
		case 0x0A: // Sand Zone
		case 0x25:
		{	// Crow
			SHEET_ADD(SHEET_CROW, 3, 4, 4);
			SHEET_LOAD(&SPR_Crow, 3,16, sheets[7].index, TRUE, 0,0, 0,1, 2,0);
			// Skullhead
			sheets[8] = (Sheet) {};
			// Omega Projectile
			sheets[9] = (Sheet) {};
		}
		break;
		case 0x09: // Labyrinth I
		case 0x26: // Labyrinth H
		case 0x27: // Labyrinth W
		{	// Purple Critter
			sheets[7] = (Sheet){ SHEET_CRITTER, 6*4, sheets[6].index + sheets[6].size };
			SHEET_LOAD(&SPR_CritterP, 6,4, sheets[7].index, TRUE, 0,0, 1,0, 2,0, 3,0, 3,1, 3,2);
			// Gaudi (Walking / Flying, but not death frames)
			sheets[8] = (Sheet) {};
			// Monster X Wheels
			sheets[9] = (Sheet) {};
		}
		break;
		case 0x29: // Clinic Ruins
		{	// Pooh Black's Bubbles
			sheets[7] = (Sheet) {};
			// Nothing
			sheets[8] = (Sheet) {};
			sheets[9] = (Sheet) {};
		}
		break;
		case 0x2D: // Labyrinth M
		{	// Purple Critter
			sheets[7] = (Sheet){ SHEET_CRITTER, 6*4, sheets[6].index + sheets[6].size };
			SHEET_LOAD(&SPR_CritterP, 6,4, sheets[7].index, TRUE, 0,0, 1,0, 2,0, 3,0, 3,1, 3,2);
			// Armored Gaudi (no death frames)
			sheets[8] = (Sheet) {};
			// Fuzz
			sheets[9] = (Sheet) {};
		}
		break;
		case 0x2F: // Core
		{	// Terminal
			// Core back
			// Minicore front
			// Minicore back
			// Small projectile
			// Wisp projectile
		}
		break;
		case 0x31: // Egg Corridor?
		{	// Critter
			// Baby dragon
			// Falling spike
		}
		break;
		case 0x35: // Outer Wall
		{	// Projectile
		}
		break;
		case 0x38: // Plantation
		{	// Bat
			// Sideways Press
		}
		break;
		case 0x3F: // Last Cave
		case 0x43:
		{	// Orange Bat
			// Orange Critter
			// Proximity Press?
		}
		break;
		case 0x40: // Throne Room
		{	// Bat
			// Black Orb
			// Lightning
		}
		break;
		case 0x41: // King's Table
		{
			// Red Bat
			// Projectile
			// Cage
		}
		break;
		case 0x44: // Black Space
		{
			// Orange Bat
			// Orange Critter
			// Projectile
		}
		break;
		default: printf("Stage %hu has no sheet set", sid);
	}
	// Consider the item menu clobbers sheets and do not use tile allocs in that area
	tiloc_index = max(sheets[sheet_num-1].index + sheets[sheet_num-1].size, 
			TILE_SHEETINDEX + 24*6);
}
