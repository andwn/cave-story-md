#include "common.h"

#include "dma.h"
#include "memory.h"
#include "system.h"
#include "tools.h"
#include "vdp.h"
#include "vdp_pal.h"

#include "vdp_tile.h"

uint16_t VDP_loadTileSet(const TileSet *tileset, uint16_t index, uint8_t use_dma) {
    uint32_t *tiles = tileset->tiles;
    //if(tileset->compression) {
    //    tiles = MEM_alloc(tileset->numTile * TILE_SIZE);
    //    if(!tiles) error_oom();
    //    lz4w_unpack((uint8_t*) tileset->tiles, (uint8_t*) tiles);
    //}
    VDP_loadTileData(tiles, index, tileset->numTile, use_dma);
    //if(tileset->compression) MEM_free(tiles);
    return TRUE;
}

uint16_t VDP_loadFont(const TileSet *font, uint8_t use_dma)
{
    return VDP_loadTileSet(font, TILE_FONTINDEX, use_dma);
}

void VDP_fillTileData(uint8_t value, uint16_t index, uint16_t num, uint16_t wait)
{
    // do DMA fill
    DMA_doVRamFill(index * 32, num * 32, value, 1);
    // wait for DMA completion
    if (wait)
        VDP_waitDMACompletion();
}


void VDP_setTileMap(uint16_t plan, uint16_t tile, uint16_t ind)
{
    volatile uint32_t *plctrl;
    volatile uint16_t *pwdata;

    const uint32_t addr = plan + (ind * 2);

    /* point to vdp port */
    plctrl = (uint32_t *) GFX_CTRL_PORT;
    pwdata = (uint16_t *) GFX_DATA_PORT;

    *plctrl = GFX_WRITE_VRAM_ADDR(addr);
    *pwdata = tile;
}

void VDP_setTileMapXY(VDPPlan plan, uint16_t tile, uint16_t x, uint16_t y)
{
    volatile uint32_t *plctrl;
    volatile uint16_t *pwdata;
    uint32_t addr;

    switch(plan.value)
    {
        case CONST_PLAN_A:
            addr = VDP_PLAN_A + ((x + (y << PLAN_WIDTH_SFT)) * 2);
            break;

        case CONST_PLAN_B:
            addr = VDP_PLAN_B + ((x + (y << PLAN_WIDTH_SFT)) * 2);
            break;

        case CONST_PLAN_WINDOW:
            addr = VDP_PLAN_WINDOW + ((x + (y << WINDOW_WIDTH_SFT)) * 2);
            break;

        default:
            return;
    }

    /* point to vdp port */
    plctrl = (uint32_t *) GFX_CTRL_PORT;
    pwdata = (uint16_t *) GFX_DATA_PORT;

    *plctrl = GFX_WRITE_VRAM_ADDR(addr);
    *pwdata = tile;
}

void VDP_fillTileMap(uint16_t plan, uint16_t tile, uint16_t ind, uint16_t num)
{
    volatile uint32_t *plctrl;
    volatile uint16_t *pwdata;
    volatile uint32_t *pldata;
    uint32_t addr;
    uint16_t i;

    VDP_setAutoInc(2);

    /* point to vdp port */
    plctrl = (uint32_t *) GFX_CTRL_PORT;
    pldata = (uint32_t *) GFX_DATA_PORT;

    addr = plan + (ind * 2);

    *plctrl = GFX_WRITE_VRAM_ADDR(addr);

    const uint32_t tile32 = (((uint32_t)(tile)) << 16) | tile;

    i = num >> 3;
    while (i--)
    {
        *pldata = tile32;
        *pldata = tile32;
        *pldata = tile32;
        *pldata = tile32;
    }

    pwdata = (uint16_t *) GFX_DATA_PORT;

    i = num & 7;
    while (i--) *pwdata = tile;
}

void VDP_fillTileMapRect(VDPPlan plan, uint16_t tile, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    volatile uint32_t *plctrl;
    volatile uint16_t *pwdata;
    uint32_t addr;
    uint32_t width;
    uint16_t i, j;

    switch(plan.value)
    {
        case CONST_PLAN_A:
            addr = VDP_PLAN_A + ((x + (y << PLAN_WIDTH_SFT)) * 2);
            width = PLAN_WIDTH;
            break;

        case CONST_PLAN_B:
            addr = VDP_PLAN_B + ((x + (y << PLAN_WIDTH_SFT)) * 2);
            width = PLAN_WIDTH;
            break;

        case CONST_PLAN_WINDOW:
            addr = VDP_PLAN_WINDOW + ((x + (y << WINDOW_WIDTH_SFT)) * 2);
            width = WINDOW_WIDTH;
            break;

        default:
            return;
    }

    VDP_setAutoInc(2);

    /* point to vdp port */
    plctrl = (uint32_t *) GFX_CTRL_PORT;
    pwdata = (uint16_t *) GFX_DATA_PORT;

    i = h;
    while (i--)
    {
        *plctrl = GFX_WRITE_VRAM_ADDR(addr);

        j = w;

        while (j--) *pwdata = tile;

        addr += width * 2;
    }
}

void VDP_clearTileMap(uint16_t plan, uint16_t ind, uint16_t num, uint16_t wait)
{
    // do DMA fill
    DMA_doVRamFill(plan + (ind * 2), num * 2, 0, 1);
    // wait for DMA completion
    if (wait)
        VDP_waitDMACompletion();
}

void VDP_clearTileMapRect(VDPPlan plan, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    VDP_fillTileMapRect(plan, 0, x, y, w, h);
}

void VDP_fillTileMapInc(uint16_t plan, uint16_t basetile, uint16_t ind, uint16_t num)
{
    volatile uint32_t *plctrl;
    volatile uint16_t *pwdata;
    volatile uint32_t *pldata;
    uint32_t addr;
    uint16_t tile;
    uint32_t tile32;
    uint32_t step;
    uint16_t i;

    VDP_setAutoInc(2);

    /* point to vdp port */
    plctrl = (uint32_t *) GFX_CTRL_PORT;
    pldata = (uint32_t *) GFX_DATA_PORT;

    addr = plan + (ind * 2);

    *plctrl = GFX_WRITE_VRAM_ADDR(addr);

    tile = basetile;
    tile32 = (((uint32_t)(tile)) << 16) | tile;
    step = 0x10001;

    i = num >> 3;
    while (i--)
    {
        *pldata = tile32;
        tile32 += step;
        *pldata = tile32;
        tile32 += step;
        *pldata = tile32;
        tile32 += step;
        *pldata = tile32;
        tile32 += step;
    }

    pwdata = (volatile uint16_t *) GFX_DATA_PORT;

    i = num & 7;
    while (i--) *pwdata = tile++;
}

void VDP_fillTileMapRectInc(VDPPlan plan, uint16_t basetile, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    volatile uint32_t *plctrl;
    volatile uint16_t *pwdata;
    uint32_t addr;
    uint32_t width;
    uint16_t tile;
    uint16_t i, j;

    switch(plan.value)
    {
        case CONST_PLAN_A:
            addr = VDP_PLAN_A + ((x + (y << PLAN_WIDTH_SFT)) * 2);
            width = PLAN_WIDTH;
            break;

        case CONST_PLAN_B:
            addr = VDP_PLAN_B + ((x + (y << PLAN_WIDTH_SFT)) * 2);
            width = PLAN_WIDTH;
            break;

        case CONST_PLAN_WINDOW:
            addr = VDP_PLAN_WINDOW + ((x + (y << WINDOW_WIDTH_SFT)) * 2);
            width = WINDOW_WIDTH;
            break;

        default:
            return;
    }

    VDP_setAutoInc(2);

    /* point to vdp port */
    plctrl = (uint32_t *) GFX_CTRL_PORT;
    pwdata = (uint16_t *) GFX_DATA_PORT;

    tile = basetile;

    i = h;
    while (i--)
    {
        *plctrl = GFX_WRITE_VRAM_ADDR(addr);

        j = w;

        while (j--) *pwdata = tile++;

        addr += width * 2;
    }
}

void VDP_setTileMapData(uint16_t plan, const uint16_t *data, uint16_t ind, uint16_t num, uint8_t use_dma)
{
    if (use_dma)
    {
        // wait for previous DMA completion
        VDP_waitDMACompletion();
        // then do DMA
        DMA_doDma(DMA_VRAM, (uint32_t) data, plan + (ind * 2), num, 2);
    }
    else
    {
        volatile uint32_t *plctrl;
        volatile uint16_t *pwdata;
        volatile uint32_t *pldata;
        const uint16_t *src;
        const uint32_t *src32;
        uint32_t addr;
        uint16_t i;

        VDP_setAutoInc(2);

        /* point to vdp port */
        plctrl = (uint32_t *) GFX_CTRL_PORT;
        pldata = (uint32_t *) GFX_DATA_PORT;

        addr = plan + (ind * 2);

        *plctrl = GFX_WRITE_VRAM_ADDR(addr);

        src32 = (uint32_t*) data;

        i = num >> 3;
        while (i--)
        {
            *pldata = *src32++;
            *pldata = *src32++;
            *pldata = *src32++;
            *pldata = *src32++;
        }

        pwdata = (uint16_t *) GFX_DATA_PORT;

        src = (uint16_t*) src32;

        i = num & 7;
        while (i--) *pwdata = *src++;
    }
}

void VDP_setTileMapDataRect(VDPPlan plan, const uint16_t *data, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    volatile uint32_t *plctrl;
    volatile uint16_t *pwdata;
    const uint16_t *src;
    uint32_t addr;
    uint32_t width;
    uint16_t i, j;

    switch(plan.value)
    {
        case CONST_PLAN_A:
            addr = VDP_PLAN_A + ((x + (y << PLAN_WIDTH_SFT)) * 2);
            width = PLAN_WIDTH;
            break;

        case CONST_PLAN_B:
            addr = VDP_PLAN_B + ((x + (y << PLAN_WIDTH_SFT)) * 2);
            width = PLAN_WIDTH;
            break;

        case CONST_PLAN_WINDOW:
            addr = VDP_PLAN_WINDOW + ((x + (y << WINDOW_WIDTH_SFT)) * 2);
            width = WINDOW_WIDTH;
            break;

        default:
            return;
    }

    VDP_setAutoInc(2);

    /* point to vdp port */
    plctrl = (uint32_t *) GFX_CTRL_PORT;
    pwdata = (uint16_t *) GFX_DATA_PORT;

    src = data;

    i = h;
    while (i--)
    {
        *plctrl = GFX_WRITE_VRAM_ADDR(addr);

        j = w;
        while (j--) *pwdata = *src++;

        addr += width * 2;
    }
}

void VDP_setTileMapDataEx(uint16_t plan, const uint16_t *data, uint16_t basetile, uint16_t ind, uint16_t num)
{
    volatile uint32_t *plctrl;
    volatile uint16_t *pwdata;
    volatile uint32_t *pldata;
    const uint16_t *src;
    const uint32_t *src32;
    uint32_t addr;
    uint16_t baseindex;
    uint16_t baseflags;
    uint32_t bi32;
    uint32_t bf32;
    uint16_t i;

    VDP_setAutoInc(2);

    /* point to vdp port */
    plctrl = (uint32_t *) GFX_CTRL_PORT;
    pldata = (uint32_t *) GFX_DATA_PORT;

    addr = plan + (ind * 2);

    *plctrl = GFX_WRITE_VRAM_ADDR(addr);

    src32 = (uint32_t*) data;
    baseindex = basetile & TILE_INDEX_MASK;
    baseflags = basetile & TILE_ATTR_MASK;
    bi32 = (((uint32_t)(baseindex)) << 16) | baseindex;
    bf32 = (((uint32_t)(baseflags)) << 16) | baseflags;

    i = num >> 3;
    while (i--)
    {
        *pldata = bf32 | (*src32++ + bi32);
        *pldata = bf32 | (*src32++ + bi32);
        *pldata = bf32 | (*src32++ + bi32);
        *pldata = bf32 | (*src32++ + bi32);
    }

    pwdata = (uint16_t *) GFX_DATA_PORT;

    src = (uint16_t*) src32;

    i = num & 7;
    while (i--) *pwdata = baseflags | (*src++ + baseindex);
}

void VDP_setTileMapRectEx(VDPPlan plan, const uint16_t *data, uint16_t baseindex, uint16_t baseflags, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    VDP_setTileMapDataRectEx(plan, data, baseflags | baseindex, x, y, w, h, w);
}

void VDP_setTileMapDataRectEx(VDPPlan plan, const uint16_t *data, uint16_t basetile, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t wm)
{
    volatile uint32_t *plctrl;
    volatile uint16_t *pwdata;
    const uint16_t *src;
    uint32_t addr;
    uint32_t width;
    uint16_t baseindex;
    uint16_t baseflags;
    uint16_t i, j;

    switch(plan.value)
    {
        case CONST_PLAN_A:
            addr = VDP_PLAN_A + ((x + (y << PLAN_WIDTH_SFT)) * 2);
            width = PLAN_WIDTH;
            break;

        case CONST_PLAN_B:
            addr = VDP_PLAN_B + ((x + (y << PLAN_WIDTH_SFT)) * 2);
            width = PLAN_WIDTH;
            break;

        case CONST_PLAN_WINDOW:
            addr = VDP_PLAN_WINDOW + ((x + (y << WINDOW_WIDTH_SFT)) * 2);
            width = WINDOW_WIDTH;
            break;

        default:
            return;
    }

    VDP_setAutoInc(2);

    /* point to vdp port */
    plctrl = (uint32_t *) GFX_CTRL_PORT;
    pwdata = (uint16_t *) GFX_DATA_PORT;

    baseindex = basetile & TILE_INDEX_MASK;
    baseflags = basetile & TILE_ATTR_MASK;
    src = data;

    i = h;
    while (i--)
    {
        *plctrl = GFX_WRITE_VRAM_ADDR(addr);

        j = w;
        while (j--) *pwdata = baseflags | (*src++ + baseindex);

        src += wm - w;
        addr += width * 2;
    }
}
