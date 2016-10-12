#include "sheet.h"

#include "player.h"
#include "resources.h"
#include "stage.h"
#include "tables.h"

#ifndef KDB_SHEET
#define puts(x) /**/
#define printf(...) /**/
#endif

void sheets_load_weapon(Weapon *w) {
	if(!w) return;
	w->sheet = sheet_num;
	switch(w->type) {
		case WEAPON_POLARSTAR:
		SHEET_ADD(SHEET_PSTAR, w->level == 1 ? &SPR_PolarB1 
							 : w->level == 2 ? &SPR_PolarB2 
							 : &SPR_PolarB3, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_SPUR:
		SHEET_ADD(SHEET_SPUR,  w->level == 1 ? &SPR_PolarB1 
							 : w->level == 2 ? &SPR_PolarB2 
							 : &SPR_PolarB3, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_SNAKE:
		SHEET_ADD(SHEET_SNAKE, w->level == 1 ? &SPR_PolarB1 
							 : w->level == 2 ? &SPR_PolarB2 
							 : &SPR_PolarB3, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_FIREBALL:
		SHEET_ADD(SHEET_FBALL, w->level == 1 ? &SPR_FirebB1 
							 : w->level == 2 ? &SPR_FirebB1 
							 : &SPR_FirebB3, 3,2,2, 0,0, 0,1, 0,2);
		break;
		case WEAPON_MACHINEGUN:
		SHEET_ADD(SHEET_MGUN,  w->level == 1 ? &SPR_MGunB1 
							 : w->level == 2 ? &SPR_MGunB2 
							 : &SPR_MGunB3, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_BUBBLER:
		SHEET_ADD(SHEET_BUBB,  w->level == 1 ? &SPR_PolarB1 
							 : w->level == 2 ? &SPR_PolarB2 
							 : &SPR_PolarB3, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_BLADE:
		SHEET_ADD(SHEET_BLADE, w->level == 1 ? &SPR_PolarB1 
							 : w->level == 2 ? &SPR_PolarB2 
							 : &SPR_PolarB3, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_NEMESIS:
		SHEET_ADD(SHEET_NEMES, w->level == 1 ? &SPR_PolarB1 
							 : w->level == 2 ? &SPR_PolarB2 
							 : &SPR_PolarB3, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_MISSILE:
		SHEET_ADD(SHEET_MISSL, w->level == 1 ? &SPR_MisslB1 
							 : w->level == 2 ? &SPR_MisslB2 
							 : &SPR_MisslB2, 1,2,2, 0,0);
		break;
		case WEAPON_SUPERMISSILE:
		SHEET_ADD(SHEET_MISSL, w->level == 1 ? &SPR_MisslB1 
							 : w->level == 2 ? &SPR_MisslB2 
							 : &SPR_MisslB2, 1,2,2, 0,0);
		break;
		default: w->sheet = NOSHEET;
	}
}

void sheets_refresh_weapon(Weapon *w) {
	if(!w) return;
	switch(w->type) {
		case WEAPON_POLARSTAR:
		SHEET_MOD(SHEET_PSTAR, w->level == 1 ? &SPR_PolarB1 
							 : w->level == 2 ? &SPR_PolarB2 
							 : &SPR_PolarB3, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_SPUR:
		SHEET_MOD(SHEET_SPUR,  w->level == 1 ? &SPR_PolarB1 
							 : w->level == 2 ? &SPR_PolarB2 
							 : &SPR_PolarB3, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_SNAKE:
		SHEET_MOD(SHEET_SNAKE, w->level == 1 ? &SPR_PolarB1 
							 : w->level == 2 ? &SPR_PolarB2 
							 : &SPR_PolarB3, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_FIREBALL:
		SHEET_MOD(SHEET_FBALL, w->level == 1 ? &SPR_FirebB1 
							 : w->level == 2 ? &SPR_FirebB1 
							 : &SPR_FirebB3, 3,2,2, 0,0, 0,1, 0,2);
		break;
		case WEAPON_MACHINEGUN:
		SHEET_MOD(SHEET_MGUN,  w->level == 1 ? &SPR_MGunB1 
							 : w->level == 2 ? &SPR_MGunB2 
							 : &SPR_MGunB3, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_BUBBLER:
		SHEET_MOD(SHEET_BUBB,  w->level == 1 ? &SPR_PolarB1 
							 : w->level == 2 ? &SPR_PolarB2 
							 : &SPR_PolarB3, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_BLADE:
		SHEET_MOD(SHEET_BLADE, w->level == 1 ? &SPR_PolarB1 
							 : w->level == 2 ? &SPR_PolarB2 
							 : &SPR_PolarB3, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_NEMESIS:
		SHEET_MOD(SHEET_NEMES, w->level == 1 ? &SPR_PolarB1 
							 : w->level == 2 ? &SPR_PolarB2 
							 : &SPR_PolarB3, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_MISSILE:
		SHEET_MOD(SHEET_MISSL, w->level == 1 ? &SPR_MisslB1 
							 : w->level == 2 ? &SPR_MisslB2 
							 : &SPR_MisslB2, 1,2,2, 0,0);
		break;
		case WEAPON_SUPERMISSILE:
		SHEET_MOD(SHEET_MISSL, w->level == 1 ? &SPR_MisslB1 
							 : w->level == 2 ? &SPR_MisslB2 
							 : &SPR_MisslB2, 1,2,2, 0,0);
		break;
	}
}

void sheets_load_stage(u16 sid, u8 init_base, u8 init_tiloc) {
	// Reset values
	if(init_base) {
		sheet_num = 0;
		memset(sheets, 0, sizeof(Sheet) * MAX_SHEETS);
		SHEET_ADD(SHEET_HEART,   &SPR_Heart,   4,2,2, 0,0, 0,1, 1,0, 1,1);
		SHEET_ADD(SHEET_MISSILE, &SPR_MisslP,  4,2,2, 0,0, 0,1, 1,0, 1,1);
		SHEET_ADD(SHEET_ENERGY,  &SPR_EnergyS, 6,1,1, 0,0, 0,1, 0,2, 0,3, 0,4, 0,5);
		SHEET_ADD(SHEET_ENERGYL, &SPR_EnergyL, 6,2,2, 0,0, 0,1, 0,2, 0,3, 0,4, 0,5);
	} else {
		sheet_num = 4;
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
			SHEET_ADD(SHEET_BASUSHOT, &SPR_BasuShot, 2,2,2, 0,0, 1,0);
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
				SHEET_ADD(SHEET_XTREAD, &SPR_XTread, 2,8,4, 0,0, 1,0 /*, 0,1, 1,1*/);
				SHEET_ADD(SHEET_XBODY, &SPR_XBody, 1,8,4, 0,0);
				SHEET_ADD(SHEET_XTARGET, &SPR_XTarget, 8,2,2, 
						0,0, 0,1, 0,2, 0,3, 1,0, 1,1, 1,2, 1,3);
				SHEET_ADD(SHEET_XFISHY, &SPR_XFishy, 8,2,2,
						0,0, 0,1, 0,2, 0,3, 0,4, 0,5, 0,6, 0,7);
				SHEET_ADD(SHEET_FFIELD, &SPR_ForceField, 4,2,2, 0,0, 0,1, 0,2, 0,3);
			}
		} break;
		case 0x2A: // Labyrinth Shop
		{	SHEET_ADD(SHEET_GAUDI, &SPR_Gaudi, 4,3,3, 0,0, 1,0, 1,2, 2,0);
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
			SHEET_ADD(SHEET_GAUDIEGG, &SPR_GaudiEgg, 2,3,3, 0,0, 1,0);
			SHEET_ADD(SHEET_TERM, &SPR_Term, 2,2,3, 0,0, 0,1);
			// Curly's gun
			if(!player_has_weapon(WEAPON_MACHINEGUN)) {
				SHEET_ADD(SHEET_CGUN, &SPR_MGun, 2,3,1, 0,0, 0,1);
				SHEET_ADD(SHEET_MGUN, &SPR_MGunB3, 2,2,2, 0,0, 1,0);
			} else {
				SHEET_ADD(SHEET_CGUN, &SPR_Polar, 2,3,1, 0,0, 0,1);
				SHEET_ADD(SHEET_PSTAR, &SPR_PolarB3, 2,2,2, 0,0, 1,0);
			}
		} break;
		case 0x2E: // Dark Place
		{	
		} break;
		case 0x2F: // Core
		{	SHEET_ADD(SHEET_TERM, &SPR_Term, 2,2,3, 0,0, 0,1);
			// Curly's gun
			if(!player_has_weapon(WEAPON_MACHINEGUN)) {
				SHEET_ADD(SHEET_CGUN, &SPR_MGun, 2,3,1, 0,0, 0,1);
				SHEET_ADD(SHEET_MGUN, &SPR_MGunB3, 2,2,2, 0,0, 1,0);
			} else {
				SHEET_ADD(SHEET_CGUN, &SPR_Polar, 2,3,1, 0,0, 0,1);
				SHEET_ADD(SHEET_PSTAR, &SPR_PolarB3, 2,2,2, 0,0, 1,0);
			}
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
	// Weapons at the end
	for(u8 i = 0; i < MAX_WEAPONS; i++) sheets_load_weapon(&playerWeapon[i]);
	// Special case for Sand Zone Bar because Curly uses the machine gun
	if(sid == 0x1D) {
		u8 msheet = NOSHEET;
		SHEET_FIND(msheet, SHEET_MGUN);
		if(msheet == NOSHEET) {
			SHEET_ADD(SHEET_MGUN, &SPR_MGunB1, 2,2,2, 0,0, 1,0);
		}
	}
	// Consider the item menu clobbers sheets and do not use tile allocs in that area
	if(init_tiloc) {
		tiloc_index = max(sheets[sheet_num-1].index + sheets[sheet_num-1].size, 
			TILE_SHEETINDEX + 24*6);
	}
}

void sheets_load_splash() {
	sheet_num = 0;
	memset(sheets, 0, sizeof(Sheet) * MAX_SHEETS);
	memset(tilocs, 0, MAX_TILOCS);
	tiloc_index = TILE_USERINDEX;
}
