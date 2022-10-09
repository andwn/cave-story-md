//#define STACK_SIZE      0x800
//#define MEMORY_HIGH     (0x01000000 - STACK_SIZE)

void mem_init();
uint16_t mem_get_free();
uint16_t mem_get_used();
void free(void *ptr);
void* malloc(uint16_t size) __attribute__((malloc));

void memset(void *to, uint8_t value, uint16_t len);
void memcpy(void *to, const void *from, uint16_t len);
