# Genesis/MegaDrive Version Differences
This file is where I list off the differences in this port and why.

### Graphics

 - The resolution is 320x224 on NTSC, so there will be an extra bit of scrolling.
 - I got a bit creative with the sprinkler effect to avoid lag. There are less particles and they alternate visibility.
 - CSMD uses a simple palette fade instead of the original effect which is very difficult to emulate on the hardware (if possible at all).
 - The message box disappears on fade out, but text is still visible. The way that works is actually that the color white in the main palette is kept white, as a result you can see the white of the HUD during fades too.
 - The font is 8x8 where the original was more like 6x8.
 - The water "foreground" is in back of the foreground map tiles. I believe this is technically possible to fix but would add too much complication to the stage/background logic (swapping the planes they use for 2 maps only).
 - Ballos's spikes in the third form are merged with the map and aren't exactly using the same sprite. This was to reduce the number of objects.
 - The pause menu does not slide onto the screen. It draws to the window plane which does not scroll.
 - Similarly, the menu and message box area hides the level map behind it revealing the background.
 - Weapon trails may not be as long as the original.

### Colors

 - The original game uses many off-shade colors that don't exist in a 9-bit colormap.
 - The Egg Corridor background is gray instead of green. The only greens available for use were also used by the enemies, so they would blend in with the background.
 - In some rooms that use bkBlue (Arthur's House and Seal Chamber), the face graphics may display some off shades of blue compared to what they are in the rest of the game. I decided to live with this, because the alternative is to make the background look like vomit.
 - King's shirt is blue instead of purple.
 - In some spots the water is brighter than the original to emphasize it (so it doesn't blend in with the background).

### Sound

 - The BGM is not exactly the same for obvious reasons.
 - Some tracks have entire channels/samples missing because they wouldn't fit.
 - A maximum of 3 sound effects can play at any time.

### Mechanics

 - When holding A during a script, text will skip instantly and NPCs will move at double speed.
 - Most objects when offscreen stop moving/updating until they are back on the screen.
 - Weapon energy is combined into a single power up instead of many being created by the same enemy.
 - In the original missile explosions create multiple (about 4) separate hitboxes. I only create one per missile.

### Game Content (Rooms/Scripts)

 - About half of the tilesets have been crushed to remove unused tiles (saving VRAM space for other things). This means the related PXA and PXM files were also modified to use those tilesets.
 - EggX and EggX2 are the same PXM with the tileset split into 2.
 - TSC are converted to a custom [bytecode](../tools/tscomp/tscomp.c).
 - The English dialog is from the Aeon Genesis translation.
 - Some bits of text (mainly item descriptions and credits text) have been edited to reduce length
 - An unused door was removed from Cent.pxe

## Other stuff that, while left out on purpose, may happen someday

 - Many particle effects like gibs, sparkles, drips aren't implemented.
 - Weapon swap icons next to the HUD aren't there. If there seems to be enough tiles left over after adding the weapon effects (like fireball trails and boom flashes) I may give it a shot.
  - Chako is the correct purple color in her house, but at the end of the game (King's Table, credits) she is gray. I haven't looked too hard into finding a free color slot to put the purple yet.
