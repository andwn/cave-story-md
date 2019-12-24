#include "common.h"
#include "vdp.h"
#include "xgm.h"
#include "xgm/z80_xgm.h" /* z80 program blob */
#include "resources.h"

static const uint8_t smp_null[0x100] __attribute__((aligned(256))) = {};

// Variables
extern volatile uint16_t xgmTempo;
extern volatile uint16_t xgmTempoDef;
extern volatile int16_t xgmTempoCnt;

// default command and status value
#define Z80_DRV_COM_PLAY                1
#define Z80_DRV_COM_STOP                16
#define Z80_DRV_STAT_PLAYING            1
#define Z80_DRV_STAT_READY              128

static volatile uint16_t* const z80_halt_port  = (uint16_t*) 0xA11100;
static volatile uint16_t* const z80_reset_port = (uint16_t*) 0xA11200;
static volatile uint8_t* const z80_bank_reg    = (uint8_t*) 0xA06000;
static volatile uint8_t* const z80_drv_command = (uint8_t*) 0xA00100;
static volatile uint8_t* const z80_drv_status  = (uint8_t*) 0xA00102;
static volatile uint8_t* const z80_drv_params  = (uint8_t*) 0xA00104;
static volatile uint8_t* const z80_smp_table   = (uint8_t*) 0xA01C00;

static uint16_t z80_bus_taken() {
    return !(*z80_halt_port & 0x0100);
}

void z80_request() {
    // take bus and end reset
    *z80_halt_port = 0x0100;
    *z80_reset_port = 0x0100;
    while(*z80_halt_port & 0x0100);
}

void z80_release() {
    *z80_halt_port = 0x0000;
}

static void z80_reset_start() {
    *z80_reset_port = 0x0000;
}

static void z80_reset_end() {
    *z80_reset_port = 0x0100;
}

static void z80_set_bank(const uint16_t bank) {
    uint16_t i = 9;
    uint16_t value = bank;
    while(i--) {
        *z80_bank_reg = value;
        value >>= 1;
    }
}

static void z80_clear() {
    const uint8_t zero = 0;
    uint8_t* dst = (uint8_t*) 0xA00000;
    uint16_t len = 0x2000;
    while(len--) *dst++ = zero;
}

static void z80_upload(const uint16_t to, const uint8_t *from, uint16_t size) {
    // copy data to Z80 RAM (need to use byte copy here)
    uint8_t* src = (uint8_t*) from;
    uint8_t* dst = (uint8_t*) (0xA00000 + to);
    while(size--) *dst++ = *src++;
}

static void ym2612_write(const uint16_t port, const uint8_t data) {
    volatile int8_t *pb = (int8_t*) 0xA04000;
    // wait while YM2612 busy
    while(*pb < 0);
    // write data
    pb[port & 3] = data;
}

static void ym2612_reset() {
    uint16_t bus_taken = z80_bus_taken();
    if(!bus_taken) z80_request();
    // enable left and right output for all channel
    for(uint16_t i = 0; i < 3; i++) {
        ym2612_write(0, 0xB4 | i);
        ym2612_write(1, 0xC0);
        ym2612_write(2, 0xB4 | i);
        ym2612_write(3, 0xC0);
    }
    // disable LFO
    ym2612_write(0, 0x22);
    ym2612_write(1, 0x00);
    // disable timer & set channel 6 to normal mode
    ym2612_write(0, 0x27);
    ym2612_write(1, 0x00);
    // ALL KEY OFF
    ym2612_write(0, 0x28);
    for(uint16_t i = 0; i < 3; i++) {
        ym2612_write(1, 0x00 | i);
        ym2612_write(1, 0x04 | i);
    }
    // disable DAC
    ym2612_write(0, 0x2B);
    ym2612_write(1, 0x00);
    if(!bus_taken) z80_release();
}

static void psg_init() {
    volatile uint8_t *pb = (uint8_t*) 0xC00011;
    for (uint16_t i = 0; i < 4; i++) {
        // set tone to 0
        *pb = 0x80 | (i << 5) | 0x00;
        *pb = 0x00;
        // set envelope to silent
        *pb = 0x90 | (i << 5) | 0x0F;
    }
}

static uint16_t xgm_get_ready() {
    // bus already taken ? just check status
    if(z80_bus_taken()) {
		return *z80_drv_status & Z80_DRV_STAT_READY;
    }
    // take the bus, check status and release bus
    z80_request();
    uint16_t rtn = *z80_drv_status & Z80_DRV_STAT_READY;
    z80_release();
    return rtn;
}

void xgm_init() {
	z80_request();
	z80_set_bank(0);
	// upload Z80 driver and reset Z80
    z80_clear();
	z80_upload(0, z80_xgm, sizeof(z80_xgm));
    z80_reset_start();
    z80_release();
    // wait bus released
    while(z80_bus_taken());
    z80_reset_end();
	// reset sound chips
	ym2612_reset();
	psg_init();
	// misc parameters initialisation
    z80_request();
    uint32_t addr = (uint32_t) smp_null;
    // null sample address (256 bytes aligned)
	z80_smp_table[0] = addr >> 8;
	z80_smp_table[1] = addr >> 16;
	// null sample length (256 bytes aligned)
	z80_smp_table[2] = sizeof(smp_null) >> 8;
	z80_smp_table[3] = sizeof(smp_null) >> 16;
	z80_release();
	// wait bus released
	while(z80_bus_taken());
	// just wait for it
	while(!xgm_get_ready()) while(z80_bus_taken());
	// Tempo
	xgmTempo = 60;
	xgmTempoDef = pal_mode ? 50 : 60;
}

void xgm_music_play(const uint8_t *song) {
    uint8_t ids[0x100-4];
    uint32_t addr;
    // prepare sample id table
    for(uint16_t i = 0; i < 0x3F; i++) {
        // sample address in sample bank data
        addr = ((uint32_t) song[(i * 4) + 0]) << 8;
        addr |= ((uint32_t) song[(i * 4) + 1]) << 16;
        // silent sample ? use null sample address
        if (addr == 0xFFFF00) addr = (uint32_t) smp_null;
        // adjust sample address (make it absolute)
        else addr += ((uint32_t) song) + 0x100;
        // write adjusted addr
        ids[(i * 4) + 0] = addr >> 8;
        ids[(i * 4) + 1] = addr >> 16;
        // and recopy len
        ids[(i * 4) + 2] = song[(i * 4) + 2];
        ids[(i * 4) + 3] = song[(i * 4) + 3];
    }
    // request Z80 BUS
    z80_request();
    // upload sample id table (first entry is silent sample, we don't transfer it)
    z80_upload(0x1C00 + 4, ids, 0x100 - 4);
    // get song address and bypass sample id table
    addr = ((uint32_t) song) + 0x100;
    // bypass sample data (use the sample data size)
    addr += ((uint32_t) song[0xFC]) << 8;
    addr += ((uint32_t) song[0xFD]) << 16;
    // and bypass the music data size field
    addr += 4;
    // set XGM music data address
    z80_drv_params[0x00] = addr >> 0;
    z80_drv_params[0x01] = addr >> 8;
    z80_drv_params[0x02] = addr >> 16;
    z80_drv_params[0x03] = addr >> 24;
    // set play XGM command
    *z80_drv_command |= (1 << 6);
    // clear pending frame
    z80_drv_params[0x0F] = 0;
    z80_release();
}

void xgm_music_stop() {
    z80_request();
    // special xgm sequence to stop any sound
    uint32_t addr = ((uint32_t) stop_xgm);
    // set XGM music data address
    z80_drv_params[0x00] = addr >> 0;
    z80_drv_params[0x01] = addr >> 8;
    z80_drv_params[0x02] = addr >> 16;
    z80_drv_params[0x03] = addr >> 24;
    // set play XGM command
    *z80_drv_command |= (1 << 6);
    // clear pending frame
    z80_drv_params[0x0F] = 0;
    z80_release();
}

void xgm_pcm_set(const uint8_t id, const uint8_t *sample, const uint32_t len) {
	//z80_request();
    // point to sample id table
    volatile uint8_t *pb = (uint8_t*) (0xA01C00 + (id * 4));
    // write sample addr
    pb[0x00] = ((uint32_t) sample) >> 8;
    pb[0x01] = ((uint32_t) sample) >> 16;
    pb[0x02] = len >> 8;
    pb[0x03] = len >> 16;
	//z80_release();
}

void xgm_pcm_play(const uint8_t id, const uint8_t priority, const uint16_t channel) {
    z80_request();
    // set PCM priority and id
    z80_drv_params[0x04 + (channel * 2)] = priority & 0xF;
    z80_drv_params[0x05 + (channel * 2)] = id;
    // set play PCM channel command
    *z80_drv_command |= (Z80_DRV_COM_PLAY << channel);
    z80_release();
}
/*
// VInt processing for XGM driver
void xgm_vblank() {
    int16_t cnt = xgmTempoCnt;
    uint16_t step = xgmTempoDef;
    uint16_t num = 0;
    while(cnt <= 0) {
        num++;
        cnt += step;
    }
    xgmTempoCnt = cnt - xgmTempo;
    // directly do the frame here as we want this code to be as fast as possible 
    // (to not waste vint time) driver should be loaded here
    // point on bus req and reset ports
    while(TRUE) {
        // take bus and end reset (fast method)
        *z80_halt_port = 0x0100;
        *z80_reset_port = 0x0100;
        // wait for bus taken
        while (*z80_halt_port & 0x100);
        // Z80 not accessing ?
        if (!z80_drv_params[0x0E]) break;
        // release bus
        *z80_halt_port = 0x0000;
        // wait a bit (about 80 cycles)
        __asm__("\t\tmovm.l %d0-%d3,-(%sp)\n");
        __asm__("\t\tmovm.l (%sp)+,%d0-%d3\n");
        // wait for bus released before requesting it again
        while (!(*z80_halt_port & 0x0100));
    }
    // increment frame to process
    z80_drv_params[0x0F] += num;
    // release bus
    *z80_halt_port = 0x0000;
}
*/