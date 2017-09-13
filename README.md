# Cave Story MD
![Comparison Shot](doc/screen01.png)
[Video](http://www.youtube.com/watch?v=aZU133ekDVk)

**Nightly/master is in the middle of an overhaul for the save data, expect things to break**

This is a rewrite/port of the popular freeware game Cave Story for Sega Mega Drive/Genesis.
The engine uses many parts of SGDK, and written mostly in C.

It should work on any console or emulator. Expect alpha quality.

## Download
"Stable" releases can be found in the [Releases](https://github.com/andwn/cave-story-md/releases) tab.

For something more bleeding edge, try the Nightly: [NTSC](http://www.cavestory.org/md/nightly.zip) / [PAL](http://www.cavestory.org/md/nightlypal.zip) <br/>

Farthest reachable point in `master`: Normal Ending / Hell B3

## Control Defaults
If you have a 6 button controller:

- `C` - Jump, confirm
- `B` - Shoot
- `A` - Fast forward through scripted events
- `Y`, `Z` - Switch weapon
- `Start` - Pause / Item Menu

For 3 button, `A` cycles through weapons. The rest is the same.

To activate Stage Select: 🡩 🡫🡨 🡪  A + Start. Like Sonic. 

## Compilation
1. Setup [marsdev](https://github.com/andwn/marsdev) and `python`
2. Clone & `make -f Makefile.marsdev`

## FAQ
#### Why?
To learn MD dev mostly.

#### Will you release this on cartridge?
No. Nicalis would sue me.

## Thanks
I did not know how to sort this list, so I did it alphabetically.

- andwhyisit: A whole lot of testing. Automated builds.
- DavisOlivier: Helped with a few music tracks, namely Gestation, Access, Cave Story, Fanfare 1/2/3
- Sik: Mega Drive tech support. Made the font used ingame.
- Other people I probably forgot
