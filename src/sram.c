#include "common.h"

#include "sram.h"

void SRAM_enable()
{
    *(volatile uint8_t*)SRAM_CONTROL = 1;
}

void SRAM_enableRO()
{
    *(volatile uint8_t*)SRAM_CONTROL = 3;
}

void SRAM_disable()
{
    *(volatile uint8_t*)SRAM_CONTROL = 0;
}


uint8_t SRAM_readByte(uint32_t offset)
{
    return *(volatile uint8_t*)(SRAM_BASE + (offset * 2));
}

void SRAM_writeByte(uint32_t offset, uint8_t val)
{
    *(volatile uint8_t*)(SRAM_BASE + (offset * 2)) = val;
}
