#include "common.h"

#include "ai.h"
#include "audio.h"
#include "camera.h"
#include "dma.h"
#include "effect.h"
#include "entity.h"
#include "hud.h"
#include "input.h"
#include "joy.h"
#include "npc.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "sprite.h"
#include "stage.h"
#include "string.h"
#include "system.h"
#include "tables.h"
#include "tsc.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_pal.h"
#include "vdp_tile.h"
#include "vdp_ext.h"
#include "weapon.h"
#include "window.h"

#include "gamemode.h"

void intro_main() {
	gamemode = GM_INTRO;
	// Init screen stuff
	VDP_setCachedPalette(PAL0, PAL_Main.data);
	VDP_setCachedPalette(PAL1, PAL_Intro.data);
	// Init some subsystems used
	sheets_load_intro();
	sprites_clear();
	effects_init();
	camera_init();
	tsc_init();
	camera.x += 6 << CSF; // Cancel the offset, we want to use absolute positions
	camera.target = NULL;
	stage_load(STAGE_INTRO);
	tsc_call_event(100);
	// Create "Studio Pixel Presents" text
	VDP_drawText("Studio Pixel Presents", 10, 8);
	
	uint16_t timer = 0;
	while(++timer <= TIME_10(400) && !joy_pressed(BUTTON_C) && !joy_pressed(BUTTON_START)) {
		input_update();
		if(timer == TIME_8(100)) VDP_clearText(10, 8, 22);
		tsc_update();
		entities_update(TRUE);
		effects_update(); // Draw Smoke
		ready = TRUE;
		vsync();
		aftervsync();
	}
	input_update(); // This pushes the joy state to avoid skipping the next menu
	VDP_fadeTo(0, 63, PAL_FadeOut, 20, FALSE);
	entities_clear();
	effects_clear();
}
