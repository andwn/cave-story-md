#include "common.h"

#include "memory.h"
#include "psg.h"
#include "smp_null.h"
#include "smp_null_pcm.h"
#include "system.h"
#include "tab_vol.h"
#include "tools.h"
#include "vdp.h"
#include "xgm.h"
#include "ym2612.h"
#include "z80_xgm.h"

#include "z80_ctrl.h"

void Z80_init() {
	uint8_t *pb;
    uint32_t addr;
    // request Z80 bus
    Z80_requestBus(TRUE);
    // set bank to 0
    Z80_setBank(0);
    // clear z80 memory
    Z80_clear(0, Z80_RAM_LEN, FALSE);
    // upload Z80 driver and reset Z80
    Z80_upload(0, z80_xgm, sizeof(z80_xgm), 1);
	// reset sound chips
	YM2612_reset();
	PSG_init();
	// misc parameters initialisation
	Z80_requestBus(TRUE);
	// point to Z80 sample id table (first entry = silent sample)
	pb = (uint8_t *) (0xA01C00);

	addr = (uint32_t) smp_null;
	// null sample address (256 bytes aligned)
	pb[0] = addr >> 8;
	pb[1] = addr >> 16;
	// null sample length (256 bytes aligned)
	pb[2] = sizeof(smp_null) >> 8;
	pb[3] = sizeof(smp_null) >> 16;
	Z80_releaseBus();
	// wait bus released
	while(Z80_isBusTaken());
	// just wait for it
	while(!Z80_isDriverReady())
		while(Z80_isBusTaken());
    XGM_setMusicTempo(IS_PALSYSTEM ? 50 : 60);
}

uint16_t Z80_isBusTaken()
{
    volatile uint16_t *pw;

    pw = (uint16_t *) Z80_HALT_PORT;
    if (*pw & 0x0100) return 0;
    else return 1;
}

void Z80_requestBus(uint16_t wait)
{
    volatile uint16_t *pw_bus;
    volatile uint16_t *pw_reset;

    // request bus (need to end reset)
    pw_bus = (uint16_t *) Z80_HALT_PORT;
    pw_reset = (uint16_t *) Z80_RESET_PORT;

    // take bus and end reset
    *pw_bus = 0x0100;
    *pw_reset = 0x0100;

    if (wait)
    {
        // wait for bus taken
        while (*pw_bus & 0x0100);
    }
}

void Z80_releaseBus()
{
    volatile uint16_t *pw;

    pw = (uint16_t *) Z80_HALT_PORT;
    *pw = 0x0000;
}


void Z80_startReset()
{
    volatile uint16_t *pw;

    pw = (uint16_t *) Z80_RESET_PORT;
    *pw = 0x0000;
}

void Z80_endReset()
{
    volatile uint16_t *pw;

    pw = (uint16_t *) Z80_RESET_PORT;
    *pw = 0x0100;
}


void Z80_setBank(const uint16_t bank)
{
    volatile uint8_t *pb;
    uint16_t i, value;

    pb = (uint8_t *) Z80_BANK_REGISTER;

    i = 9;
    value = bank;
    while (i--)
    {
        *pb = value;
        value >>= 1;
    }
}

uint8_t Z80_read(const uint16_t addr)
{
    return ((uint8_t*) Z80_RAM)[addr];
}

void Z80_write(const uint16_t addr, const uint8_t value)
{
    ((uint8_t*) Z80_RAM)[addr] = value;
}

void Z80_clear(const uint16_t to, const uint16_t size, const uint16_t resetz80)
{
    Z80_requestBus(TRUE);

    const uint8_t zero = 0;
    uint8_t* dst = (uint8_t*) (Z80_RAM + to);
    uint16_t len = size;

    while(len--) *dst++ = zero;

    if (resetz80) Z80_startReset();
    Z80_releaseBus();
    // wait bus released
    while(Z80_isBusTaken());
    if (resetz80) Z80_endReset();
}

void Z80_upload(const uint16_t to, const uint8_t *from, const uint16_t size, const uint16_t resetz80)
{
    Z80_requestBus(TRUE);

    // copy data to Z80 RAM (need to use byte copy here)
    uint8_t* src = (uint8_t*) from;
    uint8_t* dst = (uint8_t*) (Z80_RAM + to);
    uint16_t len = size;

    while(len--) *dst++ = *src++;

    if (resetz80) Z80_startReset();
    Z80_releaseBus();
    // wait bus released
    while(Z80_isBusTaken());
    if (resetz80) Z80_endReset();
}

void Z80_download(const uint16_t from, uint8_t *to, const uint16_t size)
{
    Z80_requestBus(TRUE);

    // copy data from Z80 RAM (need to use byte copy here)
    uint8_t* src = (uint8_t*) (Z80_RAM + from);
    uint8_t* dst = (uint8_t*) to;
    uint16_t len = size;

    while(len--) *dst++ = *src++;

    Z80_releaseBus();
}

uint16_t Z80_isDriverReady()
{
    uint8_t *pb;
    uint8_t ret;

    // point to Z80 status
    pb = (uint8_t *) Z80_DRV_STATUS;

    // bus already taken ? just check status
    if (Z80_isBusTaken())
        ret = *pb & Z80_DRV_STAT_READY;
    else
    {
        // take the bus, check status and release bus
        Z80_requestBus(TRUE);
        ret = *pb & Z80_DRV_STAT_READY;
        Z80_releaseBus();
    }

    return ret;
}
