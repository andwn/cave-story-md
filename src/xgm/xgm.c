#include "common.h"
#include "resources.h"
#include "smp_null.h"
#include "system.h"
#include "z80_ctrl.h"

#include "xgm.h"

// allow to access it without "public" share
extern int16_t currentDriver;
extern uint16_t driverFlags;

// specific for the XGM driver
static uint16_t xgmTempo;
static uint16_t xgmTempoDef;
// can be nice to alter it from external
int16_t xgmTempoCnt = 0;

// Z80 cpu load calculation for XGM driver
static uint16_t xgmIdleTab[32];
static uint16_t xgmWaitTab[32];
static uint16_t xgmTabInd;
static uint16_t xgmIdleMean;
static uint16_t xgmWaitMean;


// Z80_DRIVER_XGM
// XGM driver
///////////////////////////////////////////////////////////////

uint8_t XGM_isPlaying()
{
    volatile uint8_t *pb;
    uint8_t ret;

    // disable ints when requesting Z80 BUS
    

    // load the appropriate driver if not already done
    Z80_loadDriver(Z80_DRIVER_XGM, TRUE);

    // point to Z80 status
    pb = (uint8_t *) Z80_DRV_STATUS;

    Z80_requestBus(TRUE);
    // play status
    ret = *pb & (1 << 6);
    Z80_releaseBus();

    // re-enable ints
    

    return ret;
}

void XGM_startPlay(const uint8_t *song)
{
    uint8_t ids[0x100-4];
    uint32_t addr;
    uint16_t i;
    volatile uint8_t *pb;

    // disable ints when requesting Z80 BUS
    

    // load the appropriate driver if not already done
    Z80_loadDriver(Z80_DRIVER_XGM, TRUE);

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

    // re-enable ints
    
}

void XGM_stopPlay()
{
    volatile uint8_t *pb;
    uint32_t addr;

    // disable ints when requesting Z80 BUS
    

    // load the appropriate driver if not already done
    Z80_loadDriver(Z80_DRIVER_XGM, TRUE);

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

    // re-enable ints
    
}

void XGM_pausePlay()
{
    volatile uint8_t *pb;

    // disable ints when requesting Z80 BUS
    

    // load the appropriate driver if not already done
    Z80_loadDriver(Z80_DRIVER_XGM, TRUE);

    Z80_requestBus(TRUE);

    // point to Z80 command
    pb = (uint8_t *) Z80_DRV_COMMAND;
    // set pause XGM command
    *pb |= (1 << 4);

    Z80_releaseBus();

    // re-enable ints
    
}

void XGM_resumePlay()
{
    volatile uint8_t *pb;

    // disable ints when requesting Z80 BUS
    

    // load the appropriate driver if not already done
    Z80_loadDriver(Z80_DRIVER_XGM, TRUE);

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

    // re-enable ints
    
}

uint8_t XGM_isPlayingPCM(const uint16_t channel_mask)
{
    volatile uint8_t *pb;
    uint8_t ret;

    // disable ints when requesting Z80 BUS
    

    // load the appropriate driver if not already done
    Z80_loadDriver(Z80_DRIVER_XGM, TRUE);

    Z80_requestBus(TRUE);

    // point to Z80 status
    pb = (uint8_t *) Z80_DRV_STATUS;
    // play status
    ret = *pb & (channel_mask << Z80_DRV_STAT_PLAYING_SFT);

    Z80_releaseBus();

    // re-enable ints
    

    return ret;
}

void XGM_setPCM(const uint8_t id, const uint8_t *sample, const uint32_t len)
{
    // disable ints when requesting Z80 BUS
    

    // load the appropriate driver if not already done
    Z80_loadDriver(Z80_DRIVER_XGM, TRUE);

    Z80_requestBus(TRUE);

    XGM_setPCMFast(id, sample, len);

    Z80_releaseBus();

    // re-enable ints
    
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

    // disable ints when requesting Z80 BUS
    

    // load the appropriate driver if not already done
    Z80_loadDriver(Z80_DRIVER_XGM, TRUE);

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

    // re-enable ints
    
}

void XGM_stopPlayPCM(const uint16_t channel)
{
    volatile uint8_t *pb;

    // disable ints when requesting Z80 BUS
    

    // load the appropriate driver if not already done
    Z80_loadDriver(Z80_DRIVER_XGM, TRUE);

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

    // re-enable ints
    
}

void XGM_setLoopNumber(int8_t value)
{
    volatile uint8_t *pb;

    // disable ints when requesting Z80 BUS
    

    // load the appropriate driver if not already done
    Z80_loadDriver(Z80_DRIVER_XGM, TRUE);

    Z80_requestBus(TRUE);

    // point to Z80 PCM parameters
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x0C);

    // set loop argument (+1 as internally 0 = infinite)
    *pb = value + 1;

    Z80_releaseBus();

    // re-enable ints
    
}

void XGM_set68KBUSProtection(uint8_t value)
{
    volatile uint16_t *pw_bus;
    volatile uint16_t *pw_reset;
    volatile uint8_t *pb;

    // driver should be loaded here

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
        while (*pw_bus & 0x0100);

        // Z80 not accessing ?
        if (!*pb) break;

        // release bus
        *pw_bus = 0x0000;

        // wait a bit (about 80 cycles)
        asm volatile ("\t\tmovm.l %d0-%d3,-(%sp)\n");
        asm volatile ("\t\tmovm.l (%sp)+,%d0-%d3\n");

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


uint16_t XGM_getManualSync()
{
    return driverFlags & DRIVER_FLAG_MANUALSYNC_XGM;
}

void XGM_setManualSync(uint16_t value)
{
    // nothing to do
    if (currentDriver != Z80_DRIVER_XGM)
        return;

    if (value)
    {
        driverFlags |= DRIVER_FLAG_MANUALSYNC_XGM;
        // remove VInt XGM process
        VIntProcess &= ~PROCESS_XGM_TASK;
    }
    else
    {
        driverFlags &= ~DRIVER_FLAG_MANUALSYNC_XGM;
        // set VInt XGM process
        VIntProcess |= PROCESS_XGM_TASK;
    }
}

uint16_t XGM_getForceDelayDMA()
{
    return driverFlags & DRIVER_FLAG_DELAYDMA_XGM;
}

void XGM_setForceDelayDMA(uint16_t value)
{
    // nothing to do
    if (currentDriver != Z80_DRIVER_XGM)
        return;

    if (value)
        driverFlags |= DRIVER_FLAG_DELAYDMA_XGM;
    else
        driverFlags &= ~DRIVER_FLAG_DELAYDMA_XGM;
}

uint16_t XGM_getMusicTempo()
{
    return xgmTempo;
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

    // disable ints when requesting Z80 BUS
    

    Z80_requestBus(TRUE);

    // copy quickly elapsed time
    *dst++ = *pb++;
    *dst++ = *pb++;
    *dst = *pb;

    Z80_releaseBus();

    // re-enable ints
    

    result = (values[0] << 0) | (values[1] << 8) | (values[2] << 16);

    // fix possible 24 bit negative value (parsing first extra frame)
    if (result >= 0xFFFFF0) return 0;

    return result;
}

uint32_t XGM_getCPULoad()
{
    volatile uint8_t *pb;
    uint16_t idle;
    uint16_t wait;
    uint16_t ind;
    int16_t load;

    // driver should be loaded here

    // point to Z80 'idle wait loop' value
    pb = (uint8_t *) (Z80_DRV_PARAMS + 0x7C);

    // disable ints when requesting Z80 BUS
    

    Z80_requestBus(TRUE);

    // get idle
    idle = pb[0] + (pb[1] << 8);
    // reset it and point on 'dma wait loop'
    *pb++ = 0;
    *pb++ = 0;

    // get dma wait
    wait = pb[0] + (pb[1] << 8);
    // and reset it
    *pb++ = 0;
    *pb = 0;

    Z80_releaseBus();

    // re-enable ints
    

    ind = xgmTabInd;

    xgmIdleMean -= xgmIdleTab[ind];
    xgmIdleMean += idle;
    xgmIdleTab[ind] = idle;

    xgmWaitMean -= xgmWaitTab[ind];
    xgmWaitMean += wait;
    xgmWaitTab[ind] = wait;

    xgmTabInd = (ind + 1) & 0x1F;

    load = 105 - (xgmIdleMean >> 5);

    return load | ((xgmWaitMean >> 5) << 16);
}

void XGM_resetLoadCalculation()
{
    uint16_t i;
    uint16_t *s1;
    uint16_t *s2;

    s1 = xgmIdleTab;
    s2 = xgmWaitTab;
    i = 32;
    while(i--)
    {
        *s1++ = 0;
        *s2++ = 0;
    }

    xgmTabInd = 0;
    xgmIdleMean = 0;
    xgmWaitMean = 0;
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
        asm volatile ("\t\tmovm.l %d0-%d3,-(%sp)\n");
        asm volatile ("\t\tmovm.l (%sp)+,%d0-%d3\n");

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
