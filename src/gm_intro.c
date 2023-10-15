#include "common.h"

#include "audio.h"
#include "camera.h"
#include "md/dma.h"
#include "effect.h"
#include "entity.h"
#include "hud.h"
#include "md/joy.h"
#include "npc.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "system.h"
#include "tables.h"
#include "tsc.h"
#include "md/sys.h"
#include "md/vdp.h"
#include "weapon.h"
#include "window.h"
#include "md/xgm.h"

#include "gamemode.h"

void intro_main() {
	gamemode = GM_INTRO;
	// Init some subsystems used
	sheets_load_intro();
	vdp_sprites_clear();
	effects_init();
	camera_init();
    camera.target = NULL;
	tsc_init();
	stage_load(STAGE_INTRO);
	//vdp_colors_next(16, PAL_Intro.data, 16);
	tsc_call_event(100);
	// Create "Studio Pixel Presents" text
	//vdp_puts(VDP_PLANE_A, "Studio Pixel Presents", 10, 8);
    vdp_puts(VDP_PLANE_A, "Based on the Work of", 10, 6);
    vdp_puts(VDP_PLANE_A, "    Studio Pixel    ", 10, 8);
	
	uint16_t timer = 0;
    joystate_old = ~0;
	while(++timer <= TIME_10(400) && !joy_pressed(JOY_C) && !joy_pressed(JOY_START)) {
		if(timer == TIME_8(150)) {
            vdp_text_clear(VDP_PLANE_A, 10, 6, 20);
            vdp_text_clear(VDP_PLANE_A, 10, 8, 20);
		}
		camera_update();
		tsc_update();
		entities_update(TRUE);
		effects_update(); // Draw Smoke
		ready = TRUE;
		sys_wait_vblank(); aftervsync();
	}
	vdp_fade(NULL, PAL_FadeOut, 4, FALSE);
	entities_clear();
	effects_clear();
	// Get rid of the sprites
    vdp_sprites_clear();
	vdp_sprites_update();
	dma_flush();
}
