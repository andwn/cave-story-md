# Cave Story MD
![Comparison Shot](doc/screen01.png)
[Video](http://www.youtube.com/watch?v=aZU133ekDVk)

This is a rewrite/port of the popular freeware game Cave Story for Sega Mega Drive/Genesis. It should work on any console or emulator*. It's fairly stable at this point but gets buggier as the game progresses.

(*) Genesis Plus GX crashes at startup. This is the only emulator known to have the issue.

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
### Why?
To learn MD dev mostly.

#### Why not SNES?
I'm not very interested in developing for that console. Why not give it a shot yourself?

### When is the next release?
Soon(tm)

### Will you release this on cartridge?
No.

#### Can I release this on cartridge?
I'm not the one you [should be asking](https://www.nicalis.com/).

#### Someone is selling cartridges though
It's a bootleg.

### Can you add new languages and game modes and redo the soundtrack? Also port Jenka's Nightmare next
Please stop I just want to finish this and move on to something original.

## License
Various, details [here](doc/LICENSE.md)

## Thanks
I did not know how to sort this list, so I did it alphabetically.

- andwhyisit: A whole lot of testing. Automated builds.
- DavisOlivier: Helped with a few music tracks.
- Sik: Mega Drive tech support. Made the font used ingame.
- Other people I probably forgot
