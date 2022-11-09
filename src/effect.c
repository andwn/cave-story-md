#include "common.h"

#include "camera.h"
#include "md/dma.h"
#include "entity.h"
#include "math.h"
#include "md/stdlib.h"
#include "player.h"
#include "resources.h"
#include "res/tiles.h"
#include "sheet.h"
#include "tables.h"
#include "md/comp.h"
#include "md/vdp.h"

#include "effect.h"

typedef struct {
	Sprite sprite;
	uint8_t type, ttl, timer, timer2;
	int16_t x, y;
	int8_t x_speed, y_speed;
} Effect;

static Effect effDamage[MAX_DAMAGE], effSmoke[MAX_SMOKE], effMisc[MAX_MISC];
static struct {
	Entity *e;
	int16_t xoff, yoff;
} damageFollow[MAX_DAMAGE];

// Create a memory buffer of 4 tiles containing a string like "+3" or "-127"
// Then copy to VRAM via DMA transfer
uint32_t dtiles[4][8];

uint8_t dqueued;

void effects_init() {
	for(uint8_t i = 0; i < MAX_DAMAGE; i++) effDamage[i].ttl = 0;
	for(uint8_t i = 0; i < MAX_SMOKE; i++) effSmoke[i].ttl = 0;
	for(uint8_t i = 0; i < MAX_MISC; i++) effMisc[i].ttl = 0;
	// Load each frame of the small smoke sprite
	uint32_t stiles[7][32]; // [number of frames][tiles per frame * (tile bytes / sizeof(uint32_t))]
	for(uint8_t i = 0; i < 7; i++) {
		memcpy(stiles[i], SPR_TILES(&SPR_Smoke, 0, i), 128);
	}
	// Transfer to VRAM
	vdp_tiles_load(stiles[0], TILE_SMOKEINDEX, TILE_SMOKESIZE);
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
		if(damageFollow[i].e) {
			effDamage[i].x = (damageFollow[i].e->x >> CSF) + (damageFollow[i].xoff - 8);
			effDamage[i].y = (damageFollow[i].e->y >> CSF) + (damageFollow[i].yoff) - (30 - (effDamage[i].ttl >> 1));
		} else {
			effDamage[i].y -= effDamage[i].ttl & 1;
		}
		sprite_pos(&effDamage[i].sprite,
                   effDamage[i].x - sub_to_pixel(camera.x) + ScreenHalfW,
                   effDamage[i].y - sub_to_pixel(camera.y) + ScreenHalfH);
	vdp_sprite_add(&effDamage[i].sprite);
	}
	for(uint8_t i = 0; i < MAX_SMOKE; i++) {
		if(!effSmoke[i].ttl) continue;
		effSmoke[i].ttl--;
		effSmoke[i].x += effSmoke[i].x_speed;
		effSmoke[i].y += effSmoke[i].y_speed;
		// Half assed animation
		sprite_index(&effSmoke[i].sprite,
			TILE_SMOKEINDEX + 24 - ((effSmoke[i].ttl >> 2) << 2));
		sprite_pos(&effSmoke[i].sprite,
                   effSmoke[i].x - sub_to_pixel(camera.x) + ScreenHalfW - 8,
                   effSmoke[i].y - sub_to_pixel(camera.y) + ScreenHalfH - 8);
	vdp_sprite_add(&effSmoke[i].sprite);
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
					sprite_pos(&effMisc[i].sprite,
                               effMisc[i].x - sub_to_pixel(camera.x) + ScreenHalfW - 4,
                               effMisc[i].y - sub_to_pixel(camera.y) + ScreenHalfH - 4);
					vdp_sprite_add(&effMisc[i].sprite);
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
					sprite_pos(&effMisc[i].sprite,
                               effMisc[i].x - sub_to_pixel(camera.x) + ScreenHalfW - 4,
                               effMisc[i].y - sub_to_pixel(camera.y) + ScreenHalfH - 4);
					vdp_sprite_add(&effMisc[i].sprite);
				}
			}
			break;
			case EFF_ZZZ:
			{
				if(++effMisc[i].timer >= TIME_8(25)) {
					effMisc[i].timer = 0;
					effMisc[i].sprite.attr++;
				}
				sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - sub_to_pixel(camera.x) + ScreenHalfW - 4,
                           effMisc[i].y - sub_to_pixel(camera.y) + ScreenHalfH - 4);
				vdp_sprite_add(&effMisc[i].sprite);
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
					effMisc[i].sprite.attr++;
				}
				sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - sub_to_pixel(camera.x) + ScreenHalfW - 4,
                           effMisc[i].y - sub_to_pixel(camera.y) + ScreenHalfH - 4);
				vdp_sprite_add(&effMisc[i].sprite);
			}
			break;
			case EFF_QMARK:
			{
				if(effMisc[i].ttl > TIME_8(20) && (effMisc[i].ttl & 1) == 0) {
					effMisc[i].y -= 2;
				}
				sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - sub_to_pixel(camera.x) + ScreenHalfW - 4,
                           effMisc[i].y - sub_to_pixel(camera.y) + ScreenHalfH - 4);
				vdp_sprite_add(&effMisc[i].sprite);
			}
			break;
			case EFF_FANL:
			case EFF_FANR:
			{
				effMisc[i].x += effMisc[i].x_speed;
				sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - sub_to_pixel(camera.x) + ScreenHalfW - 4,
                           effMisc[i].y - sub_to_pixel(camera.y) + ScreenHalfH - 4);
				vdp_sprite_add(&effMisc[i].sprite);
			}
			break;
			case EFF_FANU:
			case EFF_FAND:
			{
				effMisc[i].y += effMisc[i].y_speed;
				sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - sub_to_pixel(camera.x) + ScreenHalfW - 4,
                           effMisc[i].y - sub_to_pixel(camera.y) + ScreenHalfH - 4);
				vdp_sprite_add(&effMisc[i].sprite);
			}
			break;
			case EFF_SPLASH:
			{
				if(++effMisc[i].timer >= 5) {
					effMisc[i].timer = 0;
					effMisc[i].y_speed++;
				}
				effMisc[i].x += effMisc[i].x_speed;
				effMisc[i].y += effMisc[i].y_speed;
				if(effMisc[i].ttl & 1) {
					sprite_pos(&effMisc[i].sprite,
                               effMisc[i].x - sub_to_pixel(camera.x) + ScreenHalfW - 4,
                               effMisc[i].y - sub_to_pixel(camera.y) + ScreenHalfH - 4);
					vdp_sprite_add(&effMisc[i].sprite);
				}
			}
			break;
            case EFF_PSTAR_HIT:
            case EFF_MGUN_HIT:
            {
                if(effMisc[i].ttl && (effMisc[i].ttl & 3) == 0) {
                    effMisc[i].sprite.attr += 4;
                }
                sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - sub_to_pixel(camera.x) + ScreenHalfW - 8,
                           effMisc[i].y - sub_to_pixel(camera.y) + ScreenHalfH - 8);
                vdp_sprite_add(&effMisc[i].sprite);
            }
            break;
		    case EFF_BUBB_POP:
            {
                if(effMisc[i].ttl == 4) effMisc[i].sprite.attr++;
                sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - sub_to_pixel(camera.x) + ScreenHalfW - 4,
                           effMisc[i].y - sub_to_pixel(camera.y) + ScreenHalfH - 4);
                vdp_sprite_add(&effMisc[i].sprite);
            }
            break;
            case EFF_FIRETRAIL:
            case EFF_SNAKETRAIL:
            {
                if((effMisc[i].ttl & 3) == 0) effMisc[i].sprite.attr++;
                sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - sub_to_pixel(camera.x) + ScreenHalfW - 4,
                           effMisc[i].y - sub_to_pixel(camera.y) + ScreenHalfH - 4);
                vdp_sprite_add(&effMisc[i].sprite);
            }
            break;
		}
	}
}

void effect_create_damage(int16_t num, Entity *follow, int16_t xoff, int16_t yoff) {
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
			tileIndex = ((negative ? 11 : 0) + mod10[num]) << 3;
			memcpy(dtiles[3 - digitCount], &TS_Numbers[tileIndex], 32);
			num = div10[num];
		}
		tileIndex = ((negative ? 11 : 0) + 10) * 8;
		memcpy(dtiles[3 - digitCount], &TS_Numbers[tileIndex], 32); // - or +
		
		if(follow) {
			damageFollow[i].e = follow;
			damageFollow[i].xoff = xoff;
			damageFollow[i].yoff = yoff;
			effDamage[i].x = (damageFollow[i].e->x >> CSF) + (xoff - 8);
			effDamage[i].y = (damageFollow[i].e->y >> CSF) + (yoff);
		} else {
			damageFollow[i].e = NULL;
			effDamage[i].x = xoff - 8;
			effDamage[i].y = yoff;
		}
		effDamage[i].ttl = 60; // 1 second
		effDamage[i].sprite = (Sprite) {
			.size = SPRITE_SIZE(digitCount+1, 1),
			.attr = TILE_ATTR(PAL0, 1, 0, 0, TILE_NUMBERINDEX + (i<<2))
		};
		TILES_QUEUE(dtiles[3-digitCount], TILE_NUMBERINDEX + (i<<2), digitCount+1);
		dqueued = TRUE;
		break;
	}
}

void effect_create_smoke(int16_t x, int16_t y) {
	for(uint8_t i = 0; i < MAX_SMOKE; i++) {
		if(effSmoke[i].ttl) continue;
		effSmoke[i].x = x;
		effSmoke[i].y = y;
		switch(rand() & 7) {
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
		effSmoke[i].sprite = (Sprite) {
			.size = SPRITE_SIZE(2, 2),
			.attr = TILE_ATTR(PAL1, 1, 0, 0, TILE_SMOKEINDEX)
		};
		break;
	}
}

void effect_create_misc(uint8_t type, int16_t x, int16_t y, uint8_t only_one) {
	for(uint8_t i = 0; i < MAX_MISC; i++) {
		if(effMisc[i].ttl) {
			if(only_one && effMisc[i].type == type) break;
			continue;
		}
		effMisc[i].type = type;
		effMisc[i].x = x;
		effMisc[i].y = y;
		switch(type) {
			case EFF_BONKL: // Dots that appear when player bonks their head on the ceiling
			case EFF_BONKR:
			{
				effMisc[i].ttl = 30;
				effMisc[i].sprite = (Sprite) {
					.size = SPRITE_SIZE(1, 1),
					.attr = TILE_ATTR(PAL0,1,0,0,1)
				};
			}
			break;
			case EFF_ZZZ: // Zzz shown above sleeping NPCs like gunsmith, mimiga, etc
			{
				uint8_t sheet = NOSHEET;
				SHEET_FIND(sheet, SHEET_ZZZ);
				if(sheet == NOSHEET) break;
				effMisc[i].ttl = TIME_8(100);
				effMisc[i].sprite = (Sprite) {
					.size = SPRITE_SIZE(1, 1),
					.attr = TILE_ATTR(PAL0,1,0,0,sheets[sheet].index)
				};
			}
			break;
			case EFF_BOOST8: // Smoke that emits while using the booster
			case EFF_BOOST2:
			{
				effMisc[i].ttl = TIME_8(20);
				effMisc[i].sprite = (Sprite) {
					.size = SPRITE_SIZE(1, 1),
					.attr = TILE_ATTR(PAL0,1,0,0,12)
				};
			}
			break;
			case EFF_QMARK:
			{
				effMisc[i].ttl = TIME_8(30);
				effMisc[i].sprite = (Sprite) {
					.size = SPRITE_SIZE(1, 1),
					.attr = TILE_ATTR(PAL0,1,0,0,TILE_QMARKINDEX)
				};
			} 
			break;
			case EFF_FANL:
			{
				effMisc[i].x_speed = -(rand() & 3) - 1;
				effMisc[i].ttl = TIME_8(20);
				effMisc[i].sprite = (Sprite) {
					.size = SPRITE_SIZE(1, 1),
					.attr = TILE_ATTR(PAL0,1,0,0,1)
				};
			}
			break;
			case EFF_FANU:
			{
				effMisc[i].y_speed = -(rand() & 3) - 1;
				effMisc[i].ttl = TIME_8(20);
				effMisc[i].sprite = (Sprite) {
					.size = SPRITE_SIZE(1, 1),
					.attr = TILE_ATTR(PAL0,1,0,0,1)
				};
			}
			break;
			case EFF_FANR:
			{
				effMisc[i].x_speed = (rand() & 3) + 1;
				effMisc[i].ttl = TIME_8(20);
				effMisc[i].sprite = (Sprite) {
					.size = SPRITE_SIZE(1, 1),
					.attr = TILE_ATTR(PAL0,1,0,0,1)
				};
			}
			break;
			case EFF_FAND:
			{
				effMisc[i].y_speed = (rand() & 3) + 1;
				effMisc[i].ttl = TIME_8(20);
				effMisc[i].sprite = (Sprite) {
					.size = SPRITE_SIZE(1, 1),
					.attr = TILE_ATTR(PAL0,1,0,0,1)
				};
			}
			break;
			case EFF_SPLASH:
			{
				if(rand() & 1) {
					effMisc[i].y_speed = -2;
					effMisc[i].ttl = 18;
				} else {
					effMisc[i].y_speed = -3;
					effMisc[i].ttl = 29;
				}
				effMisc[i].x_speed = (player.x_speed >> CSF) - 1 + (rand() & 3);
				effMisc[i].sprite = (Sprite) {
					.size = SPRITE_SIZE(1, 1),
					.attr = TILE_ATTR(PAL0,1,0,0,1)
				};
			}
			break;
		    case EFF_PSTAR_HIT:
            {
                effMisc[i].ttl = 12;
                uint8_t loc = 0;
                SHEET_FIND(loc, SHEET_PSTAR);
                effMisc[i].sprite = (Sprite) {
                    .size = SPRITE_SIZE(2, 2),
                    .attr = TILE_ATTR(PAL0,1,0,0,sheets[loc].index+8)
                };
            }
            break;
            case EFF_MGUN_HIT:
            {
                effMisc[i].ttl = 12;
                uint8_t loc = 0;
                SHEET_FIND(loc, SHEET_MGUN);
                effMisc[i].sprite = (Sprite) {
                    .size = SPRITE_SIZE(2, 2),
                    .attr = TILE_ATTR(PAL0,1,0,0,sheets[loc].index+8)
                };
            }
            break;
            case EFF_BUBB_POP:
            {
                effMisc[i].ttl = 8;
                uint8_t loc = 0;
                SHEET_FIND(loc, SHEET_BUBB);
                effMisc[i].sprite = (Sprite) {
                        .size = SPRITE_SIZE(1, 1),
                        .attr = TILE_ATTR(PAL0,1,0,0,sheets[loc].index+4)
                };
            }
            break;
            case EFF_FIRETRAIL:
            {
                effMisc[i].ttl = 12;
                uint8_t loc = 0;
                SHEET_FIND(loc, SHEET_FBALL);
                effMisc[i].sprite = (Sprite) {
                        .size = SPRITE_SIZE(1, 1),
                        .attr = TILE_ATTR(PAL1,1,0,0,sheets[loc].index+12)
                };
            }
            break;
            case EFF_SNAKETRAIL:
            {
                effMisc[i].ttl = 12;
                uint8_t loc = 0;
                SHEET_FIND(loc, SHEET_SNAKE);
                effMisc[i].sprite = (Sprite) {
                    .size = SPRITE_SIZE(1, 1),
                    .attr = TILE_ATTR(PAL1,1,0,0,sheets[loc].index+12)
                };
            }
            break;
		}
		break;
	}
}
