#include "config.h"
#include "types.h"

#include "tools.h"

#include "string.h"
#include "kdebug.h"
#include "timer.h"
#include "maths.h"
#include "memory.h"
#include "vdp.h"

//forward
static u16 getTileSetAllocSize(const TileSet *tileset);
static TileSet *allocateTileSetInternal(const TileSet *tileset, void *adr);

// internal
static u32 framecnt;
static u32 last;
u16 randbase;


void setRandomSeed(u16 seed)
{
    // xor it with a random value to avoid 0 value
    randbase = seed ^ 0xD94B;
}

u16 random()
{
    randbase ^= (randbase >> 1) ^ GET_HVCOUNTER;
    randbase ^= (randbase << 1);

    return randbase;
}

u32 getFPS()
{
    static u32 result;

    const u32 current = getSubTick();
    const u32 delta = current - last;

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

static u16 getTileSetAllocSize(const TileSet *tileset)
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
            result->tiles = (u32*) (adr + sizeof(TileSet));
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

TileSet *allocateTileSetEx(u16 numTile)
{
    // allocate
    void *adr = MEM_alloc((numTile * 32) + sizeof(TileSet));
    TileSet *result = (TileSet*) adr;

    if (result != NULL)
    {
        result->compression = COMPRESSION_NONE;
        // set tiles pointer
        result->tiles = (u32*) (adr + sizeof(TileSet));
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
            unpack(src->compression, (u8*) src->tiles, (u8*) result->tiles);
        // simple copy if needed
        else if (src->tiles != result->tiles)
            memcpy((u8*) result->tiles, (u8*) src->tiles, src->numTile * 32);
    }

    return result;
}

u32 unpack(u16 compression, u8 *src, u8 *dest)
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
