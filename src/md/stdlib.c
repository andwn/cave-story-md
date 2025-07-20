#include "types.h"
#include "system.h"
#include "md/sys.h"
#include "stdlib.h"

/* RNG */

uint16_t randbase;

void srand(uint16_t seed) {
    randbase = seed ^ 0xD94B;
}

uint16_t rand(void) {
    randbase ^= (randbase >> 1) ^ *((volatile uint16_t*) 0xC00008);
    randbase ^= (randbase << 1);

    return randbase;
}

/* Heap Stuff */

#define USED        1

extern uint32_t __heap_start;
extern uint32_t __heap_end;
extern uint32_t __heap_size;

static uint16_t* s_free;
static uint16_t* s_heap;

static uint16_t* pack(uint16_t nsize) {
    uint16_t psize;
    uint16_t *b = s_heap;
    uint16_t *best = b;
    uint16_t bsize = 0;
    while((psize = *b)) {
        if(psize & USED) {
            if(bsize != 0) {
                *best = bsize;
                if(bsize >= nsize) return best;
                bsize = 0;
            }
            b += psize >> 1;
            best = b;
        } else {
            bsize += psize;
            b += psize >> 1;
        }
    }
    if(bsize != 0) {
        *best = bsize;
        if(bsize >= nsize) return best;
    }
    return NULL;
}

void mem_init(void) {
    // define available memory (sizeof(uint16_t) is the memory reserved to indicate heap end)
    uint32_t len = (uint32_t) &__heap_size - sizeof(uint16_t);
    // define heap
    s_heap = (uint16_t*) &__heap_start;
    // and its size
    *s_heap = len;
    // free memory : whole heap
    s_free = s_heap;
    // mark end of heap memory
    s_heap[len >> 1] = 0;
}

uint16_t mem_get_free(void) {
    uint16_t bsize;
    uint16_t* b = s_heap;
    uint16_t res = 0;
    while((bsize = *b)) {
        // memory block not used --> add available size to result
        if(!(bsize & USED)) res += bsize;
        // pass to next block
        b += bsize >> 1;
    }
    return res;
}

uint16_t mem_get_used(void) {
    uint16_t bsize;
    uint16_t* b = s_heap;
    uint16_t res = 0;
    while((bsize = *b)) {
        // memory block used --> add allocated size to result
        if(bsize & USED) res += bsize & ~USED;
        // pass to next block
        b += bsize >> 1;
    }
    return res;
}

void free(void *ptr) {
    // valid block --> mark block as no more used
    if(ptr) ((uint16_t*)ptr)[-1] &= ~USED;
}

void* malloc(uint16_t size) {
    if(size == 0) return 0;
    uint16_t* p;
    // 2 bytes aligned
    uint16_t adjsize = (size + sizeof(uint16_t) + 1) & 0xFFFE;
    if(adjsize > *s_free) {
        p = pack(adjsize);
        // no enough memory
        if (p == NULL) return NULL;
        s_free = p;
    } else {
        // at this point we can allocate memory
        p = s_free;
    }
    // set free to next free block
    s_free += adjsize >> 1;
    // get remaining (old - allocated)
    uint16_t remaining = *p - adjsize;
    // adjust remaining free space
    if(remaining > 0) {
        *s_free = remaining;
    } else {
        // no more space in bloc so we have to find the next free bloc
        uint16_t *newfree = s_free;
        uint16_t bloc;
        while((bloc = *newfree) & USED) newfree += bloc >> 1;
        s_free = newfree;
    }
    // set block size, mark as used and point to free region
    *p++ = adjsize | USED;
    // return block
    return p;
}
