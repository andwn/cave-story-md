#include "common.h"

#include "dma.h"
#include "entity.h"
#include "gamemode.h"
#include "memory.h"
#include "player.h"
#include "resources.h"
#include "system.h"
#include "tables.h"
#include "vdp.h"
#include "weapon.h"

#include "hud.h"
#include "xgm.h"

#define TSIZE 8
#define SPR_TILE(x, y) (((x)*4)+(y))

#define HP_BAR  0
#define XP_BAR  8
#define WPN     16
#define AMMO    20

VDPSprite sprHUD[2];
uint32_t tileData[28][8];

// Values used to draw parts of the HUD
// If the originator's value changes that part of the HUD will be updated
uint8_t hudMaxHealth, hudHealth;
uint8_t hudWeapon, hudMaxAmmo, hudAmmo;
uint8_t hudLevel, hudMaxEnergy, hudEnergy;

// Used for bar animation
uint8_t hudEnergyPixel, hudEnergyTimer, hudEnergyDest;

uint8_t showing = FALSE;

static const uint8_t EnergyPixel[4][42] = {
        { 0, 4, 8,12,16,20,24,28,32,36,40 },
        { 0, 2, 4, 6, 8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40 },
        { 0, 1, 3, 4, 6, 7, 9,10,11,12,14,15,17,18,19,20,22,23,25,26,
         27,29,30,31,33,34,36,37,38,39,40 },
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,
         20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40 },
};

void hud_refresh_health();
void hud_refresh_energy(uint8_t hard);
void hud_refresh_weapon();
void hud_refresh_ammo();
void hud_refresh_maxammo();

// Expected to happen while the screen is off
void hud_create() {
	// Invalidate all values, forces a redraw
	hudMaxHealth = hudHealth = hudWeapon = hudLevel = hudMaxAmmo = hudAmmo = 255;
    hudEnergy = hudMaxEnergy = 10;
	hudEnergyPixel = hudEnergyTimer = hudEnergyDest = 0;
	// Create the sprites
	sprHUD[0] = (VDPSprite) {
		.x = 16 + 128,
		.y = (pal_mode ? 24 : 16) + 128,
		.size = SPRITE_SIZE(4, 4),
		.attr = TILE_ATTR(PAL0,1,0,0,TILE_HUDINDEX)
	};
	sprHUD[1] = (VDPSprite) {
		.x = 16 + 32 + 128,
		.y = (pal_mode ? 24 : 16) + 128,
		.size = SPRITE_SIZE(4, 4),
		.attr = TILE_ATTR(PAL0,1,0,0,TILE_HUDINDEX+16)
	};
	// Draw blank tiles next to weapon
	DMA_doDma(DMA_VRAM, (uint32_t)TILE_BLANK, (TILE_HUDINDEX+8)*TILE_SIZE, 16, 2);
	DMA_doDma(DMA_VRAM, (uint32_t)TILE_BLANK, (TILE_HUDINDEX+9)*TILE_SIZE, 16, 2);
	DMA_doDma(DMA_VRAM, (uint32_t)TILE_BLANK, (TILE_HUDINDEX+12)*TILE_SIZE, 16, 2);
	DMA_doDma(DMA_VRAM, (uint32_t)TILE_BLANK, (TILE_HUDINDEX+13)*TILE_SIZE, 16, 2);
}

void hud_force_redraw() {
	hud_refresh_health();
    hud_refresh_weapon();
    hud_refresh_energy(TRUE);
	hud_refresh_maxammo();
	hud_refresh_ammo();
    // Draw blank tiles next to weapon
    DMA_queueDma(DMA_VRAM, (uint32_t)TILE_BLANK, (TILE_HUDINDEX+8)*TILE_SIZE, 16, 2);
    DMA_queueDma(DMA_VRAM, (uint32_t)TILE_BLANK, (TILE_HUDINDEX+9)*TILE_SIZE, 16, 2);
    DMA_queueDma(DMA_VRAM, (uint32_t)TILE_BLANK, (TILE_HUDINDEX+12)*TILE_SIZE, 16, 2);
    DMA_queueDma(DMA_VRAM, (uint32_t)TILE_BLANK, (TILE_HUDINDEX+13)*TILE_SIZE, 16, 2);

    disable_ints;
    z80_request();
	DMA_flushQueue();
    z80_release();
    enable_ints;
}

void hud_force_energy() {
	hudLevel = 9;
}

void hud_show() {
	showing = TRUE;
}

void hud_hide() {
	showing = FALSE;
}

void hud_update() {
	//if(paused) return;
	if(!showing) return;
	vdp_sprites_add(sprHUD, 2);
	// Only refresh one part of the HUD in a single frame, at most 8 tiles will be sent
	if(hudMaxHealth != playerMaxHealth || hudHealth != player.health) {
		hud_refresh_health();
	}
    if(hudWeapon != playerWeapon[currentWeapon].type) {
		hud_refresh_weapon();
	}
    if(hudLevel != playerWeapon[currentWeapon].level) {
		hud_refresh_energy(TRUE);
	}
    if(hudEnergy != playerWeapon[currentWeapon].energy) {
		hud_refresh_energy(FALSE);
	}
    if(hudMaxAmmo != playerWeapon[currentWeapon].maxammo) {
		// Max ammo changed refresh both
		hud_refresh_maxammo();
		hud_refresh_ammo();
	}
    if(hudAmmo != playerWeapon[currentWeapon].ammo ) {
		hud_refresh_ammo();
	}
}

void hud_refresh_health() {
	// Redraw health if it changed
	hudMaxHealth = max(playerMaxHealth, 1); // Just so it's impossible to divide zero
	hudHealth = player.health;
	// 40 * hudHealth / hudMaxHealth
	int16_t fillHP = ((uint16_t)((hudHealth<<5) + (hudHealth<<3))) / hudMaxHealth;
	for(uint8_t i = 0; i < 5; i++) {
		// The TS_HudBar tileset has two rows of 8 tiles, where the section of the
		// bar is empty at tile 0 and full at tile 7
		int16_t addrHP = min(fillHP*TSIZE, 7*TSIZE);
		if(addrHP < 0) addrHP = 0;
		// Fill in the bar
		memcpy(tileData[i+3], &TS_HudBar.tiles[addrHP], TILE_SIZE);
		fillHP -= 8;
	}
	// Heart icon and two digits displaying current health
	memcpy(tileData[0], &SPR_TILES(&SPR_Hud2, 0, 0)[3*TSIZE], TILE_SIZE);
	uint8_t digit = div10[hudHealth];
	if(digit) {
		memcpy(tileData[1], &TS_Numbers.tiles[(digit)*TSIZE], TILE_SIZE);
	} else {
		memcpy(tileData[1], TILE_BLANK, TILE_SIZE);
	}
	memcpy(tileData[2], &TS_Numbers.tiles[mod10[hudHealth]*TSIZE], TILE_SIZE);
	// Queue DMA transfer for health display
	for(uint8_t i = 0; i < 8; i++)
		DMA_queueDma(DMA_VRAM, (uint32_t)tileData[i], (TILE_HUDINDEX+3+i*4)*TILE_SIZE, 16, 2);
}

void hud_refresh_energy(uint8_t hard) {
	// Energy or level changed
	if(hudLevel != playerWeapon[currentWeapon].level || hudWeapon != currentWeapon) {
		hudLevel = playerWeapon[currentWeapon].level;
		hard = TRUE;
	}
	if(playerWeapon[currentWeapon].type == WEAPON_SPUR) {
		hudMaxEnergy = spur_time[pal_mode||cfg_60fps][playerWeapon[currentWeapon].level];
	} else {
		hudMaxEnergy = max(weapon_info[playerWeapon[currentWeapon].type].experience[hudLevel-1], 10);
	}
	if(!hard) {
		if(hudEnergyTimer == 0) {
            hudEnergyPixel = EnergyPixel[div10[hudMaxEnergy] - 1][hudEnergy];
            hudEnergyDest = EnergyPixel[div10[hudMaxEnergy] - 1][playerWeapon[currentWeapon].energy];
			//hudEnergyPixel = ((hudEnergy<<5) + (hudEnergy<<3)) / hudMaxEnergy;
			//hudEnergyDest = ((playerWeapon[currentWeapon].energy<<5) + (playerWeapon[currentWeapon].energy<<3)) / hudMaxEnergy;
			hudEnergyTimer = 3;
		} else {
			hudEnergyTimer--;
		}
	} else {
		hudEnergy = playerWeapon[currentWeapon].energy;
		//hudEnergyPixel = ((hudEnergy<<5) + (hudEnergy<<3)) / hudMaxEnergy;
        hudEnergyPixel = EnergyPixel[div10[hudMaxEnergy] - 1][hudEnergy];
		hudEnergyDest = hudEnergyPixel;
		hudEnergyTimer = 0;
	}
	if(hudEnergyTimer == 0) {
		// Max energy draws "MAX"
		if(playerWeapon[currentWeapon].energy == hudMaxEnergy) {
			for(uint8_t i = 0; i < 5; i++) {
				memcpy(tileData[XP_BAR+i+3], &TS_HudMax.tiles[i * TSIZE], TILE_SIZE);
			}
			hudEnergy = hudMaxEnergy;
			hudEnergyDest = hudEnergyPixel;
		} else {
			// Even if these values are equal, we need to redraw the bar after it flashes
			if(hudEnergyPixel > hudEnergyDest) {
				// Energy decreasing
				hudEnergyPixel--;
			} else if(hudEnergyPixel < hudEnergyDest) {
				// Energy increasing
				hudEnergyPixel++;
			}
			int16_t fillXP = hudEnergyPixel;
			for(uint8_t i = 0; i < 5; i++) {
				int16_t addrXP = min(fillXP*TSIZE, 7*TSIZE);
				if(addrXP < 0) addrXP = 0;
				memcpy(tileData[XP_BAR+i+3], &TS_HudBar.tiles[addrXP + 8*TSIZE], TILE_SIZE);
				fillXP -= 8;
			}
			if(hudEnergyPixel == hudEnergyDest) {
				// Finished increasing / decreasing energy
				hudEnergy = playerWeapon[currentWeapon].energy;
			} else {
				hudEnergyTimer = 3;
			}
		}
	} else if(hudEnergyTimer == 2) {
		// Flashing while increasing / decreasing
		for(uint8_t i = 0; i < 5; i++) {
			memcpy(tileData[XP_BAR+i+3], &TS_HudFlash.tiles[i * TSIZE], TILE_SIZE);
		}
	}
	// "Lv." and 1 digit for the level
	memcpy(tileData[XP_BAR+0], &SPR_TILES(&SPR_Hud2, 0, 0)[2*TSIZE], TILE_SIZE);
	memcpy(tileData[XP_BAR+1], &SPR_TILES(&SPR_Hud2, 0, 0)[6*TSIZE], TILE_SIZE);
	memcpy(tileData[XP_BAR+2], &TS_Numbers.tiles[hudLevel*TSIZE], TILE_SIZE);
	// Queue DMA transfer for level/energy display
	for(uint8_t i = 0; i < 8; i++)
		DMA_queueDma(DMA_VRAM, (uint32_t)tileData[XP_BAR+i], (TILE_HUDINDEX+2+i*4)*TILE_SIZE, 16, 2);
}

void hud_refresh_weapon() {
	// Weapon switched
	hudWeapon = playerWeapon[currentWeapon].type;
	memcpy(tileData[WPN+0], SPR_TILES(&SPR_ArmsImage, 0, hudWeapon), TILE_SIZE*2);
	memcpy(tileData[WPN+2], &SPR_TILES(&SPR_ArmsImage, 0, hudWeapon)[TSIZE*2], TILE_SIZE*2);
	// Queue DMA transfer for icon
	DMA_queueDma(DMA_VRAM, (uint32_t)tileData[WPN+0], (TILE_HUDINDEX)*TILE_SIZE, 32, 2);
	DMA_queueDma(DMA_VRAM, (uint32_t)tileData[WPN+2], (TILE_HUDINDEX+4)*TILE_SIZE, 32, 2);
}

void hud_refresh_ammo() {
	// Top half of ammo display
	hudAmmo = playerWeapon[currentWeapon].ammo;
	if(hudMaxAmmo > 0) {
		memcpy(tileData[AMMO+0], TILE_BLANK, TILE_SIZE);
		uint8_t ammoTemp = hudAmmo;
		if(ammoTemp >= 100) {
			ammoTemp -= 100;
			memcpy(tileData[AMMO+1], &TS_Numbers.tiles[1*TSIZE], TILE_SIZE);
		} else {
			memcpy(tileData[AMMO+1], TILE_BLANK, TILE_SIZE);
		}
		memcpy(tileData[AMMO+2], &TS_Numbers.tiles[div10[ammoTemp]*TSIZE], TILE_SIZE);
		memcpy(tileData[AMMO+3], &TS_Numbers.tiles[mod10[ammoTemp]*TSIZE], TILE_SIZE);
	} else { // Weapon doesn't use ammo
		memcpy(tileData[AMMO+0], TILE_BLANK, TILE_SIZE);
		memcpy(tileData[AMMO+1], TILE_BLANK, TILE_SIZE);
		memcpy(tileData[AMMO+2], &SPR_TILES(&SPR_Hud2,0,0)[SPR_TILE(6, 0)*TSIZE], TILE_SIZE*2);
	}
	// Queue DMA transfer for ammo
	for(uint8_t i = 0; i < 4; i++)
		DMA_queueDma(DMA_VRAM, (uint32_t)tileData[AMMO+i], (TILE_HUDINDEX+16+i*4)*TILE_SIZE, 16, 2);
}

void hud_refresh_maxammo() {
	// Bottom half of ammo display
	hudMaxAmmo = playerWeapon[currentWeapon].maxammo;
	memcpy(tileData[AMMO+4], &SPR_TILES(&SPR_Hud2,0,0)[SPR_TILE(4, 1)*TSIZE], TILE_SIZE);
	if(hudMaxAmmo > 0) {
		uint8_t ammoTemp = hudMaxAmmo;
		if(ammoTemp >= 100) {
			ammoTemp -= 100;
			memcpy(tileData[AMMO+5], &TS_Numbers.tiles[1*TSIZE], TILE_SIZE);
		} else {
			memcpy(tileData[AMMO+5], TILE_BLANK, TILE_SIZE);
		}
		memcpy(tileData[AMMO+6], &TS_Numbers.tiles[div10[ammoTemp]*TSIZE], TILE_SIZE);
		memcpy(tileData[AMMO+7], &TS_Numbers.tiles[mod10[ammoTemp]*TSIZE], TILE_SIZE);
	} else { // Weapon doesn't use ammo
		memcpy(tileData[AMMO+5], TILE_BLANK, TILE_SIZE);
		memcpy(tileData[AMMO+6], &SPR_TILES(&SPR_Hud2,0,0)[SPR_TILE(6, 0)*TSIZE], TILE_SIZE*2);
	}
	// Queue DMA transfer for max ammo
	for(uint8_t i = 0; i < 4; i++)
		DMA_queueDma(DMA_VRAM, (uint32_t)tileData[AMMO+i+4], (TILE_HUDINDEX+17+i*4)*TILE_SIZE, 16, 2);
}
