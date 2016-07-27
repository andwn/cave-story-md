# Cave Story MD
![Screenshot](doc/ss01.png)

This is a rewrite/port of the popular freeware game Cave Story for Sega MegaDrive/Genesis.
The engine is built using SGDK, and written almost entirely in C.

It should work on any console or emulator (but not PAL yet).

#### Download prebuilt ROM
See: [Releases](https://github.com/andwn/cave-story-md/releases)

#### Is this beatable?
Not yet. Right now you can make it to the first room of Labyrinth, but only the parts leading up to the beginning of Sand Zone are in a well working state (a lot of NPCs don't move or are invisible as you get further into the game).
Check out the [TODO list](doc/TODO.md) for progress.

#### Controls
- C - Jump, confirm
- B - Shoot, hold to skip text
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
- There exists another open source rewrite of Cave Story known as NXEngine. While I did not know about it's existence until knee deep into my own port, I have recently been using it as a reference for the AI.
