    .include "macros.i"

/* SRAM & Mapper */
EQU SramControl,   0xA130F1   /* SRAM Mode register */
EQU SramBase,      0x200001   /* Start address for SRAM (odd bytes) */

/* extern void sram_enable(); */
FUNC sram_enable
        move.b  #1,(SramControl)
        rts

/* extern void sram_enable_ro(); */
FUNC sram_enable_ro
        move.b  #3,(SramControl)
        rts

/* extern void sram_disable(); */
FUNC sram_disable
        move.b  #0,(SramControl)
        rts

/* extern uint8 sram_read_byte(uint32 offset); */
FUNC sram_read_byte
        move.l  4(sp),d1
        add.l   d1,d1
        lea     (SramBase),a0
        lea     (a0,d1.l),a0
        moveq   #0,d0
        move.b  (a0),d0
        rts

/* extern uint16 sram_read_word(uint32 offset); */
FUNC sram_read_word
        move.l  4(sp),d1
        add.l   d1,d1
        lea     (SramBase),a0
        lea     (a0,d1.l),a0
        moveq   #0,d0
        movep.w 0(a0),d0
        rts

/* extern uint32 sram_read_long(uint32 offset); */
FUNC sram_read_long
        move.l  4(sp),d1
        add.l   d1,d1
        lea     (SramBase),a0
        lea     (a0,d1.l),a0
        movep.l 0(a0),d0
        rts

/* extern void sram_write_byte(uint32 offset, uint8 val); */
FUNC sram_write_byte
        move.l  4(sp),d1
        add.l   d1,d1
        move.b  9(sp),d0
        lea     (SramBase),a0
        lea     (a0,d1.l),a0
        move.b  d0,(a0)
        rts

/* extern void sram_write_word(uint32 offset, uint16 val); */
FUNC sram_write_word
        move.l  4(sp),d1
        add.l   d1,d1
        move.w  8(sp),d0
        lea     (SramBase),a0
        lea     (a0,d1.l),a0
        movep.w d0,0(a0)
        rts

/* extern void sram_write_long(uint32 offset, uint32 val); */
FUNC sram_write_long
        move.l  4(sp),d1
        add.l   d1,d1
        move.l  8(sp),d0
        lea     (SramBase),a0
        lea     (a0,d1.l),a0
        movep.l d0,0(a0)
        rts
