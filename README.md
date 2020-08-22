# Cave Story MD
![Comparison Shot](doc/screen01.png)
[Video](http://www.youtube.com/watch?v=aZU133ekDVk)

This is a rewrite/port of the popular freeware game Cave Story for Sega Mega Drive/Genesis. It should work on any console or emulator. The main story is "finished", only little things and bugfixes remain.

## Download
"Stable" releases can be found in the [Releases](https://github.com/andwn/cave-story-md/releases) tab.

## Control Defaults
If you have a 6 button controller:

- `C` - Jump, confirm
- `B` - Shoot
- `A` - Fast forward through scripted events
- `Y`, `Z` - Switch weapon
- `Start` - Pause / Item Menu

For 3 button, `A` cycles through weapons. The rest is the same.

### Cheats
- Stage Select: up, down, left, right, A + Start. Like Sonic.
- Infinite health/ammo: up, up, down, down, left, right, left, right, A + Start.

You can still get crushed and fall out of bounds.

Note that you can't load or save the game while cheating (including the counter).

## Compilation
1. Setup [marsdev](https://github.com/andwn/marsdev) and `python`
  - Marsdev targets: `m68k-toolchain z80-tools sik-tools`
2. Clone & `make`

## Other Information
- [FAQ](doc/FAQ.md)
- [Differences from PC](doc/DIFFERENCES.md)
- [VRAM Layout](doc/VRAM.md)

## License
Various, details [here](doc/LICENSE.md)

## Thanks
I did not know how to sort this list, so I did it alphabetically.

- andwhyisit: A whole lot of testing. Automated builds.
- DavisOlivier: Helped with a few music tracks.
- Sik: Mega Drive tech support. Made the font used ingame.
- Other people I probably forgot

## Translations
Mostly taken from cavestory.org

- English: [Aeon Genesis Translations](http://agtp.romhack.net/)
- Spanish: [Vagrant Traducciones](http://vagrant.romhackhispano.org)
- French: [Max le Fou](http://cavestory.maxlefou.com/)
- German: [Reality Dreamers](http://www.reality-dreamers.de/)
- Italian: [Simon M.](mailto:simonogatari@gmail.com)
- Portugese: [Andre Silva](mailto:andreluis.g.silva@gmail.com)
- BR Portugese: [Alex "Foffano"](mailto:foffano@gmail.com)
