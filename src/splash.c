#include "gamemode.h"

#include <genesis.h>
#include "audio.h"
#include "camera.h"
#include "common.h"
#include "effect.h"
#include "entity.h"
#include "input.h"
#include "npc.h"
#include "resources.h"
#include "sheet.h"
#include "sprite.h"
#include "system.h"
#include "tables.h"
#include "vdp_ext.h"

void splash_vblank() {
	sprites_send();
}

void splash_main() {
	//return; // lol nah
	
	SYS_disableInts();
	VDP_setEnable(FALSE);
	VDP_resetScreen();
	// Init screen stuff
	SYS_setVIntCallback(&splash_vblank);
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
	
	VDP_setEnable(TRUE);
	SYS_enableInts();
	u16 timer = 0;
	while(++timer <= TIME(250) && !joy_pressed(BUTTON_C) && !joy_pressed(BUTTON_START)) {
		input_update();
		entities_update();
		entities_draw();
		effects_update(); // Draw Smoke
		VDP_waitVSync();
	}
	input_update(); // This pushes the joy state to avoid skipping the next menu
	SYS_setVIntCallback(NULL);
	VDP_fadeTo(0, 63, PAL_FadeOut, 20, FALSE);
	entities_clear();
	effects_clear();
}
