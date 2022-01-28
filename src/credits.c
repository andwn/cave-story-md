#include "common.h"

#include "audio.h"
#include "bank_data.h"
#include "camera.h"
#include "dma.h"
#include "effect.h"
#include "entity.h"
#include "joy.h"
#include "kanji.h"
#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "system.h"
#include "tables.h"
#include "tsc.h"
#include "tools.h"
#include "vdp.h"
#include "xgm.h"

#include "gamemode.h"

#define TILE_ICONINDEX	(TILE_SHEETINDEX + 24*6 + 228)

enum CreditCmd { 
	TEXT, ICON, WAIT, MOVE, SONG, SONG_FADE, FLAG_JUMP, JUMP, LABEL, PALETTE, LOADPXE, END
};

static int8_t illScrolling = 0;

static void draw_jp_text(const uint8_t *str, uint16_t x, uint16_t y) {
    uint16_t iter = 0;
	for(uint16_t i = 0; i < 32; i++) {
		if(str[i] >= 0xE0) {
			uint16_t c = (str[i] - 0xE0) * 0x60 + (str[i+1] - 0x20);
            cjk_draw(VDP_PLAN_B, c + 0x100, x, y, 0, TRUE);
			i++;
		} else if(str[i] >= 0x20) {
            cjk_draw(VDP_PLAN_B, str[i], x, y, 0, TRUE);
		} else {
			break;
		}
		x += 1 + (iter & 1);
        iter++;
	}
    cjk_newline();
}

void credits_main() {
	gamemode = GM_CREDITS;
	
	VDPSprite icon[16] = {0};
	
	uint16_t pc = 0;
	uint16_t textX = 0, textY = 0;
	
	uint16_t waitTime = 0;
	uint16_t backScroll = 0;
	uint16_t illScroll = 0;
	
	uint8_t skipScroll = 0;

    cjk_reset(CJK_CREDITS);

	inFade = FALSE;
	ready = TRUE;
	vdp_sprites_clear();
	entities_clear();
	effects_init();
	vdp_set_window(0, 0);
	vdp_vsync(); aftervsync(); // Make sure nothing in DMA queue and music completely stops
	
	vdp_set_display(FALSE);
	// Clear planes, reset palettes
	vdp_map_clear(VDP_PLAN_A);
	vdp_map_clear(VDP_PLAN_B);
	vdp_colors(0, PAL_Main.data, 16);
	vdp_colors(16, PAL_Sym.data, 16);
	vdp_colors(48, PAL_Regu.data, 16);
	// Stick camera to upper right
	camera.target = NULL;
	camera.x = pixel_to_sub(SCREEN_HALF_W);
	camera.y = pixel_to_sub(SCREEN_HALF_H);
	camera.x_offset = camera.y_offset = 0;
	camera.x_shifted = camera.y_shifted = 0;
	camera_xmin = camera_ymin = 0;
	// Reset background and scrolling
	vdp_color(0, 0x200); // Dark blue background
	vdp_set_backcolor(0);
	stageBackground = stageBackgroundType = 0;
	vdp_set_scrollmode(HSCROLL_PLANE, VSCROLL_PLANE);
	vdp_hscroll(VDP_PLAN_B, 0);
	vdp_vscroll(VDP_PLAN_B, 0);
	vdp_hscroll(VDP_PLAN_A, 0);
	vdp_vscroll(VDP_PLAN_A, 0);
	// Text on background plane, priority 1
	vdp_font_load(TS_SysFont.tiles);
	//VDP_setTextPlan(VDP_PLAN_B);
	//VDP_setTextPriority(1);
	//ssf_setbank(7, 7); // Illustrations are in bank 7
	tsc_load_stage(ID_CREDITS); // credits.tsb
	tsc_call_event(100);
	
	textY = (SCREEN_HEIGHT >> 3) + (backScroll >> 3) + 1;
	
	vdp_set_display(TRUE);
	
    while(TRUE) {
		joystate = oldstate = 0;
		tsc_update();

        backScroll++;
        uint8_t scrolledBack = FALSE;
        if(!pal_mode && !cfg_60fps) {
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
            disable_ints;
            z80_request();
            //vdp_text_clear(VDP_PLAN_B, 0, textY & 31, 40);
            vdp_text_clear(VDP_PLAN_B, 0, (textY + 1) & 31, 40);
            z80_release();
            enable_ints;
        }

		if(waitTime) waitTime--;
		while(!waitTime) {
			uint16_t label = 0;
			switch(credits_info[pc].cmd) {
				case TEXT:
					if(cfg_language >= LANG_JA && cfg_language <= LANG_KO) {
						if(credits_info[pc].text.jstring != 0) {
							const uint8_t *str = (const uint8_t*)CREDITS_STR;
							str += (credits_info[pc].text.jstring - 1) << 5;
							draw_jp_text(str, textX, textY & 31);
						}
					} else {
						vdp_puts(VDP_PLAN_B, credits_info[pc].text.string, textX, textY & 31);
					}
					break;
				case ICON:
					for(uint8_t i = 0; i < 16; i++) {
						if(icon[i].size) continue;
						icon[i] = (VDPSprite) {
							.x = textX * 8 - 22 + 128,
							.y = SCREEN_HEIGHT - 6 + 128,
							.size = SPRITE_SIZE(3, 3),
							.attr = TILE_ATTR(credits_info[pc].icon.pal,
									1,0,0,TILE_ICONINDEX + i * 9)
						};
						TILES_QUEUE(SPR_TILES(&SPR_Casts, 
								credits_info[pc].icon.id, 0), TILE_ICONINDEX + i * 9, 9);
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
                    stageID = 0;
                    stage_load_entities();
                    break;
				// The End
				case END:
				    while(TRUE) {
                        // It's possible for the credits to end before the script
                        // So keep updating the script & illustration scrolling
                        tsc_update();
                        // Scrolling for illustrations
                        illScroll += illScrolling;
                        if(illScroll <= 0 || illScroll >= 160) illScrolling = 0;
                        effects_update();
                        entities_update(TRUE);
                        vdp_vsync(); aftervsync();
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
		
		vdp_vscroll(VDP_PLAN_B, backScroll >> 1);
		vdp_hscroll(VDP_PLAN_A, illScroll);
		ready = TRUE;
		vdp_vsync(); aftervsync();
    }
}

void credits_show_image(uint16_t id) {
	if(id > 19) return;
	if(illustration_info[id].pat == NULL) return; // Can't draw null tileset
	vdp_set_display(FALSE);
	vdp_colors(32, illustration_info[id].palette->data, 16);
	vdp_tiles_load_from_rom(illustration_info[id].pat, 16, illustration_info[id].pat_size);
	uint16_t index = pal_mode ? 0 : 20;
	for(uint16_t y = 0; y < (pal_mode ? 30 : 28); y++) {
		DMA_doDma(DMA_VRAM, (uint32_t) &illustration_info[id].map[index], VDP_PLAN_A + (y << 7) + (44 << 1), 20, 2);
		index += 20;
	}
	vdp_set_display(TRUE);
	illScrolling = 8;
}

void credits_clear_image() {
	illScrolling = -8;
}
