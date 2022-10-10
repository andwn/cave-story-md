#include "common.h"
#include "dma.h"
#include "vdp.h"

static volatile uint16_t *const vdp_data_port = (uint16_t *) 0xC00000;
static volatile uint16_t *const vdp_ctrl_port = (uint16_t *) 0xC00004;
static volatile uint32_t *const vdp_ctrl_wide = (uint32_t *) 0xC00004;

typedef struct {
    uint16_t inc;
    uint16_t len1;
    uint16_t len2;
    uint16_t from1;
    uint16_t from2;
    uint16_t from3;
    uint32_t cmd;
} QueueEntry;

extern QueueEntry *dmabuf_end;

void dma_now(uint32_t cmd, uint32_t from, uint16_t to, uint16_t len, uint16_t inc) {
    // Set auto increment
    *vdp_ctrl_port = 0x8F00 | inc;
    // Setup DMA length (in word here)
    *vdp_ctrl_port = 0x9300 + (len & 0xff);
    *vdp_ctrl_port = 0x9400 + ((len >> 8) & 0xff);
    // Setup DMA address
    from >>= 1;
    *vdp_ctrl_port = 0x9500 + (from & 0xff);
    from >>= 8;
    *vdp_ctrl_port = 0x9600 + (from & 0xff);
    from >>= 8;
    *vdp_ctrl_port = 0x9700 + (from & 0x7f);
    // Enable DMA transfer
    *vdp_ctrl_wide = cmd | (((((uint32_t) to) & 0x3FFF) << 16) + (((uint32_t) to) >> 14));
}

void dma_queue(uint32_t cmd, uint32_t from, uint16_t to, uint16_t len, uint16_t inc) {
    dmabuf_end->inc = 0x8F00 | inc;
    // Setup DMA length (in word here)
    dmabuf_end->len1 = 0x9300 + (len & 0xff);
    dmabuf_end->len2 = 0x9400 + ((len >> 8) & 0xff);
    // Setup DMA address
    from >>= 1;
    dmabuf_end->from1 = 0x9500 + (from & 0xff);
    from >>= 8;
    dmabuf_end->from2 = 0x9600 + (from & 0xff);
    from >>= 8;
    dmabuf_end->from3 = 0x9700 + (from & 0x7f);
    // Enable DMA transfer
    dmabuf_end->cmd = cmd | (((((uint32_t) to) & 0x3FFF) << 16) + (((uint32_t) to) >> 14));
    dmabuf_end++;
}
