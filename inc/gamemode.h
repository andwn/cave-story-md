/*
 * Entry points for each of the game modes, like title screen, sound test, gameplay, etc
 */

uint8_t paused;
uint8_t gameFrozen;

// Sega screen
void splash_main();

// Displays title screen, returns which option was picked
uint8_t titlescreen_main();

// Play the actual game. "load" is whether new game or continue was selected
// The return value is 0 to return to title or >0 to show the credits (1, 2 different endings)
uint8_t game_main(uint8_t load);
// Show the map - called from <MLP
void do_map();

// Listen to music, sound effects
void soundtest_main();

void credits_main();
