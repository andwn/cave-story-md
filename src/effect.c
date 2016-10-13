#include "effect.h"

#include <genesis.h>
#include "tables.h"
#include "resources.h"
#include "camera.h"
#include "vdp_ext.h"
#include "sprite.h"

typedef struct {
	VDPSprite sprite;
	u8 type, ttl;
	s16 x, y;
} Effect;

Effect effDamage[MAX_DAMAGE], effSmoke[MAX_SMOKE], effMisc[MAX_MISC];

// Create a memory buffer of 4 tiles containing a string like "+3" or "-127"
// Then copy to VRAM via DMA transfer
u32 dtiles[4][8];

void effects_init() {
	for(u8 i = 0; i < MAX_DAMAGE; i++) effDamage[i] = (Effect){};
	for(u8 i = 0; i < MAX_SMOKE; i++) effSmoke[i] = (Effect){};
	for(u8 i = 0; i < MAX_MISC; i++) effMisc[i] = (Effect){};
	// Load each frame of the small smoke sprite
	u32 stiles[7][32]; // [number of frames][tiles per frame * (tile bytes / sizeof(u32))]
	for(u8 i = 0; i < 7; i++) {
		memcpy(stiles[i], SPR_TILES(&SPR_Smoke, 0, i), 128);
	}
	// Transfer to VRAM
	VDP_loadTileData(stiles[0], TILE_SMOKEINDEX, TILE_SMOKESIZE, TRUE);
}

void effects_clear() {
	for(u8 i = 0; i < MAX_DAMAGE; i++) effDamage[i].ttl = 0;
	for(u8 i = 0; i < MAX_MISC; i++) effMisc[i].ttl = 0;
	effects_clear_smoke();
}

void effects_clear_smoke() {
	for(u8 i = 0; i < MAX_SMOKE; i++) effSmoke[i].ttl = 0;
}

void effects_update() {
	for(u8 i = 0; i < MAX_DAMAGE; i++) {
		if(!effDamage[i].ttl) continue;
		effDamage[i].ttl--;
		effDamage[i].y -= effDamage[i].ttl & 1;
		sprite_pos(effDamage[i].sprite,
			effDamage[i].x - sub_to_pixel(camera.x) + SCREEN_HALF_W,
			effDamage[i].y - sub_to_pixel(camera.y) + SCREEN_HALF_H);
		sprite_add(effDamage[i].sprite);
	}
	for(u8 i = 0; i < MAX_SMOKE; i++) {
		if(!effSmoke[i].ttl) continue;
		effSmoke[i].ttl--;
		// Half assed animation
		sprite_index(effSmoke[i].sprite,
			TILE_SMOKEINDEX + 24 - ((effSmoke[i].ttl >> 3) << 2));
		sprite_pos(effSmoke[i].sprite,
			effSmoke[i].x - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
			effSmoke[i].y - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);
		sprite_add(effSmoke[i].sprite);
	}
	for(u8 i = 0; i < MAX_MISC; i++) {
		if(!effMisc[i].ttl) continue;
		effMisc[i].ttl--;
		switch(effMisc[i].type) {
			case EFF_BONKL:
			{
				if(effMisc[i].ttl&1) {
					if(effMisc[i].ttl > 15) {
						effMisc[i].x--;
						effMisc[i].y--;
					}
					sprite_pos(effMisc[i].sprite,
						effMisc[i].x - sub_to_pixel(camera.x) + SCREEN_HALF_W - 4,
						effMisc[i].y - sub_to_pixel(camera.y) + SCREEN_HALF_H - 4);
					sprite_add(effMisc[i].sprite);
				}
			}
			break;
			case EFF_BONKR:
			{
				if(!(effMisc[i].ttl&1)) {
					if(effMisc[i].ttl > 15) {
						effMisc[i].x++;
						effMisc[i].y--;
					}
					sprite_pos(effMisc[i].sprite,
						effMisc[i].x - sub_to_pixel(camera.x) + SCREEN_HALF_W - 4,
						effMisc[i].y - sub_to_pixel(camera.y) + SCREEN_HALF_H - 4);
					sprite_add(effMisc[i].sprite);
				}
			}
			case EFF_ZZZ:
			{
				if(!(effMisc[i].ttl % TIME(25))) {
					sprite_index(effMisc[i].sprite, (effMisc[i].sprite.attribut & 11) + 1);
				}
				sprite_pos(effMisc[i].sprite,
					effMisc[i].x - sub_to_pixel(camera.x) + SCREEN_HALF_W - 4,
					effMisc[i].y - sub_to_pixel(camera.y) + SCREEN_HALF_H - 4);
				sprite_add(effMisc[i].sprite);
			}
			break;
			case EFF_BOOST:
			{
				
			}
			break;
		}
	}
}

void effect_create_damage(s16 num, s16 x, s16 y) {
	if(dqueued) return;
	for(u8 i = 0; i < MAX_DAMAGE; i++) {
		if(effDamage[i].ttl) continue;
		// Negative numbers are red and show '-' (Damage)
		// Positive are white and show '+' (Weapon energy)
		u8 negative = (num < 0);
		num = abs(num);
		u8 digitCount = 0; // Number of digit tiles: 1, 2, or 3 after loop
		// Create right to left, otherwise digits show up backwards
		u16 tileIndex;
		for(; num; digitCount++) {
			tileIndex = ((negative ? 11 : 0) + (num % 10)) * 8;
			memcpy(dtiles[3 - digitCount], &TS_Numbers.tiles[tileIndex], 32);
			num /= 10;
		}
		tileIndex = ((negative ? 11 : 0) + 10) * 8;
		memcpy(dtiles[3 - digitCount], &TS_Numbers.tiles[tileIndex], 32); // - or +
		
		effDamage[i].ttl = 60; // 1 second
		effDamage[i].x = x - 8;
		effDamage[i].y = y;
		effDamage[i].sprite = (VDPSprite) {
			.size = SPRITE_SIZE(digitCount+1, 1),
			.attribut = TILE_ATTR_FULL(PAL0, 1, 0, 0, TILE_NUMBERINDEX + i*4)
		};
		TILES_QUEUE(dtiles[3-digitCount], TILE_NUMBERINDEX + i*4, digitCount+1);
		dqueued = TRUE;
		break;
	}
}

void effect_create_smoke(s16 x, s16 y) {
	for(u8 i = 0; i < MAX_SMOKE; i++) {
		if(effSmoke[i].ttl) continue;
		effSmoke[i].x = x;
		effSmoke[i].y = y;
		effSmoke[i].ttl = 48;
		effSmoke[i].sprite = (VDPSprite) {
			.size = SPRITE_SIZE(2, 2),
			.attribut = TILE_ATTR_FULL(PAL1, 1, 0, 0, TILE_SMOKEINDEX)
		};
		break;
	}
}

void effect_create_misc(u8 type, s16 x, s16 y) {
	for(u8 i = 0; i < MAX_MISC; i++) {
		if(effMisc[i].ttl) continue;
		effMisc[i].type = type;
		effMisc[i].x = x;
		effMisc[i].y = y;
		switch(type) {
			case EFF_BONKL:
			case EFF_BONKR:
			{
				effMisc[i].ttl = 30;
				effMisc[i].sprite = (VDPSprite) {
					.size = SPRITE_SIZE(1, 1),
					.attribut = TILE_ATTR_FULL(PAL0,1,0,0,1)
				};
			}
			break;
			case EFF_ZZZ:
			{
				u8 sheet = NOSHEET;
				SHEET_FIND(sheet, SHEET_ZZZ);
				if(sheet == NOSHEET) break;
				effMisc[i].ttl = TIME(100);
				effMisc[i].sprite = (VDPSprite) {
					.size = SPRITE_SIZE(1, 1),
					.attribut = TILE_ATTR_FULL(PAL0,1,0,0,sheets[sheet].index)
				};
			}
			break;
		}
		break;
	}
}
