# Cave Story MD
![Screenshot](doc/ss01.png)

[More Screenshots](doc/SCREENS.md) / [Glitches](doc/SCREENS-FUNNY.md)

This is a rewrite/port of the popular freeware game Cave Story for Sega MegaDrive/Genesis.
The engine is built using SGDK, and written almost entirely in C.

It should work on any console or emulator (but not PAL yet).

### Download prebuilt ROM
See: [Releases](https://github.com/andwn/cave-story-md/releases)

### Controls
- C - Jump, confirm
- B - Shoot
- A - Fast forward through scenes
- Y, Z - Switch weapon
- Start - Pause / Item Menu

### How to build
Assuming you are running GNU/Linux,
1. Set up my fork of [Gendev](https://github.com/andwn/gendev.git) (Both the toolchain and SGDK are customized)
2. Clone the project and do `make`.
3. Open `out.bin` in your emulator of choice, or put it on a flash cart.

## FAQ
#### Why?
I was learning about Genesis homebrew and wanted to make something, but I'm not creative enough 
to make my own game. Also Cave Story is pretty cool.

#### Is the game beatable?
Not yet. Check out the [TODO list](doc/TODO.md) for progress.

In the current release, you can make it to the end section of Waterway if you try hard enough.

#### Where are the Windows build instructions?
There aren't any. I used to have a Makefile and bat to build this on Windows, 
however the build only succeeded ~30% of the time due to random build errors. 
The game also lags much more often when built from Windows for some reason.

#### There are no save points. What's going on?
The game detected that you have no SRAM. If this is not the case, open an issue.

#### I found a discrepancy with the original
If it's not listed in [this document](doc/DIFFERENCES.md) report it! In general I try to
base things around the original freeware (2004) version and not the one on Steam. There are
some things that are different on purpose, almost always to work around console limitations.

### I can't make these up
#### Why Genesis instead of my favorite console?
I'm not stopping you from porting CSMD to another console. Go make your dreams come true.

#### Why are you testing on emulator X instead of my favorite one?
lol

#### Isn't this just NXEngine ported to Genesis?
No! I wrote the engine from scratch, with help from a bunch of scattered documentation made by
Cave Story modders. It was not until recently that I knew of NXEngine. Now my AI code is
heavily based on Noxoid's. Also, I did steal the Booster 0.8/2.0 code.

## Shoutouts & Legal Stuff
- Cave Story's game content (Graphics, sound, characters) is property of Studio Pixel, not me.
- My music covers can be considered [CC0](https://creativecommons.org/publicdomain/zero/1.0/).
  - They were created using Deflemask tracker, based on conversions from the org2xm tool.
- Cave Editor is cool.
