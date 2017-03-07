#include "common.h"

#include "vdp.h"
#include "system.h"

#include "timer.h"

// TODO : use H counter for more accurate timer
//
// H Counter values
//
// Description            32-cell         40-cell
// ----------------------------------------------------------------------------
// Range                  00-93, E9-FF    00-B6, E4-FF
// Display area           00-7F           00-9F
// V counter increment    84, 85          A4, A5
// V-blanking in          86, 87          A7, A8
// V-blanking out         86, 87          A7, A8
// H-blanking in          93, E9          B2, E4
// H-blanking out         06, 07          06, 07


uint32_t vtimer;

static uint32_t timer[MAXTIMER];


// return elapsed time from console reset (1/76800 second based)
// WARNING : this function isn't accurate because of the VCounter rollback
uint32_t getSubTick()
{
    uint32_t vcnt;

    vcnt = GET_VCOUNTER;
    const uint32_t scrh = screenHeight;

    // as VCounter roolback in blank area we use a "medium" value
    if (vcnt >= scrh) vcnt = 16;
    else vcnt += (256 - scrh);

    const uint32_t current = (vtimer << 8) + vcnt;

    if (IS_PALSYSTEM) return current * 6;
    else return current * 5;
}

// return elapsed time from console reset (1/300 second based)
uint32_t getTick()
{
    if (IS_PALSYSTEM) return vtimer * 6;
    else return vtimer * 5;
}

// return elapsed time from console reset (1/256 second based)
uint32_t getTime(uint16_t fromTick)
{
    uint32_t result;

    if (fromTick) result = getTick() << 8;
    else result = getSubTick();

    return result / TICKPERSECOND;
}


void startTimer(uint16_t numTimer)
{
    timer[numTimer & (MAXTIMER - 1)] = getSubTick();
}

uint32_t getTimer(uint16_t numTimer, uint16_t restart)
{
    const uint32_t t = getSubTick();
    uint32_t* time = &timer[numTimer & (MAXTIMER - 1)];
    const uint32_t res = t - *time;

    if (restart) *time = t;

    return res;
}


// wait for a certain amount of subtick
// WARNING : this function isn't accurate because of the VCounter rollback
void waitSubTick(uint32_t subtick)
{
    uint32_t start;
    uint32_t current;
    uint32_t i;

	i = subtick;

	while(i--)
	{
		uint16_t j;

		// TODO: use cycle accurate wait loop in asm
		// about 100 cycles for 1 subtick
		j = 6;
		while(j--) __asm__("nop");
	}

	return;

    start = getSubTick();

    // wait until we reached subtick
    do
    {
        current = getSubTick();
        // error du to the VCounter roolback, ignore...
        if (current < start) current = start;
    }
    while ((current - start) < subtick);
}

// wait for a certain amount of tick
void waitTick(uint32_t tick)
{
    uint32_t start;
    uint32_t i;

	i = tick;

	while(i--) waitSubTick(256);

	return;

    start = getTick();
    // wait until we reached tick
    while ((getTick() - start) < tick);
}

// wait for a certain amount of millisecond (~3.33 ms based timer so use 4 ms at least)
void waitMs(uint32_t ms)
{
    waitTick((ms * TICKPERSECOND) / 1000);
}
