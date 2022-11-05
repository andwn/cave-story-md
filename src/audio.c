#include "common.h"
#include "gamemode.h"
#include "system.h"
#include "tables.h"
#include "md/comp.h"
#include "md/sys.h"
#include "md/vdp.h"
#include "md/xgm.h"
#include "audio.h"

// ID of the currently playing song, and backup of the previous,
// used for song_resume()
uint8_t songPlaying, songResume;
// soundChannel cycles between 1-3 to allow 3 sounds to play at once
uint8_t soundChannel;

void sound_init() {
    songPlaying = songResume = 0;
    // Here we are pointing the XGM driver to each sound effect in the game
    // and their length (in frames) indexed in sfx_info
    disable_ints();
    z80_pause();
    for(uint8_t i = 1; i < SOUND_COUNT; i++) {
    	uint16_t len = sfx_info[i].end - sfx_info[i].sound;
        xgm_pcm_set(0x40 + i, sfx_info[i].sound, len > 256 ? len : 256);
    }
    z80_resume();
    enable_ints();
    soundChannel = 1;
}

void sound_play(uint8_t id, uint8_t priority) {
	if(cfg_sfx_mute && gamemode != GM_SOUNDTEST) return;
	if(id >= 0x90 && id < 0xA0) id -= 0x40;
	if(id >= SOUND_COUNT || sfx_info[id].end == 0) return;
	xgm_pcm_play(0x40 + id, priority, soundChannel++);
	if(soundChannel > 3) soundChannel = 1;
}

void song_play(uint8_t id) {
	// Muted?
	if(cfg_music_mute && gamemode != GM_SOUNDTEST) {
		if(songPlaying) {
            xgm_music_pause();
			songPlaying = 0;
		}
		return;
	}
	if(id == songPlaying) return;
	songResume = songPlaying;
	// Track 0 in bgm_info is NULL, but others could be potentially
	if(bgm_info[id].song == NULL) {
		id = 0;
        xgm_music_pause();
	} else {
        xgm_music_pause();
        vdp_vsync(); aftervsync();
		vdp_vsync();
		xgm_music_play(bgm_info[id].song);
	}
	songPlaying = id;
}

void song_stop() {
    song_play(0);
}

void song_resume() {
	song_play(songResume);
}

uint8_t song_get_playing() {
	return songPlaying;
}
