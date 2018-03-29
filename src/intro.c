#include "common.h"

#include "audio.h"
#include "camera.h"
#include "dma.h"
#include "effect.h"
#include "entity.h"
#include "hud.h"
#include "joy.h"
#include "npc.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "string.h"
#include "system.h"
#include "tables.h"
#include "tsc.h"
#include "vdp.h"
#include "weapon.h"
#include "window.h"

#include "gamemode.h"

void intro_main() {
	gamemode = GM_INTRO;
	// Init some subsystems used
	sheets_load_intro();
	vdp_sprites_clear();
	effects_init();
	camera_init();
	tsc_init();
	camera.x += 6 << CSF; // Cancel the offset, we want to use absolute positions
	camera.target = NULL;
	stage_load(STAGE_INTRO);
	//vdp_colors_next(16, PAL_Intro.data, 16);
	tsc_call_event(100);
	// Create "Studio Pixel Presents" text
	vdp_puts(VDP_PLAN_A, "Studio Pixel Presents", 10, 8);
	
	uint16_t timer = 0;
	oldstate = ~0;
	while(++timer <= TIME_10(400) && !joy_pressed(BUTTON_C) && !joy_pressed(BUTTON_START)) {
		if(timer == TIME_8(100)) vdp_text_clear(VDP_PLAN_A, 10, 8, 22);
		tsc_update();
		entities_update(TRUE);
		effects_update(); // Draw Smoke
		ready = TRUE;
		vdp_vsync();
		aftervsync();
	}
	vdp_fade(NULL, PAL_FadeOut, 4, FALSE);
	entities_clear();
	effects_clear();
}
