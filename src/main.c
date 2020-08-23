#include "common.h"

#include "audio.h"
#include "dma.h"
#include "effect.h"
#include "error.h"
#include "gamemode.h"
#include "joy.h"
#include "memory.h"
#include "resources.h"
#include "stage.h"
#include "system.h"
#include "tools.h"
#include "tsc.h"
#include "vdp.h"
#include "xgm.h"

extern const uint16_t time_tab_ntsc[0x400];
extern const int16_t speed_tab_ntsc[0x400];
extern const uint16_t time_tab_pal[0x400];
extern const int16_t speed_tab_pal[0x400];

void aftervsync() {
	disable_ints;
    z80_request();

	DMA_flushQueue();
	vdp_fade_step();
	dqueued = FALSE;
	if(ready) {
		if(inFade) vdp_sprites_clear();
		vdp_sprites_update();
		ready = FALSE;
	}
	if(gamemode == GM_GAME) stage_update(); // Scrolling
	//joy_update();
    
    z80_release();
    enable_ints;

    joy_update();
}

int main() {
    setRandomSeed(0xC427); // initiate random number generator
    mem_init();
    vdp_init();
    DMA_init(0, 0);
    // VInt unconditionally does XGM processing
    // joy_init depends on the vblank complete flag to wait a few frames
    // Therefore, the next 3 lines MUST be in this exact order
    xgm_init();
    enable_ints;
	joy_init();
    // Initialize time and speed tables (framerate adjusted)
    if(pal_mode) {
		//for(uint16_t i = 0; i < 0x100; i++) {
		//	time_tab[i] = i;
		//	speed_tab[i] = i;
		//}
		time_tab = time_tab_pal;
        speed_tab = speed_tab_pal;
	} else {
		//for(uint16_t i = 0; i < 0x100; i++) {
		//	time_tab[i] = time_tab_ntsc[i];
		//	speed_tab[i] = speed_tab_ntsc[i];
		//}
        time_tab = time_tab_ntsc;
        speed_tab = speed_tab_ntsc;
	}
    // let's the fun go on !
    
    sound_init();
	
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
