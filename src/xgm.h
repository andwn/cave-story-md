static inline void z80_pause() {
    __asm__("move.w #0x100,(0xA11100)\n"
            "1:\n"
            "    btst  #0,(0xA11100)\n"
            "    bne.s 1b");
}
static inline void z80_pause_fast() {
    __asm__("move.w  #0x100,(0xA11100)");
}
static inline void z80_resume() {
    __asm__("move.w  #0x000,(0xA11100)");
}

void xgm_init();

void xgm_music_play(const uint8_t *xgc);
void xgm_music_stop();
void xgm_music_pause();

void xgm_pcm_set(uint8_t id, const uint8_t *sample, uint32_t len);
void xgm_pcm_play(uint8_t id, uint8_t priority, uint16_t channel);
