#ifndef MD_SRAM_H
#define MD_SRAM_H

#include "types.h"

extern void sram_enable(void);
extern void sram_enable_ro(void);
extern void sram_disable(void);

extern uint8_t sram_read_byte(uint32_t offset);
extern uint16_t sram_read_word(uint32_t offset);
extern uint32_t sram_read_long(uint32_t offset);

extern void sram_write_byte(uint32_t offset, uint8_t val);
extern void sram_write_word(uint32_t offset, uint16_t val);
extern void sram_write_long(uint32_t offset, uint32_t val);

#endif //MD_SRAM_H
