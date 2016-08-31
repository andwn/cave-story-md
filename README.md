# Cave Story MD
![Screenshot](doc/ss01.png)

[More Screenshots](doc/SCREENS.md) / [Glitches](doc/SCREENS-FUNNY.md)

This is a rewrite/port of the popular freeware game Cave Story for Sega MegaDrive/Genesis.
The engine is built using SGDK, and written almost entirely in C.

It should work on any console or emulator (but not PAL yet).

#### Download prebuilt ROM
See: [Releases](https://github.com/andwn/cave-story-md/releases)

#### Is this beatable?
Not yet. Check out the [TODO list](doc/TODO.md) for progress.

In the current release, you can make it to the end section of Waterway if you try hard enough.

#### Controls
- C - Jump, confirm
- B - Shoot
- A - Fast forward through scenes
- Y, Z - Switch weapon
- Start - Pause / Item Menu

#### How to build
Note: Upstream Gendev is using an outdated SGDK version. for now use [my fork](https://github.com/andwn/gendev.git).

1. Set up [Gendev](https://github.com/kubilus1/gendev.git)
2. Clone the project and do `make`.
3. Open `out.bin` in your emulator of choice.

#### Shoutouts & Legal Stuff
- Cave Story's game content (Graphics, sound, characters) is property of Studio Pixel, not me.
- My music covers can be considered [CC0](https://creativecommons.org/publicdomain/zero/1.0/).
- Most of the AI is ported over from NXEngine. Good thing that exists because I probably would have given up otherwise.
