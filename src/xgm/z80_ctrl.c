#include "config.h"
#include "types.h"

#include "z80_ctrl.h"

#include "ym2612.h"
#include "psg.h"
#include "memory.h"
#include "timer.h"
#include "sys.h"
#include "vdp.h"
#include "xgm.h"

// Z80 drivers
#include "z80_xgm.h"

#include "tab_vol.h"
#include "smp_null.h"
#include "smp_null_pcm.h"


// we don't want to share it
extern volatile uint32_t VIntProcess;

int16_t currentDriver;
uint16_t driverFlags;


// we don't want to share it
extern void XGM_resetLoadCalculation();


void Z80_init()
{
    // request Z80 bus
    Z80_requestBus(TRUE);
    // set bank to 0
    Z80_setBank(0);

    // no loaded driver
    currentDriver = Z80_DRIVER_NULL;
    driverFlags = 0;
}


uint16_t Z80_isBusTaken()
{
    volatile uint16_t *pw;

    pw = (uint16_t *) Z80_HALT_PORT;
    if (*pw & 0x0100) return 0;
    else return 1;
}

void Z80_requestBus(uint16_t wait)
{
    volatile uint16_t *pw_bus;
    volatile uint16_t *pw_reset;

    // request bus (need to end reset)
    pw_bus = (uint16_t *) Z80_HALT_PORT;
    pw_reset = (uint16_t *) Z80_RESET_PORT;

    // take bus and end reset
    *pw_bus = 0x0100;
    *pw_reset = 0x0100;

    if (wait)
    {
        // wait for bus taken
        while (*pw_bus & 0x0100);
    }
}

void Z80_releaseBus()
{
    volatile uint16_t *pw;

    pw = (uint16_t *) Z80_HALT_PORT;
    *pw = 0x0000;
}


void Z80_startReset()
{
    volatile uint16_t *pw;

    pw = (uint16_t *) Z80_RESET_PORT;
    *pw = 0x0000;
}

void Z80_endReset()
{
    volatile uint16_t *pw;

    pw = (uint16_t *) Z80_RESET_PORT;
    *pw = 0x0100;
}


void Z80_setBank(const uint16_t bank)
{
    volatile uint8_t *pb;
    uint16_t i, value;

    pb = (uint8_t *) Z80_BANK_REGISTER;

    i = 9;
    value = bank;
    while (i--)
    {
        *pb = value;
        value >>= 1;
    }
}

uint8_t Z80_read(const uint16_t addr)
{
    return ((uint8_t*) Z80_RAM)[addr];
}

void Z80_write(const uint16_t addr, const uint8_t value)
{
    ((uint8_t*) Z80_RAM)[addr] = value;
}


void Z80_clear(const uint16_t to, const uint16_t size, const uint16_t resetz80)
{
    Z80_requestBus(TRUE);

    const uint8_t zero = getZeroU8();
    uint8_t* dst = (uint8_t*) (Z80_RAM + to);
    uint16_t len = size;

    while(len--) *dst++ = zero;

    if (resetz80) Z80_startReset();
    Z80_releaseBus();
    // wait bus released
    while(Z80_isBusTaken());
    if (resetz80) Z80_endReset();
}

void Z80_upload(const uint16_t to, const uint8_t *from, const uint16_t size, const uint16_t resetz80)
{
    Z80_requestBus(TRUE);

    // copy data to Z80 RAM (need to use byte copy here)
    uint8_t* src = (uint8_t*) from;
    uint8_t* dst = (uint8_t*) (Z80_RAM + to);
    uint16_t len = size;

    while(len--) *dst++ = *src++;

    if (resetz80) Z80_startReset();
    Z80_releaseBus();
    // wait bus released
    while(Z80_isBusTaken());
    if (resetz80) Z80_endReset();
}

void Z80_download(const uint16_t from, uint8_t *to, const uint16_t size)
{
    Z80_requestBus(TRUE);

    // copy data from Z80 RAM (need to use byte copy here)
    uint8_t* src = (uint8_t*) (Z80_RAM + from);
    uint8_t* dst = (uint8_t*) to;
    uint16_t len = size;

    while(len--) *dst++ = *src++;

    Z80_releaseBus();
}


uint16_t Z80_getLoadedDriver()
{
    return currentDriver;
}

void Z80_unloadDriver()
{
    // already unloaded
    if (currentDriver == Z80_DRIVER_NULL) return;

    // clear Z80 RAM
    Z80_clear(0, Z80_RAM_LEN, TRUE);

    currentDriver = Z80_DRIVER_NULL;

    // remove XGM task if present
    VIntProcess &= ~PROCESS_XGM_TASK;
}

void Z80_loadDriver(const uint16_t driver, const uint16_t waitReady)
{
    const uint8_t *drv;
    uint16_t len;

    // already loaded
    if (currentDriver == driver) return;

    switch(driver)
    {
		/*
        case Z80_DRIVER_PCM:
            drv = z80_drv1;
            len = sizeof(z80_drv1);
            break;

        case Z80_DRIVER_2ADPCM:
            drv = z80_drv2;
            len = sizeof(z80_drv2);
            break;

        case Z80_DRIVER_4PCM_ENV:
            drv = z80_drv3;
            len = sizeof(z80_drv3);
            break;

        case Z80_DRIVER_MVS:
            drv = z80_mvs;
            len = sizeof(z80_mvs);
            break;

        case Z80_DRIVER_TFM:
            drv = z80_tfm;
            len = sizeof(z80_tfm);
            break;

        case Z80_DRIVER_VGM:
            drv = z80_vgm;
            len = sizeof(z80_vgm);
            break;
		*/
        case Z80_DRIVER_XGM:
            drv = z80_xgm;
            len = sizeof(z80_xgm);
            break;

        default:
            // no valid driver to load
            return;
    }

    // clear z80 memory
    Z80_clear(0, Z80_RAM_LEN, FALSE);
    // upload Z80 driver and reset Z80
    Z80_upload(0, drv, len, 1);

    // driver initialisation
    switch(driver)
    {
        uint8_t *pb;
        uint32_t addr;
		/*
        case Z80_DRIVER_2ADPCM:
            // misc parameters initialisation
            Z80_requestBus(TRUE);
            // point to Z80 null sample parameters
            pb = (uint8_t *) (Z80_DRV_PARAMS + 0x20);

            addr = (uint32_t) smp_null_pcm;
            // null sample address (128 bytes aligned)
            pb[0] = addr >> 7;
            pb[1] = addr >> 15;
            // null sample length (128 bytes aligned)
            pb[2] = sizeof(smp_null_pcm) >> 7;
            pb[3] = sizeof(smp_null_pcm) >> 15;
            Z80_releaseBus();
            break;

        case Z80_DRIVER_PCM:
            // misc parameters initialisation
            Z80_requestBus(TRUE);
            // point to Z80 null sample parameters
            pb = (uint8_t *) (Z80_DRV_PARAMS + 0x20);

            addr = (uint32_t) smp_null;
            // null sample address (256 bytes aligned)
            pb[0] = addr >> 8;
            pb[1] = addr >> 16;
            // null sample length (256 bytes aligned)
            pb[2] = sizeof(smp_null) >> 8;
            pb[3] = sizeof(smp_null) >> 16;
            Z80_releaseBus();
            break;

        case Z80_DRIVER_4PCM_ENV:
            // load volume table
            Z80_upload(0x1000, tab_vol, 0x1000, 0);

            // misc parameters initialisation
            Z80_requestBus(TRUE);
            // point to Z80 null sample parameters
            pb = (uint8_t *) (Z80_DRV_PARAMS + 0x20);

            addr = (uint32_t) smp_null;
            // null sample address (256 bytes aligned)
            pb[4] = addr >> 8;
            pb[5] = addr >> 16;
            // null sample length (256 bytes aligned)
            pb[6] = sizeof(smp_null) >> 8;
            pb[7] = sizeof(smp_null) >> 16;
            Z80_releaseBus();
            break;

        case Z80_DRIVER_MVS:
            // put driver in stop state
            Z80_requestBus(TRUE);

            // point to Z80 FM command
            pb = (uint8_t *) MVS_FM_CMD;
            // stop command for FM
            *pb++ = MVS_FM_STOP;
            *pb = MVS_FM_RESET;

            // point to Z80 DACcommand
            pb = (uint8_t *) MVS_DAC_CMD;
            // stop command for DAC
            *pb = MVS_DAC_STOP;

            // point to Z80 PSG command
            pb = (uint8_t *) MVS_PSG_CMD;
            // stop command for PSG
            *pb = MVS_PSG_STOP;

            Z80_releaseBus();
            break;

        case Z80_DRIVER_VGM:
            // just reset sound chips
            YM2612_reset();
            PSG_init();
            break;
		*/
        case Z80_DRIVER_XGM:
            // reset sound chips
            YM2612_reset();
            PSG_init();

            // misc parameters initialisation
            Z80_requestBus(TRUE);
            // point to Z80 sample id table (first entry = silent sample)
            pb = (uint8_t *) (0xA01C00);

            addr = (uint32_t) smp_null;
            // null sample address (256 bytes aligned)
            pb[0] = addr >> 8;
            pb[1] = addr >> 16;
            // null sample length (256 bytes aligned)
            pb[2] = sizeof(smp_null) >> 8;
            pb[3] = sizeof(smp_null) >> 16;
            Z80_releaseBus();
            break;
    }

    // wait driver for being ready
    if (waitReady)
    {
        switch(driver)
        {
			/*
            // drivers supporting ready status
            case Z80_DRIVER_2ADPCM:
            case Z80_DRIVER_PCM:
            case Z80_DRIVER_4PCM_ENV:
            * */
            case Z80_DRIVER_XGM:
                Z80_releaseBus();
                // wait bus released
                while(Z80_isBusTaken());

                // just wait for it
                while(!Z80_isDriverReady())
                    while(Z80_isBusTaken());
                break;
			/*
            // others drivers
            case Z80_DRIVER_TFM:
            case Z80_DRIVER_MVS:
            case Z80_DRIVER_VGM:
                // just wait a bit of time
                waitMs(100);
                break;
            */
        }
    }

    // new driver set
    currentDriver = driver;

    // post init stuff
    switch(driver)
    {
        // XGM driver
        case Z80_DRIVER_XGM:
            // using auto sync --> enable XGM task on VInt
            if (!(driverFlags & DRIVER_FLAG_MANUALSYNC_XGM))
                VIntProcess |= PROCESS_XGM_TASK;
            // define default tempo
            if (IS_PALSYSTEM) XGM_setMusicTempo(50);
            else XGM_setMusicTempo(60);
            // reset load calculation
            XGM_resetLoadCalculation();
            break;
    }
}

void Z80_loadCustomDriver(const uint8_t *drv, uint16_t size)
{
    // clear z80 memory
    Z80_clear(0, Z80_RAM_LEN, FALSE);
    // upload Z80 driver and reset Z80
    Z80_upload(0, drv, size, 1);

    // custom driver set
    currentDriver = Z80_DRIVER_CUSTOM;

    // remove XGM task if present
    VIntProcess &= ~PROCESS_XGM_TASK;
}

uint16_t Z80_isDriverReady()
{
    uint8_t *pb;
    uint8_t ret;

    // point to Z80 status
    pb = (uint8_t *) Z80_DRV_STATUS;

    // bus already taken ? just check status
    if (Z80_isBusTaken())
        ret = *pb & Z80_DRV_STAT_READY;
    else
    {
        // take the bus, check status and release bus
        Z80_requestBus(TRUE);
        ret = *pb & Z80_DRV_STAT_READY;
        Z80_releaseBus();
    }

    return ret;
}
