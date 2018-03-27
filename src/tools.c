#include "common.h"

#include "memory.h"
#include "string.h"
#include "vdp.h"

#include "tools.h"

uint16_t randbase;

void setRandomSeed(uint16_t seed)
{
    // xor it with a random value to avoid 0 value
    randbase = seed ^ 0xD94B;
}

uint16_t random()
{
    randbase ^= (randbase >> 1) ^ *((volatile uint16_t*) 0xC00008);
    randbase ^= (randbase << 1);

    return randbase;
}

// wait for a certain amount of subtick
// WARNING : this function isn't accurate because of the VCounter rollback
void waitSubTick(uint32_t subtick) {
    uint32_t i = subtick;

	while(i--) {
		uint16_t j;

		// TODO: use cycle accurate wait loop in asm
		// about 100 cycles for 1 subtick
		j = 6;
		while(j--) __asm__("nop");
	}
}
