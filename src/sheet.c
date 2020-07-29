#include "common.h"

#include "dma.h"
#include "entity.h"
#include "error.h"
#include "memory.h"
#include "player.h"
#include "resources.h"
#include "stage.h"
#include "string.h"
#include "system.h"
#include "tables.h"
#include "vdp.h"
#include "weapon.h"

#include "sheet.h"

#define WEP_FIND_SHEET(fsheet) { \
	uint8_t sht = NOSHEET; \
	SHEET_FIND(sht, fsheet); \
	if(sht != NOSHEET) { \
		w->sheet = sht; \
		return; \
	} \
}

void sheets_load_weapon(Weapon *w) {
	if(!w) return;
	w->sheet = sheet_num;
	switch(w->type) {
		case WEAPON_POLARSTAR:
		WEP_FIND_SHEET(SHEET_PSTAR);
		SHEET_ADD(SHEET_PSTAR, w->level == 1 ? &SPR_PolarB1 
							 : w->level == 2 ? &SPR_PolarB2 
							 : &SPR_PolarB3, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
		break;
		case WEAPON_SPUR:
		{	// Need to load both Polar Star Lv 3 and Spur
			uint8_t pstar_sheet = NOSHEET;
			SHEET_FIND(pstar_sheet, SHEET_PSTAR);
			if(pstar_sheet == NOSHEET) {
				SHEET_ADD(SHEET_PSTAR, &SPR_PolarB3, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
			}
		}
		WEP_FIND_SHEET(SHEET_SPUR);
		SHEET_ADD(SHEET_SPUR, w->level == 1 ? &SPR_SpurB1 
							: w->level == 2 ? &SPR_SpurB2
							: &SPR_SpurB3, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
		w->sheet = sheet_num - 1;
		break;
		case WEAPON_SNAKE:
		WEP_FIND_SHEET(SHEET_SNAKE);
		SHEET_ADD(SHEET_SNAKE, w->level == 1 ? &SPR_FirebB1 
							 : w->level == 2 ? &SPR_FirebB2 
							 : &SPR_FirebB3, 4,2,2, 0,0, 0,1, 0,2, 0,3);
		break;
		case WEAPON_FIREBALL:
		WEP_FIND_SHEET(SHEET_FBALL);
		SHEET_ADD(SHEET_FBALL, w->level == 1 ? &SPR_FirebB1 
							 : w->level == 2 ? &SPR_FirebB2 
							 : &SPR_FirebB3, 4,2,2, 0,0, 0,1, 0,2, 0,3);
		break;
		case WEAPON_MACHINEGUN:
		WEP_FIND_SHEET(SHEET_MGUN);
		SHEET_ADD(SHEET_MGUN, w->level == 1 ? &SPR_MGunB1 
							: w->level == 2 ? &SPR_MGunB2 
							: &SPR_MGunB3, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
		break;
		case WEAPON_BUBBLER:
		WEP_FIND_SHEET(SHEET_BUBB);
		SHEET_ADD(SHEET_BUBB, w->level == 1 ? &SPR_BubB1 
							: w->level == 2 ? &SPR_BubB2 
							: &SPR_BubB3, 6,1,1, 0,0, 0,1, 0,2, 0,3, 0,4, 0,5);
		break;
		case WEAPON_BLADE:
		WEP_FIND_SHEET(SHEET_BLADE);
		SHEET_ADD(SHEET_BLADE, w->level == 1 ? &SPR_BladeB1 
							 : w->level == 2 ? &SPR_BladeB2 
							 : &SPR_BladeB3k, 1,3,3, 0,0);
		SHEET_LOAD(&SPR_BladeB3s, 2,4, TILE_SLASHINDEX, 1);
		break;
		case WEAPON_NEMESIS:
		WEP_FIND_SHEET(SHEET_NEMES);
		SHEET_ADD(SHEET_NEMES, w->level == 1 ? &SPR_NemB1h 
							 : w->level == 2 ? &SPR_NemB2h 
							 : &SPR_NemB3h, 2,3,2, 0,0, 0,1);
		SHEET_LOAD(w->level == 1 ? &SPR_NemB1v 
				 : w->level == 2 ? &SPR_NemB2v 
				 : &SPR_NemB3v, 2,6, TILE_NEMINDEX, 1);
		break;
		case WEAPON_MISSILE:
		WEP_FIND_SHEET(SHEET_MISSL);
		SHEET_ADD(SHEET_MISSL, w->level == 1 ? &SPR_MisslB1 
							 : w->level == 2 ? &SPR_MisslB2 
							 : &SPR_MisslB1, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_SUPERMISSILE:
		WEP_FIND_SHEET(SHEET_MISSL);
		SHEET_ADD(SHEET_MISSL, w->level == 1 ? &SPR_MisslB1 
							 : w->level == 2 ? &SPR_MisslB2 
							 : &SPR_MisslB1, 2,2,2, 0,0, 1,0);
		break;
		default: w->sheet = NOSHEET;
	}
}

void sheets_refresh_weapon(Weapon *w) {
	if(!w) return;
	if(w->sheet == NOSHEET) {
		sheets_load_weapon(w);
		return;
	}
	switch(w->type) {
		case WEAPON_POLARSTAR:
		SHEET_MOD(SHEET_PSTAR, w->level == 1 ? &SPR_PolarB1 
							 : w->level == 2 ? &SPR_PolarB2 
							 : &SPR_PolarB3, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
		break;
		case WEAPON_SPUR:
		SHEET_MOD(SHEET_SPUR, w->level == 1 ? &SPR_SpurB1 
							: w->level == 2 ? &SPR_SpurB2
							: &SPR_SpurB3, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
		break;
		case WEAPON_SNAKE:
		SHEET_MOD(SHEET_SNAKE, w->level == 1 ? &SPR_FirebB1 
							 : w->level == 2 ? &SPR_FirebB2 
							 : &SPR_FirebB3, 4,2,2, 0,0, 0,1, 0,2, 0,3);
		break;
		case WEAPON_FIREBALL:
		SHEET_MOD(SHEET_FBALL, w->level == 1 ? &SPR_FirebB1 
							 : w->level == 2 ? &SPR_FirebB2 
							 : &SPR_FirebB3, 4,2,2, 0,0, 0,1, 0,2, 0,3);
		break;
		case WEAPON_MACHINEGUN:
		SHEET_MOD(SHEET_MGUN,  w->level == 1 ? &SPR_MGunB1 
							 : w->level == 2 ? &SPR_MGunB2 
							 : &SPR_MGunB3, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
		break;
		case WEAPON_BUBBLER:
		SHEET_MOD(SHEET_BUBB,  w->level == 1 ? &SPR_BubB1 
							 : w->level == 2 ? &SPR_BubB2 
							 : &SPR_BubB3, 6,1,1, 0,0, 0,1, 0,2, 0,3, 0,4, 0,5);
		break;
		case WEAPON_BLADE:
		SHEET_MOD(SHEET_BLADE, w->level == 1 ? &SPR_BladeB1 
							 : w->level == 2 ? &SPR_BladeB2 
							 : &SPR_BladeB3k, 1,3,3, 0,0);
		SHEET_LOAD(&SPR_BladeB3s, 2,4, TILE_SLASHINDEX, 1);
		break;
		case WEAPON_NEMESIS:
		SHEET_MOD(SHEET_NEMES, w->level == 1 ? &SPR_NemB1h 
							 : w->level == 2 ? &SPR_NemB2h 
							 : &SPR_NemB3h, 2,3,2, 0,0, 0,1);
		SHEET_LOAD(w->level == 1 ? &SPR_NemB1v 
				 : w->level == 2 ? &SPR_NemB2v 
				 : &SPR_NemB3v, 2,6, TILE_NEMINDEX, 1);
		break;
		case WEAPON_MISSILE:
		SHEET_MOD(SHEET_MISSL, w->level == 1 ? &SPR_MisslB1 
							 : w->level == 2 ? &SPR_MisslB2 
							 : &SPR_MisslB1, 2,2,2, 0,0, 1,0);
		break;
		case WEAPON_SUPERMISSILE:
		SHEET_MOD(SHEET_MISSL, w->level == 1 ? &SPR_MisslB1 
							 : w->level == 2 ? &SPR_MisslB2 
							 : &SPR_MisslB1, 2,2,2, 0,0, 1,0);
		break;
	}
}

void sheets_load_stage(uint16_t sid, uint8_t init_base, uint8_t init_tiloc) {
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
		memset(&sheets[4], 0, sizeof(Sheet) * (MAX_SHEETS - 4));
	}
	if(init_tiloc) {
		memset(tilocs, 0, MAX_TILOCS);
	}
	switch(sid) {
		case 0x0C: // First Cave
		{	SHEET_ADD(SHEET_BAT, &SPR_Bat, 6,2,2, 0,0, 0,1, 0,2, 1,0, 2,0, 3,0);
			SHEET_ADD(SHEET_CRITTER, &SPR_CritHB, 3,2,2, 0,0, 1,0, 2,0);
		} break;
		case 0x5A: // Gunsmith
		{	
			if(!player_has_weapon(WEAPON_SPUR) && (playerEquipment & EQUIP_BOOSTER20)) {
				Weapon w = (Weapon) { .type = WEAPON_SPUR, .level = 1 };
				sheets_load_weapon(&w);
			} //else if(!player_has_weapon(WEAPON_POLARSTAR)) {
			//	Weapon w = (Weapon) { .type = WEAPON_POLARSTAR, .level = 1 };
			//	sheets_load_weapon(&w);
			//}
			SHEET_ADD(SHEET_ZZZ, &SPR_Zzz, 4,1,1, 0,0, 0,1, 0,2, 0,3);
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
					0,0, 1,0, 0,1, 1,1, 0,2, 1,2, 0,3, 1,3, 0,4, 1,4);
		} break;
		case 0x02: // Egg Corridor
		{	SHEET_ADD(SHEET_CRITTER, &SPR_CritHG, 3,2,2, 0,0, 1,0, 2,0);
			SHEET_ADD(SHEET_BEETLE, &SPR_BtlHG, 2,2,2, 0,0, 1,0);
			SHEET_ADD(SHEET_BEHEM, &SPR_Behem, 6,4,3, 0,0, 0,1, 0,3, 1,0, 2,0, 2,1);
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
			SHEET_ADD(SHEET_JELLY, &SPR_Jelly, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
			SHEET_ADD(SHEET_BAT, &SPR_BlackBat, 6,2,2, 0,2, 0,3, 0,4, 0,1, 0,2, 0,5);
			SHEET_ADD(SHEET_CRITTER, &SPR_CritHG, 6,2,2, 0,0, 1,0, 2,0, 3,0, 3,1, 3,2);
			SHEET_ADD(SHEET_TRAP, &SPR_Trap, 1,4,3, 0,0);
		} break;
		case 0x07: // Santa's House
		{	SHEET_ADD(SHEET_TRAP, &SPR_Trap, 1,4,3, 0,0);
		} break;
		case 0x19: // Malco
		{	SHEET_ADD(SHEET_POWERS, &SPR_PowerScreen, 2,2,2, 0,0, 0,1);
			SHEET_ADD(SHEET_POWERF, &SPR_Wave, 4,2,2, 0,0, 0,1, 0,2, 0,3);
			SHEET_ADD(SHEET_IGORSHOT, &SPR_EnergyShot, 2,2,2, 0,0, 0,1);
		} break;
		case 0x1C: // Gum
		{	SHEET_ADD(SHEET_REDSHOT, &SPR_FrogShot, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_FROG, &SPR_Frog, 3,3,4, 0,0, 0,1, 1,0);
			SHEET_ADD(SHEET_PUCHI, &SPR_FrogSm, 3,2,2, 0,0, 0,0, 1,0);
			SHEET_ADD(SHEET_FAN, &SPR_Fan, 3,2,2, 3,0, 3,1, 3,2);
			SHEET_ADD(SHEET_FROGFEET, &SPR_Balfrog2, 2,3,3, 0,0, 0,1);
		} break;
		case 0x0A: // Sand Zone
		case 0x25:
		{	SHEET_ADD(SHEET_CROW, &SPR_Crow, 3,4,4, 0,0, 0,1, 2,0);
			SHEET_ADD(SHEET_TRAP, &SPR_Trap, 1,4,3, 0,0);
			SHEET_ADD(SHEET_BEETLE, &SPR_BtlHB, 2,2,2, 0,0, 1,0);
			SHEET_ADD(SHEET_POLISH, &SPR_Polish, 2,4,4, 0,0, 0,1);
			SHEET_ADD(SHEET_BABY, &SPR_Baby, 2,2,2, 0,0, 0,1);
			SHEET_ADD(SHEET_OMGSHOT, &SPR_OmgShot, 4,2,2, 0,0, 0,1, 1,0, 1,1);
			SHEET_ADD(SHEET_SKULLH, &SPR_Skullhead, 2,4,3, 1,0, 2,0);
			SHEET_ADD(SHEET_OMGLEG, &SPR_OmegaLeg, 2,4,4, 0,0, 1,0);
		} break;
		case 0x1D: // Sand Zone Bar
		{	SHEET_ADD(SHEET_BARMIMI, &SPR_CurlyMimi, 9,2,2, 
					0,0, 0,1, 0,2, 0,3, 0,4, 0,5, 0,6, 0,7, 0,8);
			SHEET_ADD(SHEET_CURLYB, &SPR_CurlyB, 5,4,3, 0,0, 1,0, 1,2, 2,0, 4,0);
		} break;
		case 0x23: // Sand Zone Storehouse
		{	SHEET_ADD(SHEET_BLOCK, &SPR_ToroBlock, 2,2,2, 0,0, 0,1);
			SHEET_ADD(SHEET_FLOWER, &SPR_ToroFlower, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
		} break;
		case 0x09: // Labyrinth I
		{	SHEET_ADD(SHEET_TERM, &SPR_Term, 2,2,3, 0,0, 0,1);
		} /* fallthrough */
		case 0x26: // Labyrinth H
		case 0x27: // Labyrinth W
		{	SHEET_ADD(SHEET_CRITTER, &SPR_CritterP, 6,2,2, 0,0, 1,0, 2,0, 3,0, 3,1, 3,2);
			SHEET_ADD(SHEET_LABSHOT, &SPR_LabShot, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_GAUDI, &SPR_Gaudi, 10,3,3, 
					0,0, 1,0, 1,2, 2,0, 7,0, 8,0, 9,0, 10,0, 10,1, 11,0);
			if(stageID == 0x27) {
				SHEET_ADD(SHEET_XTREAD, &SPR_XTread, 4,8,4, 0,0, 1,0, 0,1, 1,1);
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
			if(!player_has_weapon(WEAPON_SNAKE)) {
				Weapon w = (Weapon) { .type = WEAPON_SNAKE, .level = 1 };
				sheets_load_weapon(&w);
			}
		} break;
		case 0x29: // Clinic Ruins
		{	SHEET_ADD(SHEET_DARKBUB, &SPR_DarkBub, 2,2,2, 0,0, 1,0);
		} break;
		case STAGE_LABYRINTH_S:
		{	SHEET_ADD(SHEET_BLGMISL, &SPR_MisslB2, 1,2,2, 0,0);
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
			SHEET_ADD(SHEET_BUYOB, &SPR_BuyoBase, 3,4,4, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_BUYO, &SPR_Buyo, 2,2,2, 0,0, 0,1);
			// Curly's gun
			if(!player_has_weapon(WEAPON_MACHINEGUN)) {
				SHEET_ADD(SHEET_CGUN, &SPR_MGun, 1,3,1, 0,0);
				SHEET_ADD(SHEET_MGUN, &SPR_MGunB3, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
			} else {
				SHEET_ADD(SHEET_CGUN, &SPR_Polar, 1,3,1, 0,0);
				SHEET_ADD(SHEET_PSTAR, &SPR_PolarB3, 2,2,2, 0,0, 0,1);
			}
		} break;
		case 0x2E: // Dark Place
		{	
		} break;
		case 0x2F: // Core
		{	SHEET_ADD(SHEET_TERM, &SPR_Term, 2,2,3, 0,0, 0,1);
			SHEET_ADD(SHEET_CORES1, &SPR_CoreShot1, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_CORES3, &SPR_CoreShot2, 3,3,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_CORES4, &SPR_CoreShot4, 1,4,4, 0,0);
			// Curly's gun
			if(!player_has_weapon(WEAPON_MACHINEGUN)) {
				SHEET_ADD(SHEET_CGUN, &SPR_MGun, 1,3,1, 0,0);
				SHEET_ADD(SHEET_MGUN, &SPR_MGunB3, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
			} else {
				SHEET_ADD(SHEET_CGUN, &SPR_Polar, 1,3,1, 0,0);
				SHEET_ADD(SHEET_PSTAR, &SPR_PolarB3, 2,2,2, 0,0, 0,1);
			}
		} break;
		case 0x30: // Waterway
		{	SHEET_ADD(SHEET_JELLY, &SPR_Jelly, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
			SHEET_ADD(SHEET_BAT, &SPR_BlackBat, 6,2,2, 0,2, 0,3, 0,4, 0,1, 0,2, 0,5);
			SHEET_ADD(SHEET_CRITTER, &SPR_CritHG, 6,2,2, 0,0, 1,0, 2,0, 3,0, 3,1, 3,2);
		} break;
		case 0x1F: // Main Artery
		{	SHEET_ADD(SHEET_IKACHAN, &SPR_IkaChan, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_IRONHBLK, &SPR_IronhBlk, 2,4,4, 0,0, 0,1);
			SHEET_ADD(SHEET_BLOWFISH, &SPR_Blowfish, 4,2,2, 0,0, 0,1, 0,2, 0,3);
		} break;
		case 0x31: // Egg Corridor?
		{	SHEET_ADD(SHEET_CRITTER, &SPR_CritAqua, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_BEETLE, &SPR_Btl2, 2,2,2, 0,0, 0,1);
			SHEET_ADD(SHEET_BASU, &SPR_Basu2, 3,3,3, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_BASUSHOT, &SPR_BasuShot, 2,2,2, 0,0, 1,0);
			SHEET_ADD(SHEET_SMSTAL, &SPR_SmStal, 2,2,2, 0,0, 0,1);
			SHEET_ADD(SHEET_LGSTAL, &SPR_LgStal, 2,2,4, 0,0, 0,1);
			SHEET_ADD(SHEET_FIRE, &SPR_DraShot, 3,2,3, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_FAN, &SPR_Fan, 3,2,2, 3,0, 3,1, 3,2);
		} break;
		case 0x33: // Egg Observatory?
		{	SHEET_ADD(SHEET_SISHEAD, &SPR_SisHead, 5,4,4, 0,0, 0,1, 0,2, 0,3, 0,4);
			SHEET_ADD(SHEET_FIRE, &SPR_DraShot, 3,2,3, 0,0, 0,1, 0,2);
		} break;
		case 0x35: // Outer Wall
		{	SHEET_ADD(SHEET_NIGHTSHOT, &SPR_NightShot, 3,4,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_HOPPY, &SPR_Hoppy, 4,2,2, 0,0, 0,1, 0,2, 0,3);
		} break;
		case 0x38: // Plantation
		{	SHEET_ADD(SHEET_BAT, &SPR_BlackBat, 4,2,2, 0,2, 0,3, 0,4, 0,5);
			SHEET_ADD(SHEET_MIDO, &SPR_Midorin, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_GUNFSHOT, &SPR_BubRed, 2,1,1, 0,0, 0,1);
			SHEET_ADD(SHEET_PRESS, &SPR_PressH, 3,3,2, 0,0, 1,0, 2,0);
			SHEET_ADD(SHEET_STUMPY, &SPR_Stumpy, 2,2,2, 0,0, 0,1);
			SHEET_ADD(SHEET_DROP, &SPR_Drop, 1,1,1, 0,0);
			SHEET_ADD(SHEET_TRAP, &SPR_Trap, 1,4,3, 0,0);
		} break;
		case STAGE_LAST_CAVE_1:
		{	SHEET_ADD(SHEET_FAN, &SPR_Fan, 3,2,2, 3,0, 3,1, 3,2);
		} /* fallthrough */
		case STAGE_LAST_CAVE_2:
		{	SHEET_ADD(SHEET_BAT, &SPR_BatRed, 6,2,2, 0,2, 0,3, 0,4, 0,0, 0,1, 0,5);
			SHEET_ADD(SHEET_CRITTER, &SPR_CritRed, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_FFIELD, &SPR_ForceField, 4,2,2, 0,0, 0,1, 0,2, 0,3);
			SHEET_ADD(SHEET_ACID, &SPR_DripRed, 4,1,2, 0,0, 0,1, 0,2, 0,3);
			SHEET_ADD(SHEET_PRESS, &SPR_Press, 3,2,3, 0,0, 0,1, 0,2);
		} break;
		case 0x3E: // Balcony
		{	SHEET_ADD(SHEET_IGORSHOT, &SPR_EnergyShot, 2,2,2, 0,0, 0,1);
		} break;
		case 0x40: // Throne Room
		{	SHEET_ADD(SHEET_RING, &SPR_MizaRing, 4,2,2, 0,0, 0,1, 0,2, 0,3);
			SHEET_ADD(SHEET_IGORSHOT, &SPR_MizaRing, 3,2,2, 0,4, 0,5, 0,6);
			SHEET_ADD(SHEET_SHOCK, &SPR_MizaShock, 2,2,4, 0,0, 0,1);
			SHEET_ADD(SHEET_BAT, &SPR_MizaBat, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_BLOCK, &SPR_Block, 1,4,4, 0,0);
			SHEET_ADD(SHEET_BLOCKM, &SPR_BlockM, 1,2,2, 0,0);
		} break;
		case 0x41: // King's Table
		{	SHEET_ADD(SHEET_BAT, &SPR_DocBat, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_REDDOT, &SPR_RedDot, 2,1,1, 0,0, 0,1);
			SHEET_ADD(SHEET_MIMI, &SPR_ShovelMimi, 3,2,2, 0,0, 0,1, 0,8);
			SHEET_ADD(SHEET_DOCSHOT, &SPR_DocShot, 4,2,2, 0,0, 0,1, 0,2, 0,3);
			SHEET_ADD(SHEET_CAGE, &SPR_JailBars3, 1,4,4, 0,0);
			SHEET_ADD(SHEET_CRYSTAL, &SPR_RedCrystal, 2,1,2, 0,0, 0,1);
		} break;
		case 0x44: // Black Space
		{	SHEET_ADD(SHEET_BAT, &SPR_MizaBat, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_CRITTER, &SPR_MizaCritter, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_MUCORE, &SPR_MUCoreFront, 4,4,4, 0,0, 0,1, 0,2, 0,3);
			SHEET_ADD(SHEET_MUCORE2, &SPR_MUCoreBack, 2,2,4, 0,0, 0,1);
			SHEET_ADD(SHEET_MUCORE3, &SPR_MUCoreBottom, 2,4,1, 0,0, 0,1);
			SHEET_ADD(SHEET_ROCK, &SPR_MizaRock, 5,3,3, 0,0, 0,1, 0,2, 0,3, 0,4);
			SHEET_ADD(SHEET_CORES4, &SPR_CoreShot4, 1,4,4, 0,0);
			SHEET_ADD(SHEET_CRYSTAL, &SPR_RedCrystal, 2,1,2, 0,0, 0,1);
			SHEET_ADD(SHEET_REDDOT, &SPR_RedDot, 2,1,1, 0,0, 0,1);
		} break;
		case 0x45: // Little House
		{	// Make sure both Blade and Nemesis sheets are loaded
			if(player_has_weapon(WEAPON_BLADE)) {
				Weapon w = (Weapon) { .type = WEAPON_NEMESIS, .level = 1 };
				sheets_load_weapon(&w);
			}
			if(player_has_weapon(WEAPON_NEMESIS)) {
				Weapon w = (Weapon) { .type = WEAPON_BLADE, .level = 1 };
				sheets_load_weapon(&w);
			}
		} break;
		case 0x46: // Balcony 2
		{	SHEET_ADD(SHEET_BLOCK, &SPR_Block, 1,4,4, 0,0);
			SHEET_ADD(SHEET_BLOCKM, &SPR_BlockM, 1,2,2, 0,0);
		} break;
		case STAGE_FALLING:
		case STAGE_ENDING_SKY:
		{	SHEET_LOAD(&SPR_Cloud1, 12, 16, TILE_CLOUDINDEX, TRUE, 0,0, 0,1, 0,2, 0,3, 0,4, 0,5, 1,0, 1,1, 1,2, 1,3, 1,4, 1,5);
			SHEET_LOAD(&SPR_Cloud2, 3, 16, TILE_CLOUD2INDEX, TRUE, 0,0, 0,1, 0,2);
			SHEET_LOAD(&SPR_Cloud3, 3, 9, TILE_CLOUD3INDEX, TRUE, 0,0, 0,1, 0,2);
			SHEET_LOAD(&SPR_Cloud4, 1, 8, TILE_CLOUD4INDEX, TRUE, 0,0);
		} break;
		case STAGE_HELL_B1:
		{	SHEET_ADD(SHEET_TRAP, &SPR_Trap, 1,4,3, 0,0);
		} break;
		case STAGE_HELL_B2:
		{	// Conveniently the palette indeces for Balcony and Hell blocks are the same
			SHEET_ADD(SHEET_BLOCK, &SPR_Block, 1,4,4, 0,0);
			SHEET_ADD(SHEET_BLOCKM, &SPR_BlockM, 1,2,2, 0,0);
			SHEET_ADD(SHEET_BUTE, &SPR_Bute, 16,3,2, 
					0,0, 0,1, 0,2, 0,3, 0,4, 0,5, 0,6, 0,7, 0,8, 
					0,9, 0,10, 0,11, 0,12, 0,13, 0,14, 0,15);
			SHEET_ADD(SHEET_BUTEDIE, &SPR_Bute, 3,3,2, 0,16, 0,17, 0,18);
		} break;
		case STAGE_HELL_B3:
		{	SHEET_ADD(SHEET_TRAP, &SPR_Trap, 1,4,3, 0,0);
			SHEET_ADD(SHEET_PRESS, &SPR_Press, 3,2,3, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_BUTE, &SPR_Bute, 16,3,2, 
					0,0, 0,1, 0,2, 0,3, 0,4, 0,5, 0,6, 0,7, 0,8, 
					0,9, 0,10, 0,11, 0,12, 0,13, 0,14, 0,15);
			SHEET_ADD(SHEET_BUTEDIE, &SPR_Bute, 3,3,2, 0,16, 0,17, 0,18);
			SHEET_ADD(SHEET_BUTEARW, &SPR_ButeArw, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
			SHEET_ADD(SHEET_ROLLING, &SPR_Rolling, 3,2,2, 0,0, 0,1, 0,2);
			SHEET_ADD(SHEET_DELEET, &SPR_Deleet, 3,3,3, 0,1, 0,2, 0,0);
			SHEET_ADD(SHEET_HPLIT, &SPR_HeavyPressL, 3,4,4, 0,0, 0,1, 0,2);
			if(player_has_weapon(WEAPON_BLADE)) {
				Weapon w = (Weapon) { .type = WEAPON_NEMESIS, .level = 1 };
				sheets_load_weapon(&w);
			}
		} break;
		case STAGE_MA_PIGNON:
		{	// Makes copies of itself, better to have a common sheet
			SHEET_ADD(SHEET_MAPI, &SPR_MaPignon, 12,2,2, 
					0,0, 0,1, 0,2, 0,3, 0,4, 0,5, 0,6, 0,7, 0,8, 0,9, 0,10, 0,11);
			SHEET_ADD(SHEET_ROCK, &SPR_Rock, 3,2,2, 0,0, 0,1, 0,2);
		} break;
		case STAGE_HELL_PASSAGEWAY_2:
		case STAGE_HELL_STATUE:
		case STAGE_HELL_OUTER_PASSAGE:
		{
			if(player_has_weapon(WEAPON_BLADE)) {
				Weapon w = (Weapon) { .type = WEAPON_NEMESIS, .level = 1 };
				sheets_load_weapon(&w);
			}
		} break;
		case STAGE_SEAL_CHAMBER:
		{	SHEET_ADD(SHEET_BONE, &SPR_Bone, 4,2,2, 0,0, 0,1, 0,2, 0,3);
			SHEET_ADD(SHEET_DEVIL, &SPR_GrnDevil, 2,2,2, 0,0, 0,1);
			SHEET_ADD(SHEET_PLATF, &SPR_BallosPlat, 1,4,2, 0,0);
			SHEET_ADD(SHEET_ROT, &SPR_BallosRot, 2,4,4, 0,0, 0,1);
			SHEET_ADD(SHEET_TARGET, &SPR_Target, 1,3,3, 0,0);
			if(player_has_weapon(WEAPON_BLADE)) {
				Weapon w = (Weapon) { .type = WEAPON_NEMESIS, .level = 1 };
				sheets_load_weapon(&w);
			}
			SHEET_ADD(SHEET_BUTE, &SPR_ButeRed, 6,3,2, 0,0, 0,1, 0,2, 0,3, 0,4, 0,5);
			SHEET_ADD(SHEET_BUTEDIE, &SPR_ButeRed2, 4,2,2, 0,0, 0,1, 0,2, 0,3);
			SHEET_ADD(SHEET_BUTEARW, &SPR_ButeArw, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
		} break;
		case STAGE_ENDING_LABYRINTH:
		{
			SHEET_ADD(SHEET_GAUDI, &SPR_GaudiEnd, 4,3,2, 0,0, 0,1, 0,2, 0,3);
		} break;
		case STAGE_ENDING_LAB: 
		{	SHEET_ADD(SHEET_AHCHOO, cfg_language ? &SPR_AhchooJ : &SPR_AhchooE, 2,2,2, 0,0, 0,1);
		} break;
	}
	// Weapons at the end
	for(uint8_t i = 0; i < MAX_WEAPONS; i++) sheets_load_weapon(&playerWeapon[i]);
	// Special case for Sand Zone Bar because Curly uses the machine gun
	if(sid == 0x1D) {
		uint8_t msheet = NOSHEET;
		SHEET_FIND(msheet, SHEET_MGUN);
		if(msheet == NOSHEET) {
			SHEET_ADD(SHEET_MGUN, &SPR_MGunB1, 5,2,2, 0,0, 0,1, 0,2, 0,3, 0,4);
		}
	}
	// Consider the item menu clobbers sheets and do not use tile allocs in that area
	if(init_tiloc) {
		tiloc_index = max(sheets[sheet_num-1].index + sheets[sheet_num-1].size, 
			TILE_SHEETINDEX + MAX_ITEMS*6);
	}
}

void sheets_load_splash() {
	sheet_num = 0;
	memset(sheets, 0, sizeof(Sheet) * MAX_SHEETS);
	memset(tilocs, 0, MAX_TILOCS);
	tiloc_index = TILE_USERINDEX;
}

void sheets_load_intro() {
	sheet_num = 0;
	memset(sheets, 0, sizeof(Sheet) * MAX_SHEETS);
	memset(tilocs, 0, MAX_TILOCS);
	tiloc_index = TILE_USERINDEX;
}
