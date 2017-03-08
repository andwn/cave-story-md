#include "common.h"

#include "vdp.h"

#include "psg.h"

void PSG_init()
{
    volatile uint8_t *pb;
    uint16_t i;

    pb = (uint8_t*) PSG_PORT;

    for (i = 0; i < 4; i++)
    {
        // set tone to 0
        *pb = 0x80 | (i << 5) | 0x00;
        *pb = 0x00;

        // set envelope to silent
        *pb = 0x90 | (i << 5) | 0x0F;
    }
}


void PSG_write(uint8_t data)
{
    volatile uint8_t *pb;

    pb = (uint8_t*) PSG_PORT;
    *pb = data;
}


void PSG_setEnvelope(uint8_t channel, uint8_t value)
{
    volatile uint8_t *pb;

    pb = (uint8_t*) PSG_PORT;
    *pb = 0x90 | ((channel & 3) << 5) | (value & 0xF);
}

void PSG_setTone(uint8_t channel, uint16_t value)
{
    volatile uint8_t *pb;

    pb = (uint8_t*) PSG_PORT;
    *pb = 0x80 | ((channel & 3) << 5) | (value & 0xF);
    *pb = (value >> 4) & 0x3F;
}

void PSG_setFrequency(uint8_t channel, uint16_t value)
{
    uint16_t data;

    if (value)
    {
        // frequency to tone conversion
        if (IS_PALSYSTEM) data = 3546893 / (value * 32);
        else data = 3579545 / (value * 32);
    }
    else data = 0;

    PSG_setTone(channel, data);
}

void PSG_setNoise(uint8_t type, uint8_t frequency)
{
    volatile uint8_t *pb;

    pb = (uint8_t *) PSG_PORT;
    *pb = 0xE0 | ((type & 1) << 2) | (frequency & 0x3);
}
