#ifndef MD_SYS_H
#define MD_SYS_H

#include "types.h"

// 256 bytes of zero
extern const uint32_t BlankData[64];
// VBlank stuff
extern volatile uint8_t vblank;


static inline void sys_hard_reset() {
    __asm__("move   #0x2700,%sr\n\t" \
            "move.l (0),%a7\n\t"     \
            "jmp    _hard_reset");
}

static inline void enable_ints() {
    __asm__("move #0x2500,%sr");
}

static inline void disable_ints() {
    __asm__("move #0x2700,%sr");
}

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

#endif //MD_SYS_H
