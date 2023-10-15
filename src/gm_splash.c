#include "common.h"

#include "audio.h"
#include "camera.h"
#include "md/sys.h"
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
#include "md/vdp.h"
#include "res/pal.h"
#include "weapon.h"
#include "window.h"

#include "gamemode.h"

void splash_main() {
	gamemode = GM_SPLASH;
	
	// Init screen stuff
	vdp_colors(0, PAL_Sega, 16);
	vdp_colors(16, PAL_Sym, 16);
	// Init some subsystems used
	sheets_load_splash();
	effects_init();
	camera_init();
	camera.y -= 8 << CSF; // Cancel the offset, we want to use absolute positions
	camera.target = NULL;
	// Create Balrog entity
	Entity *blg = entity_create(pixel_to_sub(ScreenHalfW), -pixel_to_sub(16), OBJ_BALROGSPLASH, 0);
	blg->linkedEntity = entity_create(pixel_to_sub(ScreenHalfW), pixel_to_sub(ScreenHalfH), OBJ_SEGALOGO, 0);
	
	uint16_t timer = 0;
    joystate_old = ~0;
	while(++timer <= TIME(250) && !joy_pressed(JOY_C) && !joy_pressed(JOY_START)) {
		entities_update(TRUE);
		effects_update(); // Draw Smoke
		ready = TRUE;
		sys_wait_vblank();
		aftervsync();
	}
	vdp_fade(NULL, PAL_FadeOut, 4, FALSE);
	entities_clear();
	effects_clear();
    vdp_sprites_clear();
}
