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
#include "md/sys.h"
#include "hud.h"
#include "gamemode.h"

#include "effect.h"

static Effect effDamage[MAX_DAMAGE], effSmoke[MAX_SMOKE], effMisc[MAX_MISC];
static struct {
	Entity *e;
	int16_t xoff, yoff;
} damageFollow[MAX_DAMAGE];

// Create a memory buffer of 4 tiles containing a string like "+3" or "-127"
// Then copy to VRAM via DMA transfer
uint32_t dtiles[MAX_DAMAGE][4][8];
uint8_t digitCount[MAX_DAMAGE];
uint8_t dqueued;

int8_t wipeFadeTimer;
uint8_t wipeFadeDir;

void effects_init(void) {
	wipeFadeTimer = -1;
	for(uint8_t i = 0; i < MAX_DAMAGE; i++) effDamage[i].ttl = 0;
	for(uint8_t i = 0; i < MAX_SMOKE; i++) effSmoke[i].ttl = 0;
	for(uint8_t i = 0; i < MAX_MISC; i++) effMisc[i].ttl = 0;
	effects_reload_tiles();
}

void effects_clear(void) {
	for(uint8_t i = 0; i < MAX_DAMAGE; i++) effDamage[i].ttl = 0;
	for(uint8_t i = 0; i < MAX_MISC; i++) effMisc[i].ttl = 0;
	effects_clear_smoke();
}

void effects_reload_tiles(void) {
	SHEET_LOAD(&SPR_Bonk, 1, 1, 1, 1, 0);
	SHEET_LOAD(&SPR_QMark, 1, 1, TILE_QMARKINDEX, 1, 0);
	SHEET_LOAD(&SPR_Smoke, 7, 2*2, TILE_SMOKEINDEX, TRUE, 0,1,2,3,4,5,6);
	SHEET_LOAD(&SPR_Dissipate, 4, 2*2, TILE_DISSIPINDEX, TRUE, 0,1,2,3);
	SHEET_LOAD(&SPR_Gib, 4, 1, TILE_GIBINDEX, TRUE, 0,1,2,3);
}

void effects_clear_smoke(void) {
	for(uint8_t i = 0; i < MAX_SMOKE; i++) effSmoke[i].ttl = 0;
}

void effects_update(void) {
	for(uint8_t i = 0; i < MAX_DAMAGE; i++) {
		if(!effDamage[i].ttl) continue;
		effDamage[i].ttl--;
		if(effDamage[i].ttl < 8) {
			// Erase effect
			for(uint16_t x = 0; x < 4; x++) {
				for(uint16_t y = 0; y < 7; y++) {
					dtiles[i][x][y] = dtiles[i][x][y+1];
				}
				dtiles[i][x][7] = 0;
			}
			dma_queue(DmaVRAM, (uint32_t) dtiles[i][3-digitCount[i]], TILE_NUMBERINDEX*32 + i*4*32, 4*16, 2);
		} else {
			if(damageFollow[i].e) {
				if(effDamage[i].ttl > 30) damageFollow[i].yoff -= effDamage[i].ttl & 1;
				effDamage[i].x = (damageFollow[i].e->x >> CSF) + (damageFollow[i].xoff - 8);
				effDamage[i].y = (damageFollow[i].e->y >> CSF) + (damageFollow[i].yoff - 8);
			} else {
				if(effDamage[i].ttl > 30) effDamage[i].y -= effDamage[i].ttl & 1;
			}
		}
		sprite_pos(&effDamage[i].sprite,
                   effDamage[i].x - camera.x_shifted,
                   effDamage[i].y - camera.y_shifted);
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
                   effSmoke[i].x - camera.x_shifted - 8,
                   effSmoke[i].y - camera.y_shifted - 8);
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
                               effMisc[i].x - camera.x_shifted - 4,
                               effMisc[i].y - camera.y_shifted - 4);
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
                               effMisc[i].x - camera.x_shifted - 4,
                               effMisc[i].y - camera.y_shifted - 4);
					vdp_sprite_add(&effMisc[i].sprite);
				}
			}
			break;
			case EFF_ZZZ:
			{
				if(++effMisc[i].timer > TIME_8(25)) {
					effMisc[i].timer = 0;
					effMisc[i].sprite.attr++;
				}
				sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - camera.x_shifted - 4,
                           effMisc[i].y - camera.y_shifted - 4);
				vdp_sprite_add(&effMisc[i].sprite);
			}
			break;
			case EFF_BOOST8:
			case EFF_BOOST2:
			{
				effMisc[i].x += effMisc[i].x_speed;
				effMisc[i].y += effMisc[i].y_speed;
				if(++effMisc[i].timer >= TIME_8(5)) {
					effMisc[i].timer = 0;
					effMisc[i].sprite.attr++;
				}
				sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - camera.x_shifted - 4,
                           effMisc[i].y - camera.y_shifted - 4);
				vdp_sprite_add(&effMisc[i].sprite);
			}
			break;
			case EFF_QMARK:
			{
				if(effMisc[i].ttl > TIME_8(20) && (effMisc[i].ttl & 1) == 0) {
					effMisc[i].y -= 2;
				}
				sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - camera.x_shifted - 4,
                           effMisc[i].y - camera.y_shifted - 4);
				vdp_sprite_add(&effMisc[i].sprite);
			}
			break;
			case EFF_FANL:
			case EFF_FANR:
			{
				effMisc[i].x += effMisc[i].x_speed;
				sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - camera.x_shifted - 4,
                           effMisc[i].y - camera.y_shifted - 4);
				vdp_sprite_add(&effMisc[i].sprite);
			}
			break;
			case EFF_FANU:
			case EFF_FAND:
			{
				effMisc[i].y += effMisc[i].y_speed;
				sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - camera.x_shifted - 4,
                           effMisc[i].y - camera.y_shifted - 4);
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
                               effMisc[i].x - camera.x_shifted - 4,
                               effMisc[i].y - camera.y_shifted - 4);
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
                           effMisc[i].x - camera.x_shifted - 8,
                           effMisc[i].y - camera.y_shifted - 8);
                vdp_sprite_add(&effMisc[i].sprite);
            }
            break;
		    case EFF_BUBB_POP:
            {
                if(effMisc[i].ttl == 4) effMisc[i].sprite.attr++;
                sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - camera.x_shifted - 4,
                           effMisc[i].y - camera.y_shifted - 4);
                vdp_sprite_add(&effMisc[i].sprite);
            }
            break;
            case EFF_FIRETRAIL:
            case EFF_SNAKETRAIL:
            {
                if((effMisc[i].ttl & 3) == 0) effMisc[i].sprite.attr++;
                sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - camera.x_shifted - 4,
                           effMisc[i].y - camera.y_shifted - 4);
                vdp_sprite_add(&effMisc[i].sprite);
            }
            break;
			case EFF_DISSIPATE:
			{
				if((effMisc[i].ttl & 3) == 3) effMisc[i].sprite.attr += 4;
                sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - camera.x_shifted - 8,
                           effMisc[i].y - camera.y_shifted - 8);
                vdp_sprite_add(&effMisc[i].sprite);
			}
			break;
			case EFF_GIB:
			{
				effMisc[i].x += effMisc[i].x_speed;
				effMisc[i].y += effMisc[i].y_speed;
				if((effMisc[i].ttl & 3) == 3) effMisc[i].sprite.attr++;
                sprite_pos(&effMisc[i].sprite,
                           effMisc[i].x - camera.x_shifted - 4,
                           effMisc[i].y - camera.y_shifted - 4);
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
		digitCount[i] = 0; // Number of digit tiles: 1, 2, or 3 after loop
		// Create right to left, otherwise digits show up backwards
		uint16_t tileIndex;
		for(; num; digitCount[i]++) {
			tileIndex = ((negative ? 11 : 0) + mod10[num]) << 3;
			memcpy(dtiles[i][3 - digitCount[i]], &TS_Numbers[tileIndex], 32);
			num = div10[num];
		}
		tileIndex = ((negative ? 11 : 0) + 10) * 8;
		memcpy(dtiles[i][3 - digitCount[i]], &TS_Numbers[tileIndex], 32); // - or +
		
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
			.size = SPRITE_SIZE(digitCount[i]+1, 1),
			.attr = TILE_ATTR(PAL0, 1, 0, 0, TILE_NUMBERINDEX + (i<<2))
		};
		TILES_QUEUE(dtiles[i][3-digitCount[i]], TILE_NUMBERINDEX + (i<<2), digitCount[i]+1);
		dqueued = TRUE;
		break;
	}
}

Effect* effect_create_smoke(int16_t x, int16_t y) {
	for(uint8_t i = 0; i < MAX_SMOKE; i++) {
		if(effSmoke[i].ttl) continue;
		effSmoke[i].x = x;
		effSmoke[i].y = y;
		switch(rand() & 7) {
			case 0: effSmoke[i].x_speed = 0;  effSmoke[i].y_speed = 0; break;
			case 1:	effSmoke[i].x_speed = -1; effSmoke[i].y_speed = 0; break;
			case 2: effSmoke[i].x_speed = -1; effSmoke[i].y_speed = -1; break;
			case 3: effSmoke[i].x_speed = 0;  effSmoke[i].y_speed = -1; break;
			case 4: effSmoke[i].x_speed = 1;  effSmoke[i].y_speed = -1; break;
			case 5:	effSmoke[i].x_speed = 1;  effSmoke[i].y_speed = 0; break;
			case 6: effSmoke[i].x_speed = 1;  effSmoke[i].y_speed = 1; break;
			case 7: effSmoke[i].x_speed = 1;  effSmoke[i].y_speed = 1; break;
		}
		effSmoke[i].ttl = 24;
		effSmoke[i].sprite = (Sprite) {
			.size = SPRITE_SIZE(2, 2),
			.attr = TILE_ATTR(PAL1, 1, 0, 0, TILE_SMOKEINDEX)
		};
		return &effSmoke[i];
	}
	return NULL;
}

Effect* effect_create_misc(uint8_t type, int16_t x, int16_t y, uint8_t only_one) {
	uint16_t i;
	for(i = 0; i < MAX_MISC; i++) {
		if(effMisc[i].ttl) {
			if(only_one && effMisc[i].type == type) break;
			continue;
		}
		effMisc[i].type = type;
		effMisc[i].x = x;
		effMisc[i].y = y;
		effMisc[i].x_speed = 0;
		effMisc[i].y_speed = 0;
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
				effMisc[i].timer = 0;
				effMisc[i].ttl = TIME_8(100);
				effMisc[i].sprite = (Sprite) {
					.size = SPRITE_SIZE(1, 1),
					.attr = TILE_ATTR(PAL0,1,0,0,sheets[sheet].index)
				};
			}
			break;
			case EFF_BOOST8: // Smoke that emits while using the booster
			{
				effMisc[i].y_speed = 1;
			} // Fallthrough
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
			case EFF_DISSIPATE:
            {
                effMisc[i].ttl = 15;
                effMisc[i].sprite = (Sprite) {
                    .size = SPRITE_SIZE(2, 2),
                    .attr = TILE_ATTR(PAL0,1,0,0,TILE_DISSIPINDEX)
                };
            }
            break;
			case EFF_GIB:
            {
				switch(rand() & 7) {
					case 0: effSmoke[i].x_speed = 0;  effSmoke[i].y_speed = 0; break;
					case 1:	effSmoke[i].x_speed = -1; effSmoke[i].y_speed = 0; break;
					case 2: effSmoke[i].x_speed = -1; effSmoke[i].y_speed = -1; break;
					case 3: effSmoke[i].x_speed = 0;  effSmoke[i].y_speed = -1; break;
					case 4: effSmoke[i].x_speed = 1;  effSmoke[i].y_speed = -1; break;
					case 5:	effSmoke[i].x_speed = 1;  effSmoke[i].y_speed = 0; break;
					case 6: effSmoke[i].x_speed = 1;  effSmoke[i].y_speed = 1; break;
					case 7: effSmoke[i].x_speed = 1;  effSmoke[i].y_speed = 1; break;
				}
                effMisc[i].ttl = 15;
                effMisc[i].sprite = (Sprite) {
                    .size = SPRITE_SIZE(2, 2),
                    .attr = TILE_ATTR(PAL1,1,0,0,TILE_GIBINDEX)
                };
            }
            break;
		}
		break;
	}
	if(i < MAX_MISC) {
		return &effMisc[i];
	} else {
		return NULL;
	}
}

// Fading

#define hval (1<<15) | TILE_FADEINDEX
#define lval TILE_FADEINDEX
static const uint16_t winmap[40] = { 
	hval,hval,hval,hval,hval,hval,hval,hval,hval,hval,
	hval,hval,hval,hval,hval,hval,hval,hval,hval,hval,
	hval,hval,hval,hval,hval,hval,hval,hval,hval,hval,
	hval,hval,hval,hval,hval,hval,hval,hval,hval,hval,
};
static const uint16_t winmap_intro[40] = { 
	lval,lval,lval,lval,lval,lval,lval,lval,lval,lval,
	lval,lval,lval,lval,lval,lval,lval,lval,lval,lval,
	lval,lval,lval,lval,lval,lval,lval,lval,lval,lval,
	lval,lval,lval,lval,lval,lval,lval,lval,lval,lval,
};
static const uint32_t tblack[8] = {
	0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,
};

static void fade_setup(Sprite *spr0, Sprite *spr1, Sprite *spr2, uint8_t dir, uint8_t fadein) {
	// Fill window plane
	for(uint16_t y = 0; y < ScreenHeight / 8; y++) {
		if(gamemode == GM_INTRO && y < 8) {
			// low priority at top of intro, as not to cover "Studio Pixel" message
			dma_now(DmaVRAM, (uint32_t) winmap_intro, VDP_PLANE_W + y*128, 40, 2);
		} else {
			dma_now(DmaVRAM, (uint32_t) winmap, VDP_PLANE_W + y*128, 40, 2);
		}
	}
	// Setup tiles for sprites+window wipe
	SHEET_LOAD(&SPR_Fade, 2, 4*4, TILE_HUDINDEX, TRUE, 0,1);
	SHEET_LOAD(&SPR_Fade, 1, 4*4, TILE_NUMBERINDEX, TRUE, 2);
	dma_now(DmaVRAM, (uint32_t) tblack, TILE_FADEINDEX*TILE_SIZE, TILE_SIZE/2, 2);
	// Setup sprites
	for(uint16_t i = 0; i < (pal_mode ? 8 : 7); i++) {
		spr0[i].y = 0x80 + i * 32;
		spr1[i].y = 0x80 + i * 32;
		spr2[i].y = 0x80 + i * 32;
		if(fadein) {
			if(dir) { // Start from right
				spr0[i].x = 0x80 + ScreenWidth + 64;
				spr1[i].x = 0x80 + ScreenWidth + 32;
				spr2[i].x = 0x80 + ScreenWidth + 0;
			} else { // Start from left
				spr0[i].x = 0x80 - 96;
				spr1[i].x = 0x80 - 64;
				spr2[i].x = 0x80 - 32;
			}
		} else {
			if(dir) { // Start from right
				spr0[i].x = 0x80 + ScreenWidth + 0;
				spr1[i].x = 0x80 + ScreenWidth + 32;
				spr2[i].x = 0x80 + ScreenWidth + 64;
			} else { // Start from left
				spr0[i].x = 0x80 - 32;
				spr1[i].x = 0x80 - 64;
				spr2[i].x = 0x80 - 96;
			}
		}
		if(i == 7) {
			spr0[i].size = SPRITE_SIZE(4,2);
			spr1[i].size = SPRITE_SIZE(4,2);
			spr2[i].size = SPRITE_SIZE(4,2);
		} else {
			spr0[i].size = SPRITE_SIZE(4,4);
			spr1[i].size = SPRITE_SIZE(4,4);
			spr2[i].size = SPRITE_SIZE(4,4);
		}
		const uint16_t pri = (gamemode == GM_INTRO && i < 2) ? 0 : 1; // Low pri for intro message
		spr0[i].attr = TILE_ATTR(PAL0,pri,0,(fadein?dir:!dir), TILE_HUDINDEX);
		spr1[i].attr = TILE_ATTR(PAL0,pri,0,(fadein?dir:!dir), TILE_HUDINDEX+16);
		spr2[i].attr = TILE_ATTR(PAL0,pri,0,(fadein?dir:!dir), TILE_NUMBERINDEX);
	}
}

void do_fadeout_wipe(uint8_t dir) {
	Sprite *spr0 = (Sprite*) dtiles[0];
	Sprite *spr1 = (Sprite*) dtiles[1];
	Sprite *spr2 = (Sprite*) dtiles[2];
	fade_setup(spr0, spr1, spr2, dir, FALSE);
	// Fade loop
	for(uint16_t f = 0; f < ScreenWidth / 16 + 6; f++) {
		// Update Sprites
		for(uint16_t i = 0; i < (pal_mode ? 8 : 7); i++) {
			spr0[i].x += dir ? -16 : 16;
			spr1[i].x += dir ? -16 : 16;
			spr2[i].x += dir ? -16 : 16;
		}
		vdp_sprites_add(spr0, pal_mode ? 8 : 7);
		vdp_sprites_add(spr1, pal_mode ? 8 : 7);
		vdp_sprites_add(spr2, pal_mode ? 8 : 7);
		player_draw();
		entities_draw();
		sys_wait_vblank();
		// Update window plane
		if(f > 5) {
			const uint8_t x = dir ? (20 - (f-5)) | 0x80 : f-5;
			vdp_set_window(x, 0);
		}
		ready = TRUE;
		aftervsync();
	}
	vdp_colors(0, PAL_FadeOut, 64);
	vdp_set_window(0, 0);
}

void start_fadein_wipe(uint8_t dir) {
	// I guess it would be better to use a union or "work" structs, this is pretty dirty cast
	Sprite *spr0 = (Sprite*) dtiles[0];
	Sprite *spr1 = (Sprite*) dtiles[1];
	Sprite *spr2 = (Sprite*) dtiles[2];
	fade_setup(spr0, spr1, spr2, dir, TRUE);
	wipeFadeTimer = ScreenWidth / 16 + 6;
	wipeFadeDir = dir;
	// Cover screen with window (black) and write target palette now
	sys_wait_vblank(); // Delay until vblank so we don't get the CRAM dots
	vdp_set_window(20, 0);
	vdp_colors_apply_next_now();
}

void update_fadein_wipe(void) {
	wipeFadeTimer--;
	if(wipeFadeTimer >= 0) {
		Sprite *spr0 = (Sprite*) dtiles[0];
		Sprite *spr1 = (Sprite*) dtiles[1];
		Sprite *spr2 = (Sprite*) dtiles[2];
		// Update Sprites
		for(uint16_t i = 0; i < (pal_mode ? 8 : 7); i++) {
			spr0[i].x += wipeFadeDir ? -16 : 16;
			spr1[i].x += wipeFadeDir ? -16 : 16;
			spr2[i].x += wipeFadeDir ? -16 : 16;
		}
		vdp_sprites_add(spr0, pal_mode ? 8 : 7);
		vdp_sprites_add(spr1, pal_mode ? 8 : 7);
		vdp_sprites_add(spr2, pal_mode ? 8 : 7);
		// Update window plane
		if(wipeFadeTimer > 5) {
			const uint8_t x = wipeFadeDir ? (wipeFadeTimer-5) : (20 - (wipeFadeTimer-5)) | 0x80;
			vdp_set_window(x, 0);
		} else {
			vdp_set_window(0, 0);
		}
	} else {
		// After the fade is done, need to restore HUD tiles we clobbered
		hud_force_redraw();
	}
}
