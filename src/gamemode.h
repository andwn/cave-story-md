#ifndef GAMEMODE_H
#define GAMEMODE_H

/*
 * Entry points for each of the game modes, like title screen, sound test, gameplay, etc
 */
 
enum GameMode {
	GM_INTRO, // Studio Pixel presents
	GM_TITLE, // Title screen
	GM_SOUNDTEST, // Sound Test
	GM_CONFIG, // Settings like control mapping / bug toggling / clear SRAM
	GM_SAVESEL, // Save picker
	GM_GAME, // Play the game
	GM_CREDITS // End credits
};

extern uint8_t gamemode;
extern uint8_t paused;
extern uint8_t gameFrozen;

// Print version string at bottom of the screen
void print_version(void);

void intro_main(void);

// Displays title screen, returns which option was picked
uint8_t titlescreen_main(void);

uint8_t saveselect_main(void);

// Play the actual game. "load" is whether new game or continue was selected
void game_main(uint8_t load);

// Listen to music, sound effects
void soundtest_main(void);

// Change controls and stuff
void config_main(void);

void credits_main(void);
void credits_show_image(uint16_t id);
void credits_clear_image(void);

#endif
