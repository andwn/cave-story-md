/**
 *  \file timer.h
 *  \brief Timer support
 *  \author Stephane Dallongeville
 *  \date 08/2011
 *
 * This unit provides basic timer functions (useful for profiling).<br>
 * This unit uses V-Int to count frame so disabling V-Int will make timer methods to not work anymore.
 */

/**
 *  \brief
 *      Number of subtick per second.
 */
#define SUBTICKPERSECOND    76800
/**
 *  \brief
 *      Number of tick per second.
 */
#define TICKPERSECOND       300
/**
 *  \brief
 *      Time sub division per second.
 */
#define TIMEPERSECOND       256

/**
 *  \brief
 *      Maximum number of timer.
 */
#define MAXTIMER            16

extern uint32_t vtimer;


/**
 *  \brief
 *      Returns elapsed subticks from console reset.
 *
 * Returns elapsed subticks from console reset (1/76800 second based).<br>
 * <b>WARNING:</b> this function isn't accurate because of the VCounter rollback.
 */
uint32_t  getSubTick();
/**
 *  \brief
 *      Returns elapsed ticks from console reset.
 *
 * Returns elapsed ticks from console reset (1/300 second based).
 */
uint32_t  getTick();

/**
 *  \brief
 *      Returns elapsed time from console reset.
 *
 *  \param fromTick
 *      Choose tick or sub tick (more accurate) calculation.
 *
 * Returns elapsed time from console reset (1/256 second based).
 */
uint32_t  getTime(uint16_t fromTick);

/**
 *  \brief
 *      Start internal timer (0 <= numtimer < MAXTIMER)
 *
 *  \param numTimer
 *      Timer number (0-MAXTIMER)
 */
void startTimer(uint16_t numTimer);
/**
 *  \brief
 *      Get elapsed subticks for specified timer.
 *
 *  \param numTimer
 *      Timer number (0-MAXTIMER)
 *  \param restart
 *      Restart timer if TRUE
 *
 * Returns elapsed subticks from last call to startTimer(numTimer).
 */
uint32_t  getTimer(uint16_t numTimer, uint16_t restart);

/**
 *  \brief
 *      Wait for a certain amount of subticks.
 *
 *  \param subtick
 *      Number of subtick to wait for.
 *
 * <b>WARNING:</b> this function isn't accurate because of the VCounter rollback.<br>
 * Also you cannot use this method from the V-Int callback (the method will return immediatly).
 */
void waitSubTick(uint32_t subtick);
/**
 *  \brief
 *      Wait for a certain amount of ticks.
 *
 *  \param tick
 *      Number of tick to wait for.
 *
 * <b>WARNING:</b> 5/6 (PAL/NTSC) ticks based timer so use 5/6 ticks as minimum wait value.<br>
 * Also you cannot use this method from the V-Int callback (the method will return immediatly).
 */
void waitTick(uint32_t tick);
/**
 *  \brief
 *      Wait for a certain amount of millisecond.
 *
 *  \param ms
 *      Number of millisecond to wait for.
 *
 * <b>WARNING:</b> ~3.33 ms based timer so use 4 ms as minimum wait value.<br>
 * Also you cannot use this method from the V-Int callback (the method will return immediatly).
 */
void waitMs(uint32_t ms);
