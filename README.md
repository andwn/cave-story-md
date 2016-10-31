# Cave Story MD
![Screenshot](doc/ss01.png)

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
There is an experimental tool to convert saves (one way only PC -> MD).
Beware that SRAM formats differ between emulators.

1. `make tools`
2. Download & extract a Profile.dat from [here](http://www.cavestory.org/download/saves.php)
3. Example:
   - BlastEm: `./prof2sram ~/Downloads/Profile.dat.beforeCoreBattleW-OBooster ~/.local/share/blastem/doukutsu/save.sram`
   - Gens: `./prof2gens ~/Downloads/Profile.dat.beforeCoreBattleW-OBooster ~/.gens/doukutsu.srm`

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
I was learning about Genesis homebrew and wanted to make something, but I'm not creative enough 
to make my own game. Also Cave Story is pretty cool.

#### Where are the Windows build instructions?
There aren't any. I used to have a Makefile and bat to build this on Windows, 
however the build only succeeded ~30% of the time due to random build errors. 
The game also lags much more often when built from Windows for some reason.

#### There are no save points. What's going on?
The game detected that you have no SRAM. If this is not the case, open an issue.

#### I found a discrepancy with the original, what do?
Report it, but first make sure it's not in [this document](doc/DIFFERENCES.md)

#### Will there be a physical release of this?
Probably not. This is a fan port (not endorsed by Pixel or Nicalis), 
and I'm not going to dance around copyright laws more than I already am.

#### Isn't this just NXEngine ported to Genesis?
The AI code is, but the rest I wrote from scratch with the help of various modding docs.

#### Could I port mods to this?
It would be extremely painful.

## Shoutouts & Legal Stuff
- Cave Story's game content (Graphics, sound, characters) is property of Studio Pixel, not me.
- My music covers can be considered [CC0](https://creativecommons.org/publicdomain/zero/1.0/).
  - They were created using Deflemask tracker, based on conversions from the org2xm tool.
- Cave Editor is cool.
