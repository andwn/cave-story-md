#include "common.h"

#include "vdp.h"
#include "xgm.h"

#include "vdp_pal.h"

#define PALETTEFADE_FRACBITS    8
#define PALETTEFADE_ROUND_VAL   ((1 << (PALETTEFADE_FRACBITS - 1)) - 1)

const uint16_t palette_grey[16] =
{
    0x0000,
    0x0222,
    0x0444,
    0x0666,
    0x0888,
    0x0AAA,
    0x0CCC,
    0x0EEE,

    0x0EEE,
    0x0EEE,
    0x0EEE,
    0x0EEE,
    0x0EEE,
    0x0EEE,
    0x0EEE,
    0x0EEE
};

// used for palette fading (consumes 902 bytes of memory)
static uint16_t final_pal[64];
static uint16_t fading_palR[64];
static uint16_t fading_palG[64];
static uint16_t fading_palB[64];
static uint16_t fading_stepR[64];
static uint16_t fading_stepG[64];
static uint16_t fading_stepB[64];
static uint16_t fading_from;
static uint16_t fading_to;

// forward
static void setFadePalette();

uint16_t VDP_getPaletteColor(uint16_t index)
{
    volatile uint16_t *pw;
    volatile uint32_t *pl;
    uint16_t addr;

    /* Point to vdp port */
    pw = (uint16_t *) GFX_DATA_PORT;
    pl = (uint32_t *) GFX_CTRL_PORT;

    addr = index * 2;
    *pl = GFX_READ_CRAM_ADDR(addr);

    return *pw;
}

void VDP_setPaletteColor(uint16_t index, uint16_t value)
{
    volatile uint16_t *pw;
    volatile uint32_t *pl;
    uint16_t addr;

    /* Point to vdp port */
    pw = (uint16_t *) GFX_DATA_PORT;
    pl = (uint32_t *) GFX_CTRL_PORT;

    addr = index * 2;
    *pl = GFX_WRITE_CRAM_ADDR(addr);

    *pw = value;
}

static void setFadePalette()
{
    uint16_t *palR;
    uint16_t *palG;
    uint16_t *palB;
    volatile uint16_t *pw;
    volatile uint32_t *pl;
    uint16_t addr;
    uint16_t i;

    // lazy optimization
    if (VDP_getAutoInc() != 2)
        VDP_setAutoInc(2);

    /* point to vdp port */
    pw = (uint16_t *) GFX_DATA_PORT;
    pl = (uint32_t *) GFX_CTRL_PORT;

    addr = fading_from * 2;
    *pl = GFX_WRITE_CRAM_ADDR(addr);

    i = fading_from;

    palR = &fading_palR[i];
    palG = &fading_palG[i];
    palB = &fading_palB[i];

    i = (fading_to - fading_from) + 1;
    while(i--)
    {
        uint16_t col;

        col = (((*palR++ + PALETTEFADE_ROUND_VAL) >> PALETTEFADE_FRACBITS) << VDPPALETTE_REDSFT) & VDPPALETTE_REDMASK;
        col |= (((*palG++ + PALETTEFADE_ROUND_VAL) >> PALETTEFADE_FRACBITS) << VDPPALETTE_GREENSFT) & VDPPALETTE_GREENMASK;
        col |= (((*palB++ + PALETTEFADE_ROUND_VAL) >> PALETTEFADE_FRACBITS) << VDPPALETTE_BLUESFT) & VDPPALETTE_BLUEMASK;

        *pw = col;
    } 
}

uint16_t VDP_doStepFading()
{
    // last step --> just recopy the final palette
    if (--fading_cnt <= 0)
    {
        // we are inside VInt callback --> just set palette colors immediately
        VDP_setPaletteColors(fading_from, final_pal + fading_from, (fading_to - fading_from) + 1);
        return 0;
    }

    uint16_t *palR;
    uint16_t *palG;
    uint16_t *palB;
    uint16_t *stepR;
    uint16_t *stepG;
    uint16_t *stepB;
    uint16_t i;

    i = fading_from;

    palR = &fading_palR[i];
    palG = &fading_palG[i];
    palB = &fading_palB[i];
    stepR = &fading_stepR[i];
    stepG = &fading_stepG[i];
    stepB = &fading_stepB[i];

    i = (fading_to - fading_from) + 1;
    while(i--)
    {
        *palR++ += *stepR++;
        *palG++ += *stepG++;
        *palB++ += *stepB++;
    }

    // set current fade palette
    setFadePalette();

    return 1;
}

uint16_t VDP_initFading(uint16_t fromcol, uint16_t tocol, const uint16_t *palsrc, const uint16_t *paldst, uint16_t numframe)
{
    const uint16_t *src;
    const uint16_t *dst;
    uint16_t *save;
    uint16_t *palR;
    uint16_t *palG;
    uint16_t *palB;
    uint16_t *stepR;
    uint16_t *stepG;
    uint16_t *stepB;
    uint16_t i;

    // can't do a fade on 0 frame !
    if (numframe == 0) return 0;

    fading_from = fromcol;
    fading_to = tocol;
    fading_cnt = numframe;

    src = palsrc;
    dst = paldst;
    save = final_pal + fromcol;
    palR = fading_palR + fromcol;
    palG = fading_palG + fromcol;
    palB = fading_palB + fromcol;
    stepR = fading_stepR + fromcol;
    stepG = fading_stepG + fromcol;
    stepB = fading_stepB + fromcol;

    i = (tocol - fromcol) + 1;
    while(i--)
    {
        const uint16_t s = *src++;
        const uint16_t d = *dst++;

        const uint16_t R = ((s & VDPPALETTE_REDMASK) >> VDPPALETTE_REDSFT) << PALETTEFADE_FRACBITS;
        const uint16_t G = ((s & VDPPALETTE_GREENMASK) >> VDPPALETTE_GREENSFT) << PALETTEFADE_FRACBITS;
        const uint16_t B = ((s & VDPPALETTE_BLUEMASK) >> VDPPALETTE_BLUESFT) << PALETTEFADE_FRACBITS;

        // fix detination palette
        *save++ = d;

        *stepR++ = ((((d & VDPPALETTE_REDMASK) >> VDPPALETTE_REDSFT) << PALETTEFADE_FRACBITS) - R) / numframe;
        *stepG++ = ((((d & VDPPALETTE_GREENMASK) >> VDPPALETTE_GREENSFT) << PALETTEFADE_FRACBITS) - G) / numframe;
        *stepB++ = ((((d & VDPPALETTE_BLUEMASK) >> VDPPALETTE_BLUESFT) << PALETTEFADE_FRACBITS) - B) / numframe;

        *palR++ = R;
        *palG++ = G;
        *palB++ = B;
    }

    // set current fade palette
    setFadePalette();

    return 1;
}

void VDP_fade(uint16_t fromcol, uint16_t tocol, const uint16_t *palsrc, const uint16_t *paldst, uint16_t numframe, uint8_t async)
{
    // error during fading initialization --> exit
    if (!VDP_initFading(fromcol, tocol, palsrc, paldst, numframe)) return;

    // process asynchrone fading
    if (!async) {
        // process fading immediatly
        while (VDP_doStepFading(TRUE)) {
			vsync();
			XGM_doVBlankProcess();
		}
    }
}

void VDP_fadeTo(uint16_t fromcol, uint16_t tocol, const uint16_t *pal, uint16_t numframe, uint8_t async)
{
    uint16_t tmp_pal[64];
    // read current palette
    VDP_getPaletteColors(fromcol, tmp_pal, (tocol - fromcol) + 1);
    // do the fade
    VDP_fade(fromcol, tocol, tmp_pal, pal, numframe, async);
}
