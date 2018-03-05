#include "common.h"

#include "camera.h"
#include "dma.h"
#include "memory.h"
#include "resources.h"
#include "sheet.h"
#include "sprite.h"
#include "tables.h"
#include "tools.h"
#include "vdp.h"
#include "vdp_tile.h"
#include "vdp_ext.h"

#include "effect.h"

typedef struct {
	VDPSprite sprite;
	uint8_t type, ttl, timer, timer2;
	int16_t x, y;
	int8_t x_speed, y_speed;
} Effect;

Effect effDamage[MAX_DAMAGE], effSmoke[MAX_SMOKE], effMisc[MAX_MISC];

// Create a memory buffer of 4 tiles containing a string like "+3" or "-127"
// Then copy to VRAM via DMA transfer
uint32_t dtiles[4][8];

void effects_init() {
	for(uint8_t i = 0; i < MAX_DAMAGE; i++) effDamage[i] = (Effect){};
	for(uint8_t i = 0; i < MAX_SMOKE; i++) effSmoke[i] = (Effect){};
	for(uint8_t i = 0; i < MAX_MISC; i++) effMisc[i] = (Effect){};
	// Load each frame of the small smoke sprite
	uint32_t stiles[7][32]; // [number of frames][tiles per frame * (tile bytes / sizeof(uint32_t))]
	for(uint8_t i = 0; i < 7; i++) {
		memcpy(stiles[i], SPR_TILES(&SPR_Smoke, 0, i), 128);
	}
	// Transfer to VRAM
	VDP_loadTileData(stiles[0], TILE_SMOKEINDEX, TILE_SMOKESIZE, TRUE);
}

void effects_clear() {
	for(uint8_t i = 0; i < MAX_DAMAGE; i++) effDamage[i].ttl = 0;
	for(uint8_t i = 0; i < MAX_MISC; i++) effMisc[i].ttl = 0;
	effects_clear_smoke();
}

void effects_clear_smoke() {
	for(uint8_t i = 0; i < MAX_SMOKE; i++) effSmoke[i].ttl = 0;
}

void effects_update() {
	for(uint8_t i = 0; i < MAX_DAMAGE; i++) {
		if(!effDamage[i].ttl) continue;
		effDamage[i].ttl--;
		effDamage[i].y -= effDamage[i].ttl & 1;
		sprite_pos(effDamage[i].sprite,
			effDamage[i].x - sub_to_pixel(camera.x) + SCREEN_HALF_W,
			effDamage[i].y - sub_to_pixel(camera.y) + SCREEN_HALF_H);
		sprite_add(effDamage[i].sprite);
	}
	for(uint8_t i = 0; i < MAX_SMOKE; i++) {
		if(!effSmoke[i].ttl) continue;
		effSmoke[i].ttl--;
		effSmoke[i].x += effSmoke[i].x_speed;
		effSmoke[i].y += effSmoke[i].y_speed;
		// Half assed animation
		sprite_index(effSmoke[i].sprite,
			TILE_SMOKEINDEX + 24 - ((effSmoke[i].ttl >> 2) << 2));
		sprite_pos(effSmoke[i].sprite,
			effSmoke[i].x - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
			effSmoke[i].y - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);
		sprite_add(effSmoke[i].sprite);
	}
	for(uint8_t i = 0; i < MAX_MISC; i++) {
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
			break;
			case EFF_ZZZ:
			{
				if(++effMisc[i].timer >= TIME_8(25)) {
					effMisc[i].timer = 0;
					effMisc[i].sprite.attribut++;
				}
				sprite_pos(effMisc[i].sprite,
					effMisc[i].x - sub_to_pixel(camera.x) + SCREEN_HALF_W - 4,
					effMisc[i].y - sub_to_pixel(camera.y) + SCREEN_HALF_H - 4);
				sprite_add(effMisc[i].sprite);
			}
			break;
			case EFF_BOOST8:
			{
				effMisc[i].y++;
			} /* fallthrough */
			case EFF_BOOST2:
			{
				if(++effMisc[i].timer >= TIME_8(5)) {
					effMisc[i].timer = 0;
					effMisc[i].sprite.attribut++;
				}
				sprite_pos(effMisc[i].sprite,
					effMisc[i].x - sub_to_pixel(camera.x) + SCREEN_HALF_W - 4,
					effMisc[i].y - sub_to_pixel(camera.y) + SCREEN_HALF_H - 4);
				sprite_add(effMisc[i].sprite);
			}
			break;
			case EFF_QMARK:
			{
				if(effMisc[i].ttl > TIME_8(60) && (effMisc[i].ttl & 3) == 0) {
					effMisc[i].y--;
				}
				effMisc[i].ttl--;
				sprite_pos(effMisc[i].sprite,
					effMisc[i].x - sub_to_pixel(camera.x) + SCREEN_HALF_W - 4,
					effMisc[i].y - sub_to_pixel(camera.y) + SCREEN_HALF_H - 4);
				sprite_add(effMisc[i].sprite);
			}
			break;
		}
	}
}

void effect_create_damage(int16_t num, int16_t x, int16_t y) {
	if(dqueued) return;
	for(uint8_t i = 0; i < MAX_DAMAGE; i++) {
		if(effDamage[i].ttl) continue;
		// Negative numbers are red and show '-' (Damage)
		// Positive are white and show '+' (Weapon energy)
		uint8_t negative = (num < 0);
		num = abs(num);
		uint8_t digitCount = 0; // Number of digit tiles: 1, 2, or 3 after loop
		// Create right to left, otherwise digits show up backwards
		uint16_t tileIndex;
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

void effect_create_smoke(int16_t x, int16_t y) {
	for(uint8_t i = 0; i < MAX_SMOKE; i++) {
		if(effSmoke[i].ttl) continue;
		effSmoke[i].x = x;
		effSmoke[i].y = y;
		switch(random() & 7) {
			case 0: effSmoke[i].x_speed = 0; break;
			case 1:	effSmoke[i].y_speed = 0; break;
			case 2: effSmoke[i].x_speed = 1; break;
			case 3: effSmoke[i].y_speed = 1; break;
			case 4: effSmoke[i].x_speed = -1; break;
			case 5:	effSmoke[i].y_speed = -1; break;
			case 6: effSmoke[i].x_speed ^= 1; break;
			case 7: effSmoke[i].y_speed ^= 1; break;
		}
		effSmoke[i].ttl = 24;
		effSmoke[i].sprite = (VDPSprite) {
			.size = SPRITE_SIZE(2, 2),
			.attribut = TILE_ATTR_FULL(PAL1, 1, 0, 0, TILE_SMOKEINDEX)
		};
		break;
	}
}

void effect_create_misc(uint8_t type, int16_t x, int16_t y) {
	for(uint8_t i = 0; i < MAX_MISC; i++) {
		if(effMisc[i].ttl) continue;
		effMisc[i].type = type;
		effMisc[i].x = x;
		effMisc[i].y = y;
		switch(type) {
			case EFF_BONKL: // Dots that appear when player bonks their head on the ceiling
			case EFF_BONKR:
			{
				effMisc[i].ttl = 30;
				effMisc[i].sprite = (VDPSprite) {
					.size = SPRITE_SIZE(1, 1),
					.attribut = TILE_ATTR_FULL(PAL0,1,0,0,1)
				};
			}
			break;
			case EFF_ZZZ: // Zzz shown above sleeping NPCs like gunsmith, mimiga, etc
			{
				uint8_t sheet = NOSHEET;
				SHEET_FIND(sheet, SHEET_ZZZ);
				if(sheet == NOSHEET) break;
				effMisc[i].ttl = TIME(100);
				effMisc[i].sprite = (VDPSprite) {
					.size = SPRITE_SIZE(1, 1),
					.attribut = TILE_ATTR_FULL(PAL0,1,0,0,sheets[sheet].index)
				};
			}
			break;
			case EFF_BOOST8: // Smoke that emits while using the booster
			case EFF_BOOST2:
			{
				effMisc[i].ttl = TIME(20);
				effMisc[i].sprite = (VDPSprite) {
					.size = SPRITE_SIZE(1, 1),
					.attribut = TILE_ATTR_FULL(PAL0,1,0,0,12)
				};
			}
			break;
			case EFF_QMARK:
			{
				effMisc[i].ttl = TIME(80);
				effMisc[i].sprite = (VDPSprite) {
					.size = SPRITE_SIZE(1, 1),
					.attribut = TILE_ATTR_FULL(PAL0,1,0,0,TILE_QMARKINDEX)
				};
			} 
			break;
		}
		break;
	}
}
