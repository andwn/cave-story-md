# Cave Story MD
[![Video](http://img.youtube.com/vi/aZU133ekDVk/0.jpg)](http://www.youtube.com/watch?v=aZU133ekDVk)

This is a rewrite/port of the popular freeware game Cave Story for Sega MegaDrive/Genesis.
The engine is built using SGDK, and written almost entirely in C.

It should work on any console or emulator.

### Download
"Stable" releases can be found in the [Releases](https://github.com/andwn/cave-story-md/releases) tab.

For something more bleeding edge, try the [Nightly](http://www.cavestory.org/md/nightly.zip) build. (thanks andywhyisit)

### Progress
Farthest reachable point in `master`: Balcony

Also looky here: [TODO list](doc/TODO.md)

### Controls
There may be an option to configure this in the future.

- C - Jump, confirm
- B - Shoot
- A - Fast forward through scripted events
- Y, Z - Switch weapon
- Start - Pause / Item Menu

### Save files
There are SRAM files for various points in the game to help debugging in the save directory.
The `.srm` are for most emulators (skips even bytes), and `.sram` for BlastEm (doesn't skip bytes).

### Compilation
Assuming you are running GNU/Linux. It might also work on BSDs

0. Dependencies for all this:
  - The base developer tools form your distro, like `build-essential` on Debian or `base-devel` on Arch
  - `texinfo` - Newer GCC might pull this in but make sure it is installed
  - `Java` - For the compression tool lz4w. I may do something about this eventually
  - `Python` - Required to generate a header file, both 2 and 3 are known to work
1. Set up my fork of [Gendev](https://github.com/andwn/gendev.git) (Both the toolchain and SGDK are customized)
  - The command to build and install Gendev is `sudo make`
  - Compiling a GNU toolchain takes a very long time, up to an hour on slow machines
  - If after the fact you want to only update SGDK, run `sudo make sgdk-clean` then `sudo make sgdk`. This way you don't have to wait forever to rebuild the whole toolchain
2. Clone this repository and do `make`.
3. Open `doukutsu.bin` in your emulator of choice, or put it on a flash cart.

## FAQ
#### Why?
¯\_(ツ)_/¯

#### There are no save points. What's going on?
The game detected that you have no SRAM. If this is not the case, open an issue.

#### I found a discrepancy with the original, should I report it?
Sure, as long as it isn't an obviously impossible fix because of hardware limitations.

#### Will this be released/sold on a cart?
- Short answer: No
- Long answer: My country enforces copyright law and Nicalis has the rights to Cave Story

## Shoutouts & Legal Stuff
- Cave Story's game content (Graphics, sound, characters) is property of Studio Pixel, not me.
- Cave Editor is useful to investigate problematic objects and NPCs.
- Most music covers are my own, created in Deflemask. Consider them CC-BY. Exceptions include:
  - Eyes of Flame & Gestation: @DavisOlivier
  - Access, Cave Story, Fanfare 1/2/3: Originally me, improved by @DavisOlivier

