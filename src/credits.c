#include "credits.h"

#include <genesis.h>
//#include "sprite.h"
#include "resources.h"
#include "input.h"
#include "tables.h"
#include "audio.h"

void credits_main(u8 ending) {
	SYS_disableInts();
	VDP_setEnable(false);
	//sprites_clear();
	song_stop();
	VDP_resetScreen();
	VDP_setPalette(PAL0, PAL_Main.data);
	//VDP_setPalette(PAL1, PAL_Credits.data);
	VDP_drawText("Nothing here yet.", 4, 8);
	VDP_setEnable(true);
	SYS_enableInts();
    while(true) {
		input_update();
		if(joy_pressed(BUTTON_START) || joy_pressed(BUTTON_C)) {
			break;
		}
		VDP_waitVSync();
    }
}
