### Genesis Version Differences
This file is where I list off the *intended* differences in this port and why.

#### Graphics
 - The resolution is 320x224 instead of 320x240 in NTSC mode. This causes rooms like First Cave and Egg Corridor to scroll vertically 16 pixels.
 - Where Courier New was used in the original game I am using the default 8x8 SGDK font. The font may change but it will likely still be 8x8.
 - CSMD uses a simple palette fade instead of the original effect which is very difficult to emulate on the hardware.
 - The smoke effect is just 1 puff instead of many that burst into different directions.
 - The HUD only displays the current weapon's icon.
 - The HUD also doesn't have a dark red background behind the health value.
 - Missiles do not have a trail of smoke behind them.
 - The message box disappears on fade out. Only text is still visible.
 - The message box is always on the bottom of the screen (<MS2 and <MS3 treated same as <MSG).
 - Planned: The sprinkler drops will be prerendered and probably reach a smaller area.

#### Colors
 - The original game uses many off-shade colors that don't exist in a 9-bit colormap.
 - Backgrounds tend to be brighter than the original game. They have to share a palette with either NPCs or the map.
 - King's shirt is blue.
 - Misery's face images have a red background and her shirt is green.
 - Balrog is his normal color and not shaded green in the scene before he is transformed into Balfrog.

#### Sound
 - The BGM is manually (every note) copied from org2xm exports into Deflemask and exported to VGM that can be played on a Genesis.
 - Some tracks have entire channels/samples missing because they wouldn't fit.
 - Gestation and Mischievous Robot are missing a background PCM sound in the intro.
 - Sound effects should sound almost exactly the same, but only 3 can play at once.
 - Persistent background sounds are missing.

#### Physics
 - This version is 60 FPS and the speed values needed to be adjusted for it. They may be just slightly off (like CS+).

#### Mechanics
 - Text can be "skipped" by holding B.
 - Hold A and press Start on the title screen to enable debug. In debug holding A lets you float and move through walls.
 - The map item cannot be used (I may change my mind about this later).
 - Most objects when offscreen stop moving/updating until they are back on the screen.
 
#### Game Content (Rooms/Scripts)
 - About half of the tilesets have been crushed to remove unused tiles. This means the related PXA and PXM files were also modified to use those tilesets.
 - EggX and EggX2 are the same PXM with the tileset split into 2.
 - TSC are converted to a custom bytecode.
