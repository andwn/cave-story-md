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
 *      SGDK font length
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
// 12 tiles at the end for nemesis vertical frames
#define TILE_NEMINDEX (0xFE80 >> 5)
// 8 tiles after window plane for blade L3
#define TILE_SLASHINDEX ((0xC000 >> 5) - 8)
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
 *      Set VDP register value.
 *
 *  \param reg
 *      Register number we want to set value.
 *  \param value
 *      value to set.
 */
uint8_t VDP_getReg(uint16_t reg);
void VDP_setReg(uint16_t reg, uint8_t value);

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
 *      Set vertical resolution to 240 pixels.
 *
 *  Only work on PAL system.
 */
void VDP_setScreenHeight240();

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
