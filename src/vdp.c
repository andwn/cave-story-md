#include "common.h"

#include "dma.h"
#include "memory.h"
#include "resources.h"
#include "sprite.h"
#include "string.h"
#include "tools.h"
#include "vdp_bg.h"
#include "vdp_pal.h"
#include "vdp_tile.h"

#include "vdp.h"

#define WINDOW_DEFAULT          0xB000
#define HSCRL_DEFAULT           0xF800
#define SLIST_DEFAULT           0xFC00
#define APLAN_DEFAULT           0xE000
#define BPLAN_DEFAULT           0xC000

static uint8_t regValues[0x13];

// constants for plan
const VDPPlan PLAN_A = { CONST_PLAN_A };
const VDPPlan PLAN_B = { CONST_PLAN_B };
const VDPPlan PLAN_WINDOW = { CONST_PLAN_WINDOW };

void VDP_init() {
    volatile uint16_t *pw;
    uint16_t i;

    // wait for DMA completion
    VDP_waitDMACompletion();
    
    regValues[0x00] = 0x04;
    regValues[0x01] = 0x74;                     /* reg. 1 - Enable display, VBL, DMA + VCell size */
    regValues[0x02] = APLAN_DEFAULT / 0x400;       /* reg. 2 - Plane A =$30*$400=$C000 */
    regValues[0x03] = WINDOW_DEFAULT / 0x400;      /* reg. 3 - Window  =$2C*$400=$B000 */
    regValues[0x04] = BPLAN_DEFAULT / 0x2000;      /* reg. 4 - Plane B =$7*$2000=$E000 */
    regValues[0x05] = SLIST_DEFAULT / 0x200;       /* reg. 5 - sprite table begins at $BC00=$5E*$200 */
    regValues[0x06] = 0x00;                     /* reg. 6 - not used */
    regValues[0x07] = 0x00;                     /* reg. 7 - Background Color number*/
    regValues[0x08] = 0x00;                     /* reg. 8 - not used */
    regValues[0x09] = 0x00;                     /* reg. 9 - not used */
    regValues[0x0A] = 0x01;                     /* reg 10 - HInterrupt timing */
    regValues[0x0B] = 0x00;                     /* reg 11 - $0000abcd a=extr.int b=vscr cd=hscr */
    regValues[0x0C] = 0x81;                     /* reg 12 - hcell mode + shadow/highight + interlaced mode (40 cell, no shadow, no interlace) */
    regValues[0x0D] = HSCRL_DEFAULT / 0x400;       /* reg 13 - HScroll Table =$2E*$400=$B800 */
    regValues[0x0E] = 0x00;                     /* reg 14 - not used */
    regValues[0x0F] = 0x02;                     /* reg 15 - auto increment data */
    regValues[0x10] = 0x01;                     /* reg 16 - scrl screen v&h size (64x32) */
    regValues[0x11] = 0x00;                     /* reg 17 - window hpos */
    regValues[0x12] = 0x00;                     /* reg 18 - window vpos */
    
    pal_mode = GET_VDPSTATUS(VDP_PALMODE_FLAG);
    
    if(pal_mode) regValues[0x01] |= 0x08; // 240 screen height

    // set registers
    pw = (uint16_t *) GFX_CTRL_PORT;
    for (i = 0x00; i < 0x13; i++) *pw = 0x8000 | (i << 8) | regValues[i];

    // reset video memory (len = 0 is a special value to define 0x10000)
    DMA_doVRamFill(0, 0, 0, 1);
    // wait for DMA completion
    VDP_waitDMACompletion();

    // load defaults palettes
    VDP_setPalette(PAL0, palette_grey);

    // load default font
    VDP_loadFont(&TS_SysFont, 0);
    
    // reset scroll for plan A & B
    VDP_setVerticalScroll(PLAN_A, 0);
    VDP_setVerticalScroll(PLAN_B, 0);
    VDP_setHorizontalScroll(PLAN_A, 0);
    VDP_setHorizontalScroll(PLAN_B, 0);

    // default plan and base tile attribut for draw text method
    VDP_setTextPlan(PLAN_A);
    VDP_setTextPalette(PAL0);
    VDP_setTextPriority(TRUE);
}

uint8_t VDP_getReg(uint16_t reg)
{
    if (reg < 0x13) return regValues[reg];
    else return 0;
}

void VDP_setReg(uint16_t reg, uint8_t value)
{
    volatile uint16_t *pw;

    if (reg < 0x13) regValues[reg] = value;

    pw = (uint16_t *) GFX_CTRL_PORT;
    *pw = 0x8000 | (reg << 8) | value;
}

void VDP_setEnable(uint8_t value)
{
    volatile uint16_t *pw;

    if (value) regValues[0x01] |= 0x40;
    else regValues[0x01] &= ~0x40;

    pw = (uint16_t *) GFX_CTRL_PORT;
    *pw = 0x8100 | regValues[0x01];
}

void VDP_setScrollingMode(uint16_t hscroll, uint16_t vscroll)
{
    volatile uint16_t *pw;

    regValues[0x0B] = ((vscroll & 1) << 2) | (hscroll & 3);

    pw = (uint16_t *) GFX_CTRL_PORT;
    *pw = 0x8B00 | regValues[0x0B];
}

uint8_t VDP_getBackgroundColor()
{
    return regValues[0x07];
}

void VDP_setBackgroundColor(uint8_t value)
{
    volatile uint16_t *pw;

    regValues[0x07] = value & 0x3F;

    pw = (uint16_t *) GFX_CTRL_PORT;
    *pw = 0x8700 | regValues[0x07];
}

uint8_t VDP_getAutoInc()
{
    return regValues[0x0F];
}

void VDP_setAutoInc(uint8_t value)
{
    volatile uint16_t *pw;

    regValues[0x0F] = value;

    pw = (uint16_t *) GFX_CTRL_PORT;
    *pw = 0x8F00 | value;
}

void VDP_waitDMACompletion()
{
    while(GET_VDPSTATUS(VDP_DMABUSY_FLAG));
}

void VDP_waitFIFOEmpty()
{
    while(!GET_VDPSTATUS(VDP_FIFOEMPTY_FLAG));
}

void VDP_resetScreen()
{
    VDP_clearPlan(PLAN_A, TRUE);
    VDP_waitDMACompletion();
    VDP_clearPlan(PLAN_B, TRUE);
    VDP_waitDMACompletion();

    VDP_setPalette(PAL0, palette_grey);

    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    VDP_setHorizontalScroll(PLAN_A, 0);
    VDP_setHorizontalScroll(PLAN_B, 0);
    VDP_setVerticalScroll(PLAN_A, 0);
    VDP_setVerticalScroll(PLAN_B, 0);
}
