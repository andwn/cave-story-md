/**
 *  \file vdp.h
 *  \brief VDP main
 *  \author Stephane Dallongeville
 *  \date 08/2011
 *
 * This unit provides general VDP methods :<br>
 * - initialisation<br>
 * - get / set register<br>
 * - get / set resolution<br>
 * - enable / disable VDP features<br>
 * <br>
 * VRAM should always be organized in a way that tile data are always located before map in VRAM:<br>
 * 0000-XXXX = tile data<br>
 * XXXX-FFFF = maps & tables (H scroll table, sprite table, B/A plan and window plan).
 */

/**
 *  \brief
 *      VDP Data port address.
 */
#define GFX_DATA_PORT           0xC00000
/**
 *  \brief
 *      VDP Control port address.
 */
#define GFX_CTRL_PORT           0xC00004
/**
 *  \brief
 *      VDP HV counter port address.
 */
#define GFX_HVCOUNTER_PORT      0xC00008

#define VDP_FIFOEMPTY_FLAG      (1 << 9)
#define VDP_FIFOFULL_FLAG       (1 << 8)
#define VDP_VINTPENDING_FLAG    (1 << 7)
#define VDP_SPROVERFLOW_FLAG    (1 << 6)
#define VDP_SPRCOLLISION_FLAG   (1 << 5)
#define VDP_ODDFRAME_FLAG       (1 << 4)
#define VDP_VBLANK_FLAG         (1 << 3)
#define VDP_HBLANK_FLAG         (1 << 2)
#define VDP_DMABUSY_FLAG        (1 << 1)
#define VDP_PALMODE_FLAG        (1 << 0)

#define VDP_PLAN_A              0xE000
#define VDP_PLAN_B              0xC000
#define VDP_PLAN_WINDOW         0xB000
#define VDP_HSCROLL_TABLE       0xF800
#define VDP_SPRITE_TABLE        0xFC00
#define VDP_MAPS_START          VDP_PLAN_WINDOW

#define HSCROLL_PLANE           0
#define HSCROLL_TILE            2
#define HSCROLL_LINE            3
#define VSCROLL_PLANE           0
#define VSCROLL_2TILE           1

/**
 *  \brief
 *      SGDL font length
 */
#define FONT_LEN    96

/**
 *  \brief
 *      Size of a single tile in byte.
 */
#define TILE_SIZE               32
#define TILE_INDEX_MASK         (0xFFFF / TILE_SIZE)

/**
 *  \brief
 *      Space in byte for tile in VRAM (tile space ends where maps starts)
 */
#define TILE_SPACE              VDP_MAPS_START

/**
 *  \brief
 *      Maximum number of tile in VRAM (related to TILE_SPACE).
 */
#define TILE_MAXNUM             (TILE_SPACE / TILE_SIZE)
/**
 *  \brief
 *      Maximum tile index in VRAM (related to TILE_MAXNUM).
 */
#define TILE_MAXINDEX           (TILE_MAXNUM - 1)
/**
 *  \brief
 *      System base tile index in VRAM.
 */
#define TILE_SYSTEMINDEX        0x0000
/**
 *  \brief
 *      Number of system tile.
 */
#define TILE_SYSTEMLENGTH       16
/**
 *  \deprecated Use TILE_SYSTEMLENGTH instead.
 */
#define TILE_SYSTEMLENGHT       TILE_SYSTEMLENGTH
/**
 *  \brief
 *      User base tile index.
 */
#define TILE_USERINDEX          (TILE_SYSTEMINDEX + TILE_SYSTEMLENGTH)
/**
 *  \brief
 *      Font base tile index.
 */
#define TILE_FONTINDEX          (TILE_MAXNUM - FONT_LEN)
/**
 *  \brief
 *      Number of available user tile.
 */
#define TILE_USERLENGTH         (TILE_FONTINDEX - TILE_USERINDEX)
/**
 *  \deprecated Use TILE_USERLENGTH instead.
 */
#define TILE_USERLENGHT         TILE_USERLENGTH
/**
 *  \brief
 *      Maximum tile index in VRAM for user.
 */
#define TILE_USERMAXINDEX       (TILE_USERINDEX + TILE_USERLENGTH - 1)
/**
 *  \brief
 *      System tile address in VRAM.
 */
#define TILE_SYSTEM             (TILE_SYSTEMINDEX * TILE_SIZE)
/**
 *  \brief
 *      User tile address in VRAM.
 */
#define TILE_USER               (TILE_USERINDEX * TILE_SIZE)
/**
 *  \brief
 *      Font tile address in VRAM.
 */
#define TILE_FONT               (TILE_FONTINDEX * TILE_SIZE)

// Tileset width/height
#define TS_WIDTH 32
#define TS_HEIGHT 16

// Stage tileset is first in USERINDEX
#define TILE_TSINDEX TILE_USERINDEX
#define TILE_TSSIZE (TS_WIDTH * TS_HEIGHT)
// Face graphics
#define TILE_FACEINDEX (TILE_TSINDEX + TILE_TSSIZE)
#define TILE_FACESIZE 36
// 16 tiles for the map name display
#define TILE_NAMEINDEX (TILE_FACEINDEX + TILE_FACESIZE)
#define TILE_NAMESIZE 16
// Space for shared sprite sheets
#define TILE_SHEETINDEX (TILE_NAMEINDEX + TILE_NAMESIZE)
#define TILE_SHEETSIZE (TILE_FONTINDEX - TILE_SHEETINDEX)
// Space for prompt/item display at the end of the sprite tiles
#define TILE_PROMPTINDEX (TILE_SHEETINDEX + TILE_SHEETSIZE - 28)
#define TILE_AIRTANKINDEX (TILE_PROMPTINDEX - 9)
// PLAN_A and PLAN_B are resized to 64x32 instead of 64x64, sprite list + hscroll table is
// also moved to the end as to not overlap the window plane (0xF800)
// These index the 2 unused areas between for some extra tile space
#define TILE_EXTRA1INDEX (0xD000 >> 5)
#define TILE_EXTRA2INDEX (0xF000 >> 5)
// Allocation of EXTRA1 (128 tiles) - background & HUD
#define TILE_BACKINDEX TILE_EXTRA1INDEX
#define TILE_BACKSIZE 96
#define TILE_HUDINDEX (TILE_BACKINDEX + TILE_BACKSIZE)
#define TILE_HUDSIZE 32
// Allocation of EXTRA2 (64 tiles) - Effects, window, misc
#define TILE_NUMBERINDEX TILE_EXTRA2INDEX
#define TILE_NUMBERSIZE 16
#define TILE_SMOKEINDEX (TILE_NUMBERINDEX + TILE_NUMBERSIZE)
#define TILE_SMOKESIZE 28
#define TILE_WINDOWINDEX (TILE_SMOKEINDEX + TILE_SMOKESIZE)
#define TILE_WINDOWSIZE 9
#define TILE_AIRINDEX (TILE_WINDOWINDEX + TILE_WINDOWSIZE)
#define TILE_AIRSIZE 7
// Unused palette color tiles area
#define TILE_PLAYERINDEX (TILE_SYSTEMINDEX + 2)
#define TILE_PLAYERSIZE 4
#define TILE_WEAPONINDEX (TILE_PLAYERINDEX + TILE_PLAYERSIZE)
#define TILE_WEAPONSIZE 6

/**
 *  \brief
 *      Palette 0
 */
#define PAL0                    0
/**
 *  \brief
 *      Palette 1
 */
#define PAL1                    1
/**
 *  \brief
 *      Palette 2
 */
#define PAL2                    2
/**
 *  \brief
 *      Palette 3
 */
#define PAL3                    3

/**
 *  \brief
 *      Set VDP command to read specified VRAM address.
 */
#define GFX_READ_VRAM_ADDR(adr)     ((0x0000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x00)
/**
 *  \brief
 *      Set VDP command to read specified CRAM address.
 */
#define GFX_READ_CRAM_ADDR(adr)     ((0x0000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x20)
/**
 *  \brief
 *      Set VDP command to read specified VSRAM address.
 */
#define GFX_READ_VSRAM_ADDR(adr)    ((0x0000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x10)

/**
 *  \brief
 *      Set VDP command to write at specified VRAM address.
 */
#define GFX_WRITE_VRAM_ADDR(adr)    ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x00)
/**
 *  \brief
 *      Set VDP command to write at specified CRAM address.
 */
#define GFX_WRITE_CRAM_ADDR(adr)    ((0xC000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x00)
/**
 *  \brief
 *      Set VDP command to write at specified VSRAM address.
 */
#define GFX_WRITE_VSRAM_ADDR(adr)   ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x10)

/**
 *  \brief
 *      Set VDP command to issue a DMA transfert to specified VRAM address.
 */
#define GFX_DMA_VRAM_ADDR(adr)      ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x80)
/**
 *  \brief
 *      Set VDP command to issue a DMA transfert to specified CRAM address.
 */
#define GFX_DMA_CRAM_ADDR(adr)      ((0xC000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x80)
/**
 *  \brief
 *      Set VDP command to issue a DMA transfert to specified VSRAM address.
 */
#define GFX_DMA_VSRAM_ADDR(adr)     ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x90)

/**
 *  \brief
 *      Set VDP command to issue a DMA VRAM copy to specified VRAM address.
 */
#define GFX_DMA_VRAMCOPY_ADDR(adr)  ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0xC0)

/**
 *  \brief
 *      Helper to write in vertical scroll table (same as GFX_WRITE_VSRAM_ADDR).
 */
#define GFX_VERT_SCROLL(adr)        GFX_WRITE_VSRAM_ADDR(adr)
/**
 *  \brief
 *      Helper to write in horizontal scroll table (same as GFX_WRITE_VRAM_ADDR(VDP_SCROLL_H + adr)).
 */
#define GFX_HORZ_SCROLL(adr)        GFX_WRITE_VRAM_ADDR(VDP_SCROLL_H + (adr))

/**
 *  \brief
 *      Tests VDP status against specified flag (see VDP_XXX_FLAG).
 */
#define GET_VDPSTATUS(flag)         ((*(volatile uint16_t*)(GFX_CTRL_PORT)) & (flag))
/**
 *  \brief
 *      Tests if current system is a PAL system (50 Hz).
 */
#define IS_PALSYSTEM                GET_VDPSTATUS(VDP_PALMODE_FLAG)

/**
 *  \brief
 *      Returns HV counter.
 */
#define GET_HVCOUNTER               (*(volatile uint16_t*)(GFX_HVCOUNTER_PORT))
/**
 *  \brief
 *      Returns Horizontal counter.
 */
#define GET_HCOUNTER                (GET_HVCOUNTER & 0xFF)
/**
 *  \brief
 *      Returns Vertical counter.
 */
#define GET_VCOUNTER                (GET_HVCOUNTER >> 8)


/**
 * Internal use
 */
#define CONST_PLAN_A                0
#define CONST_PLAN_B                1
#define CONST_PLAN_WINDOW           2

/**
 *  \brief
 *      Current screen width (horizontale resolution)
 */
extern uint16_t screenWidth;
/**
 *  \brief
 *      Current screen height (verticale resolution)
 */
extern uint16_t screenHeight;
/**
 *  \brief
 *      Current background plan width (in tile)
 *
 *  Possible values are: 32, 64, 128
 */
extern uint16_t planWidth;
/**
 *  \brief
 *      Current background plan height (in tile)
 *
 *  Possible values are: 32, 64, 128
 */
extern uint16_t planHeight;
/**
 *  \brief
 *      Current window width (in tile)
 *
 *  Possible values are: 32, 64
 */
extern uint16_t windowWidth;
/**
 *  \brief
 *      Current background plan width bit shift
 *
 *  Possible values are: 5, 6 or 7 (corresponding to plan width 32, 64 and 128)
 */
extern uint16_t planWidthSft;
/**
 *  \brief
 *      Current background plan height bit shift
 *
 *  Possible values are: 5, 6 or 7 (corresponding to plan height 32, 64 and 128)
 */
extern uint16_t planHeightSft;
/**
 *  \brief
 *      Current window width bit shift
 *
 *  Possible values are: 5 or 6 (corresponding to window width 32 or 64)
 */
extern uint16_t windowWidthSft;


/**
 *  \brief
 *      Constante to represent VDP background A plan (used by some methods)
 */
extern const VDPPlan PLAN_A;
/**
 *  \brief
 *      Constante to represent VDP background B plan (used by some methods)
 */
extern const VDPPlan PLAN_B;
/**
 *  \brief
 *      Constante to represent VDP window plan (used by some methods)
 */
extern const VDPPlan PLAN_WINDOW;


/**
 *  \brief
 *      Initialize the VDP sub system.
 *
 * Reset VDP registers, clear VRAM, set defaults grey, red, green & blue palette.
 */
void VDP_init();

/**
 *  \brief
 *      Get VDP register value.
 *
 *  \param reg
 *      Register number we want to retrieve value.
 *  \return specified register value.
 */
uint8_t   VDP_getReg(uint16_t reg);
/**
 *  \brief
 *      Set VDP register value.
 *
 *  \param reg
 *      Register number we want to set value.
 *  \param value
 *      value to set.
 */
void VDP_setReg(uint16_t reg, uint8_t value);

/**
 *  \brief
 *      Returns VDP enable state.
 */
uint8_t   VDP_getEnable();
/**
 *  \brief
 *      Set VDP enable state.
 *
 *  You can temporary disable VDP to speed up VDP memory transfert.
 */
void VDP_setEnable(uint8_t value);

/**
 *  \brief
 *      Returns number of total scanline.
 *
 *  312 for PAL system and 262 for NTSC system.
 */
uint16_t  VDP_getScanlineNumber();
/**
 *  \brief
 *      Returns vertical screen resolution.
 *
 *  Always returns 224 on NTSC system as they only support this mode.<br>
 *  PAL system supports 240 pixels mode.
 */
uint16_t  VDP_getScreenHeight();
/**
 *  \brief
 *      Set vertical resolution to 224 pixels.
 *
 *  This is the only accepted mode for NTSC system.
 */
void VDP_setScreenHeight224();
/**
 *  \brief
 *      Set vertical resolution to 240 pixels.
 *
 *  Only work on PAL system.
 */
void VDP_setScreenHeight240();
/**
 *  \brief
 *      Returns horizontal screen resolution.
 *
 *  Returns 320 or 256 depending current horizontal resolution mode.
 */
uint16_t  VDP_getScreenWidth();
/**
 *  \brief
 *      Set horizontal resolution to 256 pixels.
 */
void VDP_setScreenWidth256();
/**
 *  \brief
 *      Set horizontal resolution to 320 pixels.
 */
void VDP_setScreenWidth320();

/**
 *  \deprecated Use the <i>planWidth</i> variable directly.
 */
uint16_t  VDP_getPlanWidth();
/**
 *  \deprecated Use the <i>planHeight</i> variable directly.
 */
uint16_t  VDP_getPlanHeight();
/**
 *  \brief
 *      Set background plan size (in tile).
 *
 *  \param w
 *      width in tile.<br>
 *      Possible values are 32, 64 or 128.
 *  \param h
 *      height in tile.<br>
 *      Possible values are 32, 64 or 128.
 */
void VDP_setPlanSize(uint16_t w, uint16_t h);

/**
 *  \brief
 *      Returns plan horizontal scrolling mode.
 *
 *  Possible values are: HSCROLL_PLANE, HSCROLL_TILE, HSCROLL_LINE
 *
 *  \see VDP_setScrollingMode for more informations about scrolling mode.
 */
uint8_t VDP_getHorizontalScrollingMode();
/**
 *  \brief
 *      Returns plan vertical scrolling mode.
 *
 *  Possible values are: VSCROLL_PLANE, VSCROLL_2TILE
 *
 *  \see VDP_setScrollingMode for more informations about scrolling mode.
 */
uint8_t VDP_getVerticalScrollingMode();
/**
 *  \brief
 *      Set plan scrolling mode.
 *
 *  \param hscroll
 *      Horizontal scrolling mode :<br>
 *      <b>HSCROLL_PLANE</b> = Scroll offset is applied to the whole plan.<br>
 *      <b>HSCROLL_TILE</b> = Scroll offset is applied on a tile basis granularity (8 pixels bloc).<br>
 *      <b>HSCROLL_LINE</b> = Scroll offset is applied on a line basis granularity (1 pixel).<br>
 *  \param vscroll
 *      Vertical scrolling mode :<br>
 *      <b>VSCROLL_PLANE</b> = Scroll offset is applied to the whole plan.<br>
 *      <b>VSCROLL_2TILE</b> = Scroll offset is applied on 2 tiles basis granularity (16 pixels bloc).<br>
 *
 *  \see VDP_setHorizontalScroll() to set horizontal scroll offset in mode plane.<br>
 *  \see VDP_setHorizontalScrollTile() to set horizontal scroll offset(s) in mode tile.<br>
 *  \see VDP_setHorizontalScrollLine() to set horizontal scroll offset(s) in mode line.<br>
 *  \see VDP_setVerticalScroll() to set vertical scroll offset in mode plane.<br>
 *  \see VDP_setVerticalScrollTile() to set vertical scroll offset(s) in mode 2-tile.<br>
 */
void VDP_setScrollingMode(uint16_t hscroll, uint16_t vscroll);


/**
 *  \brief
 *      Returns the background color index.
 */
uint8_t VDP_getBackgroundColor();
/**
 *  \brief
 *      Set the background color index.
 */
void VDP_setBackgroundColor(uint8_t value);

/**
 *  \brief
 *      Returns auto increment register value.
 */
uint8_t   VDP_getAutoInc();
/**
 *  \brief
 *      Set auto increment register value.
 */
void VDP_setAutoInc(uint8_t value);

/**
 *  \brief
 *      Enable or Disable Horizontal interrupt.
 *
 *  \see VDP_setHIntCounter()
 */
void VDP_setHInterrupt(uint8_t value);
/**
 *  \brief
 *      Enable or Disable Hilight / Shadow effect.
 */
void VDP_setHilightShadow(uint8_t value);

/**
 *  \brief
 *      Get Horizontal interrupt counter value.
 */
uint8_t   VDP_getHIntCounter();
/**
 *  \brief
 *      Set Horizontal interrupt counter value.
 *
 *  When Horizontal interrupt is enabled, setting 5 here means that H int will occurs each (5+1) scanline.<br>
 *  Set value 0 to get H int at each scanline.
 */
void VDP_setHIntCounter(uint8_t value);

/**
 *  \brief
 *      Get VRAM address (location) of Plan A tilemap.
 */
uint16_t VDP_getAPlanAddress();
/**
 *  \brief
 *      Get VRAM address (location) of Plan B tilemap.
 */
uint16_t VDP_getBPlanAddress();
/**
 *  \brief
 *      Get VRAM address (location) of Window tilemap.
 */
uint16_t VDP_getWindowAddress();
/**
 *  \deprecated
 *      Use #VDP_getWindowAddress(..) instead.
 */
uint16_t VDP_getWindowPlanAddress();
/**
 *  \brief
 *      Get VRAM address (location) of Sprite list.
 */
uint16_t VDP_getSpriteListAddress();
/**
 *  \brief
 *      Get VRAM address (location) of H SCroll table.
 */
uint16_t VDP_getHScrollTableAddress();

/**
 *  \brief
 *      Set VRAM address (location) of Plan A tilemap.
 *      <br>
 *      WARNING: the window tilemap should always be the first object attribute in VRAM:<br>
 *      | system tiles<br>
 *      | user tiles<br>
 *      | window plan<br>
 *      v others (plan a, plan b, ...)<br>
 *      <br>
 *      The window tilemap address is used internally to calculated how much space is available for tiles.
 *
 *  EX:<br>
 *      VDP_setAPlanAddress(0xC000)<br>
 *      Will set the Plan A to at address 0xC000 in VRAM.
 */
void VDP_setAPlanAddress(uint16_t value);
/**
 *  \brief
 *      Set VRAM address (location) of Window tilemap.<br>
 *      <br>
 *      WARNING: the window tilemap should always be the first object attribute in VRAM:<br>
 *      | system tiles<br>
 *      | user tiles<br>
 *      | window plan<br>
 *      v others (plan a, plan b, ...)<br>
 *      <br>
 *      The window tilemap address is used internally to calculated how much space is available for tiles.
 *
 *  EX:<br>
 *      VDP_setWindowAddress(0xA000)<br>
 *      Will set the Window tilemap to at address 0xA000 in VRAM.
 */
void VDP_setWindowAddress(uint16_t value);
/**
 *  \deprecated
 *      Use #VDP_setWindowAddress(..) instead.
 */
void VDP_setWindowPlanAddress(uint16_t value);
/**
 *  \brief
 *      Set VRAM address (location) of Plan B tilemap.<br>
 *      <br>
 *      WARNING: the window tilemap should always be the first object attribute in VRAM:<br>
 *      | system tiles<br>
 *      | user tiles<br>
 *      | window plan<br>
 *      v others (plan a, plan b, ...)<br>
 *      <br>
 *      The window tilemap address is used internally to calculated how much space is available for tiles.
 *
 *  EX:<br>
 *      VDP_setBPlanAddress(0xE000)<br>
 *      Will set the Plan B to at address 0xE000 in VRAM.
 */
void VDP_setBPlanAddress(uint16_t value);
/**
 *  \brief
 *      Set VRAM address (location) of Sprite list.<br>
 *      <br>
 *      WARNING: the window tilemap should always be the first object attribute in VRAM:<br>
 *      | system tiles<br>
 *      | user tiles<br>
 *      | window plan<br>
 *      v others (plan a, plan b, ...)<br>
 *      <br>
 *      The window tilemap address is used internally to calculated how much space is available for tiles.
 *
 *  EX:<br>
 *      VDP_setSpriteListAddress(0xB800)<br>
 *      Will set the Sprite list to at address 0xB800 in VRAM.
 */
void VDP_setSpriteListAddress(uint16_t value);
/**
 *  \brief
 *      Set VRAM address (location) of H Scroll table.<br>
 *      <br>
 *      WARNING: the window tilemap should always be the first object attribute in VRAM:<br>
 *      | system tiles<br>
 *      | user tiles<br>
 *      | window plan<br>
 *      v others (plan a, plan b, ...)<br>
 *      <br>
 *      The the window tilemap address is used internally to calculated how much space is available for tiles.
 *
 *  EX:<br>
 *      VDP_setHScrollTableAddress(0xB400)<br>
 *      Will set the HScroll table to at address 0xB400 in VRAM.
 */
void VDP_setHScrollTableAddress(uint16_t value);

/**
 *  \brief
 *      Sets the scan mode of the display.
 *
 *  \param mode
 *      Accepted values : #INTERLACED_NONE, #INTERLACED_MODE1, #INTERLACED_MODE2
 *
 * This function changes the scanning mode on the next display blanking period.<br>
 * In Interlaced Mode 1, the same pattern will be displayed on the adjacent lines of even and odd numbered fields.<br>
 * In Interlaced Mode 2, different patterns can be displayed on the adjacent lines of even and odd numbered fields.<br>
 * The number of cells on the screen stays the same regardless of which scanning mode is active.
 */
void VDP_setScanMode(uint16_t mode);

/**
 *  \brief
 *      Sets the window Horizontal position.
 *
 *  \param right
 *      If set to <i>FALSE</i> the window is displayed from column 0 up to column <i>pos</i>
 *      If set to <i>TRUE</i> the window is displayed from column <i>pos</i> up to last column
 *  \param pos
 *      The Horizontal position of the window in 2 tiles unit (16 pixels).
 */
void VDP_setWindowHPos(uint16_t right, uint16_t pos);
/**
 *  \brief
 *      Sets the window Vertical position.
 *
 *  \param down
 *      If set to <i>FALSE</i> the window is displayed from row 0 up to row <i>pos</i>
 *      If set to <i>TRUE</i> the window is displayed from row <i>pos</i> up to last row
 *  \param pos
 *      The Vertical position of the window in 1 tile unit (8 pixels).
 */
void VDP_setWindowVPos(uint16_t down, uint16_t pos);

/**
 *  \brief
 *      Wait for DMA operation to complete.
 *  \deprecated Use #DMA_waitCompletion() instead
 */
void VDP_waitDMACompletion();
/**
 *  \brief
 *      Wait for VDP FIFO to be empty.
 */
void VDP_waitFIFOEmpty();

/**
 *  \brief
 *      Wait for Vertical Synchro.
 *
 *  The method actually wait for the next start of Vertical blanking.
 */
void VDP_waitVSync();

/**
 *  \brief
 *      Reset background plan and palette.
 *
 *  Clear background plans, reset palette to grey / red / green / blue and reset scrolls.
 */
void VDP_resetScreen();

/**
 *  \brief
 *      Display number of Frame Per Second.
 *
 *  \param float_display
 *      Display as float number.
 *
 * This function actually display the number of time it was called in the last second.<br>
 * i.e: for benchmarking you should call this method only once per frame update.
 */
void VDP_showFPS(uint16_t float_display);
