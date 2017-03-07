#include "common.h"

#include "memory.h"
#include "string.h"
#include "timer.h"
#include "vdp.h"

#include "tools.h"

//forward
static uint16_t getTileSetAllocSize(const TileSet *tileset);
static TileSet *allocateTileSetInternal(const TileSet *tileset, void *adr);

// internal
static uint32_t framecnt;
static uint32_t last;
uint16_t randbase;


void setRandomSeed(uint16_t seed)
{
    // xor it with a random value to avoid 0 value
    randbase = seed ^ 0xD94B;
}

uint16_t random()
{
    randbase ^= (randbase >> 1) ^ GET_HVCOUNTER;
    randbase ^= (randbase << 1);

    return randbase;
}

uint32_t getFPS()
{
    static uint32_t result;

    const uint32_t current = getSubTick();
    const uint32_t delta = current - last;

    if (delta > 19200)
    {
        result = framecnt / delta;
        if (result > 999) result = 999;
        last = current;
        framecnt = 76800;
    }
    else framecnt += 76800;

    return result;
}

static uint16_t getTileSetAllocSize(const TileSet *tileset)
{
    // need space to decompress
    if (tileset->compression != COMPRESSION_NONE)
        return tileset->numTile * 32;

    return 0;
}

static TileSet *allocateTileSetInternal(const TileSet *tileset, void *adr)
{
    // cast
    TileSet *result = (TileSet*) adr;

    if (result != NULL)
    {
        result->compression = COMPRESSION_NONE;

        if (tileset->compression != COMPRESSION_NONE)
            // allocate sub buffers (no need to allocate palette as we directly use the source pointer)
            result->tiles = (uint32_t*) (adr + sizeof(TileSet));
        else
            // tileset is not compressed --> directly use source pointer
            result->tiles = tileset->tiles;
    }

    return result;
}

TileSet *allocateTileSet(const TileSet *tileset)
{
    return allocateTileSetInternal(tileset, MEM_alloc(getTileSetAllocSize(tileset) + sizeof(TileSet)));
}

TileSet *allocateTileSetEx(uint16_t numTile)
{
    // allocate
    void *adr = MEM_alloc((numTile * 32) + sizeof(TileSet));
    TileSet *result = (TileSet*) adr;

    if (result != NULL)
    {
        result->compression = COMPRESSION_NONE;
        // set tiles pointer
        result->tiles = (uint32_t*) (adr + sizeof(TileSet));
    }

    return result;
}

TileSet *unpackTileSet(const TileSet *src, TileSet *dest)
{
    TileSet *result;

    if (dest) result = dest;
    else result = allocateTileSet(src);

    if (result != NULL)
    {
        // fill infos
        result->numTile = src->numTile;

        // unpack tiles
        if (src->compression != COMPRESSION_NONE)
            unpack(src->compression, (uint8_t*) src->tiles, (uint8_t*) result->tiles);
        // simple copy if needed
        else if (src->tiles != result->tiles)
            memcpy((uint8_t*) result->tiles, (uint8_t*) src->tiles, src->numTile * 32);
    }

    return result;
}

uint32_t unpack(uint16_t compression, uint8_t *src, uint8_t *dest)
{
    switch(compression)
    {
//        case COMPRESSION_NONE:
//            // cannot do anything...
//            if (size == 0) return FALSE;
//
//            // use simple memory copy
//            memcpy(dest, &src[offset], size);
//            break;

//        case COMPRESSION_APLIB:
//            return aplib_unpack(src, dest);

        case COMPRESSION_LZ4W:
            return lz4w_unpack(src, dest);

        default:
            return 0;
    }
}
