# Cave Story MD
![Comparison Shot](doc/screen01.png)
[Video](http://www.youtube.com/watch?v=aZU133ekDVk)

This is a rewrite/port of the popular freeware game Cave Story for Sega Mega Drive/Genesis.
The engine uses many parts of SGDK, and written mostly in C.

It should work on any console or emulator. Expect alpha quality.

## Download
"Stable" releases can be found in the [Releases](https://github.com/andwn/cave-story-md/releases) tab.

For something more bleeding edge, try the Nightly: [NTSC](http://www.cavestory.org/md/nightly.zip) / [PAL](http://www.cavestory.org/md/nightlypal.zip) <br/>

Farthest reachable point in `master`: Undead Core

## Controls
If you have a 6 button controller:

- `C` - Jump, confirm
- `B` - Shoot
- `A` - Fast forward through scripted events
- `Y`, `Z` - Switch weapon
- `Start` - Pause / Item Menu

For 3 button, `A` cycles through weapons. The rest is the same.

To activate Stage Select: 🡩 🡫🡨 🡪  A + Start. Like Sonic. 

## Compilation
0. Dependencies for all this:
  - Ubuntu/Debian: `sudo apt install wget unzip build-essential texinfo python`
  - RedHat/CentOS: `sudo yum groupinstall "Development Tools" && sudo yum install wget unzip texinfo python`
  - Arch: `sudo pacman -Sy wget unzip base-devel texinfo python`
1. Set up my fork of [Gendev](https://github.com/andwn/gendev.git) (Customized GCC 6.3.0 toolchain)
  - The command to build and install Gendev is `sudo make`. You likely don't want to do that on an important production machine, so if you can run 32-bit programs grab the tarball on the releases page instead.
  - The `sgdk-build` step is now optional as the parts of SGDK used are merged into CSMD.
2. Clone this repository and do `make`, or `make pal`.
3. Open `doukutsu.bin` in your emulator of choice, or put it on a flash cart.

## FAQ
#### Why?
Not sure myself but it's fun in a way.

#### Something is wrong. What DO?
Open an issue if one doesn't already exist, unless

- It's because of a hardware limitation, but if you have a realistic suggestion to improve it go ahead.
- It has to do with the following parts which are not ready for testing: Undead Core, Credits, Hell
- It's a bug that exists in the original Cave Story. [Example](https://www.youtube.com/watch?v=HFzS0bpc5kA)

#### Will you release this on cartridge?
No. Nicalis would sue me.

## Thanks
I did not know how to sort this list, so I did it alphabetically.

- andwhyisit: A whole lot of testing. Automated builds.
- DavisOlivier: Helped with a few music tracks, namely Gestation, Access, Cave Story, Fanfare 1/2/3
- Sik: Mega Drive tech support. Made the font used ingame.
- Other people I probably forgot
