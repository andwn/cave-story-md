#ifdef SLOW_MODE

#include "types.h"

#include "vdp.h"
#include "dma.h"

#define VRAM_DMA_CMD    0x40000080
#define CRAM_DMA_CMD    0xC0000080
#define VSRAM_DMA_CMD   0x40000090

static volatile uint16_t *const vdp_data_port = (uint16_t *) 0xC00000;
static volatile uint16_t *const vdp_ctrl_port = (uint16_t *) 0xC00004;
static volatile uint32_t *const vdp_ctrl_wide = (uint32_t *) 0xC00004;

void dma_queue(uint32_t cmd, uint32_t from, uint16_t to, uint16_t len, uint16_t inc) {
    __asm__("move #0x2700,%sr");
    *vdp_ctrl_port = 0x8F00 | inc; // Auto inc
    uint32_t adr = to;
    if(cmd == VRAM_DMA_CMD)  {
        *vdp_ctrl_wide = (((0x4000LU + ((adr) & 0x3FFFLU)) << 16) + (((adr) >> 14) | 0x00));
    }
    if(cmd == CRAM_DMA_CMD) {
        *vdp_ctrl_wide = (((0xC000LU + ((adr) & 0x3FFFLU)) << 16) + (((adr) >> 14) | 0x00));
    }
    if(cmd == VSRAM_DMA_CMD) {
        *vdp_ctrl_wide = (((0x4000LU + ((adr) & 0x3FFFLU)) << 16) + (((adr) >> 14) | 0x10));
    }
    const uint16_t *ptr = (uint16_t*) from;
    for(uint16_t i = 0; i < len; i++) {
        *vdp_data_port = *ptr++;
    }
    *vdp_ctrl_port = 0x8F02;
    __asm__("move #0x2300,%sr");
}

void dma_pop() { }
void dma_flush() { }
void dma_clear() { }

#endif
