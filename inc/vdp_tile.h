/**
 *  \file vdp_tile.h
 *  \brief VDP General Tile / Tilemap operations
 *  \author Stephane Dallongeville
 *  \date 08/2011
 *
 * This unit provides methods to manipulate VDP tiles and tilemap :<br>
 * - upload tiles to VDP memory<br>
 * - upload tiles to VDP memory from bitmap data<br>
 * - clear / fill / set tile map data<br>
 */

/**
 *  \brief
 *      Bit shift for the tile priority attribute in tilemap data.
 */
#define TILE_ATTR_PRIORITY_SFT      15
/**
 *  \brief
 *      Bit shift for the tile palette attribute in tilemap data.
 */
#define TILE_ATTR_PALETTE_SFT       13
/**
 *  \brief
 *      Bit shift for the tile V flip attribute in tilemap data.
 */
#define TILE_ATTR_VFLIP_SFT         12
/**
 *  \brief
 *      Bit shift for the tile H flip attribute in tilemap data.
 */
#define TILE_ATTR_HFLIP_SFT         11
/**
 *  \brief
 *      Mask for the tile priority attribute in tilemap data.
 */
#define TILE_ATTR_PRIORITY_MASK     (1 << TILE_ATTR_PRIORITY_SFT)
/**
 *  \brief
 *      Mask for the tile palette attribute in tilemap data.
 */
#define TILE_ATTR_PALETTE_MASK      (3 << TILE_ATTR_PALETTE_SFT)
/**
 *  \brief
 *      Mask for the tile V flip attribute in tilemap data.
 */
#define TILE_ATTR_VFLIP_MASK        (1 << TILE_ATTR_VFLIP_SFT)
/**
 *  \brief
 *      Mask for the tile H flip attribute in tilemap data.
 */
#define TILE_ATTR_HFLIP_MASK        (1 << TILE_ATTR_HFLIP_SFT)
/**
 *  \brief
 *      Mask for the tile attributes (priority, palette and flip) in tilemap data.
 */
#define TILE_ATTR_MASK              (TILE_ATTR_PRIORITY_MASK | TILE_ATTR_PALETTE_MASK | TILE_ATTR_VFLIP_MASK | TILE_ATTR_HFLIP_MASK)

/**
 *  \brief
 *      Encode tile attributes for tilemap data.
 *
 *  \param pal
 *      Palette index
 *  \param prio
 *      Tile priority
 *  \param flipV
 *      Vertical flip
 *  \param flipH
 *      Horizontal flip
 */
#define TILE_ATTR(pal, prio, flipV, flipH)               (((flipH) << TILE_ATTR_HFLIP_SFT) + ((flipV) << TILE_ATTR_VFLIP_SFT) + ((pal) << TILE_ATTR_PALETTE_SFT) + ((prio) << TILE_ATTR_PRIORITY_SFT))
/**
 *  \brief
 *      Encode tile attributes for tilemap data.
 *
 *  \param pal
 *      Palette index
 *  \param prio
 *      Tile priority
 *  \param flipV
 *      Vertical flip
 *  \param flipH
 *      Horizontal flip
 *  \param index
 *      Tile index
 */
#define TILE_ATTR_FULL(pal, prio, flipV, flipH, index)   (((flipH) << TILE_ATTR_HFLIP_SFT) + ((flipV) << TILE_ATTR_VFLIP_SFT) + ((pal) << TILE_ATTR_PALETTE_SFT) + ((prio) << TILE_ATTR_PRIORITY_SFT) + (index))

/**
 *  \brief
 *      Load tile data (pattern) in VRAM.
 *
 *  \param data
 *      Pointer to tile data.
 *  \param index
 *      Tile index where start tile data load (use TILE_USERINDEX as base user index).
 *  \param num
 *      Number of tile to load.
 *  \param use_dma
 *      Use DMA transfert (faster but can lock Z80 execution).
 *
 *  Transfert rate:<br>
 *  ~90 bytes per scanline in software (during blanking)<br>
 *  ~190 bytes per scanline in hardware (during blanking)
 */
void VDP_loadTileData(const uint32_t *data, uint16_t index, uint16_t num, uint8_t use_dma);
/**
 *  \brief
 *      Load tile data (pattern) in VRAM.
 *
 *  \param tileset
 *      Pointer to TileSet structure.<br>
 *      The TileSet is unpacked "on-the-fly" if needed (require some memory)
 *  \param index
 *      Tile index where start tile data load (use TILE_USERINDEX as base user index).
 *  \param use_dma
 *      Use DMA transfert (faster but can lock Z80 execution).
 *  \return
 *      FALSE if there is not enough memory to unpack the specified TileSet (only if compression was enabled).
 *
 *  Transfert rate:<br>
 *  ~90 bytes per scanline in software (during blanking)<br>
 *  ~190 bytes per scanline in hardware (during blanking)
 */
uint16_t VDP_loadTileSet(const TileSet *tileset, uint16_t index, uint8_t use_dma);
/**
 *  \brief
 *      Load font tile data in VRAM.<br>
 *      Note that you should prefer the VDP_loadBMPFont(..) method to this one (easier to use).
 *
 *  \param font
 *      Pointer to font tile data.
 *  \param length
 *      Number of characters of the font (max = FONT_LEN).
 *  \param use_dma
 *      Use DMA transfert (faster but can lock Z80 execution).
 *
 *  This fonction permits to replace system font by user font.<br>
 *  The font tile data are loaded to TILE_FONTINDEX and can contains FONT_LEN characters at max.<br>
 *  Each character should fit in one tile (8x8 pixels bloc).<br>
 *  See also VDP_loadFont(..) and VDP_loadTileData(..)
 */
void VDP_loadFontData(const uint32_t *font, uint16_t length, uint8_t use_dma);
/**
 *  \brief
 *      Load font from the specified TileSet structure.
 *
 *  \param font
 *      TileSet containing the font.<br>
 *      The TileSet is unpacked "on-the-fly" if needed (require some memory)
 *  \param use_dma
 *      Use DMA transfert (faster but can lock Z80 execution).
 *  \return
 *      FALSE if there is not enough memory to unpack the specified font (only if compression was enabled).
 *
 *  This fonction permits to replace system font by user font.<br>
 *  The font tile data are loaded to TILE_FONTINDEX and can contains FONT_LEN characters at max.<br>
 *  Each character should fit in one tile (8x8 pixels bloc).<br>
 *  See also VDP_loadFontData(..)
 */
uint16_t VDP_loadFont(const TileSet *font, uint8_t use_dma);

/**
 *  \brief
 *      Fill tile data in VRAM.
 *
 *  \param value
 *      Value (byte) used to fill VRAM tile data.
 *  \param index
 *      Tile index where start tile data fill (use TILE_USERINDEX as base user index).
 *  \param num
 *      Number of tile to fill.
 *  \param wait
 *      Wait the operation to complete when set to TRUE otherwise it returns immediately
 *      but then you will require to wait for DMA completion (#DMA_waitCompletion()) before accessing the VDP.
 *
 *  This function is generally used to clear tile data in VRAM.
 */
void VDP_fillTileData(uint8_t value, uint16_t index, uint16_t num, uint16_t wait);

/**
 *  \brief
 *      Set tilemap data at single position.
 *
 *  \param plan
 *      Plan where we want to set tilemap data.<br>
 *      Accepted values are:<br>
 *      - VDP_PLAN_A<br>
 *      - VDP_PLAN_B<br>
 *      - VDP_WINDOW<br>
 *  \param tile
 *      tile attributes data (see TILE_ATTR_FULL() and TILE_ATTR() macros).
 *  \param ind
 *      position in tilemap.
 */
void VDP_setTileMap(uint16_t plan, uint16_t tile, uint16_t ind);
/**
 *  \brief
 *      Set tilemap data at single position.
 *
 *  \param plan
 *      Plan where we want to set tilemap data.<br>
 *      Accepted values are:<br>
 *      - PLAN_A<br>
 *      - PLAN_B<br>
 *      - PLAN_WINDOW<br>
 *  \param tile
 *      tile attributes data (see TILE_ATTR_FULL() and TILE_ATTR() macros).
 *  \param x
 *      X position (in tile).
 *  \param y
 *      y position (in tile).
 */
void VDP_setTileMapXY(VDPPlan plan, uint16_t tile, uint16_t x, uint16_t y);
/**
 *  \brief
 *      Fill tilemap data.
 *
 *  \param plan
 *      Plan where we want to fill tilemap data.<br>
 *      Accepted values are:<br>
 *      - VDP_PLAN_A<br>
 *      - VDP_PLAN_B<br>
 *      - VDP_WINDOW<br>
 *  \param tile
 *      tile attributes data (see TILE_ATTR_FULL() and TILE_ATTR() macros).
 *  \param ind
 *      tile index where to start fill.
 *  \param num
 *      Number of tile to fill.
 *
 *  \see VDP_fillTileMapRect()
 *  \see VDP_fillTileMapRectInc()
 */
void VDP_fillTileMap(uint16_t plan, uint16_t tile, uint16_t ind, uint16_t num);
/**
 *  \brief
 *      Fill tilemap data at specified region.
 *
 *  \param plan
 *      Plan where we want to fill tilemap region.<br>
 *      Accepted values are:<br>
 *      - PLAN_A<br>
 *      - PLAN_B<br>
 *      - PLAN_WINDOW<br>
 *  \param tile
 *      tile attributes data (see TILE_ATTR_FULL() and TILE_ATTR() macros).
 *  \param x
 *      Region X start position (in tile).
 *  \param y
 *      Region Y start position (in tile).
 *  \param w
 *      Region Width (in tile).
 *  \param h
 *      Region Heigh (in tile).
 *
 *  Fill the specified tilemap region with specified tile attributes values.<br>
 *
 *  \see VDP_fillTileMap() (faster method)
 *  \see VDP_fillTileMapRectInc()
 */
void VDP_fillTileMapRect(VDPPlan plan, uint16_t tile, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
/**
 *  \brief
 *      Clear tilemap data.
 *
 *  \param plan
 *      Plan where we want to clear tilemap region.<br>
 *      Accepted values are:<br>
 *      - VDP_PLAN_A<br>
 *      - VDP_PLAN_B<br>
 *      - VDP_WINDOW<br>
 *  \param ind
 *      Tile index where to start fill.
 *  \param num
 *      Number of tile to fill.
 *  \param wait
 *      Wait the operation to complete when set to TRUE otherwise it returns immediately
 *      but then you will require to wait for DMA completion (#DMA_waitCompletion()) before accessing the VDP.
 *
 *  \see VDP_clearTileMapRect()
 *  \see VDP_fillTileMap()
 *  \see VDP_fillTileMapRectInc()
 */
void VDP_clearTileMap(uint16_t plan, uint16_t ind, uint16_t num, uint16_t wait);
/**
 *  \brief
 *      Clear tilemap data at specified region.
 *
 *  \param plan
 *      Plan where we want to clear tilemap region.<br>
 *      Accepted values are:<br>
 *      - PLAN_A<br>
 *      - PLAN_B<br>
 *      - PLAN_WINDOW<br>
 *  \param x
 *      Region X start position (in tile).
 *  \param y
 *      Region Y start position (in tile).
 *  \param w
 *      Region Width (in tile).
 *  \param h
 *      Region Heigh (in tile).
 *
 *  \see VDP_clearTileMap() (faster method)
 */
void VDP_clearTileMapRect(VDPPlan plan, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
/**
 *  \brief
 *      Fill tilemap data with index auto increment.
 *
 *  \param plan
 *      Plan where we want to fill tilemap region.<br>
 *      Accepted values are:<br>
 *      - VDP_PLAN_A<br>
 *      - VDP_PLAN_B<br>
 *      - VDP_WINDOW<br>
 *  \param basetile
 *      Base tile attributes data (see TILE_ATTR_FULL() and TILE_ATTR() macros).
 *  \param ind
 *      tile index where to start fill.
 *  \param num
 *      Number of tile to fill.
 *
 *  Set the specified tilemap with specified tile attributes values.<br>
 *  The function auto increments tile index in tile attribute :<br>
 *  tilemap at index : basetile, basetile+1, basetile+2, basetile+3, ...<br>
 *  ...<br>
 *  So this function is pratical to display image.<br>
 *
 *  \see also VDP_fillTileMap()
 *  \see also VDP_fillTileMapRectInc()
 */
void VDP_fillTileMapInc(uint16_t plan, uint16_t basetile, uint16_t ind, uint16_t num);
/**
 *  \brief
 *      Fill tilemap data with index auto increment at specified region.
 *
 *  \param plan
 *      Plan where we want to fill tilemap region.<br>
 *      Accepted values are:<br>
 *      - PLAN_A<br>
 *      - PLAN_B<br>
 *      - PLAN_WINDOW<br>
 *  \param basetile
 *      Base tile attributes data (see TILE_ATTR_FULL() and TILE_ATTR() macros).
 *  \param x
 *      Region X start position (in tile).
 *  \param y
 *      Region Y start position (in tile).
 *  \param w
 *      Region Width (in tile).
 *  \param h
 *      Region Heigh (in tile).
 *
 *  Set the specified tilemap region with specified tile attributes values.<br>
 *  The function auto increments tile index in tile attribute :<br>
 *  tilemap line 0 : basetile, basetile+1, basetile+2, basetile+3, ...<br>
 *  tilemap line 1 : basetile+w, basetile+w+1, basetile+w+2, ...<br>
 *  ...<br>
 *  So this function is pratical to display image.<br>
 *
 *  \see also VDP_fillTileMapInc() (faster method)
 *  \see also VDP_fillTileMapRect()
 */
void VDP_fillTileMapRectInc(VDPPlan plan, uint16_t basetile, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
/**
 *  \brief
 *      Load tilemap data at specified index.
 *
 *  \param plan
 *      Plan where we want to load tilemap data.<br>
 *      Accepted values are:<br>
 *      - VDP_PLAN_A<br>
 *      - VDP_PLAN_B<br>
 *      - VDP_WINDOW<br>
 *  \param data
 *      Tile attributes data pointer (see TILE_ATTR_FULL() and TILE_ATTR() macros).
 *  \param ind
 *      Tile index where to start to set tilemap.
 *  \param num
 *      Number of tile to set.
 *  \param use_dma
 *      Use DMA transfert (faster but can lock Z80 execution).
 *
 *  Set the specified tilemap with specified tile attributes values.<br>
 *  Transfert rate:<br>
 *  ~90 bytes per scanline in software (during blanking)<br>
 *  ~190 bytes per scanline in hardware (during blanking)
 *
 *  \see VDP_setTileMapDataEx().
 *  \see VDP_setTileMapDataRect().
 */
void VDP_setTileMapData(uint16_t plan, const uint16_t *data, uint16_t ind, uint16_t num, uint8_t use_dma);
/**
 *  \brief
 *      Load tilemap data at specified region.
 *
 *  \param plan
 *      Plan where we want to load tilemap data.<br>
 *      Accepted values are:<br>
 *      - PLAN_A<br>
 *      - PLAN_B<br>
 *      - PLAN_WINDOW<br>
 *  \param data
 *      tile attributes data pointer (see TILE_ATTR_FULL() and TILE_ATTR() macros).
 *  \param x
 *      Region X start position (in tile).
 *  \param y
 *      Region Y start position (in tile).
 *  \param w
 *      Region Width (in tile).
 *  \param h
 *      Region Heigh (in tile).
 *
 *  Set the specified tilemap region with specified tile attributes values.
 *
 *  \see VDP_setTileMapDataRectEx().
 *  \see VDP_setTileMapData().
 */
void VDP_setTileMapDataRect(VDPPlan plan, const uint16_t *data, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
/**
 *  \brief
 *      Load tilemap data at specified index (extended version).
 *
 *  \param plan
 *      Plan where we want to load tilemap data.<br>
 *      Accepted values are:<br>
 *      - VDP_PLAN_A<br>
 *      - VDP_PLAN_B<br>
 *      - VDP_WINDOW<br>
 *  \param data
 *      tile attributes data pointer (see TILE_ATTR_FULL() and TILE_ATTR() macros).
 *  \param basetile
 *      Base tile index and flags for tile attributes (see TILE_ATTR_FULL() macro).
 *  \param ind
 *      Tile index where to start to set tilemap.
 *  \param num
 *      Number of tile to set.
 *
 *  Set the specified tilemap with specified tile attributes values.
 *
 *  \see VDP_setTileMapData()
 *  \see VDP_setTileMapDataRectEx()
 */
void VDP_setTileMapDataEx(uint16_t plan, const uint16_t *data, uint16_t basetile, uint16_t ind, uint16_t num);
/**
 *  \brief
 *      Load tilemap data at specified region (extended version).
 *
 *  \param plan
 *      Plan where we want to load tilemap data.<br>
 *      Accepted values are:<br>
 *      - PLAN_A<br>
 *      - PLAN_B<br>
 *      - PLAN_WINDOW<br>
 *  \param data
 *      tile attributes data pointer (see TILE_ATTR_FULL() macro).
 *  \param basetile
 *      Base index and flags for tile attributes (see TILE_ATTR_FULL() macro).
 *  \param x
 *      Region X start position (in tile).
 *  \param y
 *      Region Y start position (in tile).
 *  \param w
 *      Region Width (in tile).
 *  \param h
 *      Region Heigh (in tile).
 *  \param wm
 *      Source tilemap data width (in tile).
 *
 *  Set the specified tilemap region with specified tile attributes values.
 *
 *  \see VDP_setTileMapDataRect()
 *  \see VDP_setTileMapDataEx()
 */
void VDP_setTileMapDataRectEx(VDPPlan plan, const uint16_t *data, uint16_t basetile, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t wm);
