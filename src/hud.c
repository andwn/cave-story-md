#include "hud.h"

#include <genesis.h>
#include "player.h"
#include "resources.h"
#include "tables.h"
#include "vdp_ext.h"

#define HEALTH_DECREASE_TIME 5

#define TSIZE 8

// Sprite tiles are stored top to bottom THEN left to right
#define SPR_TILE(x, y) (((x)*4)+(y))

// Values used to draw parts of the HUD
// If the originator's value changes that part of the HUD will be updated
u8 hudMaxHealth, hudHealth, hudHealthTime;
u8 hudWeapon, hudMaxAmmo, hudAmmo;
u8 hudLevel, hudMaxEnergy, hudEnergy;

// HUD Sprite
Sprite *sprHUD = NULL;

// DMA tile data
u32 tileData[32][TSIZE];

// Keeps track of whether the HUD is loaded or not to avoid double loads/frees
bool showing = false;

void hud_redraw_health();
void hud_decrease_health();
void hud_redraw_weapon();
void hud_redraw_ammo();
void hud_prepare_dma();

void hud_create() {
	// Health
	hudMaxHealth = playerMaxHealth;
	hudHealth = player.health;
	hudHealthTime = HEALTH_DECREASE_TIME;
	// Weapon
	hudWeapon = playerWeapon[currentWeapon].type;
	hudLevel = playerWeapon[currentWeapon].level;
	hudMaxEnergy = weapon_info[playerWeapon[currentWeapon].type].experience[hudLevel-1];
	hudEnergy = playerWeapon[currentWeapon].energy;
	// Ammo
	hudMaxAmmo = playerWeapon[currentWeapon].maxammo;
	hudAmmo = playerWeapon[currentWeapon].ammo;
	// Create HUD sprite, manual tile/visibility
	sprHUD = SPR_addSpriteEx(&SPR_Hud2, tile_to_pixel(2), tile_to_pixel(1),
		TILE_ATTR_FULL(PAL0, 1, 0, 0, TILE_HUDINDEX), 0, SPR_FLAG_AUTO_SPRITE_ALLOC);
	memcpy(tileData[0], SPR_TILESET(SPR_Hud2, 0, 0)->tiles, sizeof(u32) * TSIZE * 32);
	// Prepare DMA -- populate tileData
	hud_redraw_weapon();
	hud_prepare_dma();
}

void hud_show() {
	SPR_setVisibility(sprHUD, VISIBLE);
	showing = true;
}

void hud_hide() {
	SPR_setVisibility(sprHUD, HIDDEN);
	showing = false;
}

void hud_redraw_health() {
	hudMaxHealth = playerMaxHealth;
	hudHealth = player.health;
	hudRedrawPending = true;
}

void hud_decrease_health() {
	hudHealthTime--;
	if(hudHealthTime == 0) {
		hudHealth--;
		hudHealthTime = HEALTH_DECREASE_TIME;
	}
	hudRedrawPending = true;
}

void hud_redraw_weapon() {
	hudWeapon = playerWeapon[currentWeapon].type;
	hudLevel = playerWeapon[currentWeapon].level;
	hudMaxEnergy = weapon_info[playerWeapon[currentWeapon].type].experience[hudLevel-1];
	hudEnergy = playerWeapon[currentWeapon].energy;
	hudMaxAmmo = playerWeapon[currentWeapon].maxammo;
	hudAmmo = playerWeapon[currentWeapon].ammo;
	memcpy(tileData[SPR_TILE(1, 0)], 
		SPR_TILESET(SPR_ArmsImage, 0, hudWeapon)->tiles, sizeof(u32) * TSIZE * 2);
	memcpy(tileData[SPR_TILE(2, 0)], 
		&SPR_TILESET(SPR_ArmsImage, 0, hudWeapon)->tiles[TSIZE * 2], sizeof(u32) * TSIZE * 2);
	hudRedrawPending = true;
}

void hud_update() {
	if(!showing) return;
	// Redraw health if it changed
	if(hudMaxHealth < playerMaxHealth) {
		hud_redraw_health();
	} else if(hudHealth < player.health) {
		hud_redraw_health();
	} else if(hudHealth > player.health) {
		hud_decrease_health();
	}
	// Same with weapon and ammo
	if(hudWeapon != playerWeapon[currentWeapon].type ||
			hudLevel != playerWeapon[currentWeapon].level) {
		hud_redraw_weapon();
	} else if(hudEnergy != playerWeapon[currentWeapon].energy) {
		hudEnergy = playerWeapon[currentWeapon].energy;
		hudRedrawPending = true;
	} else if(hudAmmo != playerWeapon[currentWeapon].ammo ||
			hudMaxAmmo != playerWeapon[currentWeapon].maxammo) {
		hudMaxAmmo = playerWeapon[currentWeapon].maxammo;
		hudAmmo = playerWeapon[currentWeapon].ammo;
		hudRedrawPending = true;
	}
	if(hudRedrawPending) hud_prepare_dma();
}

void hud_update_vblank() {
	VDP_loadTileData(tileData[0], TILE_HUDINDEX, 32, true);
	hudRedrawPending = false;
}

void hud_prepare_dma() {
	// The bars are 40 pixels wide, 5 tiles of 8 pixels
	s16 fillHP, fillXP;
	fillHP = 40 * hudHealth / hudMaxHealth;
	if(hudMaxEnergy > 0) {
		fillXP = 40 * hudEnergy / hudMaxEnergy;
	} else {
		fillXP = 0;
	}
	for(u8 i = 0; i < 5; i++) {
		// The TS_HudBar tileset has two rows of 8 tiles, where the section of the
		// bar is empty at tile 0 and full at tile 7
		s16 addrHP = min(fillHP*TSIZE, 7*TSIZE);
		s16 addrXP = min(fillXP*TSIZE, 7*TSIZE);
		if(addrHP < 0) addrHP = 0;
		if(addrXP < 0) addrXP = 0;
		// Copy health/expbar tiles
		memcpy(tileData[SPR_TILE(i+3, 3)], &TS_HudBar.tiles[addrHP], sizeof(u32) * TSIZE);
		memcpy(tileData[SPR_TILE(i+3, 2)], &TS_HudBar.tiles[addrXP + 8*TSIZE], sizeof(u32) * TSIZE);
		fillHP -= 8;
		fillXP -= 8;
	}
	// Digit displaying current weapon's level
	memcpy(tileData[SPR_TILE(2, 2)], &TS_Numbers.tiles[hudLevel*TSIZE], sizeof(u32) * TSIZE);
	// Two digits displaying current health
	memcpy(tileData[SPR_TILE(1, 3)], &TS_Numbers.tiles[(hudHealth / 10)*TSIZE], sizeof(u32) * TSIZE);
	memcpy(tileData[SPR_TILE(2, 3)], &TS_Numbers.tiles[(hudHealth % 10)*TSIZE], sizeof(u32) * TSIZE);
	if(hudMaxAmmo > 0) { // Max and current ammo
		if(hudAmmo >= 100) {
			memcpy(tileData[SPR_TILE(5, 0)], &TS_Numbers.tiles[1*TSIZE], sizeof(u32) * TSIZE);
		} else {
			memcpy(tileData[SPR_TILE(5, 0)], TILE_BLANK, sizeof(u32) * TSIZE);
		}
		memcpy(tileData[SPR_TILE(6, 0)], &TS_Numbers.tiles[((hudAmmo / 10) % 10)*TSIZE], sizeof(u32) * TSIZE);
		memcpy(tileData[SPR_TILE(7, 0)], &TS_Numbers.tiles[(hudAmmo % 10)*TSIZE], sizeof(u32) * TSIZE);
		if(hudMaxAmmo == 100) {
			memcpy(tileData[SPR_TILE(5, 1)], &TS_Numbers.tiles[1*TSIZE], sizeof(u32) * TSIZE);
		} else {
			memcpy(tileData[SPR_TILE(5, 1)], TILE_BLANK, sizeof(u32) * TSIZE);
		}
		memcpy(tileData[SPR_TILE(6, 1)], &TS_Numbers.tiles[((hudMaxAmmo / 10) % 10)*TSIZE], sizeof(u32) * TSIZE);
		memcpy(tileData[SPR_TILE(7, 1)], &TS_Numbers.tiles[(hudMaxAmmo % 10)*TSIZE], sizeof(u32) * TSIZE);
	} else {
		TileSet *ts = SPR_TILESET(SPR_Hud2, 0, 0);
		memcpy(tileData[SPR_TILE(5, 0)], TILE_BLANK, sizeof(u32) * TSIZE);
		memcpy(tileData[SPR_TILE(5, 1)], TILE_BLANK, sizeof(u32) * TSIZE);
		memcpy(tileData[SPR_TILE(6, 0)], &ts->tiles[SPR_TILE(6, 0)*TSIZE], sizeof(u32) * TSIZE * 2);
		memcpy(tileData[SPR_TILE(7, 0)], &ts->tiles[SPR_TILE(7, 0)*TSIZE], sizeof(u32) * TSIZE * 2);
	}
}
