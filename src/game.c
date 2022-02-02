#include "common.h"

#include "audio.h"
#include "camera.h"
#include "dma.h"
#include "effect.h"
#include "entity.h"
#include "error.h"
#include "hud.h"
#include "joy.h"
#include "pause.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "string.h"
#include "system.h"
#include "tables.h"
#include "tools.h"
#include "tsc.h"
#include "vdp.h"
#include "weapon.h"
#include "window.h"
#include "xgm.h"

#include "gamemode.h"

// Initializes or re-initializes the game after "try again"
void game_reset(uint8_t load);

void game_main(uint8_t load) {
	gamemode = GM_GAME;
	
	vdp_colors(0, PAL_FadeOut, 64);
	//vdp_color(15, 0x000);
	// This is the SGDK font with a blue background for the message window
	if(cfg_language != LANG_JA) {
		vdp_font_load(TS_MsgFont.tiles);
	}
	effects_init();
	game_reset(load);
	vdp_set_window(0, 0);
	// Load game doesn't run a script that fades in and shows the HUD, so do it manually
	if(load) {
		hud_show();
		stage_setup_palettes();
		vdp_fade(PAL_FadeOut, NULL, 4, TRUE);
	}
	paused = FALSE;

	while(TRUE) {
		PF_BGCOLOR(0x000);
		//#ifdef PROFILE_BG
		//vdp_set_backcolor(0);
		//#endif

		if(paused) {
			paused = update_pause();
		} else {
			// Pressing start opens the item menu (unless a script is running)
			if(joy_pressed(btn[cfg_btn_pause]) && !tscState) {
				// This unloads the stage's script and loads the "ArmsItem" script in its place
				tsc_load_stage(255);
				draw_itemmenu(TRUE);
				paused = TRUE;
			} else if(joy_pressed(btn[cfg_btn_map]) && joytype == JOY_TYPE_PAD6 
					&& !tscState && (playerEquipment & EQUIP_MAPSYSTEM)) {
				// Shorthand to open map system
                disable_ints;
                z80_request();
				vdp_set_display(FALSE);
				if(stageBackgroundType == 4) {
					// Hide water
					static const uint32_t black[8] = {
						0x11111111,0x11111111,0x11111111,0x11111111,
						0x11111111,0x11111111,0x11111111,0x11111111
					};
					vdp_tiles_load_from_rom(black, TILE_FACEINDEX, 1);
					vdp_map_fill_rect(VDP_PLAN_W, TILE_ATTR(PAL0,1,0,0,TILE_FACEINDEX), 0, 0, 40, 30, 0);
				} else {
					vdp_map_clear(VDP_PLAN_W);
				}
				vdp_set_window(0, pal_mode ? 30 : 28);
				vdp_set_display(TRUE);
                z80_release();
                enable_ints;

				paused = TRUE; // This will stop the counter in Hell
				do_map();
				paused = FALSE;
				vdp_set_display(FALSE);
				hud_force_redraw();

                disable_ints;
                z80_request();
				sheets_load_stage(stageID, TRUE, FALSE);
                z80_release();
                enable_ints;

				player_draw();
				entities_draw();
				hud_show();
				vdp_sprites_update();
				vdp_set_window(0, 0);
				vdp_set_display(TRUE);
			} else {
				// HUD on top
				hud_update();
				// Boss health, camera
				if(!gameFrozen) {
					if(showingBossHealth) tsc_update_boss_health();
					camera_update();
				}
				// Run the next set of commands in a script if it is running
				uint8_t rtn = tsc_update();
				// Nonzero return values exit the game, or switch to the ending sequence
				if(rtn > 0) {
					if(rtn == 1) { // Return to title screen
						SYS_hardReset();
					} else if(rtn == 2) {
						vdp_colors(0, PAL_FadeOut, 64);
						vdp_color(15, 0x000);
						stageBackground = 255; // Force background redraw
						game_reset(TRUE); // Reload save
						hud_show();
						playerIFrames = 0;
						stage_setup_palettes();
						vdp_fade(NULL, NULL, 4, TRUE);
						continue;
					} else if(rtn == 3) {
						vdp_colors(0, PAL_FadeOut, 64);
						vdp_color(15, 0x000);
						game_reset(FALSE); // Start from beginning
						continue;
					} else { // End credits
						break;
					}
				}
				window_update();
				// Handle controller locking
				uint16_t lockstate = joystate, oldlockstate = oldstate;
				if(controlsLocked) joystate = oldstate = 0;
				// Don't update this stuff if a script is using <PRI
				effects_update();
				if(!gameFrozen) {
					player_update();
					entities_update(TRUE);
				} else {
					player_draw();
					entities_draw();
				}
				// Restore controller locking if it was locked
				joystate = lockstate;
				oldstate = oldlockstate;
			}
		}
		PF_BGCOLOR(0xEEE);
		system_update();
		ready = TRUE;
		PF_BGCOLOR(0x000);
		vdp_vsync();
		PF_BGCOLOR(0x00E);
		aftervsync();
	}
}

void game_reset(uint8_t load) {
	vdp_map_clear(VDP_PLAN_B);
	camera_init();
	tsc_init();
	hud_create();
	// Default sprite sheets
	sheets_load_stage(255, TRUE, TRUE);
	
	gameFrozen = FALSE;
	if(load >= 4) {
		system_load_levelselect(load - 4);
	} else {
		system_load(sram_file);
	}
	const SpriteDefinition *wepSpr = weapon_info[playerWeapon[currentWeapon].type].sprite;
	if(wepSpr) TILES_QUEUE(SPR_TILES(wepSpr,0,0), TILE_WEAPONINDEX,6);
	
	SHEET_LOAD(&SPR_Bonk, 1, 1, 1, 1, 0,0);
	SHEET_LOAD(&SPR_QMark, 1, 1, TILE_QMARKINDEX, 1, 0,0);
	// Load up the main palettes
	//vdp_colors_next(0, PAL_Main.data, 16);
	//vdp_colors_next(16, PAL_Sym.data, 16);
	//vdp_colors(0, PAL_FadeOut, 64);
}
