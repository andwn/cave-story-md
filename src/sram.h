/**
 *  \file sram.h
 *  \brief SRAM (Static RAM used for save backup) support.
 *  \author Chilly Willy & Stephane Dallongeville
 *  \date 08/2012
 *
 * This unit provides methods to read from or write to SRAM.
 */

#define SRAM_CONTROL    0xA130F1
#define SRAM_BASE       0x200001


/**
 *  \brief
 *      Enable SRAM in Read Write mode.
 */
void SRAM_enable();
/**
 *  \brief
 *      Enable SRAM in Read Only mode.
 */
void SRAM_enableRO();
/**
 *  \brief
 *      Disable SRAM.
 */
void SRAM_disable();

/**
 *  \brief
 *      Read a byte from the SRAM.
 *
 *  \param offset
 *      Offset where we want to read.
 *  \return value.
 */
uint8_t SRAM_readByte(uint32_t offset);
/**
 *  \brief
 *      Read a word from the SRAM.
 *
 *  \param offset
 *      Offset where we want to read.
 *  \return value.
 */
uint16_t SRAM_readWord(uint32_t offset);
/**
 *  \brief
 *      Read a long from the SRAM.
 *
 *  \param offset
 *      Offset where we want to read.
 *  \return value.
 */
uint32_t SRAM_readLong(uint32_t offset);
/**
 *  \brief
 *      Write a byte to the SRAM.
 *
 *  \param offset
 *      Offset where we want to write.
 *  \param val
 *      Value wto write.
 */
void SRAM_writeByte(uint32_t offset, uint8_t val);
/**
 *  \brief
 *      Write a word to the SRAM.
 *
 *  \param offset
 *      Offset where we want to write.
 *  \param val
 *      Value wto write.
 */
void SRAM_writeWord(uint32_t offset, uint16_t val);
/**
 *  \brief
 *      Write a long to the SRAM.
 *
 *  \param offset
 *      Offset where we want to write.
 *  \param val
 *      Value wto write.
 */
void SRAM_writeLong(uint32_t offset, uint32_t val);
