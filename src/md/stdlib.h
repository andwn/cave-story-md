#ifndef MD_STDLIB_H
#define MD_STDLIB_H

#include "types.h"

void srand(uint16_t seed);
uint16_t rand(void);

void mem_init(void);
uint16_t mem_get_free(void);
uint16_t mem_get_used(void);
void free(void *ptr);
void* malloc(uint16_t size) __attribute__((malloc));

void memclr(void *to, uint16_t len);
void memset(void *to, uint8_t value, uint16_t len);
void memcpy(void *to, const void *from, uint16_t len);

#endif //MD_STDLIB_H
