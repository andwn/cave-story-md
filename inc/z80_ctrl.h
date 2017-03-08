/**
 *  \file z80_ctrl.h
 *  \brief Z80 control
 *  \author Stephane Dallongeville
 *  \date 08/2011
 *
 * This unit provides Z80 access from the YM2612 :<br>
 * - enable / disable Z80<br>
 * - request / release Z80 BUS<br>
 * - upload / download data to / from Z80 memory<br>
 * - set Z80 external Bank<br>
 * - Z80 driver handling<br>
 */

#define Z80_HALT_PORT                   0xA11100
#define Z80_RESET_PORT                  0xA11200

/**
 *  \brief
 *
 * Z80 RAM start address.
 */
#define Z80_RAM_START                   0xA00000
/**
 *  \brief
 *
 * Z80 RAM end address.
 */
#define Z80_RAM_END                     0xA01FFF
/**
 *  \brief
 *
 * Z80 RAM address.
 */
#define Z80_RAM                         Z80_RAM_START
/**
 *  \brief
 *
 * Z80 RAM length in byte.
 */
#define Z80_RAM_LEN                     ((Z80_RAM_END - Z80_RAM_START) + 1)
/**
 *  \brief
 *
 * Z80 YM2612 port address.
 */
#define Z80_YM2612                      0xA04000
/**
 *  \brief
 *
 * Z80 Bank register address.
 */
#define Z80_BANK_REGISTER               0xA06000

/**
 *  \brief
 *
 * Z80 default driver command address.
 */
#define Z80_DRV_COMMAND                 0xA00100
/**
 *  \brief
 *
 * Z80 default driver status address.
 */
#define Z80_DRV_STATUS                  0xA00102
/**
 *  \brief
 *
 * Z80 default driver parameters address.
 */
#define Z80_DRV_PARAMS                  0xA00104

// default command and status value
#define Z80_DRV_COM_PLAY_SFT            0
#define Z80_DRV_COM_STOP_SFT            4
#define Z80_DRV_STAT_PLAYING_SFT        0
#define Z80_DRV_STAT_READY_SFT          7

/**
 *  \brief
 *      Z80 default driver play command.
 */
#define Z80_DRV_COM_PLAY                (1 << Z80_DRV_COM_PLAY_SFT)
/**
 *  \brief
 *      Z80 default driver stop command.
 */
#define Z80_DRV_COM_STOP                (1 << Z80_DRV_COM_STOP_SFT)
/**
 *  \brief
 *      Z80 default driver play status.
 */
#define Z80_DRV_STAT_PLAYING            (1 << Z80_DRV_STAT_PLAYING_SFT)
/**
 *  \brief
 *      Z80 default driver ready status.
 */
#define Z80_DRV_STAT_READY              (1 << Z80_DRV_STAT_READY_SFT)

// channel definition
#define Z80_DRV_CH0_SFT                 0
#define Z80_DRV_CH1_SFT                 1
#define Z80_DRV_CH2_SFT                 2
#define Z80_DRV_CH3_SFT                 3

/**
 *  \brief
 *      Z80 default driver channel 0 id.
 */
#define Z80_DRV_CH0                     (1 << Z80_DRV_CH0_SFT)
/**
 *  \brief
 *      Z80 default driver channel 1 id.
 */
#define Z80_DRV_CH1                     (1 << Z80_DRV_CH1_SFT)
/**
 *  \brief
 *      Z80 default driver channel 2 id.
 */
#define Z80_DRV_CH2                     (1 << Z80_DRV_CH2_SFT)
/**
 *  \brief
 *      Z80 default driver channel 3 id.
 */
#define Z80_DRV_CH3                     (1 << Z80_DRV_CH3_SFT)

/**
 *  \brief
 *      Initialize Z80 sub system.
 *
 *  Request Z80 BUS and reset bank number.
 */
void Z80_init();

/**
 *  \brief
 *      Return Z80 BUS taken state.
 */
uint16_t  Z80_isBusTaken();
/**
 *  \brief
 *      Request Z80 BUS.
 *  \param wait
 *      Wait for BUS request operation to complete.
 */
void Z80_requestBus(uint16_t wait);
/**
 *  \brief
 *      Release Z80 BUS.
 */
void Z80_releaseBus();

/**
 *  \brief
 *      Start Z80 reset.
 */
void Z80_startReset();
/**
 *  \brief
 *      End Z80 reset.
 */
void Z80_endReset();

/**
 *  \brief
 *      Set Z80 memory bank.
 *  \param bank
 *      Bank number to set (0x000-0x1FF)
 */
void Z80_setBank(const uint16_t bank);

/**
 *  \brief
 *      Read Z80 memory (Z80_RAM).
 *  \param addr
 *      Address to read (relative to start of Z80_RAM).
 *
 *  You need to request Z80 BUS to access Z80 memory.
 *
 *  \see Z80_requestBus(uint16_t)
 */
uint8_t Z80_read(const uint16_t addr);
/**
 *  \brief
 *      Write to Z80 memory (Z80_RAM).
 *  \param addr
 *      Address to write (relative to start of Z80_RAM).
 *  \param value
 *      Value to write.
 *
 *  You need to request Z80 BUS to access Z80 memory.
 *
 *  \see Z80_requestBus(uint16_t)
 */
void Z80_write(const uint16_t addr, const uint8_t value);

/**
 *  \brief
 *      Clear Z80 memory.
 *  \param dest
 *      Destination address (Z80 memory).
 *  \param size
 *      Size in byte of region to clear.
 *  \param resetz80
 *      Reset Z80 if set to TRUE.
 */
void Z80_clear(const uint16_t dest, const uint16_t size, const uint16_t resetz80);
/**
 *  \brief
 *      Upload data in Z80 memory.
 *  \param dest
 *      Destination address (Z80 memory).
 *  \param data
 *      Data to upload.
 *  \param size
 *      Size in byte of data to upload.
 *  \param resetz80
 *      Reset Z80 if set to TRUE.
 */
void Z80_upload(const uint16_t dest, const uint8_t *data, const uint16_t size, const uint16_t resetz80);
/**
 *  \brief
 *      Read data from Z80 memory.
 *
 *  \param from
 *      Source address (Z80 memory).
 *  \param dest
 *      Destination where to write data.
 *  \param size
 *      Size in byte of data to read.
 */
void Z80_download(const uint16_t from, uint8_t *dest, const uint16_t size);

/**
 *  \brief
 *      Return driver ready state (only for non custom driver).
 */
uint16_t  Z80_isDriverReady();
