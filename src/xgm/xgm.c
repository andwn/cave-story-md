#include "common.h"

#include "resources.h"
#include "smp_null.h"
#include "system.h"
#include "vdp.h"
#include "z80_ctrl.h"

#include "xgm.h"

// specific for the XGM driver
static uint16_t xgmTempo;
static uint16_t xgmTempoDef;
static int16_t xgmTempoCnt = 0;

// Z80_DRIVER_XGM
// XGM driver
///////////////////////////////////////////////////////////////

uint8_t XGM_isPlaying()
{
    volatile uint8_t *pb;
    uint8_t ret;

    // point to Z80 status
    pb = (uint8_t *) Z80_DRV_STATUS;

    Z80_requestBus(TRUE);
    // play status
    ret = *pb & (1 << 6);
    Z80_releaseBus();

    return ret;
}

void XGM_startPlay(const uint8_t *song)
{
    uint8_t ids[0x100-4];
    uint32_t addr;
    uint16_t i;
    volatile uint8_t *pb;

    // prepare sample id table
    for(i = 0; i < 0x3F; i++)
    {
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
    Z80_upload(0x1C00 + 4, ids, 0x100 - 4, FALSE);

    // get song address and bypass sample id table
    addr = ((uint32_t) song) + 0x100;
    // bypass sample data (use the sample data size)
    addr += song[0xFC] << 8;
    addr += song[0xFD] << 16;
    // and bypass the music data size field
    addr += 4;

    // request Z80 BUS
    Z80_requestBus(TRUE);

    // point to Z80 XGM address parameter
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x00);
    // set XGM music data address
    pb[0x00] = addr >> 0;
    pb[0x01] = addr >> 8;
    pb[0x02] = addr >> 16;
    pb[0x03] = addr >> 24;

    // point to Z80 command
    pb = (uint8_t *) Z80_DRV_COMMAND;
    // set play XGM command
    *pb |= (1 << 6);

    // point to PENDING_FRM parameter
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x0F);
    // clear pending frame
    *pb = 0;

    Z80_releaseBus();
}

void XGM_stopPlay()
{
    volatile uint8_t *pb;
    uint32_t addr;

    Z80_requestBus(TRUE);

    // special xgm sequence to stop any sound
    addr = ((uint32_t) stop_xgm);

    // point to Z80 XGM address parameter
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x00);

    // set XGM music data address
    pb[0x00] = addr >> 0;
    pb[0x01] = addr >> 8;
    pb[0x02] = addr >> 16;
    pb[0x03] = addr >> 24;

    // point to Z80 command
    pb = (uint8_t *) Z80_DRV_COMMAND;
    // set play XGM command
    *pb |= (1 << 6);

    // point to PENDING_FRM parameter
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x0F);
    // clear pending frame
    *pb = 0;

    Z80_releaseBus();
}

void XGM_pausePlay()
{
    volatile uint8_t *pb;

    Z80_requestBus(TRUE);

    // point to Z80 command
    pb = (uint8_t *) Z80_DRV_COMMAND;
    // set pause XGM command
    *pb |= (1 << 4);

    Z80_releaseBus();
}

void XGM_resumePlay()
{
    volatile uint8_t *pb;
    
    Z80_requestBus(TRUE);

    // point to Z80 command
    pb = (uint8_t *) Z80_DRV_COMMAND;
    // set resume XGM command
    *pb |= (1 << 5);

    // point to PENDING_FRM parameter
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x0F);
    // clear pending frame
    *pb = 0;

    Z80_releaseBus();
}

uint8_t XGM_isPlayingPCM(const uint16_t channel_mask)
{
    volatile uint8_t *pb;
    uint8_t ret;

    Z80_requestBus(TRUE);

    // point to Z80 status
    pb = (uint8_t *) Z80_DRV_STATUS;
    // play status
    ret = *pb & (channel_mask << Z80_DRV_STAT_PLAYING_SFT);

    Z80_releaseBus();

    return ret;
}

void XGM_setPCM(const uint8_t id, const uint8_t *sample, const uint32_t len)
{
    Z80_requestBus(TRUE);
    XGM_setPCMFast(id, sample, len);
    Z80_releaseBus();
}

void XGM_setPCMFast(const uint8_t id, const uint8_t *sample, const uint32_t len)
{
    volatile uint8_t *pb;
    // point to sample id table
    pb = (uint8_t *) (0xA01C00 + (id * 4));
    // write sample addr
    pb[0x00] = ((uint32_t) sample) >> 8;
    pb[0x01] = ((uint32_t) sample) >> 16;
    pb[0x02] = len >> 8;
    pb[0x03] = len >> 16;
}

void XGM_startPlayPCM(const uint8_t id, const uint8_t priority, const uint16_t channel)
{
    volatile uint8_t *pb;

    Z80_requestBus(TRUE);

    // point to Z80 PCM parameters
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x04 + (channel * 2));

    // set PCM priority and id
    pb[0x00] = priority & 0xF;
    pb[0x01] = id;

    // point to Z80 command
    pb = (uint8_t *) Z80_DRV_COMMAND;
    // set play PCM channel command
    *pb |= (Z80_DRV_COM_PLAY << channel);

    Z80_releaseBus();
}

void XGM_stopPlayPCM(const uint16_t channel)
{
    volatile uint8_t *pb;

    Z80_requestBus(TRUE);

    // point to Z80 PCM parameters
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x04 + (channel * 2));

    // use silent PCM (id = 0) with maximum priority
    pb[0x00] = 0xF;
    pb[0x01] = 0;

    // point to Z80 command
    pb = (uint8_t *) Z80_DRV_COMMAND;
    // set play PCM channel command
    *pb |= (Z80_DRV_COM_PLAY << channel);

    Z80_releaseBus();
}

void XGM_set68KBUSProtection(uint8_t value)
{
    volatile uint16_t *pw_bus;
    volatile uint16_t *pw_reset;
    volatile uint8_t *pb;

    // point on bus req and reset ports
    pw_bus = (uint16_t *) Z80_HALT_PORT;
    pw_reset = (uint16_t *) Z80_RESET_PORT;

    // take bus and end reset (fast method)
    *pw_bus = 0x0100;
    *pw_reset = 0x0100;
    // wait for bus taken
    while (*pw_bus & 0x0100);

    // point to Z80 PROTECT parameter
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x0D);

    *pb = value;

    // release bus
    *pw_bus = 0x0000;
}

void XGM_nextXFrame(uint16_t num)
{
    volatile uint16_t *pw_bus;
    volatile uint16_t *pw_reset;
    volatile uint8_t *pb;

    // point on bus req and reset ports
    pw_bus = (uint16_t *) Z80_HALT_PORT;
    pw_reset = (uint16_t *) Z80_RESET_PORT;
    // point to MODIFYING_F parameter
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x0E);

    while(TRUE)
    {
        // take bus and end reset (fast method)
        *pw_bus = 0x0100;
        *pw_reset = 0x0100;
        // wait for bus taken
        while (*pw_bus & 0x0100);

        // Z80 not accessing ?
        if (!*pb) break;

        // release bus
        *pw_bus = 0x0000;

        // wait a bit (about 80 cycles)
        __asm__ volatile ("\t\tmovm.l %d0-%d3,-(%sp)\n");
        __asm__ volatile ("\t\tmovm.l (%sp)+,%d0-%d3\n");

        // wait for bus released before requesting it again
        while (!(*pw_bus & 0x0100));
    }

    // point to PENDING_FRM parameter
    pb++;
    // increment frame to process
    *pb += num;

    // release bus
    *pw_bus = 0x0000;
}

void XGM_setMusicTempo(uint16_t value)
{
    xgmTempo = value;
    if (IS_PALSYSTEM) xgmTempoDef = 50;
    else xgmTempoDef = 60;
}

uint32_t XGM_getElapsed()
{
    volatile uint8_t *pb;
    uint8_t *dst;
    uint8_t values[3];
    uint32_t result;

    // point to ELAPSED value
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x90);
    dst = values;

    Z80_requestBus(TRUE);

    // copy quickly elapsed time
    *dst++ = *pb++;
    *dst++ = *pb++;
    *dst = *pb;

    Z80_releaseBus();

    result = (values[0] << 0) | (values[1] << 8) | (values[2] << 16);

    // fix possible 24 bit negative value (parsing first extra frame)
    if (result >= 0xFFFFF0) return 0;

    return result;
}

// VInt processing for XGM driver
void XGM_doVBlankProcess()
{
    volatile uint16_t *pw_bus;
    volatile uint16_t *pw_reset;
    volatile uint8_t *pb;
    int16_t cnt = xgmTempoCnt;
    uint16_t step = xgmTempoDef;
    uint16_t num = 0;

    while(cnt <= 0)
    {
        num++;
        cnt += step;
    }

    xgmTempoCnt = cnt - xgmTempo;

    // directly do the frame here as we want this code to be as fast as possible (to not waste vint time)
    // driver should be loaded here

    // point on bus req and reset ports
    pw_bus = (uint16_t *) Z80_HALT_PORT;
    pw_reset = (uint16_t *) Z80_RESET_PORT;
    // point to MODIFYING_F parameter
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x0E);

    while(TRUE)
    {
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
        __asm__ volatile ("\t\tmovm.l %d0-%d3,-(%sp)\n");
        __asm__ volatile ("\t\tmovm.l (%sp)+,%d0-%d3\n");

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
