#include "common.h"

#include "audio.h"
#include "dma.h"
#include "effect.h"
#include "error.h"
#include "gamemode.h"
#include "input.h"
#include "joy.h"
#include "memory.h"
#include "resources.h"
#include "sprite.h"
#include "stage.h"
#include "system.h"
#include "tools.h"
#include "tsc.h"
#include "vdp.h"
#include "vdp_pal.h"
#include "vdp_tile.h"
#include "xgm.h"

extern const uint16_t time_tab_ntsc[0x100];
extern const uint16_t speed_tab_ntsc[0x100];

void vsync() {
	vblank = 0;
	while(!vblank);
	vblank = 0;
}

void aftervsync() {
	xgm_vblank();
	DMA_flushQueue();
	if(fading_cnt > 0) VDP_doStepFading(FALSE);
	dqueued = FALSE;
	if(ready) {
		if(inFade) { spr_num = 0; }
		else { sprites_send(); }
		ready = FALSE;
	}
	if(gamemode == GM_GAME) stage_update(); // Scrolling
	JOY_update();
}

int main() {
	// initiate random number generator
    setRandomSeed(0xC427);
    // enable interrupts
    __asm__("move #0x2500,%sr");
    // init part
    MEM_init();
    VDP_init();
    DMA_init(0, 0);
    JOY_init();
    xgm_init();
    // Initialize time and speed tables (framerate adjusted)
    SCREEN_HEIGHT = pal_mode ? 240 : 224;
	SCREEN_HALF_H = SCREEN_HEIGHT >> 1;
	YCUTOFF = SCREEN_HEIGHT + 32;
	FPS = pal_mode ? 50 : 60;
    if(pal_mode) {
		for(uint16_t i = 0; i < 0x100; i++) {
			time_tab[i] = i;
			speed_tab[i] = i;
		}
	} else {
		for(uint16_t i = 0; i < 0x100; i++) {
			time_tab[i] = time_tab_ntsc[i];
			speed_tab[i] = speed_tab_ntsc[i];
		}
	}
    // let's the fun go on !
    
    sound_init();
	input_init();
	
	// Error Tests
	//__asm__("move.w (1),%d0"); // Address Error
	//__asm__("illegal"); // Illegal Instruction
	//__asm__("divu #0,%d0"); // Zero Divide
	//error_oom(); // Out of memory
	//error_other("Test test!\nTest!"); // Fatal message

	splash_main();
	intro_main();
    while(TRUE) {
		uint8_t select = titlescreen_main();
		if(select == 0) {
			select = saveselect_main();
			if(select >= 4) continue;
			game_main(select);
			credits_main();
		} else if(select == 2) {
			soundtest_main();
		} else if(select == 3) {
			config_main();
		} else {
			game_main(select);
			credits_main();
		}
    }
	return 0;
}
