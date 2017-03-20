/*
 * Entry points for each of the game modes, like title screen, sound test, gameplay, etc
 */
 
enum GameMode {
	GM_SPLASH, // Sega logo
	GM_INTRO, // Studio Pixel presents
	GM_TITLE, // Title screen
	GM_SOUNDTEST, // Sound Test
	GM_CONFIG, // TODO: Settings like control mapping / bug toggling / clear SRAM
	GM_SAVESEL, // TODO: If I ever add capability for multisave
	GM_GAME, // Play the game
	GM_CREDITS // End credits
};

uint8_t gamemode;
uint8_t paused;
uint8_t gameFrozen;

// Sega screen
void splash_main();

// Displays title screen, returns which option was picked
uint8_t titlescreen_main();

// Play the actual game. "load" is whether new game or continue was selected
void game_main(uint8_t load);
// Show the map - called from <MLP
void do_map();

// Listen to music, sound effects
void soundtest_main();

void credits_main();
