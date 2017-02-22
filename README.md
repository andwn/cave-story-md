# Cave Story MD
[![Video](http://img.youtube.com/vi/aZU133ekDVk/0.jpg)](http://www.youtube.com/watch?v=aZU133ekDVk)

This is a rewrite/port of the popular freeware game Cave Story for Sega MegaDrive/Genesis.
The engine is built using SGDK, and written almost entirely in C.

It should work on any console or emulator. Expect alpha quality and avoid PAL if you can.

### Download
"Stable" releases can be found in the [Releases](https://github.com/andwn/cave-story-md/releases) tab.

For something more bleeding edge, try the [Nightly](http://www.cavestory.org/md/nightly.zip) build.

Farthest reachable point in `master`: Last Boss (Balcony, not Hell), which doesn't spawn and therefore cannot be defeated.

### Controls
If you have a 6 button controller:

- C - Jump, confirm
- B - Shoot
- A - Fast forward through scripted events
- Y, Z - Switch weapon
- Start - Pause / Item Menu

For 3 button, A cycles through weapons. The rest is the same.

### Level Select (Next release)
🡩 🡫🡨 🡪  A+Start. Like Sonic. 
In the future I want to make weapons, ammo, health, and specific items/equips configurable.

This replaces the old method:

There are SRAM files for various points in the game to help debugging in the save directory.
The `.gens.srm` are for most emulators (odd bytes), and `.blastem.sram` for BlastEm (contiguous).

### Compilation
I've built on Ubuntu 16.04, Debian 8 and Arch, 32 and 64 bit:

0. Dependencies for all this:
  - For Ubuntu/Debian: `sudo apt install wget unzip build-essential texinfo openjdk-8-jre-headless python`
  - For Arch: `sudo pacman -Sy wget unzip base-devel texinfo jre8-openjdk python`
1. Set up my fork of [Gendev](https://github.com/andwn/gendev.git) (Both the toolchain and SGDK are customized)
  - The command to build and install Gendev is `sudo make`. Yes, this is awful. No, I'm not fixing it.
  - Compiling a GNU toolchain takes a very long time, over an hour on slow machines
  - If after the fact you want to only update SGDK, run `sudo make sgdk-clean` then `sudo make sgdk`. This way you don't have to wait forever to rebuild the whole toolchain
2. Clone this repository and do `make`.
3. Open `doukutsu.bin` in your emulator of choice, or put it on a flash cart.

## FAQ
#### Why?
¯\\\_(ツ)\_/¯

#### There are no save points. What's going on?
The game detected that you have no SRAM. If this is not the case, open an issue.
Note that if you have no save, the game will restart from the beginning if you die.

#### Bad things are happening what do?
Open an issue if one doesn't already exist for said bad thing, unless

- It's because of a hardware limitation, though I am open to suggestions on how to make stuff better without lagging the game
- It has to do with the following parts which are not ready for testing: Undead Core, Ending Scene, Credits, Hell
- It's a bug that exists in the original Cave Story. [1](https://www.youtube.com/watch?v=HFzS0bpc5kA) [2](http://cavestory.wikia.com/wiki/Sisters#Confrontation) [3](https://www.youtube.com/watch?v=wt7D3-COHJQ)

#### Will this be released/sold on a cart?
Short answer: No

Long answer: I will not do it. <br />
You can do it yourself [without my permission](LICENSE). However, Cave Story is *not* my IP. 
[These guys](http://www.nicalis.com/) are the current owners, and perhaps you will need [the creator](http://studiopixel.jp/)'s permission too.

#### No Windows build instructions, what the HECK?
Whenever I built on Windows the game lagged with way less objects on screen. This is for your own good.

## Thanks
I did not know how to sort this list, so I did it alphabetically.

- andwhyisit: A whole lot of testing. Automated builds.
- DavisOlivier: Helped with a few music tracks, namely Gestation, Access, Cave Story, Fanfare 1/2/3
- matteus: For reviving this project after I lost interest in 2015.
- Noxid: The AI code is mostly taken from NXEngine and I often read the source to verify other parts.
- Pixel: He made this video game.
- Sik: Mega Drive tech support. Made the font used ingame.
- Other people I probably forgot
