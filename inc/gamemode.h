#ifndef INC_GAMEMODE_H_
#define INC_GAMEMODE_H_

#include "common.h"

/*
 * Entry points for each of the game modes, like title screen, sound test, gameplay, etc
 */

bool paused;
bool gameFrozen;

// Displays title screen, returns which option was picked
u8 titlescreen_main();

// Play the actual game. "load" is whether new game or continue was selected. If SRAM is blank,
// the "load" will be ignored, and a new game will start.
// The return value is 0 to return to title or >0 to show the credits (1, 2 different endings)
u8 game_main(bool load);

// Listen to music, sound effects
void soundtest_main();

// NPC Test lets you cycle between different NPCs sprites and data
void npctest_main();

void credits_main(u8 ending);

#endif /* INC_GAMEMODE_H_ */
