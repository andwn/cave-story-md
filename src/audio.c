#include "audio.h"

#include <genesis.h>
#include "tables.h"

// ID of the currently playing song, and backup of the previous,
// used for song_resume()
u8 songPlaying, songResume;
// soundChannel cycles between 1-3 to allow 3 sounds to play at once
u8 soundChannel;

void sound_init() {
	songPlaying = songResume = 0;
	// Here we are pointing the XGM driver to each sound effect in the game
	// and their length (in frames) indexed in sound_info
	for(u8 i = 1; i < SOUND_COUNT; i++) {
		SND_setPCM_XGM(0x80 + i, sound_info[i].sound, sound_info[i].length);
	}
	soundChannel = 1;
}

void sound_play(u8 id, u8 priority) {
	if(sound_info[id].length == 0) return;
	SND_startPlayPCM_XGM(0x80 + id, priority, soundChannel++);
	if(soundChannel > 3) soundChannel = 1;
}

void song_play(u8 id) {
	songResume = songPlaying;
	// Track 0 in song_info is NULL, but others could be potentially
	if(song_info[id].song == NULL) {
		id = 0;
		SND_stopPlay_XGM();
	} else {
		SND_startPlay_XGM(song_info[id].song);
	}
	songPlaying = id;
}

void song_stop() {
	song_play(0);
}

void song_resume() {
	song_play(songResume);
}
