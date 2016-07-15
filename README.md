# Cave Story MD
![Screenshot](doc/ss01.png)

This is a rewrite/port of the popular freeware game Cave Story for Sega MegaDrive/Genesis.
The engine is built using SGDK, and written almost entirely in C.

#### Download prebuilt ROM
See: [Releases](https://github.com/andwn/cave-story-md/releases)

#### Is this beatable?
Not yet. Right now you can technically make it to Labyrinth, but only the parts leading up to the beginning of Grasstown are in a well working state (As you progress into Grasstown and Sand Zone you will notice the game getting emptier and buggier)
Check out the [TODO list](doc/TODO.md) for progress. I may replace this list with issues eventually.

#### Controls
- C - Jump, confirm
- B - Shoot, hold to skip text
- Y, Z - Switch weapon
- Start - Pause / Item Menu

#### How to build? (GNU/Linux)
Note: Gendev is still using outdated SGDK 1.1 and will not build my project without [changes](https://github.com/kubilus1/gendev/issues/17).

1. Set up [Gendev](https://github.com/kubilus1/gendev.git)
2. Clone the project and do `make`.
3. Open `out.bin` in your emulator of choice.

#### How to build? (Windows)
1. Download and install [SGDK](https://github.com/Stephane-D/SGDK.git) using the instructions from that page.
2. Clone the repo `git clone https://github.com/andwn/cave-story-md`
3. Run "build.bat" and if everything goes well the ROM should be located at `out/rom.bin`.
  - Sometimes GCC will throw an error about heap space. Just retry until it completes or you get a different error.

#### Shoutouts & Legal Stuff
- Cave Story's game content (Graphics, sound, characters) is property of Studio Pixel, not me.
- My music covers can be considered [CC0](https://creativecommons.org/publicdomain/zero/1.0/).
- There exists another open source rewrite of Cave Story known as NXEngine. While I did not know about it's existence until knee deep into my own port, I have recently been using it as a reference for the AI.
