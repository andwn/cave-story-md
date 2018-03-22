#include "common.h"
#include "vdp.h"
#include "xgm.h"
#include "xgm/z80_xgm.h" /* z80 program blob */
#include "resources.h"

const uint8_t smp_null[0x100] __attribute__((aligned(256))) = {};

// Variables
static uint16_t xgmTempo;
static uint16_t xgmTempoDef;
static int16_t xgmTempoCnt;

// Constants
#define Z80_HALT_PORT                   0xA11100
#define Z80_RESET_PORT                  0xA11200
#define Z80_RAM_START                   0xA00000
#define Z80_RAM_END                     0xA01FFF
#define Z80_RAM                         Z80_RAM_START
#define Z80_RAM_LEN                     ((Z80_RAM_END - Z80_RAM_START) + 1)
#define Z80_BANK_REGISTER               0xA06000
#define Z80_DRV_COMMAND                 0xA00100
#define Z80_DRV_STATUS                  0xA00102
#define Z80_DRV_PARAMS                  0xA00104
// default command and status value
#define Z80_DRV_COM_PLAY                1
#define Z80_DRV_COM_STOP                16
#define Z80_DRV_STAT_PLAYING            1
#define Z80_DRV_STAT_READY              128
// channel definition
#define Z80_DRV_CH0                     1
#define Z80_DRV_CH1                     2
#define Z80_DRV_CH2                     4
#define Z80_DRV_CH3                     8

static uint16_t z80_bus_taken() {
    volatile uint16_t *pw = (uint16_t*) Z80_HALT_PORT;
    if(*pw & 0x0100) return 0;
    else return 1;
}

void z80_request() {
    volatile uint16_t *pw_bus = (uint16_t*) Z80_HALT_PORT;
    volatile uint16_t *pw_reset = (uint16_t*) Z80_RESET_PORT;
    // take bus and end reset
    *pw_bus = 0x0100;
    *pw_reset = 0x0100;
    while(*pw_bus & 0x0100);
}

void z80_release() {
    volatile uint16_t *pw = (uint16_t*) Z80_HALT_PORT;
    *pw = 0x0000;
}

static void z80_reset_start() {
    volatile uint16_t *pw = (uint16_t*) Z80_RESET_PORT;
    *pw = 0x0000;
}

static void z80_reset_end() {
    volatile uint16_t *pw = (uint16_t*) Z80_RESET_PORT;
    *pw = 0x0100;
}

static void z80_set_bank(const uint16_t bank) {
    volatile uint8_t *pb = (uint8_t*) Z80_BANK_REGISTER;
    uint16_t i = 9;
    uint16_t value = bank;
    while(i--) {
        *pb = value;
        value >>= 1;
    }
}

static void z80_clear(const uint16_t to, const uint16_t size, const uint16_t resetz80) {
    z80_request(TRUE);
    const uint8_t zero = 0;
    uint8_t* dst = (uint8_t*) (Z80_RAM + to);
    uint16_t len = size;
    while(len--) *dst++ = zero;
    if(resetz80) z80_reset_start();
    z80_release();
    // wait bus released
    while(z80_bus_taken());
    if(resetz80) z80_reset_end();
}

static void z80_upload(const uint16_t to, const uint8_t *from, const uint16_t size, const uint16_t resetz80) {
    z80_request(TRUE);
    // copy data to Z80 RAM (need to use byte copy here)
    uint8_t* src = (uint8_t*) from;
    uint8_t* dst = (uint8_t*) (Z80_RAM + to);
    uint16_t len = size;
    while(len--) *dst++ = *src++;
    if(resetz80) z80_reset_start();
    z80_release();
    // wait bus released
    while(z80_bus_taken());
    if(resetz80) z80_reset_end();
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
    if(!bus_taken) z80_request(TRUE);
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
    uint8_t ret;
    volatile uint8_t *pb = (uint8_t*) Z80_DRV_STATUS;
    // bus already taken ? just check status
    if (z80_bus_taken()) {
		ret = *pb & Z80_DRV_STAT_READY;
    } else {
        // take the bus, check status and release bus
        z80_request(TRUE);
        ret = *pb & Z80_DRV_STAT_READY;
        z80_release();
    }
    return ret;
}

void xgm_init() {
	z80_request(TRUE);
	z80_set_bank(0);
	z80_clear(0, Z80_RAM_LEN, FALSE);
	// upload Z80 driver and reset Z80
	z80_upload(0, z80_xgm, sizeof(z80_xgm), 1);
	// reset sound chips
	ym2612_reset();
	psg_init();
	// misc parameters initialisation
	z80_request(TRUE);
	// point to Z80 sample id table (first entry = silent sample)
	volatile uint8_t *pb = (uint8_t*) (0xA01C00);
	uint32_t addr = (uint32_t) smp_null;
	// null sample address (256 bytes aligned)
	pb[0] = addr >> 8;
	pb[1] = addr >> 16;
	// null sample length (256 bytes aligned)
	pb[2] = sizeof(smp_null) >> 8;
	pb[3] = sizeof(smp_null) >> 16;
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
        addr = song[(i * 4) + 0] << 8;
        addr |= song[(i * 4) + 1] << 16;
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
    // upload sample id table (first entry is silent sample, we don't transfer it)
    z80_upload(0x1C00 + 4, ids, 0x100 - 4, FALSE);
    // get song address and bypass sample id table
    addr = ((uint32_t) song) + 0x100;
    // bypass sample data (use the sample data size)
    addr += song[0xFC] << 8;
    addr += song[0xFD] << 16;
    // and bypass the music data size field
    addr += 4;
    // request Z80 BUS
    z80_request(TRUE);
    // point to Z80 XGM address parameter
    volatile uint8_t *pb = (uint8_t*) (Z80_DRV_PARAMS + 0x00);
    // set XGM music data address
    pb[0x00] = addr >> 0;
    pb[0x01] = addr >> 8;
    pb[0x02] = addr >> 16;
    pb[0x03] = addr >> 24;
    // point to Z80 command
    pb = (uint8_t*) Z80_DRV_COMMAND;
    // set play XGM command
    *pb |= (1 << 6);
    // point to PENDING_FRM parameter
    pb = (uint8_t*) (Z80_DRV_PARAMS + 0x0F);
    // clear pending frame
    *pb = 0;
    z80_release();
}

void xgm_music_stop() {
    z80_request(TRUE);
    // special xgm sequence to stop any sound
    uint32_t addr = ((uint32_t) stop_xgm);
    // point to Z80 XGM address parameter
    volatile uint8_t *pb = (uint8_t*) (Z80_DRV_PARAMS + 0x00);
    // set XGM music data address
    pb[0x00] = addr >> 0;
    pb[0x01] = addr >> 8;
    pb[0x02] = addr >> 16;
    pb[0x03] = addr >> 24;
    // point to Z80 command
    pb = (uint8_t*) Z80_DRV_COMMAND;
    // set play XGM command
    *pb |= (1 << 6);
    // point to PENDING_FRM parameter
    pb = (uint8_t*) (Z80_DRV_PARAMS + 0x0F);
    // clear pending frame
    *pb = 0;
    z80_release();
}

void xgm_music_pause() {
    z80_request(TRUE);
    volatile uint8_t *pb = (uint8_t*) Z80_DRV_COMMAND;
    // set pause XGM command
    *pb |= (1 << 4);
    z80_release();
}

void xgm_music_resume() {
    z80_request(TRUE);
    volatile uint8_t *pb = (uint8_t*) Z80_DRV_COMMAND;
    // set resume XGM command
    *pb |= (1 << 5);
    // point to PENDING_FRM parameter
    pb = (uint8_t*) (Z80_DRV_PARAMS + 0x0F);
    // clear pending frame
    *pb = 0;
    z80_release();
}

void xgm_pcm_set(const uint8_t id, const uint8_t *sample, const uint32_t len) {
	z80_request(TRUE);
    // point to sample id table
    volatile uint8_t *pb = (uint8_t*) (0xA01C00 + (id * 4));
    // write sample addr
    pb[0x00] = ((uint32_t) sample) >> 8;
    pb[0x01] = ((uint32_t) sample) >> 16;
    pb[0x02] = len >> 8;
    pb[0x03] = len >> 16;
	z80_release();
}

void xgm_pcm_play(const uint8_t id, const uint8_t priority, const uint16_t channel) {
    z80_request(TRUE);
    // point to Z80 PCM parameters
    volatile uint8_t *pb = (uint8_t*) (Z80_DRV_PARAMS + 0x04 + (channel * 2));
    // set PCM priority and id
    pb[0x00] = priority & 0xF;
    pb[0x01] = id;
    // point to Z80 command
    pb = (uint8_t*) Z80_DRV_COMMAND;
    // set play PCM channel command
    *pb |= (Z80_DRV_COM_PLAY << channel);
    z80_release();
}

void xgm_protect(uint8_t enable) {
    // point on bus req and reset ports
    volatile uint16_t *pw_bus = (uint16_t*) Z80_HALT_PORT;
    volatile uint16_t *pw_reset = (uint16_t*) Z80_RESET_PORT;
    // take bus and end reset (fast method)
    *pw_bus = 0x0100;
    *pw_reset = 0x0100;
    // wait for bus taken
    while (*pw_bus & 0x0100);
    // point to Z80 PROTECT parameter
    volatile uint8_t *pb = (uint8_t *) (Z80_DRV_PARAMS + 0x0D);
    *pb = enable;
    // release bus
    *pw_bus = 0x0000;
}

uint32_t xgm_get_elapsed() {
    volatile uint8_t *pb;
    uint8_t *dst;
    uint8_t values[3];
    uint32_t result;

    // point to ELAPSED value
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x90);
    dst = values;

    z80_request(TRUE);

    // copy quickly elapsed time
    *dst++ = *pb++;
    *dst++ = *pb++;
    *dst = *pb;

    z80_release();

    result = (values[0] << 0) | (values[1] << 8) | (values[2] << 16);

    // fix possible 24 bit negative value (parsing first extra frame)
    if (result >= 0xFFFFF0) return 0;

    return result;
}

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
    volatile uint16_t *pw_bus = (uint16_t *) Z80_HALT_PORT;
    volatile uint16_t *pw_reset = (uint16_t *) Z80_RESET_PORT;
    // point to MODIFYING_F parameter
    volatile uint8_t *pb = (uint8_t *) (Z80_DRV_PARAMS + 0x0E);
    while(TRUE) {
        // take bus and end reset (fast method)
        *pw_bus = 0x0100;
        *pw_reset = 0x0100;
        // wait for bus taken
        while (*pw_bus & 0x100);
        // Z80 not accessing ?
        if (!*pb) break;
        // release bus
        *pw_bus = 0x0000;
        // wait a bit (about 80 cycles)
        __asm__("\t\tmovm.l %d0-%d3,-(%sp)\n");
        __asm__("\t\tmovm.l (%sp)+,%d0-%d3\n");
        // wait for bus released before requesting it again
        while (!(*pw_bus & 0x0100));
    }
    // point to PENDING_FRM parameter
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x0F);
    // increment frame to process
    *pb += num;
    // release bus
    *pw_bus = 0x0000;
}
