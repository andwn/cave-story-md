#include "common.h"

#include "audio.h"
#include "res/tiles.h"
#include "res/pal.h"
#include "md/dma.h"
#include "md/joy.h"
#include "system.h"
#include "tables.h"
#include "md/sys.h"
#include "md/vdp.h"
#include "md/xgm.h"

#include "gamemode.h"

static const char hexchars[16] = "0123456789ABCDEF";
static void print_hex(char *str, uint32_t val, uint16_t digits, uint16_t x, uint16_t y) {
	if(digits > 8) digits = 8;
    for(uint16_t i = 0; i < digits; i++) {
        str[digits - i - 1] = hexchars[(val >> (i << 2)) & 0xF];
    }
    str[digits] = 0;
    vdp_puts(VDP_PLANE_A, str, x, y);
}

#define DRAW_BYTE(b, x, y) { \
	char byte[4]; \
	print_hex(byte, b, 2, x, y); \
}

enum { STOPPED, PLAYING, PAUSED };

// Music 0x00 - 0x7F, Sound Effects 0x80 - 0xFF
#define FIRST_SOUND 0x80

// Track # and name
void draw_track_info(uint8_t track) {
	DRAW_BYTE(track, 2, 6);
	vdp_text_clear(VDP_PLANE_A, 5, 6, 33);
	vdp_puts(VDP_PLANE_A, bgm_info[track].name, 5, 6);
}

void draw_status(uint8_t status) {
	switch(status) {
		case STOPPED: vdp_puts(VDP_PLANE_A, "Stopped", 2, 4); break;
		case PLAYING: vdp_puts(VDP_PLANE_A, "Playing", 2, 4); break;
		case PAUSED:  vdp_puts(VDP_PLANE_A, "Paused ", 2, 4); break;
	}
}

void soundtest_main() {
	gamemode = GM_SOUNDTEST;
	
	uint8_t track = 0;
	uint8_t status = STOPPED, oldstatus = STOPPED;

	//disable_ints();
    //z80_pause_fast();
	vdp_set_display(FALSE);
	
	vdp_sprites_clear();
	vdp_map_clear(VDP_PLANE_A);
	// Background picture
	vdp_tiles_load_uftc(UFTC_bkSndTest, 16, 0, 208);
	uint16_t index = pal_mode ? 0 : 80;// << 1;
	for(uint16_t y = 0; y < (pal_mode ? 30 : 28); y++) {
		dma_now(DmaVRAM, (uint32_t) &MAP_bkSndTest[index], VDP_PLANE_B + (y << 7), 40, 2);
		index += 40;// << 1;
	}

	draw_status(status);
	vdp_puts(VDP_PLANE_A, "Sound Test", 15, 2);
	vdp_puts(VDP_PLANE_A, "Track: ", 2, 8);
	{
        vdp_puts(VDP_PLANE_A, btnName[cfg_btn_jump], 2, 15);
        vdp_puts(VDP_PLANE_A, "-Play",  4, 15);
        vdp_puts(VDP_PLANE_A, btnName[cfg_btn_shoot], 10,15);
        vdp_puts(VDP_PLANE_A, "-Stop",  12,15);
        vdp_puts(VDP_PLANE_A, btnName[cfg_btn_pause], 18,15);
        vdp_puts(VDP_PLANE_A, "-Quit",  20,15);
	}
	
	DRAW_BYTE(track, 10, 8);
	
	vdp_colors(0, PAL_Main, 16);
	vdp_colors(16, PAL_SndTest, 16);
	
	vdp_set_display(TRUE);
    //z80_resume();
    //enable_ints();

	song_stop();
    joystate_old = ~0;
    while(TRUE) {
		// Switch between tracks
		// Skip the gap between the last song and first sfx
		if(joy_pressed(JOY_LEFT)) {
			if(track == 0) track = FIRST_SOUND + SOUND_COUNT - 1;
			else if(track == FIRST_SOUND) track = SONG_COUNT - 1;
			else track--;
			DRAW_BYTE(track, 10, 8);
		} else if(joy_pressed(JOY_RIGHT)) {
			if(track == SONG_COUNT - 1) track = FIRST_SOUND;
			else if(track == FIRST_SOUND + SOUND_COUNT - 1) track = 0;
			else track++;
			DRAW_BYTE(track, 10, 8);
		}
		if(joy_pressed(btn[cfg_btn_jump])) {
			// Play
			if(track < SONG_COUNT) {
				song_play(track);
				status = PLAYING;
				draw_track_info(track);
			} else if(track >= FIRST_SOUND && track < FIRST_SOUND + SOUND_COUNT){
				sound_play(track - FIRST_SOUND, 5);
			}
		} else if(joy_pressed(btn[cfg_btn_shoot])) {
			// Stop
			song_stop();
			status = STOPPED;
		}
		if(joy_pressed(btn[cfg_btn_pause])) {
			return;
		}
		if(status != oldstatus) {
			draw_status(status);
			oldstatus = status;
		}
		ready = TRUE;
        sys_wait_vblank();
		aftervsync();
    }
}
