#include "sheet.h"

#include "player.h"
#include "resources.h"
#include "stage.h"
#include "tables.h"

#ifndef KDB_SHEET
#define puts(x) /**/
#define printf(...) /**/
#endif

void sheets_refresh_polarstar(u8 level) {
	const SpriteDefinition *def;
	if(level == 1) def = &SPR_PolarB1;
	else if(level == 2) def = &SPR_PolarB2;
	else if(level == 3) def = &SPR_PolarB3;
	else return;
	VDP_loadTileData(SPR_TILES(def, 0, 0), sheets[0].index, 4, 1);
	VDP_loadTileData(SPR_TILES(def, 1, 0), sheets[0].index + 4, 4, 1);
}

void sheets_refresh_fireball(u8 level) {
	const SpriteDefinition *def;
	if(level == 1) def = &SPR_FirebB1;
	else if(level == 2) def = &SPR_FirebB1;
	else if(level == 3) def = &SPR_FirebB3;
	else return;
	VDP_loadTileData(SPR_TILES(def, 0, 0), sheets[1].index, 4, 1);
	VDP_loadTileData(SPR_TILES(def, 0, 1), sheets[1].index + 4, 4, 1);
	VDP_loadTileData(SPR_TILES(def, 0, 2), sheets[1].index + 8, 4, 1);
}

void sheets_refresh_machinegun(u8 level) {
	const SpriteDefinition *def;
	if(level == 1) def = &SPR_MGunB1;
	else if(level == 2) def = &SPR_MGunB2;
	else if(level == 3) def = &SPR_MGunB3;
	else return;
	VDP_loadTileData(SPR_TILES(def, 0, 0), sheets[2].index, 4, 1);
	VDP_loadTileData(SPR_TILES(def, 1, 0), sheets[2].index + 4, 4, 1);
}

void sheets_refresh_missile(u8 level) {
	const SpriteDefinition *def;
	if(level == 1) def = &SPR_MisslB1;
	else if(level == 2) def = &SPR_MisslB2;
	else if(level == 3) def = &SPR_MisslB2;
	else return;
	VDP_loadTileData(SPR_TILES(def, 0, 0), sheets[3].index, 4, 1);
}

void sheets_refresh_weapons() {
	Weapon *pstar = player_find_weapon(WEAPON_POLARSTAR);
	Weapon *fball = player_find_weapon(WEAPON_FIREBALL);
	Weapon *mgun =  player_find_weapon(WEAPON_MACHINEGUN);
	Weapon *bubb =  player_find_weapon(WEAPON_BUBBLER);
	Weapon *blade = player_find_weapon(WEAPON_BLADE);
	Weapon *snake = player_find_weapon(WEAPON_SNAKE);
	Weapon *nemes = player_find_weapon(WEAPON_NEMESIS);
	Weapon *spur =  player_find_weapon(WEAPON_SPUR);
	if(pstar) {
		sheets_refresh_polarstar(pstar->level);
	} else if(spur) {
		sheets_refresh_polarstar(spur->level);
	} else if(snake) {
		sheets_refresh_polarstar(snake->level);
	}
	if(fball) {
		sheets_refresh_fireball(fball->level);
	}
	if(mgun) {
		sheets_refresh_machinegun(mgun->level);
	}
	if(bubb) {
		sheets_refresh_polarstar(bubb->level);
	}
	if(blade) {
		sheets_refresh_polarstar(blade->level);
	} else if(nemes) {
		sheets_refresh_polarstar(nemes->level);
	}
	// Missiles
	Weapon *missl = player_find_weapon(WEAPON_MISSILE);
	if(!missl) missl = player_find_weapon(WEAPON_SUPERMISSILE);
	if(missl) sheets_refresh_missile(missl->level);
}

void sheets_load_stage(u16 sid, u8 init_base, u8 init_tiloc) {
	// Reset values
	if(init_base) {
		sheet_num = 0;
		memset(sheets, 0, sizeof(Sheet) * MAX_SHEETS);
		SHEET_ADD(SHEET_PSTAR,   &SPR_PolarB1, 2,2,2, 0,0, 1,0);
		SHEET_ADD(SHEET_FBALL,   &SPR_FirebB1, 3,2,2, 0,0, 0,1, 0,2);
		SHEET_ADD(SHEET_MGUN,    &SPR_MGunB1,  2,2,2, 0,0, 1,0);
		SHEET_ADD(SHEET_MISSL,   &SPR_MisslB1, 1,2,2, 0,0);
		sheets_refresh_weapons();
		SHEET_ADD(SHEET_HEART,   &SPR_Heart,   4,2,2, 0,0, 0,1, 1,0, 1,1);
		SHEET_ADD(SHEET_MISSILE, &SPR_MisslP,  4,2,2, 0,0, 0,1, 1,0, 1,1);
		SHEET_ADD(SHEET_ENERGY,  &SPR_EnergyS, 6,1,1, 0,0, 0,1, 0,2, 0,3, 0,4, 0,5);
		SHEET_ADD(SHEET_ENERGYL, &SPR_EnergyL, 6,2,2, 0,0, 0,1, 0,2, 0,3, 0,4, 0,5);
	} else {
		sheet_num = 8;
	}
	if(init_tiloc) {
		memset(tilocs, 0, MAX_TILOCS);
	}
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
		case 0x19: // Malco
		{
			SHEET_ADD(SHEET_IGORSHOT, &SPR_EnergyShot, 2,2,2, 0,0, 0,1);
		} break;
		case 0x1C: // Gum
		{	SHEET_ADD(SHEET_REDSHOT, &SPR_FrogShot, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_FROG, &SPR_Frog, 3,3,4, 0,0, 0,1, 1,0);
			SHEET_ADD(SHEET_PUCHI, &SPR_FrogSm, 3,2,2, 0,0, 0,0, 1,0);
			SHEET_ADD(SHEET_FAN, &SPR_Fan, 3,2,2, 3,0, 3,1, 3,2);
		} break;
		case 0x0A: // Sand Zone
		case 0x25:
		{	SHEET_ADD(SHEET_CROW, &SPR_Crow, 3,4,4, 0,0, 0,1, 2,0);
			SHEET_ADD(SHEET_TRAP, &SPR_Trap, 1,4,3, 0,0);
			SHEET_ADD(SHEET_BEETLE, &SPR_BtlHB, 2,2,2, 0,0, 1,0);
			SHEET_ADD(SHEET_POLISH, &SPR_Polish, 2,4,4, 0,0, 0,1);
			SHEET_ADD(SHEET_BABY, &SPR_Baby, 2,2,2, 0,0, 0,1);
			SHEET_ADD(SHEET_OMGSHOT, &SPR_OmgShot, 4,2,2, 0,0, 0,1, 1,0, 1,1);
			SHEET_ADD(SHEET_SKULLH, &SPR_Skullhead, 3,4,3, 0,0, 1,0, 2,0);
			SHEET_ADD(SHEET_OMGLEG, &SPR_OmegaLeg, 2,4,4, 0,0, 1,0);
		} break;
		case 0x1D: // Sand Zone Bar
		{	SHEET_ADD(SHEET_BARMIMI, &SPR_CurlyMimi, 8,2,2, 
					0,0, 0,1, 0,2, 0,3, 0,4, 0,5, 0,6, 0,7);
			SHEET_ADD(SHEET_CURLYB, &SPR_CurlyB, 5,4,3, 0,0, 1,0, 1,2, 2,0, 4,0);
		} break;
		case 0x09: // Labyrinth I
		{	SHEET_ADD(SHEET_TERM, &SPR_Term, 2,2,3, 0,0, 0,1);
		}
		case 0x26: // Labyrinth H
		case 0x27: // Labyrinth W
		{	SHEET_ADD(SHEET_CRITTER, &SPR_CritterP, 6,2,2, 0,0, 1,0, 2,0, 3,0, 3,1, 3,2);
			SHEET_ADD(SHEET_LABSHOT, &SPR_LabShot, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_GAUDI, &SPR_Gaudi, 10,3,3, 
					0,0, 1,0, 1,2, 2,0, 7,0, 8,0, 9,0, 10,0, 10,1, 11,0);
			if(stageID == 0x27) {
				SHEET_ADD(SHEET_XTREAD, &SPR_XTread, 4,8,4, 0,0, 1,0, 2,0, 3,0);
				SHEET_ADD(SHEET_XBODY, &SPR_XBody, 1,4,4, 0,0);
				SHEET_ADD(SHEET_XTARGET, &SPR_XTarget, 8,3,3, 
						0,0, 0,1, 1,0, 1,1, 2,0, 2,1, 3,0, 3,1);
				SHEET_ADD(SHEET_XFISHY, &SPR_XFishy, 8,2,2,
						0,0, 0,1, 0,2, 0,3, 0,4, 0,5, 0,6, 0,7);
			}
		} break;
		case 0x29: // Clinic Ruins
		{	SHEET_ADD(SHEET_DARKBUB, &SPR_DarkBub, 2,2,2, 0,0, 1,0);
		} break;
		case 0x2D: // Labyrinth M
		{	SHEET_ADD(SHEET_CRITTER, &SPR_CritterP, 6,2,2, 0,0, 1,0, 2,0, 3,0, 3,1, 3,2);
			SHEET_ADD(SHEET_GAUDI, &SPR_GaudiArmor, 7,3,3, 
					0,0, 1,0, 2,0, 3,0, 4,0, 5,0, 6,0);
			SHEET_ADD(SHEET_FUZZ, &SPR_Fuzz, 1,2,2, 0,0);
			SHEET_ADD(SHEET_FUZZC, &SPR_FuzzCore, 1,4,4, 0,0);
			SHEET_ADD(SHEET_FIREWSHOT, &SPR_FireWShot, 3,2,4, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_LABSHOT, &SPR_LabShot, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_GAUDISHOT, &SPR_GaudiShot, 3,2,2, 0,0, 0,1, 0,2);
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
	if(init_tiloc) {
		tiloc_index = max(sheets[sheet_num-1].index + sheets[sheet_num-1].size, 
			TILE_SHEETINDEX + 24*6);
	}
}
