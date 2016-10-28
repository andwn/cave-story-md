### Genesis Version Differences
This file is where I list off the differences in this port and why.

#### TV Standards
Cave Story's native resolution and framerate is 320x240x50. This won't be an issue on PAL (once I get that to work), but on NTSC:
 - All constants (and sometimes variables) related to time and velocity need to be adjusted with a formula like `time * 60 / 50` and `speed * 50 / 60`. I have macros TIME(x) and SPEED(x) to do this for me, and the compiler will do the math.
 - This means that the physics/timings may be slightly off in a few places.
 - Many of the rooms are meant to exactly fit a 240 Y resolution. These will now scroll up/down 16 pixels to make up for the loss.

#### Graphics
 - I got a bit creative with the sprinkler effect to avoid lag. There are less particles and they alternate visibility.
 - The message box disappears on fade out, but text is still visible. The way that works is actually that the color white in the main palette is kept white, as a result you can see the white of the HUD during fades too.
 - Where Courier New was used in the original game I am using the default 8x8 SGDK font. The font may change but it will likely still be 8x8.
 - CSMD uses a simple palette fade instead of the original effect which is very difficult to emulate on the hardware.
 - The water "foreground" is in back of the foreground map tiles. I believe this is technically possible to fix but would add too much complication to the stage/background logic (swapping the planes they use for 2 maps only).

#### Colors
 - The original game uses many off-shade colors that don't exist in a 9-bit colormap.
 - King's shirt is blue.
 - Misery's face images have a red background and her shirt is green.
 - Balrog is his normal color and not shaded green in the scene before he is transformed into Balfrog.
 - Balrog's face graphic in both Gum room and Jenka's house are slightly dis-colored.

#### Sound
 - The BGM is not exactly the same for obvious reasons.
 - Some tracks have entire channels/samples missing because they wouldn't fit.
 - A maximum of 3 sound effects can play at any time.

#### Mechanics
 - When holding A during a script, text will skip instantly and NPCs will move at double speed.
 - Most objects when offscreen stop moving/updating until they are back on the screen.
 - Weapon energy is combined into a single power up instead of many being created by the same enemy.

#### Game Content (Rooms/Scripts)
 - About half of the tilesets have been crushed to remove unused tiles (saving VRAM space for other things). This means the related PXA and PXM files were also modified to use those tilesets.
 - EggX and EggX2 are the same PXM with the tileset split into 2.
 - TSC are converted to a custom [bytecode](../tools/tscomp/tscomp.c).
 - The game is using the Aeon Genesis English translation. I have no plans on supporting other languages, but might if I get bored one day.
 - One line of text has been edited for length (Silver Locket description).
