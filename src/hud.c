#include "hud.h"

#include <genesis.h>
#include "player.h"
#include "resources.h"
#include "tables.h"
#include "vdp_ext.h"
#include "sprite.h"

#define TSIZE 8
#define SPR_TILE(x, y) (((x)*4)+(y))

VDPSprite sprHUD[2];
u32 tileData[8][8];

// Values used to draw parts of the HUD
// If the originator's value changes that part of the HUD will be updated
u8 hudMaxHealth, hudHealth;
u8 hudWeapon, hudMaxAmmo, hudAmmo;
u8 hudLevel, hudMaxEnergy, hudEnergy;

u8 showing = FALSE;

void hud_refresh_health();
void hud_refresh_energy();
void hud_refresh_weapon();
void hud_refresh_ammo();
void hud_refresh_maxammo();

// Expected to happen while the screen is off
void hud_create() {
	// Invalidate all values, forces a redraw
	hudMaxHealth = hudHealth = hudWeapon = hudLevel = 
			hudMaxEnergy = hudEnergy = hudMaxAmmo = hudAmmo = 255;
	// Create the sprites
	sprHUD[0] = (VDPSprite) {
		.x = 16 + 128,
		.y = 8 + 128,
		.size = SPRITE_SIZE(4, 4),
		.attribut = TILE_ATTR_FULL(PAL0,1,0,0,TILE_HUDINDEX)
	};
	sprHUD[1] = (VDPSprite) {
		.x = 16 + 32 + 128,
		.y = 8 + 128,
		.size = SPRITE_SIZE(4, 4),
		.attribut = TILE_ATTR_FULL(PAL0,1,0,0,TILE_HUDINDEX+16)
	};
}

void hud_show() {
	showing = TRUE;
}

void hud_hide() {
	showing = FALSE;
}

void hud_update() {
	if(!showing) return;
	sprite_addq(sprHUD, 2);
	// Only refresh one part of the HUD in a single frame, at most 8 tiles will be sent
	if(hudMaxHealth != playerMaxHealth || hudHealth != player.health) {
		hud_refresh_health();
	} else if(hudWeapon != playerWeapon[currentWeapon].type) {
		hud_refresh_weapon();
	} else if(hudLevel != playerWeapon[currentWeapon].level ||
			hudEnergy != playerWeapon[currentWeapon].energy) {
		hud_refresh_energy();
	} else if(hudMaxAmmo != playerWeapon[currentWeapon].maxammo) {
		// Max ammo changed refresh both
		hud_refresh_maxammo();
		hud_refresh_ammo();
	} else if(hudAmmo != playerWeapon[currentWeapon].ammo ) {
		hud_refresh_ammo();
	}
}

void hud_refresh_health() {
	// Redraw health if it changed
	hudMaxHealth = playerMaxHealth;
	hudHealth = player.health;
	s16 fillHP = 40 * hudHealth / hudMaxHealth;
	for(u8 i = 0; i < 5; i++) {
		// The TS_HudBar tileset has two rows of 8 tiles, where the section of the
		// bar is empty at tile 0 and full at tile 7
		s16 addrHP = min(fillHP*TSIZE, 7*TSIZE);
		if(addrHP < 0) addrHP = 0;
		// Fill in the bar
		memcpy(tileData[i+3], &TS_HudBar.tiles[addrHP], TILE_SIZE);
		fillHP -= 8;
	}
	// Heart icon and two digits displaying current health
	memcpy(tileData[0], &SPR_TILES(&SPR_Hud2, 0, 0)[3*TSIZE], TILE_SIZE);
	memcpy(tileData[1], &TS_Numbers.tiles[(hudHealth / 10)*TSIZE], TILE_SIZE);
	memcpy(tileData[2], &TS_Numbers.tiles[(hudHealth % 10)*TSIZE], TILE_SIZE);
	// Queue DMA transfer for health display
	DMA_queueDma(DMA_VRAM, (u32)tileData[0], 
			(TILE_HUDINDEX+3)*TILE_SIZE, 8*(TSIZE/2), 4*TSIZE);
}

void hud_refresh_energy() {
	// Energy or level changed
	hudLevel = playerWeapon[currentWeapon].level;
	hudMaxEnergy = weapon_info[playerWeapon[currentWeapon].type].experience[hudLevel-1];
	hudEnergy = playerWeapon[currentWeapon].energy;
	// Same deal as HP with the bar
	s16 fillXP = 40 * hudEnergy / hudMaxEnergy;
	for(u8 i = 0; i < 5; i++) {
		s16 addrXP = min(fillXP*TSIZE, 7*TSIZE);
		if(addrXP < 0) addrXP = 0;
		memcpy(tileData[i+3], &TS_HudBar.tiles[addrXP + 8*TSIZE], TILE_SIZE);
		fillXP -= 8;
	}
	// "Lv." and 1 digit for the level
	memcpy(tileData[0], &SPR_TILES(&SPR_Hud2, 0, 0)[2*TSIZE], TILE_SIZE);
	memcpy(tileData[1], &SPR_TILES(&SPR_Hud2, 0, 0)[6*TSIZE], TILE_SIZE);
	memcpy(tileData[2], &TS_Numbers.tiles[hudLevel*TSIZE], TILE_SIZE);
	// Queue DMA transfer for level/energy display
	DMA_queueDma(DMA_VRAM, (u32)tileData[0], 
			(TILE_HUDINDEX+2)*TILE_SIZE, 8*(TSIZE/2), 4*TSIZE);
}

void hud_refresh_weapon() {
	// Weapon switched
	hudWeapon = playerWeapon[currentWeapon].type;
	memcpy(tileData[0], SPR_TILES(&SPR_ArmsImage, 0, hudWeapon), TILE_SIZE*2);
	memcpy(tileData[2], &SPR_TILES(&SPR_ArmsImage, 0, hudWeapon)[TSIZE*2], TILE_SIZE*2);
	// Queue DMA transfer for level/energy display
	DMA_queueDma(DMA_VRAM, (u32)tileData[0], 
			(TILE_HUDINDEX)*TILE_SIZE, 2*(TSIZE/2), TILE_SIZE);
	// Queue DMA transfer for level/energy display
	DMA_queueDma(DMA_VRAM, (u32)tileData[2], 
			(TILE_HUDINDEX+1)*TILE_SIZE, 2*(TSIZE/2), TILE_SIZE);
}

void hud_refresh_ammo() {
	// Top half of ammo display
	hudAmmo = playerWeapon[currentWeapon].ammo;
	if(hudMaxAmmo > 0) {
		memcpy(tileData[0], TILE_BLANK, TILE_SIZE);
		if(hudAmmo >= 100) {
			memcpy(tileData[1], &TS_Numbers.tiles[1*TSIZE], TILE_SIZE);
		} else {
			memcpy(tileData[1], TILE_BLANK, TILE_SIZE);
		}
		memcpy(tileData[2], &TS_Numbers.tiles[((hudAmmo / 10) % 10)*TSIZE], TILE_SIZE);
		memcpy(tileData[3], &TS_Numbers.tiles[(hudAmmo % 10)*TSIZE], TILE_SIZE);
	} else { // Weapon doesn't use ammo
		memcpy(tileData[0], TILE_BLANK, TILE_SIZE);
		memcpy(tileData[1], TILE_BLANK, TILE_SIZE);
		memcpy(tileData[2], &SPR_TILES(&SPR_Hud2,0,0)[SPR_TILE(6, 0)*TSIZE], TILE_SIZE*2);
	}
	// Queue DMA transfer for ammo
	DMA_queueDma(DMA_VRAM, (u32)tileData[0], 
			(TILE_HUDINDEX+16)*TILE_SIZE, 4*(TSIZE/2), 4*TSIZE);
}

void hud_refresh_maxammo() {
	// Bottom half of ammo display
	hudMaxAmmo = playerWeapon[currentWeapon].maxammo;
	if(hudMaxAmmo > 0) {
		memcpy(tileData[4], &SPR_TILES(&SPR_Hud2,0,0)[SPR_TILE(4, 0)*TSIZE], TILE_SIZE*2);
		if(hudMaxAmmo >= 100) {
			memcpy(tileData[5], &TS_Numbers.tiles[1*TSIZE], TILE_SIZE);
		} else {
			memcpy(tileData[5], TILE_BLANK, TILE_SIZE);
		}
		memcpy(tileData[6], &TS_Numbers.tiles[((hudMaxAmmo / 10) % 10)*TSIZE], TILE_SIZE);
		memcpy(tileData[7], &TS_Numbers.tiles[(hudMaxAmmo % 10)*TSIZE], TILE_SIZE);
	} else { // Weapon doesn't use ammo
		memcpy(tileData[4], &SPR_TILES(&SPR_Hud2,0,0)[SPR_TILE(4, 1)*TSIZE], TILE_SIZE*4);
	}
	// Queue DMA transfer for max ammo
	DMA_queueDma(DMA_VRAM, (u32)tileData[4], 
			(TILE_HUDINDEX+17)*TILE_SIZE, 4*(TSIZE/2), 4*TSIZE);
}
