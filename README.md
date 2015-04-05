# Fun Cave Game
This is a rewrite/port of the popular freeware game Cave Story. It is built to run on a Genesis/Mega Drive.
Tested to work on Kega Fusion, Gens GS, and an NTSC Genesis Model 1.
The engine is built using SGDK, and written almost entirely in C.

## Download ROM
I update this every so often: [Cave-Story-MD.bin](https://dl.dropboxusercontent.com/u/105253377/Cave-Story-MD.bin)

## Is this beatable?
Not yet.
From "New Game" without cheats you can make it to Grasstown. 
About 30% of the game's music is implemented, and all of the sound effects are.
Only Polar Star works but Y and Z can still be used to switch weapons.
None of the enemies move yet, but they can be killed and drop power ups. Same with bosses.
The game can be saved and resumed. I have yet to notice any data related bugs. The SRAM is not compatible with the real game, but I will probably write a converter.

## How to build? (Windows)
1. Download and install [SGDK](http://github.com/Stephane-D/SGDK.git) using the instructions from that page.
2. After everything is in order clone the repo `git clone https://github.com/aderosier/cave-story-md.git`
3. For Windows, run "build.bat" and if everything goes well the ROM should be located at out/rom.bin.

## How to build? (GNU/Linux)
1. Set up [Gendev](https://code.google.com/p/gendev/)
2. Clone the project and do `make`.

## Caveats
Many of the colors are more vivid or even wrong sometimes. The usable palette on the Genesis is limited:
- 4 Palettes
- 15 opaque colors per palette, and 1 transparent
- Usable colormap totals 512 colors to choose from
- Shadow/Highlight modes increase this a bit however I do not use them
Some effects are not possible or difficult to implement without lag
- Particle effects such as sprinkler
- Sprite text
- Health bars
- Screen fading (with pattern)

## Legal Stuff
- The game engine was written by me from the ground up and has been given the GPLv3 free software license.
- The music tracks are covers of the original game's soundtrack created by me in Deflemask tracker.
- The art assets and sound effects are property of Studio Pixel.
- As stated in the GPL, there is no warranty. While I test this often on my model 1 Genesis I cannot guarantee it won't damage your console somehow and will not be held liable.

## TODO
Physics/Collision:
- [ ] Fix slope handling so player does not fall into peaks
- [ ] Wind tiles
- [ ] Give bullets a bounding box instead of using 8x8 always
- [ ] Movement types (UNI)
- [ ] Follow moving platforms
Behavior/AI:
- [ ] Create update functions for bats and critters so they can move
- [ ] Moving platform in egg corridor
- [ ] Event to start Igor fight does not trigger
TSC/Menus:
- [ ] Multiline instant text
- [ ] Slow normal text speed to 2 frames, make it 1 frame when holding C
- [ ] Display -- WARP -- in teleport menu
- [ ] Add the Item Menu
- [ ] Add the Map
- [ ] Credits
- [ ] Item graphics in message window
- [ ] Display stage name
Player/Weapons:
- [ ] Air and drowning
- [ ] Missiles
- [ ] Jump sound
- [ ] Walk sound
- [ ] Display question mark when pressing down
- [ ] Display stars when bonking head
Game Content:
- [ ] Sprites for remaining mimiga village NPCs
- [ ] Sprites for graveyard enemies
- [ ] Egg corridor music
- [ ] Graveyard and farm music
- [ ] Grasstown music
- [ ] Sprites for santa, chako, malco, booster
- [ ] Backgrounds: Moon & Fog
Cleanup/Refactoring:
- [ ] Change the entity list to use DLIST
- [ ] Use DMA to morph stage
