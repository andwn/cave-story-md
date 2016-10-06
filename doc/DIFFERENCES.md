### Genesis Version Differences
This file is where I list off the differences in this port and why.

#### Graphics
 - CANTFIX: The resolution is 320x224 instead of 320x240 in NTSC mode. This causes rooms like First Cave and Egg Corridor to scroll vertically 16 pixels.
 - WONTFIX: I got a bit creative with the sprinkler effect to avoid lag. There are less particles and they alternate visibility.
 - WONTFIX: The message box disappears on fade out, but text is still visible. The way that works is actually that the color white in the main palette is kept white, as a result you can see the white of the HUD during fades too.
 - WONTFIX: Where Courier New was used in the original game I am using the default 8x8 SGDK font. The font may change but it will likely still be 8x8.
 - WONTFIX: CSMD uses a simple palette fade instead of the original effect which is very difficult to emulate on the hardware.
 - WONTFIX: Chaba's desk is shortened by 2 pixels on the right.
 - MIGHTFIX: Core and Dark Place have Sonic water instead of using the background layer.
 - MIGHTFIX: The smoke effect is just 1 immobile puff instead of many that burst into different directions.
 - MIGHTFIX: The HUD only displays the currently selected weapon's icon.
 - MIGHTFIX: The HUD also doesn't have a dark red background behind the heart and health numbers.
 - MIGHTFIX: Polar Star doesn't have the "star" effect when shooting, or the animation for hitting a wall.
 - MIGHTFIX: Missiles do not have a trail of smoke behind them.
 - MIGHTFIX: Fireball L2 & 3 also do not have trails behind them.
 - MIGHTFIX: Water drops are missing (effect where water drops from ceiling in some rooms)
 - MIGHTFIX: Water splash effect is missing
 - MIGHTFIX: The bubble that carries Toroko and the Air Tank both use a different (smaller) bubble sprite.

#### Colors
 - CANTFIX: The original game uses many off-shade colors that don't exist in a 9-bit colormap.
 - WONTFIX: King's shirt is blue.
 - WONTFIX: Misery's face images have a red background and her shirt is green.
 - WONTFIX: Balrog is his normal color and not shaded green in the scene before he is transformed into Balfrog.
 - WONTFIX: Balrog's face graphic in both Gum room and Jenka's house are slightly dis-colored. That's because it uses the 4th (shared) palette which are different in those rooms. I rearranged the palettes so that his face would make sense.
 - MIGHTFIX: Mimigas in the bar with Curly have yellow-ish shirts instead of lime green.
 - MIGHTFIX: The bats in grasstown are blue like the first cave ones.
 - MIGHTFIX: Rooms like first cave have a solid black background instead of having a hint of blue.
 - MIGHTFIX: Backgrounds tend to be brighter than the original game. They have to share a palette with either NPCs or the map.

#### Sound
 - CANTFIX: The BGM is not exactly the same for obvious reasons.
 - CANTFIX: Some tracks have entire channels/samples missing because they wouldn't fit.
 - CANTFIX: Sound effects should sound almost exactly the same, but only 3 can play at once.
 - MIGHTFIX: Persistent background sounds (stream) are missing.
 - MIGHTFIX: Gestation and Mischievous Robot are missing a background PCM sound in the intro.
 
#### Physics
 - CANTFIX: Some times and physics will be slightly off on NTSC as Cave Story's native framerate is 50.

#### Mechanics
 - FEATURE: When holding A during a script, text will skip instantly and NPCs will move at double speed.
 - FEATURE: Hold A and press Start on the title screen to enable debug. In debug holding A lets you float and move through walls.
 - WONTFIX: Most objects when offscreen stop moving/updating until they are back on the screen.
 - WONTFIX: Weapon energy is combined into a single power up instead of many being created by the same enemy.

#### Game Content (Rooms/Scripts)
 - About half of the tilesets have been crushed to remove unused tiles (saving VRAM space for other things). This means the related PXA and PXM files were also modified to use those tilesets.
 - EggX and EggX2 are the same PXM with the tileset split into 2.
 - TSC are converted to a custom [bytecode](../tools/tscomp/tscomp.c).
 - The game is using the Aeon Genesis English translation. I have no plans on supporting other languages, but might if I get bored one day.
