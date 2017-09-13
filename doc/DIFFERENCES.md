### Genesis Version Differences
This file is where I list off the differences in this port and why.

#### Graphics
 - The resolution is 320x224 on NTSC, so there will be an extra bit of scrolling.
 - I got a bit creative with the sprinkler effect to avoid lag. There are less particles and they alternate visibility.
 - The message box disappears on fade out, but text is still visible. The way that works is actually that the color white in the main palette is kept white, as a result you can see the white of the HUD during fades too.
 - The font is 8x8 where the original was more like 6x8.
 - CSMD uses a simple palette fade instead of the original effect which is very difficult to emulate on the hardware.
 - The water "foreground" is in back of the foreground map tiles. I believe this is technically possible to fix but would add too much complication to the stage/background logic (swapping the planes they use for 2 maps only).
 - The original game uses many off-shade colors that don't exist in a 9-bit colormap.

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
 - The English dialog is from the Aeon Genesis translation.
 - Some bits of text (mainly item descriptions and credits text) have been edited to reduce length
