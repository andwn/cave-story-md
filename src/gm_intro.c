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

void intro_main(void) {
	gamemode = GM_INTRO;
	// Init some subsystems used
	sheets_load_intro();
	vdp_sprites_clear();
	effects_init();
	tsc_init();
	stage_load(STAGE_INTRO);
	camera_init();
	camera.x_shifted = (camera.x >> CSF) - ScreenHalfW;
	camera.y_shifted = (camera.y >> CSF) - ScreenHalfH;
    camera.target = NULL;
	tsc_call_event(100);
	// Moved to Plane B, the window layer is used for the "wipe fade", but can
	// still keep this message on top changing the priority bit for this special case
    vdp_puts(VDP_PLANE_B, "Based on the Work of", 10, 5);
    vdp_puts(VDP_PLANE_B, "    Studio Pixel    ", 10, 7);
	
	uint16_t timer = 0;
    joystate_old = ~0;
	while(++timer <= TIME_10(400) && !joy_pressed(JOY_C) && !joy_pressed(JOY_START)) {
		k_hex8(wipeFadeTimer);
		if(wipeFadeTimer >= 0) {
			update_fadein_wipe();
		}
		if(timer == TIME_8(150)) {
            vdp_text_clear(VDP_PLANE_B, 10, 5, 20);
            vdp_text_clear(VDP_PLANE_B, 10, 7, 20);
		}
		stage_update();
		tsc_update();
		entities_update(TRUE);
		effects_update();
		ready = TRUE;
		sys_wait_vblank();
		aftervsync();
	}
	do_fadeout_wipe(1);
	entities_clear();
	effects_clear();
	// Get rid of the sprites
    vdp_sprites_clear();
	vdp_sprites_update();
	dma_flush();
}
