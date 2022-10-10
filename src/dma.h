extern const uint32_t DmaVRAM;
extern const uint32_t DmaCRAM;
extern const uint32_t DmaVSRAM;

extern void dma_now(uint32_t cmd, uint32_t from, uint16_t to, uint16_t len, uint16_t inc);
extern void dma_queue(uint32_t cmd, uint32_t from, uint16_t to, uint16_t len, uint16_t inc);
//extern void dma_pop();
extern void dma_flush();
extern void dma_clear();
