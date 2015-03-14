#include "soundtest.h"

#include <genesis.h>

#include "sprite.h"
#include "resources.h"
#include "input.h"
#include "tables.h"

#define STOPPED	0
#define PLAYING	1
#define PAUSED	2

u8 track = 0;

u8 status = STOPPED, oldstatus = STOPPED;

void draw_byte(u8 n, u16 x, u16 y) {
	char b[2] = { (n>>4)+0x30, (n&0xF)+0x30 };
	for(u8 i=0;i<2;i++) { 
		if(b[i] > 0x39) b[i] += 0x7;
	}
	VDP_drawText(b, x, y);
}

void draw_track_info() {
	draw_byte(track, 2, 6);
	VDP_clearText(5, 6, 33);
	VDP_drawText(song_info[track].name, 5, 6);
}

void draw_status() {
	switch(status) {
		case STOPPED: VDP_drawText("Stopped", 2, 4); break;
		case PLAYING: VDP_drawText("Playing", 2, 4); break;
		case PAUSED:  VDP_drawText("Paused ", 2, 4); break;
	}
}

void soundtest_main() {
	sprites_clear();
	VDP_setEnable(false);
	VDP_setPalette(PAL1, PAL_SndTest.data);
	VDP_loadTileSet(&TS_SndTest, TILE_USERINDEX, true);
	//VDP_fillTileMapRectInc(BPLAN,
	//		TILE_ATTR_FULL(PAL1, 0, 0, 0, TILE_USERINDEX), 0, 0, 40, 28);
	VDP_clearPlan(APLAN, true);
	draw_status();
	VDP_drawText("Sound Test", 15, 2);
	VDP_drawText("Track: ", 2, 8);
	VDP_drawText("C-Play B-Stop", 2, 14);
	VDP_drawText("A-Pause Start-Quit", 2, 16);
	draw_byte(track, 10, 8);
	VDP_setEnable(true);
    while(true) {
		input_update();
		if(joy_pressed(BUTTON_LEFT)) {
			if(track == 0) track = SONG_COUNT - 1;
			else track--;
			draw_byte(track, 10, 8);
		} else if(joy_pressed(BUTTON_RIGHT)) {
			if(track == SONG_COUNT - 1) track = 0;
			else track++;
			draw_byte(track, 10, 8);
		}
		if(joy_pressed(BUTTON_C)) {
			// Play
			if(song_info[track].song != NULL)
				SND_startPlay_XGM(song_info[track].song);
			status = PLAYING;
			draw_track_info();
		} else if(joy_pressed(BUTTON_B)) {
			// Stop
			SND_stopPlay_XGM();
			status = STOPPED;
		} else if(joy_pressed(BUTTON_A)) {
			// Pause
			if(status == PLAYING) {
				SND_stopPlay_XGM();
				status = PAUSED;
			} else if(status == PAUSED) {
				SND_resumePlay_XGM();
				status = PLAYING;
			}
		}
		if(joy_pressed(BUTTON_START)) {
			SND_stopPlay_XGM();
			oldstate |= BUTTON_START;
			break;
		}
		if(status != oldstatus) {
			draw_status();
			oldstatus = status;
		}
		VDP_waitVSync();
    }
}
