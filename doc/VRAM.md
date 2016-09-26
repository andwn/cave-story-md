### CSMD VRAM layout
My VRAM map has become too elaborate to remember in my head so hopefully this document helps.

#### 0x000 (TILE_SYSTEMINDEX)
First 16 tiles set by SGDK as plain palette colors.
I leave the first (transparency) alone and overwrite the rest like this:
- `0x1` - 1 tile for the little head bonk stars
- `0x2` - 4 tiles for Quote
- `0x6` - 6 tiles for the current weapon (3 horiz + 3 vert)
- `0xC` - 1 tile for water drops (sprinkler/splash effects)
- `0xD` - 1 tile for the Zzz effect that cycles while the effect is active
- `0xE` - 2 unused tiles (maybe whimsical star later)

#### 0x010 (TILE_USERINDEX and TILE_TSINDEX)
On the title screen I toss the full walking animation of Quote here because I'm lazy.
In game it's where the current map's tileset goes, they are at most 256x112 pixels (448 tiles).
Some of them had to be crushed with a custom utility.

#### 0x1D0 (TILE_FACEINDEX)
36 tiles used to display a character's 6x6 face graphic in the message window.
It is occasionally overwritten in scenarios where a face is guaranteed to never be displayed.

#### 0x1F4 (TILE_NAMEINDEX)
16 tiles to display the current map's name in the center of the screen.
This space is reused by boss battles to display the boss health bar.

#### 0x204 (TILE_SHEETINDEX)
Space for shared sprite sheets - the first 3 are bullets:
0. Polar star bullets, 2x2 horiz, 2x2 vert
1. Fireball bullets, 2x2x2 tiles
2. Machine gun bullets, 2x2 horiz, 2x2 vert
The snake will replace 0 & 1 if equipped, as they are mutually exclusive.
Then 4 power ups:
3. Heart, 2x2x4 tiles
4. Missile, 2x2x4 tiles
5. Small Energy, 1x1x6 tiles
6. Large Energy, 2x2x6 tiles
The remaining depend on the current map. There is no limit to how much space can be used.

#### Sprite Tile allocation space
This either begins at `TILE_SHEETINDEX + 24 * 6`, or after the last sheet. Whichever is smaller.
The reason for the minimum index is to prevent the item menu from overwriting it.
This space is controlled by an allocation array that represents 4 tile segments.
A game object must "ask" for a location large enough to store it's own sprite and on every
animation frame change will replace the tiles at that spot.

#### Just before TILE_FONTINDEX
The window uses this area to display the "Yes/No" prompt, cursor, or the item pick up.
It could potentially collide with the above I'm just hoping that doesn't happen for now.

#### TILE_FONTINDEX
This is the 32-127 ASCII font included with SGDK. In game a blue background version is used.
The blue version also replaces the equal sign with a filled circle.

#### Address 0xB000 - PLAN_W (64x32)

#### Address 0xC000 - PLAN_B (64x32)

#### TILE_EXTRA1INDEX
Space for 128 tiles between PLAN_B and PLAN_A.
- 96 tiles to be used for the map's background, if applicable
- 32 tiles for the HUD

#### Address 0xE000 - PLAN_A (64x32)

#### TILE_EXTRA2INDEX
Space for 64 tiles between PLAN_A and the sprite list/hscroll table.
- 16 tiles to display damage (numbers like '-127') for up to 4 objects at once
- 28 tiles for smoke animation
- 9 tiles for the blue window border/skin
- 7 tiles for the AIR display (during hell, replaced by timer)

#### Address 0xF800 - HSCROLL and sprite list
