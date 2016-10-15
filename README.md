# Cave Story MD
![Screenshot](doc/ss01.png)

[More Screenshots](doc/SCREENS.md)

This is a rewrite/port of the popular freeware game Cave Story for Sega MegaDrive/Genesis.
The engine is built using SGDK, and written almost entirely in C.

It should work on any console or emulator (PAL is untested).

### Download prebuilt ROM
See: [Releases](https://github.com/andwn/cave-story-md/releases)

### Progress
In the current release `0.2` you can make it to Ironhead if you try really hard. Labyrinth and
Waterway are really glitchy though.

Also looky here: [TODO list](doc/TODO.md)

### Controls
There may be an option to configure this in the future.

- C - Jump, confirm
- B - Shoot
- A - Fast forward through scripted events
- Y, Z - Switch weapon
- Start - Pause / Item Menu

### Compilation
Assuming you are running GNU/Linux. It might also work on BSDs

1. Set up my fork of [Gendev](https://github.com/andwn/gendev.git) (Both the toolchain and SGDK are customized)
2. Clone this repository and do `make`.
3. Open `doukutsu.bin` in your emulator of choice, or put it on a flash cart.

## FAQ
#### Why?
I was learning about Genesis homebrew and wanted to make something, but I'm not creative enough 
to make my own game. Also Cave Story is pretty cool.

#### Where are the Windows build instructions?
There aren't any. I used to have a Makefile and bat to build this on Windows, 
however the build only succeeded ~30% of the time due to random build errors. 
The game also lags much more often when built from Windows for some reason.

#### There are no save points. What's going on?
The game detected that you have no SRAM. If this is not the case, open an issue.

#### I found a discrepancy with the original, what do?
Report it, but first make sure

1. You are comparing with the original freeware Cave Story, not CS+
2. It's not in [this document](doc/DIFFERENCES.md)

#### Why are you testing on emulator X instead of my favorite one?
lol

#### Isn't this just NXEngine ported to Genesis?
The AI code is, but the rest I wrote from scratch with the help of various modding docs.

## Shoutouts & Legal Stuff
- Cave Story's game content (Graphics, sound, characters) is property of Studio Pixel, not me.
- My music covers can be considered [CC0](https://creativecommons.org/publicdomain/zero/1.0/).
  - They were created using Deflemask tracker, based on conversions from the org2xm tool.
- Cave Editor is cool.
