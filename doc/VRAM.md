### CSMD VRAM layout
My VRAM map has become too elaborate to remember in my head so hopefully this document helps.

Some things are different from this when drawing Japanese text. Maybe I'll document it
eventually.

#### 0x000
First 16 tiles that were reserved by SGDK when I still used it.
I leave the first (transparency) alone and use the rest like this:
- `0x1` - 1 tile for the little head bonk stars
- `0x2` - 4 tiles for Quote
- `0x6` - 6 tiles for the current weapon (3 horiz + 3 vert)
- `0xC` - 4 tiles for the boosters' smoke trail

#### 0x010 (TILE_TSINDEX)
On the title screen I toss the full walking animation of Quote here because I'm lazy.
In-game it's where the current map's tileset goes, they are at most 256x112 pixels (448 tiles).
Some of them had to be crushed with a custom utility.

#### 0x1D0 (TILE_FACEINDEX)
36 tiles used to display a character's 6x6 face graphic in the message window.
It is occasionally overwritten in scenarios where a face is guaranteed to never be displayed.
For example, the item menu clobbers this:
- 4 tiles for the cursor
- 12 tiles for shadowed numbers '0123456789-/' ("Lv." text is borrowed from the HUD)
- 20 tiles for the weapon icons, a maximum of 5 which are 4 tiles each

#### 0x1F4 (TILE_NAMEINDEX)
16 tiles to display the current map's name in the center of the screen.
This space is reused by boss battles to display the boss health bar.

#### 0x204 (TILE_SHEETINDEX)
Space for shared sprite sheets - the first 5 are always:
1. Heart, 2x2x4 tiles
2. Missile, 2x2x4 tiles
3. Small Energy, 1x1x6 tiles
4. Large Energy, 2x2x6 tiles
5. Whimsical Star, 3x1x1 tiles
The remaining depend on the current map and which weapons the player currently owns.
The maximum number of sheets is 24, but the allocation space is not limited or bounds checked in any way.
Be careful.

#### Sprite Tile allocation "Tiloc" space
This either begins at `TILE_SHEETINDEX + 24 * 6`, or after the last sheet. Whichever is larger.
The reason for the minimum index is to prevent the item menu & map system from overwriting it.
This space is controlled by an allocation array, each index represents an "in use" or "not in use"
state for a segment of 4 tiles.
A game object must "ask" for a location large enough to store its own sprite and on every
animation frame change will replace the tiles at that spot.

#### Just before TILE_FONTINDEX
The window uses this area to display the "Yes/No" prompt, cursor, or the item pickup.
It could potentially collide with the above I'm just hoping that doesn't happen for now.

#### TILE_FONTINDEX
Glyphs to represent the ASCII range 32-127. In game a blue background version is used.
The blue version also replaces the equal sign with a filled circle.

#### Address 0xB000 - PLAN_WINDOW (64x32)

#### Bottom 2 rows of PLAN_WINDOW (TILE_SLASHINDEX)
Since this never gets displayed on the screen I put the Blade Lv3 slash here (8 tiles)

#### Address 0xC000 - PLAN_B (64x32)

#### TILE_EXTRA1INDEX
Space for 128 tiles between PLAN_B and PLAN_A.
- 96 tiles to be used for the map's background, if applicable. There are some exceptions to this, including:
  - During the Core fight, I put the Minicores' tiles here
  - Only the top portion of the Moon background is stored here. The tileset for clouds is 188 tiles. Thankfully the map's tileset is small enough to leave a 192 tile gap, so I use that.
- 32 tiles for the HUD

#### Address 0xE000 - PLAN_A (64x32)

#### TILE_EXTRA2INDEX
Space for 64 tiles between PLAN_A and the sprite list/hscroll table.
- 16 tiles to display float text (numbers like '-127') for up to 4 objects at once
- 28 tiles for smoke animation
- 9 tiles for the blue window border/skin
- 8 tiles for either the AIR display or the counter in Hell
- 1 tile for question mark (TODO)
- 2 tiles for Whimsical Star (TODO)

#### Address 0xF800 - HSCROLL and sprite list

#### Address 0xFE80 (TILE_NEMINDEX)
Similarly to the Blade slashes, the vertical sprite tiles for Nemesis bullets go here (12 tiles)
