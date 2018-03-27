    .globl  SRAM_readWord
    .type   SRAM_readWord, @function
    | extern uint16_t SRAM_readWord(uint32_t offset);
SRAM_readWord:
    move.l  4(%sp),%d1
    add.l   %d1,%d1
    lea     0x200001,%a0
    lea     (%a0,%d1.l),%a0
    moveq   #0,%d0
    movep.w 0(%a0),%d0
    rts

    .globl	SRAM_readLong
    .type   SRAM_readLong, @function
    | extern uint32_t SRAM_readLong(uint32_t offset);
SRAM_readLong:
    move.l  4(%sp),%d1
    add.l   %d1,%d1
    lea     0x200001,%a0
    lea     (%a0,%d1.l),%a0
    movep.l 0(%a0),%d0
    rts

    .globl  SRAM_writeWord
    .type   SRAM_writeWord, @function
    | extern void SRAM_writeWord(uint32_t offset, uint16_t val);
SRAM_writeWord:
    move.l  4(%sp),%d1
    add.l   %d1,%d1
*    move.l  8(%sp),%d0              | values on stack are always long
    move.w  8(%sp),%d0              | No you fool!
    lea     0x200001,%a0
    lea     (%a0,%d1.l),%a0
    movep.w %d0,0(%a0)
    rts

    .globl  SRAM_writeLong
    .type   SRAM_writeLong, @function
    | extern void SRAM_writeLong(uint32_t offset, uint32_t val);
SRAM_writeLong:
    move.l  4(%sp),%d1
    add.l   %d1,%d1
    move.l  8(%sp),%d0
    lea     0x200001,%a0
    lea     (%a0,%d1.l),%a0
    movep.l %d0,0(%a0)
    rts
