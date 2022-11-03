#include "common.h"
#include "vdp.h"
#include "xgm.h"
#include "bank_data.h"

extern const uint8_t smp_null[0x100];

// Variables
extern volatile uint16_t xgmTempo;
extern volatile uint16_t xgmTempoDef;
extern volatile int16_t xgmTempoCnt;

#define Z80_RAM 0xA00000
static volatile uint8_t* const z80_drv_command = (uint8_t*) (Z80_RAM+0x0100);
static volatile uint8_t* const z80_drv_params  = (uint8_t*) (Z80_RAM+0x0104);
static volatile uint8_t* const z80_smp_table   = (uint8_t*) (Z80_RAM+0x1C00);

void xgm_init() {
    // Hold reset, request bus, and release reset
    __asm__("move.w  #0x000,(0xA11200)\n\t"
            "move.w  #0x100,(0xA11100)\n\t"
            "move.w  #0x100,(0xA11200)");
    // Copy XGM driver blob to Z80 memory
    uint16_t size = (uint16_t) (z80_xgm_end-z80_xgm);
    uint8_t* src = (uint8_t*) z80_xgm;
    uint8_t* dst = (uint8_t*) Z80_RAM;
    while(size--) *dst++ = *src++;
    // Need to override first sample with smp_null address and size
    const uint16_t smp_addr = (uint32_t) smp_null >> 8;
    const uint16_t smp_size = sizeof(smp_null) >> 8;
    z80_smp_table[0] = smp_addr;
    z80_smp_table[1] = smp_addr >> 8;
    z80_smp_table[2] = smp_size;
    z80_smp_table[3] = smp_size >> 8;
    // Hold reset for ~200 cycles, then release both reset and bus
    __asm__("move.w  #0x000,(0xA11200)");
    for(volatile uint16_t i = 20; i--;);
    __asm__("move.w  #0x100, (0xA11200)\n\t"
            "move.w  #0x000, (0xA11100)");
	// Set the tempo
	xgmTempo = 60;
	xgmTempoDef = pal_mode ? 50 : 60;
}

void xgm_music_play(const uint8_t *xgc) {
    disable_ints;
    z80_pause();
    // Prepare the sample ID table
    volatile uint8_t *z80_smptr = z80_smp_table + 4;
    const uint8_t *xgc_smptr = xgc;
    for(uint16_t i = 0; i < 0x3F; i++) {
        // First 2 bytes is the sample address (aligned to 256)
        uint16_t addr = *xgc_smptr++;
        addr |= *xgc_smptr++ << 8;
        if(addr == 0xFFFF) {
            // Special value for null sample, copy smp_null address instead
            const uint16_t smp_addr = (uint32_t) smp_null >> 8;
            addr = smp_addr;
        } else {
            // Make address absolute
            addr += ((uint32_t) xgc >> 8) + 1;
        }
        *z80_smptr++ = addr;
        *z80_smptr++ = addr >> 8;
        // Copy sample length
        *z80_smptr++ = *xgc_smptr++;
        *z80_smptr++ = *xgc_smptr++;
    }
    // Get music sequence data address and bypass sample data
    uint32_t seq_addr = (uint32_t) xgc + 0x104;
    seq_addr += (uint32_t) xgc[0xFC] << 8;
    seq_addr += (uint32_t) xgc[0xFD] << 16;
    // set XGM music data address
    z80_drv_params[0x00] = seq_addr >> 0;
    z80_drv_params[0x01] = seq_addr >> 8;
    z80_drv_params[0x02] = seq_addr >> 16;
    z80_drv_params[0x03] = seq_addr >> 24;
    // set play XGM command
    uint8_t cmd = *z80_drv_command | (1 << 6);
    *z80_drv_command = cmd;
    // clear pending frame
    z80_drv_params[0x0F] = 0;
    z80_resume();
    enable_ints;
}

void xgm_music_stop() {
    disable_ints;
    z80_pause();
    // special xgm sequence to stop any sound
    const uint32_t addr = ((uint32_t) stop_xgm);
    // set XGM music data address
    z80_drv_params[0x00] = addr >> 0;
    z80_drv_params[0x01] = addr >> 8;
    z80_drv_params[0x02] = addr >> 16;
    z80_drv_params[0x03] = addr >> 24;
    // set play XGM command
    uint8_t cmd = *z80_drv_command | (1 << 6);
    *z80_drv_command = cmd;
    // force immediate music process
    z80_drv_params[0x0F] = 3;
    z80_resume();
    enable_ints;
}

void xgm_music_pause() {
    disable_ints;
    z80_pause();
    // set play XGM command
    uint8_t cmd = *z80_drv_command | (1 << 4);
    *z80_drv_command = cmd;
    z80_resume();
    enable_ints;
}

void xgm_pcm_set(uint8_t id, const uint8_t *sample, uint32_t len) {
    // point to sample id table
    volatile uint8_t *pb = z80_smp_table + (id << 2);
    // write sample addr
    pb[0x00] = ((uint32_t) sample) >> 8;
    pb[0x01] = ((uint32_t) sample) >> 16;
    pb[0x02] = len >> 8;
    pb[0x03] = len >> 16;
}

void xgm_pcm_play(uint8_t id, uint8_t priority, uint16_t channel) {
    disable_ints;
    z80_pause();
    // set PCM priority and id
    uint16_t off = 4 + (channel << 1);
    z80_drv_params[off++] = priority;
    z80_drv_params[off] = id;
    // set play PCM channel command
    uint8_t cmd = *z80_drv_command | (1 << channel);
    *z80_drv_command = cmd;
    z80_resume();
    enable_ints;
}
