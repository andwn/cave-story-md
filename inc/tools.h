/**
 *  \file tools.h
 *  \brief Misc tools methods
 *  \author Stephane Dallongeville
 *  \date 08/2011
 *
 * This unit provides some misc tools methods as getFPS(), unpack()...
 */

/**
 *  \brief
 *      Set the randomizer seed (to allow reproductible value if we are lucky with HV counter :p)
 */
void setRandomSeed(uint16_t seed);
/**
 *  \brief
 *      Return a random uint16_t integer.
 */
uint16_t random();

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

//uint16_t lz4w_unpack(const uint8_t *src, uint8_t *dest);

void DecompressSlzC(const void *indata, void *outdata);
