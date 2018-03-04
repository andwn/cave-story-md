# Cave Story MD
![Comparison Shot](doc/screen01.png)
[Video](http://www.youtube.com/watch?v=aZU133ekDVk)

This is a rewrite/port of the popular freeware game Cave Story for Sega Mega Drive/Genesis.
The engine uses many parts of SGDK, and written mostly in C.

It should work on any console or emulator. It's fairly stable at this point but gets buggier as the game progresses.

## Download
"Stable" releases can be found in the [Releases](https://github.com/andwn/cave-story-md/releases) tab.

For something more bleeding edge, try the [Nightly](http://www.cavestory.org/md/nightly.zip).

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
2. Clone & `make`

## FAQ
#### Why?
To learn MD dev mostly.

#### When is the next release?
Soon(tm)

#### Will you release this on cartridge?
No.

#### Can I release this on cartridge?
Not if you plan on selling it, unless you somehow got Nicalis permission.

## License
Various, details in [doc/LICENSE.md]

## Thanks
I did not know how to sort this list, so I did it alphabetically.

- andwhyisit: A whole lot of testing. Automated builds.
- DavisOlivier: Helped with a few music tracks, namely Gestation, Access, Cave Story, Fanfare 1/2/3
- Sik: Mega Drive tech support. Made the font used ingame.
- Other people I probably forgot
