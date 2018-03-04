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

void splash_main() {
	gamemode = GM_SPLASH;
	
	// Init screen stuff
	VDP_setPalette(PAL0, PAL_Sega.data);
	VDP_setPalette(PAL1, PAL_Sym.data);
	// Init some subsystems used
	sheets_load_splash();
	effects_init();
	camera_init();
	camera.y -= 8 << CSF; // Cancel the offset, we want to use absolute positions
	camera.target = NULL;
	// Create Balrog entity
	Entity *blg = entity_create(SCREEN_HALF_W<<CSF, -(16<<CSF), OBJ_BALROGSPLASH, 0);
	blg->linkedEntity = entity_create(SCREEN_HALF_W<<CSF, SCREEN_HALF_H<<CSF, OBJ_SEGALOGO, 0);
	
	uint16_t timer = 0;
	while(++timer <= TIME(250) && !joy_pressed(BUTTON_C) && !joy_pressed(BUTTON_START)) {
		input_update();
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
