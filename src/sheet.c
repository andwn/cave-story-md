#include "sheet.h"

#include <genesis.h>
#include "stage.h"
#include "resources.h"
#include "tables.h"
#include "player.h"

#ifndef KDB_SHEET
#define puts(x) /**/
#define printf(...) /**/
#endif

void sheets_init() {
	// Polar star
	sheets[0] = (Sheet){ SHEET_PSTAR, 2*4, TILE_SHEETINDEX };
	// Machine Gun
	sheets[1] = (Sheet){ SHEET_MGUN, 2*4, sheets[0].index + sheets[0].size };
	// Fireball
	sheets[2] = (Sheet){ SHEET_FBALL, 3*4, sheets[1].index + sheets[1].size };
	// Heart
	sheets[3] = (Sheet){ SHEET_HEART, 4*4, sheets[2].index + sheets[2].size };
	// Missile
	sheets[4] = (Sheet){ SHEET_MISSILE, 4*4, sheets[3].index + sheets[3].size };
	// Small Energy
	sheets[5] = (Sheet){ SHEET_ENERGY, 6*1, sheets[4].index + sheets[4].size };
	// Large Energy
	sheets[6] = (Sheet){ SHEET_ENERGYL, 6*4, sheets[5].index + sheets[5].size };
	// The rest are blank
	for(u8 i = 7; i < 10; i++) sheets[i] = (Sheet) {};
	// Actually load the tiles - assume the VDP is disabled
	// TODO: Move these to player_init
	Weapon *pstar = player_find_weapon(WEAPON_POLARSTAR);
	Weapon *mgun = player_find_weapon(WEAPON_MACHINEGUN);
	Weapon *fball = player_find_weapon(WEAPON_FIREBALL);
	sheets_refresh_polarstar(pstar != NULL ? pstar->level : 1);
	sheets_refresh_machinegun(mgun != NULL ? mgun->level : 1);
	sheets_refresh_fireball(fball != NULL ? fball->level : 1);
	// Heart
	VDP_loadTileData(SPR_TILESET(&SPR_Heart,0,0)->tiles, sheets[3].index, 4, true);
	VDP_loadTileData(SPR_TILESET(&SPR_Heart,0,1)->tiles, sheets[3].index + 4, 4, true);
	VDP_loadTileData(SPR_TILESET(&SPR_Heart,1,0)->tiles, sheets[3].index + 8, 4, true);
	VDP_loadTileData(SPR_TILESET(&SPR_Heart,1,1)->tiles, sheets[3].index + 12, 4, true);
	// Missile
	VDP_loadTileData(SPR_TILESET(&SPR_MisslP,0,0)->tiles, sheets[4].index, 4, true);
	VDP_loadTileData(SPR_TILESET(&SPR_MisslP,0,1)->tiles, sheets[4].index + 4, 4, true);
	VDP_loadTileData(SPR_TILESET(&SPR_MisslP,1,0)->tiles, sheets[4].index + 8, 4, true);
	VDP_loadTileData(SPR_TILESET(&SPR_MisslP,1,1)->tiles, sheets[4].index + 12, 4, true);
	// Small Energy
	VDP_loadTileData(SPR_TILESET(&SPR_EnergyS,0,0)->tiles, sheets[5].index, 1, false);
	VDP_loadTileData(SPR_TILESET(&SPR_EnergyS,0,1)->tiles, sheets[5].index + 1, 1, false);
	VDP_loadTileData(SPR_TILESET(&SPR_EnergyS,0,2)->tiles, sheets[5].index + 2, 1, false);
	VDP_loadTileData(SPR_TILESET(&SPR_EnergyS,0,3)->tiles, sheets[5].index + 3, 1, false);
	VDP_loadTileData(SPR_TILESET(&SPR_EnergyS,0,4)->tiles, sheets[5].index + 4, 1, false);
	VDP_loadTileData(SPR_TILESET(&SPR_EnergyS,0,5)->tiles, sheets[5].index + 5, 1, false);
	// Large Energy
	VDP_loadTileData(SPR_TILESET(&SPR_EnergyL,0,0)->tiles, sheets[6].index, 4, false);
	VDP_loadTileData(SPR_TILESET(&SPR_EnergyL,0,1)->tiles, sheets[6].index + 4, 4, false);
	VDP_loadTileData(SPR_TILESET(&SPR_EnergyL,0,2)->tiles, sheets[6].index + 8, 4, false);
	VDP_loadTileData(SPR_TILESET(&SPR_EnergyL,0,3)->tiles, sheets[6].index + 12, 4, false);
	VDP_loadTileData(SPR_TILESET(&SPR_EnergyL,0,4)->tiles, sheets[6].index + 16, 4, false);
	VDP_loadTileData(SPR_TILESET(&SPR_EnergyL,0,5)->tiles, sheets[6].index + 20, 4, false);
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
	VDP_loadTileData(SPR_TILESET(def, 0, 0)->tiles, sheets[0].index, 4, true);
	VDP_loadTileData(SPR_TILESET(def, 1, 0)->tiles, sheets[0].index + 4, 4, true);
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
	VDP_loadTileData(SPR_TILESET(def, 0, 0)->tiles, sheets[1].index, 4, true);
	VDP_loadTileData(SPR_TILESET(def, 1, 0)->tiles, sheets[1].index + 4, 4, true);
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
	VDP_loadTileData(SPR_TILESET(def, 0, 0)->tiles, sheets[2].index, 4, true);
	VDP_loadTileData(SPR_TILESET(def, 0, 1)->tiles, sheets[2].index + 4, 4, true);
	VDP_loadTileData(SPR_TILESET(def, 0, 2)->tiles, sheets[2].index + 8, 4, true);
}

void sheets_load_stage(u16 sid) {
	switch(sid) {
		case 0x0C: // First Cave
		// Bat
		sheets[7] = (Sheet){ SHEET_BAT, 6*4, sheets[6].index + sheets[6].size };
		VDP_loadTileData(SPR_TILESET(&SPR_Bat,0,0)->tiles,sheets[7].index,   4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Bat,0,1)->tiles,sheets[7].index+4, 4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Bat,0,2)->tiles,sheets[7].index+8, 4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Bat,1,0)->tiles,sheets[7].index+12,4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Bat,2,0)->tiles,sheets[7].index+16,4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Bat,3,0)->tiles,sheets[7].index+20,4,true);
		// Cave Critter
		sheets[8] = (Sheet){ SHEET_CRITTER, 3*4, sheets[7].index + sheets[7].size };
		VDP_loadTileData(SPR_TILESET(&SPR_CritHB,0,0)->tiles,sheets[8].index,   4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_CritHB,1,0)->tiles,sheets[8].index+4, 4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_CritHB,2,0)->tiles,sheets[8].index+8, 4,true);
		// Nothing
		sheets[9] = (Sheet) {};
		break;
		case 0x10: // Graveyard
		// Pignon
		sheets[7] = (Sheet){ SHEET_PIGNON, 5*4, sheets[6].index + sheets[6].size };
		VDP_loadTileData(SPR_TILESET(&SPR_Pignon,0,0)->tiles,sheets[7].index,   4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Pignon,1,0)->tiles,sheets[7].index+4, 4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Pignon,1,2)->tiles,sheets[7].index+8, 4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Pignon,2,0)->tiles,sheets[7].index+12,4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Pignon,3,0)->tiles,sheets[7].index+16,4,true);
		// Nothing
		sheets[8] = (Sheet) {};
		sheets[9] = (Sheet) {};
		break;
		case 0x06: // Grasstown
		case 0x30: // Waterway
		// Jelly
		sheets[7] = (Sheet){ SHEET_JELLY, 5*4, sheets[6].index + sheets[6].size };
		VDP_loadTileData(SPR_TILESET(&SPR_Jelly,0,0)->tiles,sheets[7].index,   4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Jelly,0,1)->tiles,sheets[7].index+4, 4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Jelly,0,2)->tiles,sheets[7].index+8, 4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Jelly,0,3)->tiles,sheets[7].index+12,4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Jelly,0,4)->tiles,sheets[7].index+16,4,true);
		// Bat
		sheets[8] = (Sheet){ SHEET_BAT, 6*4, sheets[7].index + sheets[7].size };
		VDP_loadTileData(SPR_TILESET(&SPR_Bat,0,0)->tiles,sheets[8].index,   4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Bat,0,1)->tiles,sheets[8].index+4, 4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Bat,0,2)->tiles,sheets[8].index+8, 4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Bat,1,0)->tiles,sheets[8].index+12,4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Bat,2,0)->tiles,sheets[8].index+16,4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Bat,3,0)->tiles,sheets[8].index+20,4,true);
		// Green Critter
		sheets[9] = (Sheet){ SHEET_CRITTER, 6*4, sheets[8].index + sheets[8].size };
		VDP_loadTileData(SPR_TILESET(&SPR_CritHG,0,0)->tiles,sheets[9].index,   4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_CritHG,1,0)->tiles,sheets[9].index+4, 4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_CritHG,2,0)->tiles,sheets[9].index+8, 4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_CritHG,3,0)->tiles,sheets[9].index+12,4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_CritHG,3,1)->tiles,sheets[9].index+16,4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_CritHG,3,2)->tiles,sheets[9].index+20,4,true);
		break;
		case 0x1C: // Gum
		// Balfrog jumping
		sheets[7] = (Sheet){ SHEET_BALFROG, 9*11, sheets[6].index + sheets[6].size };
		VDP_loadTileData(SPR_TILESET(&SPR_Balfrog2,0,0)->tiles,sheets[7].index,9*11,true);
		// Nothing
		sheets[8] = (Sheet) {};
		sheets[9] = (Sheet) {};
		break;
		case 0x0A: // Sand Zone
		case 0x25:
		// Crow
		sheets[7] = (Sheet){ SHEET_CROW, 3*16, sheets[6].index + sheets[6].size };
		VDP_loadTileData(SPR_TILESET(&SPR_Crow,0,0)->tiles,sheets[7].index,   4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Crow,0,1)->tiles,sheets[7].index+16,4,true);
		VDP_loadTileData(SPR_TILESET(&SPR_Crow,2,0)->tiles,sheets[7].index+32,4,true);
		// Nothing
		sheets[8] = (Sheet) {};
		sheets[9] = (Sheet) {};
		break;
		default:
		printf("Stage %hu has no sheet set", sid);
		// Nothing
		sheets[7] = (Sheet) {};
		sheets[8] = (Sheet) {};
		sheets[9] = (Sheet) {};
	}
}
