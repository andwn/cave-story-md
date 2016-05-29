#include "effect.h"

#include <genesis.h>
#include "tables.h"
#include "resources.h"
#include "camera.h"
#include "vdp_ext.h"

#define MAX_DAMAGE 4
#define MAX_SMOKE 6

const u32 TILE_BLANK[8] = {0,0,0,0,0,0,0,0};

typedef struct {
	Sprite *sprite;
	u8 ttl;
	s16 x, y;
} Effect;

Effect effDamage[MAX_DAMAGE], effSmoke[MAX_SMOKE];

void effects_init() {
	for(u8 i = 0; i < MAX_DAMAGE; i++) effDamage[i] = (Effect) { NULL, 0, 0, 0 };
	for(u8 i = 0; i < MAX_SMOKE; i++) effSmoke[i] = (Effect) { NULL, 0, 0, 0 };
	// Load each frame of the small smoke sprite
	u32 tiles[7][32]; // [number of frames][tiles per frame * (tile bytes / sizeof(u32))]
	for(u8 i = 0; i < 7; i++) {
		memcpy(tiles[i], SPR_TILESET(SPR_Smoke, 0, i)->tiles, 128);
	}
	// Transfer to VRAM
	VDP_loadTileData(tiles[0], TILE_SMOKEINDEX, TILE_SMOKESIZE, true);
}

void effects_clear() {
	for(u8 i = 0; i < MAX_DAMAGE; i++) {
		effDamage[i].ttl = 0;
		SPR_SAFERELEASE(effDamage[i].sprite);
	}
	for(u8 i = 0; i < MAX_SMOKE; i++) {
		effSmoke[i].ttl = 0;
		SPR_SAFERELEASE(effSmoke[i].sprite);
	}
}

void effects_update() {
	for(u8 i = 0; i < MAX_DAMAGE; i++) {
		if(effDamage[i].ttl == 0) continue;
		if(--effDamage[i].ttl == 0) {
			SPR_SAFERELEASE(effDamage[i].sprite);
		} else {
			if(effDamage[i].ttl & 1) effDamage[i].y -= 1;
			SPR_setPosition(effDamage[i].sprite, 
				effDamage[i].x - sub_to_pixel(camera.x) + SCREEN_HALF_W,
				effDamage[i].y - sub_to_pixel(camera.y) + SCREEN_HALF_H);
		}
	}
	for(u8 i = 0; i < MAX_SMOKE; i++) {
		if(effSmoke[i].ttl == 0) continue;
		if(--effSmoke[i].ttl == 0) {
			SPR_SAFERELEASE(effSmoke[i].sprite);
		} else {
			// Half assed animation
			SPR_setVRAMTileIndex(effSmoke[i].sprite, 
				TILE_SMOKEINDEX + 24 - ((effSmoke[i].ttl >> 3) << 2));
			SPR_setPosition(effSmoke[i].sprite, 
				effSmoke[i].x - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
				effSmoke[i].y - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);
		}
	}
}

void effect_create_damage(s16 num, s16 x, s16 y, u8 ttl) {
	for(u8 i = 0; i < MAX_DAMAGE; i++) {
		if(effDamage[i].ttl > 0) continue;
		// Negative numbers are red and show '-' (Damage)
		// Positive are white and show '+' (Weapon energy)
		bool negative = (num < 0);
		num = abs(num);
		u8 digitCount = 0; // Number of digit tiles: 1, 2, or 3 after loop
		// Create a memory buffer of 4 tiles containing a string like "+3" or "-127"
		// Then copy to VRAM via DMA transfer
		u32 tiles[4][8];
		u16 tileIndex = (negative * 11 + 10) * 8;
		memcpy(tiles[0], &TS_Numbers.tiles[tileIndex], 32); // - or +
		for(; num; digitCount++) {
			tileIndex = (negative * 11 + (num % 10)) * 8;
			memcpy(tiles[digitCount+1], &TS_Numbers.tiles[tileIndex], 32);
			num /= 10;
		}
		// Fill any remaining digits blank
		for(u8 i = digitCount + 1; i < 4; i++) memcpy(tiles[i], TILE_BLANK, 32);
		effDamage[i].ttl = 60; // 1 second
		effDamage[i].x = x;
		effDamage[i].y = y;
		effDamage[i].sprite = SPR_addSpriteEx(&SPR_Dummy4x1, 
			x - sub_to_pixel(camera.x) + SCREEN_HALF_W,
			y - sub_to_pixel(camera.y) + SCREEN_HALF_H, 
			TILE_ATTR_FULL(PAL0, 1, 0, 0, TILE_NUMBERINDEX + (i * 4)), 0,
			SPR_FLAG_AUTO_VISIBILITY | SPR_FLAG_AUTO_SPRITE_ALLOC);
		SYS_disableInts();
		VDP_loadTileData(tiles[0], TILE_NUMBERINDEX + (i * 4), 4, true);
		SYS_enableInts();
		break;
	}
}

void effect_create_smoke(u8 type, s16 x, s16 y) {
	for(u8 i = 0; i < MAX_SMOKE; i++) {
		if(effSmoke[i].ttl > 0) continue;
		effSmoke[i].x = x;
		effSmoke[i].y = y;
		effSmoke[i].ttl = 48;
		effSmoke[i].sprite = SPR_addSpriteEx(&SPR_Dummy2x2, 
			x - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
			y - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8, 
			TILE_ATTR_FULL(PAL1, 1, 0, 0, TILE_SMOKEINDEX), 0,
			SPR_FLAG_AUTO_VISIBILITY | SPR_FLAG_AUTO_SPRITE_ALLOC);
		break;
	}
}
