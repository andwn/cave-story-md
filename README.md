# Fun Cave Game
This is a rewrite/port of the popular freeware game Cave Story. It is built to run on a Genesis/Mega Drive.
Tested to work on Kega Fusion, Gens GS, and an NTSC Genesis Model 1.
The engine is built using SGDK, and written almost entirely in C.

## Download ROM
I update this every so often: [Cave-Story-MD.bin](https://dl.dropboxusercontent.com/u/105253377/Cave-Story-MD.bin)

#### Is this beatable?
Not yet.
From "New Game" without cheats you can make it to Grasstown. 
About 30% of the game's music is implemented, and all of the sound effects are.
Only Polar Star works but Y and Z can still be used to switch weapons.
Only enemies in First Cave move, but all can be killed and drop power ups. Same with bosses.
The game can be saved and resumed. I have yet to notice any data related bugs. The SRAM is not compatible with the real game, but I am writing a converter.

## How to build? (Windows)
1. Download and install [SGDK](http://github.com/Stephane-D/SGDK.git) using the instructions from that page.
2. Clone the repo `git clone https://github.com/aderosier/cave-story-md.git`
3. Run "build.bat" and if everything goes well the ROM should be located at out/rom.bin.
  - Sometimes GCC will throw an error about heap space, especially on newer versions of Windows. Just retry until it completes or you get a different error.

### How to build? (GNU/Linux)
1. Set up [Gendev](https://code.google.com/p/gendev/)
2. Clone the project and do `make`.
  - This setup can be fairly error prone. I'm looking into ways to make this easier but for now [this thread](http://gendev.spritesmind.net/forum/viewtopic.php?t=1248&sid=062e63d4c70d3d74012e31e680089aab) can help.

#### Caveats
- Many of the colors are more vivid or even wrong sometimes due to palette limitations.
- Some effects like the sprinkler and screen fading are not possible or difficult to implement without lag

### Areas
- [x] First Cave
- [ ] Mimiga
  - [x] Completable
  - [x] Sprites
  - [x] Music
    - [x] Village
    - [x] Farm
  - [ ] AI/actions
- [ ] Graveyard
  - [x] Completable
  - [x] Sprites
  - [ ] Music
  - [ ] AI/actions
- [ ] Eggs
  - [x] Completable
  - [ ] Sprites
  - [ ] AI/actions
  - [ ] Weapon: Missiles
- [ ] Grasstown
  - [ ] Completable
  - [ ] Sprites
  - [ ] Music
  - [ ] AI/actions
  - [ ] Weapon: Fireball
- [ ] Sand Zone
- [ ] Labyrinth
- [ ] Core
- [ ] Waterway
- [ ] Eggs2
- [ ] Outer Wall
- [ ] Plantation
- [ ] Last Cave
- [ ] Balcony / Ring
- [ ] Credits
- [ ] Sacred Ground

#### Legal Disclaimer
The art assets and sound effects are property of Studio Pixel and are in no way owned by me.
This repository focuses on the engine (GPLv3) and music covers (CC0).

