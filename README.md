# Cave Story MD
![Screenshot](doc/ss01.png)

This is a rewrite/port of the popular freeware game Cave Story for Sega MegaDrive/Genesis.
The engine is built using SGDK, and written almost entirely in C.

#### Download prebuilt ROM
See: [Releases](https://github.com/andwn/cave-story-md/releases)

#### Is this beatable?
Not yet. Right now Grasstown is as far as you can make it.
Check out the [TODO list](doc/TODO.md) for progress.

#### Controls
- C - Jump, confirm
- B - Shoot, hold to skip text
- Y, Z - Switch weapon
- Start - Pause / Item Menu

#### How to build? (GNU/Linux)
1. Set up [Gendev](https://github.com/kubilus1/gendev.git)
2. Clone the project and do `make`.

#### How to build? (Windows)
**Note**: I don't use Windows and can't guarantee this will work. If it does fail, show me the make output.

1. Download and install [SGDK](https://github.com/Stephane-D/SGDK.git) using the instructions from that page.
2. Clone the repo `git clone https://github.com/andwn/cave-story-md.git`
3. Run "build.bat" and if everything goes well the ROM should be located at out/rom.bin.
  - Sometimes GCC will throw an error about heap space, especially on newer versions of Windows. Just retry until it completes or you get a different error.

#### Legal Stuff
Cave Story's game content (Graphics, sound, characters) is property of Studio Pixel, not me.
My music covers can be considered [CC0](https://creativecommons.org/publicdomain/zero/1.0/).
