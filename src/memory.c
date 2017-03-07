#include "common.h"

#include "string.h"
#include "system.h"

#include "memory.h"

#define USED        1

// end of bss segment --> start of heap
extern uint32_t _bend;

/*
 * When memory is initialized HEAP point to first bloc as well than FREE.
 *
 *  memory start = HEAP     +-----------+
 *  HEAP+$0                 | size = ?? |
 *  HEAP+$2                 |           |
 *                          |           |
 *                          |           |
 *                          |           |
 *  HEAP+size = FREE        +-----------+
 *  FREE+$0                 | size = 0  |
 *  memory end              +-----------+
 *
 *
 *  1. Before allocation
 *
 *  FREE = HEAP = $FF0100
 *  +-------------------+
 *  | size = $1000      |
 *  +-------------------+
 *
 *  HEAP = $FF0100; *HEAP = $1000
 *  FREE = $FF0100; *FREE = $1000
 *  END  = $FF1100; *END  = $0      --> end memory
 *
 *
 *  2. After allocation of $100 bytes
 *
 *  FREE = $FF0100 + ($100+2) = $FF0202
 *  +--------------------------------+
 *  | size = $1000 - ($100+2) = $EFE |
 *  +--------------------------------+
 *
 *  HEAP = $FF0100; *HEAP = $102+1 (bit 0 = 1 --> used)
 *  FREE = $FF0202; *FREE = $EFE+0 (bit 0 = 0 --> free)
 *  END  = $FF1100; *END  = $0
 *
 *
 *  3. After allocation of $250 bytes
 *
 *  FREE = $FF0202 + ($250+2) = $FF0454
 *  +--------------------------------+
 *  | size = $EFE - ($250+2) = $CAC  |
 *  +--------------------------------+
 *
 *  HEAP = $FF0100; *HEAP = $102+1 (used)
 *  ---- = $FF0202; *---- = $252+1 (used)
 *  FREE = $FF0454; *FREE = $CAC+0 (free)
 *  END  = $FF1100; *END  = $0
 *
 *
 *  4. After allocation of $200, $150 and $730 bytes
 *
 *  FREE = $FF0454 + ($200+2) = $FF0656
 *  FREE = $FF0656 + ($150+2) = $FF07A8
 *  FREE = $FF07A8 + ($730+2) = $FF0EDA
 *  +--------------------------------+
 *  | size = $CAC - ($200+2) = $AAA  |
 *  | size = $AAA - ($150+2) = $958  |
 *  | size = $958 - ($730+2) = $226  |
 *  +--------------------------------+
 *
 *  HEAP = $FF0100; *HEAP = $102+1 (used)
 *  ---- = $FF0202; *---- = $252+1 (used)
 *  ---- = $FF0454; *---- = $202+1 (used)
 *  ---- = $FF0656; *---- = $152+1 (used)
 *  ---- = $FF07A8; *---- = $732+1 (used)
 *  FREE = $FF0EDA; *FREE = $226+0 (free)
 *  END  = $FF1100; *END  = $0
 *
 *
 *  5. After release of $FF0204-2
 *
 *  HEAP = $FF0100; *HEAP = $102+1 (used)
 *  ---- = $FF0202; *---- = $252+0 (free)
 *  ---- = $FF0454; *---- = $202+1 (used)
 *  ---- = $FF0656; *---- = $152+1 (used)
 *  ---- = $FF07A8; *---- = $732+1 (used)
 *  FREE = $FF0EDA; *FREE = $226+0 (free)
 *  END  = $FF1100; *END  = $0
 *
 *
 *  6. After release of $FF0456-2
 *
 *  HEAP = $FF0100; *HEAP = $102+1 (used)
 *  ---- = $FF0202; *---- = $252+0 (free)
 *  ---- = $FF0454; *---- = $202+1 (free)
 *  ---- = $FF0656; *---- = $152+1 (used)
 *  ---- = $FF07A8; *---- = $732+1 (used)
 *  FREE = $FF0EDA; *FREE = $226+0 (free)
 *  END  = $FF1100; *END  = $0
 *
 *
 *  7. After allocation of $400 bytes
 *
 *  *FREE = $226 < $400  --> packing
 *
 *  Starting from HEAP, find contiguous free block and collapse them.
 *  FREE becomes the first collaped block with size >= $400
 *
 *  HEAP = $FF0100; *HEAP = $102+1 (used)
 *  FREE = $FF0202; *FREE = $454+0 (free)
 *  ---- = $FF0656; *---- = $152+1 (used)
 *  ---- = $FF07A8; *---- = $732+1 (used)
 *  ---- = $FF0EDA; *---- = $226+0 (free)
 *  END  = $FF1100; *END  = $0
 *
 *  then
 *
 *  HEAP = $FF0100; *HEAP = $102+1 (used)
 *  ---- = $FF0202; *---- = $402+1 (used)
 *  FREE = $FF0604; *FREE = $052+0 (free)
 *  ---- = $FF0656; *---- = $152+1 (used)
 *  ---- = $FF07A8; *---- = $732+1 (used)
 *  ---- = $FF0EDA; *---- = $226+0 (free)
 *  END  = $FF1100; *END  = $0
 *
 *
 *  8. After allocation of $52 bytes
 *
 *  HEAP = $FF0100; *HEAP = $102+1 (used)
 *  ---- = $FF0202; *---- = $402+1 (used)
 *  ---- = $FF0604; *---- = $052+1 (used)
 *  ---- = $FF0656; *---- = $152+1 (used)
 *  ---- = $FF07A8; *---- = $732+1 (used)
 *  FREE = $FF0EDA; *FREE = $226+0 (free)
 *  END  = $FF1100; *END  = $0
 *
 */

 // forward
static uint16_t* pack(uint16_t nsize);

static uint16_t* free;
static uint16_t* heap;

void MEM_init()
{
    uint32_t h;
    uint32_t len;

    // point to end of bss (start of heap)
    h = (uint32_t)&_bend;
    // 2 bytes aligned
    h += 1;
    h >>= 1;
    h <<= 1;

    // define available memory (sizeof(uint16_t) is the memory reserved to indicate heap end)
    len = MEMORY_HIGH - (h + sizeof(uint16_t));

    // define heap
    heap = (uint16_t*) h;
    // and its size
    *heap = len;

    // free memory : whole heap
    free = heap;

    // mark end of heap memory
    heap[len >> 1] = 0;
}

uint16_t MEM_getFree()
{
    uint16_t* b;
    uint16_t bsize;
    uint16_t res;

    b = heap;
    res = 0;

    while ((bsize = *b))
    {
        // memory block not used --> add available size to result
        if (!(bsize & USED))
            res += bsize;

        // pass to next block
        b += bsize >> 1;
    }

    return res;
}

uint16_t MEM_getAllocated()
{
    uint16_t* b;
    uint16_t bsize;
    uint16_t res;

    b = heap;
    res = 0;

    while ((bsize = *b))
    {
        // memory block used --> add allocated size to result
        if (bsize & USED)
            res += bsize & ~USED;

        // pass to next block
        b += bsize >> 1;
    }

    return res;
}

void MEM_free(void *ptr)
{
    // valid block --> mark block as no more used
    if (ptr)
        ((uint16_t*)ptr)[-1] &= ~USED;
}

void* MEM_alloc(uint16_t size)
{
    uint16_t* p;
    uint16_t adjsize;
    uint16_t remaining;

    if (size == 0)
        return 0;

    // 2 bytes aligned
    adjsize = (size + sizeof(uint16_t) + 1) & 0xFFFE;

    if (adjsize > *free)
    {
        p = pack(adjsize);

        // no enough memory
        if (p == NULL)
        {
#if (LIB_DEBUG != 0)
            KDebug_Alert("MEM_alloc failed: no enough memory !");
#endif

            return NULL;
        }

        free = p;
    }
    else
        // at this point we can allocate memory
        p = free;

    // set free to next free block
    free += adjsize >> 1;

    // get remaining (old - allocated)
    remaining = *p - adjsize;
    // adjust remaining free space
    if (remaining > 0) *free = remaining;
    else
    {
        // no more space in bloc so we have to find the next free bloc
        uint16_t *newfree = free;
        uint16_t bloc;

        while((bloc = *newfree) & USED)
            newfree += bloc >> 1;

        free = newfree;
    }

    // set block size, mark as used and point to free region
    *p++ = adjsize | USED;

    // return block
    return p;
}

/*
 * Pack free block and return first matching free block.
 */
static uint16_t* pack(uint16_t nsize)
{
    uint16_t *b;
    uint16_t *best;
    uint16_t bsize, psize;

    b = heap;
    best = b;
    bsize = 0;

    while ((psize = *b))
    {
        if (psize & USED)
        {
            if (bsize != 0)
            {
                *best = bsize;

                if (bsize >= nsize)
                    return best;

                bsize = 0;
            }

            b += psize >> 1;
            best = b;
        }
        else
        {
            bsize += psize;
            b += psize >> 1;
        }
    }

    if (bsize != 0)
    {
        *best = bsize;

        if (bsize >= nsize)
            return best;
    }

    return NULL;
}
