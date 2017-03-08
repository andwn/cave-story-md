#include "common.h"

#include "vdp.h"
#include "z80_ctrl.h"

#include "ym2612.h"

void YM2612_reset()
{
    uint16_t i;
    uint16_t bus_taken;

    bus_taken = Z80_isBusTaken();
    if (!bus_taken)
        Z80_requestBus(TRUE);

    // enable left and right output for all channel
    for(i = 0; i < 3; i++)
    {
        YM2612_write(0, 0xB4 | i);
        YM2612_write(1, 0xC0);
        YM2612_write(2, 0xB4 | i);
        YM2612_write(3, 0xC0);
    }

    // disable LFO
    YM2612_write(0, 0x22);
    YM2612_write(1, 0x00);

    // disable timer & set channel 6 to normal mode
    YM2612_write(0, 0x27);
    YM2612_write(1, 0x00);

    // ALL KEY OFF
    YM2612_write(0, 0x28);
    for (i = 0; i < 3; i++)
    {
        YM2612_write(1, 0x00 | i);
        YM2612_write(1, 0x04 | i);
    }

    // disable DAC
    YM2612_write(0, 0x2B);
    YM2612_write(1, 0x00);

    if (!bus_taken)
        Z80_releaseBus();
}


uint8_t YM2612_read(const uint16_t port)
{
    volatile uint8_t *pb;

    pb = (uint8_t*) YM2612_BASEPORT;

    return pb[port & 3];
}

void YM2612_write(const uint16_t port, const uint8_t data)
{
    volatile int8_t *pb;

    pb = (int8_t*) YM2612_BASEPORT;

    // wait while YM2612 busy
    while (*pb < 0);
    // write data
    pb[port & 3] = data;
}

void YM2612_writeSafe(const uint16_t port, const uint8_t data)
{
    YM2612_write(port, data);
}

void YM2612_writeReg(const uint16_t part, const uint8_t reg, const uint8_t data)
{
    volatile int8_t *pb;
    uint16_t port;

    pb = (int8_t*) YM2612_BASEPORT;
    port = (part << 1) & 2;

    // wait while YM2612 busy
    while (*pb < 0);
    // set reg
    pb[port + 0] = reg;

    // busy flag is not updated immediatly, force wait (needed on MD2)
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");

    // wait while YM2612 busy
    while (*pb < 0);
    // set data
    pb[port + 1] = data;
}

void YM2612_writeRegSafe(const uint16_t part, const uint8_t reg, const uint8_t data)
{
    YM2612_writeReg(part, reg, data);
}


void YM2612_enableDAC()
{
    // enable DAC
    YM2612_write(0, 0x2B);
    YM2612_write(1, 0x80);
}

void YM2612_disableDAC()
{
    // disable DAC
    YM2612_write(0, 0x2B);
    YM2612_write(1, 0x00);
}
