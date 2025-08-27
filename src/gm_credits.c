#include "common.h"

#include "audio.h"
#include "res/system.h"
#include "res/local.h"
#include "res/pal.h"
#include "camera.h"
#include "md/dma.h"
#include "effect.h"
#include "entity.h"
#include "md/comp.h"
#include "md/sys.h"
#include "md/vdp.h"
#include "md/xgm.h"
#include "md/joy.h"
#include "system.h"
#include "cjk.h"
#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

#include "gamemode.h"

#define TILE_ICONINDEX	(TILE_SHEETINDEX + 24*6 + 228)

// Determines which member of the union in credits_info is used
enum CreditCmd {
	TEXT,		// Add a string of text ([)
	ICON,		// Show an icon from casts.pbm (])
	WAIT,		// Delay until next instruction (-)
	MOVE,		// Sets the text X position (+)
	SONG,		// Change the music (!)
	SONG_FADE,	// Fade out music (~)
	FLAG_JUMP,	// Jump to label if a flag is set (f)
	JUMP,		// Jump to label (j)
	LABEL,		// Label that can be jumped to (l)
	PALETTE,	// Swap one of the palettes (no CS equivalent)
	LOADPXE,    // Load 0.pxe
	TEXTMODE,
	END,		// Stop scrolling and pause indefinitely (/)
};

static const SpriteDef *casts_spr[4] = {
	&SPR_CastsSym, &SPR_CastsSym, &SPR_Casts, &SPR_Casts,
};

extern int16_t hscrollTable[64]; // stage.c

static int8_t illScrolling;

__attribute__((noreturn))
void credits_main(void) {
	gamemode = GM_CREDITS;
	
	Sprite icon[16] = {0};
	
	uint16_t pc = 0;
	uint16_t textX = 0, textY = 0;
	
	uint16_t waitTime = 0;
	uint16_t backScroll = 0;
	uint16_t illScroll = 0;
	
	uint8_t skipScroll = 0;
	uint8_t opaqueTextMode = FALSE;

    cjk_reset(CJK_CREDITS);

	inFade = FALSE;
	ready = TRUE;
	vdp_sprites_clear();
	entities_clear();
	effects_init();
	vdp_set_window(0, 0);
    sys_wait_vblank(); aftervsync(); // Make sure nothing in DMA queue and music completely stops
	
	vdp_set_display(FALSE);
	// Clear planes, reset palettes
	vdp_map_clear(VDP_PLANE_A);
	vdp_map_clear(VDP_PLANE_B);
	vdp_colors(0, PAL_Main, 16);
	vdp_colors(16, PAL_Sym, 16);
	vdp_colors(48, PAL_Regu, 16);
	// Stick camera to upper right
	camera.target = NULL;
	camera.x = pixel_to_sub(ScreenHalfW);
	camera.y = pixel_to_sub(ScreenHalfH);
	camera.x_offset = camera.y_offset = 0;
	camera.x_shifted = camera.y_shifted = 0;
	camera_xmin = camera_ymin = 0;
	// Reset background and scrolling
	vdp_color(0, 0x200); // Dark blue background
	vdp_set_backcolor(0);
	g_stage.back_id = 0;
	g_stage.back_type = 0;
	vdp_set_scrollmode(HSCROLL_PLANE, VSCROLL_PLANE);
	vdp_hscroll(VDP_PLANE_B, 0);
	vdp_vscroll(VDP_PLANE_B, 0);
	vdp_hscroll(VDP_PLANE_A, 0);
	vdp_vscroll(VDP_PLANE_A, 0);
	// Text on background plane, priority 1
	vdp_font_load(UFTC_SysFont);
	tsc_load_stage(ID_CREDITS); // credits.tsb
	tsc_call_event(100);
	
	textY = (ScreenHeight >> 3) + (backScroll >> 3) + 1;

	if(cfg_language != LANG_EN) {
		system_set_flag(FLAG_CREDITS_NOAGT, TRUE);
	}
	
	vdp_set_display(TRUE);
	
    while(TRUE) {
        joystate = joystate_old = 0;
		tsc_update();
		if(opaqueTextMode) {
			vdp_color(1, 0x200);
			vdp_color(2, 0x200);
			vdp_color_next(1, 0x200);
			vdp_color_next(2, 0x200);
		}

        backScroll++;
        uint8_t scrolledBack = FALSE;
        if(!use_pal_speed) {
            // Slow the scrolling down slightly for NTSC
            skipScroll++;
            if(skipScroll == 6) {
                backScroll--;
                waitTime++;
                skipScroll = 0;
                scrolledBack = TRUE;
            }
        }
        if((backScroll & 15) == 0 && !scrolledBack) {
            textY++;
			if(opaqueTextMode) {
				const uint16_t map = TILE_ATTR(0,1,0,0,TILE_FADEINDEX);
				vdp_text_clear_ex(VDP_PLANE_B, 0, (textY + 1) & 31, 20, map);
				vdp_text_clear_ex(VDP_PLANE_B, 20, (textY + 1) & 31, 20, 0);
			} else {
				const uint16_t map = TILE_ATTR(0,1,0,0,opaqueTextMode?TILE_FADEINDEX:0);
				vdp_text_clear_ex(VDP_PLANE_B, 0, (textY + 1) & 31, 40, map);
			}
			
        }

		if(waitTime) waitTime--;
		while(!waitTime) {
			uint16_t label = 0;
			switch(credits_info[pc].cmd) {
				case TEXT:
					if(credits_info[pc].text.jstring != 0) {
						const uint8_t *str = (const uint8_t*)CREDITS_STR;
						str += (credits_info[pc].text.jstring - 1) << 5;
						if(str[0] != '_') { // Ignore lines starting with "_"
							loc_vdp_nputs(VDP_PLANE_B, str, textX, textY & 31, 32, opaqueTextMode ? 2 : 0);
						}
					}
					break;
				case ICON:
					for(uint8_t i = 0; i < 16; i++) {
						if(icon[i].size) continue;
						icon[i] = (Sprite) {
							.x = textX * 8 - 24 + 128,
							.y = ScreenHeight - 3 + 128,
							.size = SPRITE_SIZE(3, 3),
							.attr = TILE_ATTR(credits_info[pc].icon.pal,
									1,0,0,TILE_ICONINDEX + i * 9)
						};
						if(textX == 16) icon[i].x += 4;
						if(cfg_language >= LANG_JA && cfg_language <= LANG_RU) icon[i].y -= 4;
						TILES_QUEUE(SPR_TILES(casts_spr[credits_info[pc].icon.pal], 
								credits_info[pc].icon.id), TILE_ICONINDEX + i * 9, 9);
						break;
					}
					break;
				case WAIT:
					waitTime = credits_info[pc].wait.ticks;
					break;
				case MOVE:
					textX = credits_info[pc].move.pos >> 3;
					break;
				case SONG:
					song_play(credits_info[pc].song.id);
					break;
				case SONG_FADE:
					song_stop(); // TODO: Figure out how to fade
					break;
				case FLAG_JUMP:
					if(!system_get_flag(credits_info[pc].fjump.flag)) break;
					label = credits_info[pc].fjump.label;
					/* fallthrough */
				case JUMP:
				{
					if(!label) label = credits_info[pc].jump.label;
					do {
						pc++;
					} while(credits_info[pc].cmd != LABEL || credits_info[pc].label.value != label);
				}
				break;
				case LABEL: /* Ignore */ break;
				case PALETTE: /* Unused */ break;
				case LOADPXE:
                    g_stage.id = 0;
                    stage_load_entities();
                    break;
				case TEXTMODE:
				{
					if(credits_info[pc].textmode.value == 1) {
						opaqueTextMode = TRUE;
						if(cfg_language < LANG_JA || cfg_language >= LANG_RU) {
							vdp_font_load((const uint32_t*)*TS_MSGTEXT);
						}
						vdp_color(1, 0x200);
						vdp_color(2, 0x200);
					} else if(credits_info[pc].textmode.value == 0) {
						opaqueTextMode = FALSE;
						if(cfg_language < LANG_JA) {
							vdp_font_load(UFTC_SysFont);
						} else if(cfg_language >= LANG_RU && cfg_language <= LANG_UA) {
							vdp_font_load(UFTC_SysFontRU);
						}
					} else {
						vdp_color(1, 0x000);
						vdp_color(2, 0x400);
					}
				}
				break;
				// The End
				case END:
				    while(TRUE) {
                        // It's possible for the credits to end before the script
                        // So keep updating the script & illustration scrolling
                        tsc_update();

						effects_update();
                        entities_update(TRUE);

                        // Scrolling for illustrations
                        illScroll += illScrolling;
                        if(illScroll <= 0 || illScroll >= 160) illScrolling = 0;
                        
						vdp_hscroll(VDP_PLANE_A, illScroll);
						ready = TRUE;
                        sys_wait_vblank(); aftervsync();
                    }
				    break;
			}
			pc++;
		}
		
		effects_update();
		entities_update(TRUE);

		// Scrolling for illustrations
		illScroll += illScrolling;
		if(illScroll <= 0 || illScroll >= 160) illScrolling = 0;
		// Icon sprites
		for(uint8_t i = 0; i < 16; i++) {
			if(!icon[i].size) continue;
			if((backScroll & 1) == 0 && !scrolledBack) {
				if(--icon[i].y < -22 + 128) icon[i].size = 0;
			}
			vdp_sprite_add(&icon[i]);
		}
		
		// Hscroll is only really updated in Balcony, otherwise used for illustrations
		if(g_stage.id == STAGE_ENDING_BALCONY) {
			g_stage.back_scroll_timer--;
			hscrollTable[0] = 0;
			uint16_t y = pal_mode ? 29 : 27;
			uint16_t off = pal_mode ? 1 : 0;
			while(y >= 21 + off) {
				hscrollTable[y] = hscrollTable[0];
				hscrollTable[y+32] = g_stage.back_scroll_timer << 1;
				y--;
			}
			while(y >= 17 + off) {
				hscrollTable[y] = hscrollTable[0];
				hscrollTable[y+32] = g_stage.back_scroll_timer;
				y--;
			}
			while(y >= 14 + off) {
				hscrollTable[y] = hscrollTable[0];
				hscrollTable[y+32] = g_stage.back_scroll_timer >> 1;
				y--;
			}
			while(y >= 10 + off) {
				hscrollTable[y] = hscrollTable[0];
				hscrollTable[y+32] = g_stage.back_scroll_timer >> 2;
				y--;
			}
			while(y >= 1) {
				hscrollTable[y] = hscrollTable[0];
				y--;
			}
		}
		
		ready = TRUE;
        sys_wait_vblank();
		if(g_stage.id == STAGE_ENDING_BALCONY) {
			// Reverse order from stage.c, we are scrolling Plane A not B
			dma_now(DmaVRAM, (uint32_t) &hscrollTable[32], VDP_HSCROLL_TABLE, pal_mode ? 30 : 28, 32);
			dma_now(DmaVRAM, (uint32_t) hscrollTable, VDP_HSCROLL_TABLE+2, pal_mode ? 30 : 28, 32);
		} else {
			vdp_hscroll(VDP_PLANE_A, illScroll);
		}
		vdp_vscroll(VDP_PLANE_B, backScroll >> 1);
		aftervsync();
    }
}

void credits_show_image(uint16_t id) {
	if(id > 19) return;
	if(illustration_info[id].pat == NULL) return; // Can't draw null tileset

	vdp_set_display(FALSE);
	dma_queue_rom(DmaVRAM, (uint32_t) illustration_info[id].pat, 16*32, illustration_info[id].pat_size*16, 2);
	dma_flush();
	//vdp_tiles_load(illustration_info[id].pat, 16, illustration_info[id].pat_size);
	uint16_t index = pal_mode ? 0 : 20;
	uint8_t set_pal = FALSE;
	vblank = 0;
	for(uint16_t y = 0; y < (pal_mode ? 30 : 28); y++) {
		dma_queue_rom(DmaVRAM, (uint32_t) &illustration_info[id].map[index], VDP_PLANE_A + (y << 7) + (44 << 1), 20, 2);
		if(!set_pal && vblank) {
			vdp_colors(32, illustration_info[id].palette, 16);
			set_pal = TRUE;
		}
		dma_flush();
		index += 20;
	}
	if(!set_pal) {
		vdp_colors(32, illustration_info[id].palette, 16);
	}
	vdp_set_display(TRUE);

    illScrolling = 8;
}

void credits_clear_image(void) {
	illScrolling = -8;
}
