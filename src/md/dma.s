    .include "macros.i"

/* VDP DMA Registers */
EQU VDPREG_DMALEN_L, 0x9300  /* DMA length (low) */
EQU VDPREG_DMALEN_H, 0x9400  /* DMA length (high) */
EQU VDPREG_DMASRC_L, 0x9500  /* DMA source (low) */
EQU VDPREG_DMASRC_M, 0x9600  /* DMA source (mid) */
EQU VDPREG_DMASRC_H, 0x9700  /* DMA source (high) */

/* VDP DMA Commands */
EQU VRAM_DMA_CMD,    0x40000080
EQU CRAM_DMA_CMD,    0xC0000080
EQU VSRAM_DMA_CMD,   0x40000090

    .section .bss

    .align 2
VAR dmabuf,     w, 8*48
VAR dmabuf_end, l, 1

    .section .text

    .ifndef SLOW_MODE

/* void dma_queue(uint32_t cmd, uint32_t from, uint16_t to, uint16_t len, uint16_t inc) */
FUNC dma_queue
        move.l  (dmabuf_end),a0

        move.w  16(sp),d0 /* inc */

        or.w    #VDPREG_INCR,d0
        move.w  d0,(a0)+

        move.l  8(sp), d0 /* from */
        move.w  14(sp),d1 /* len */

        lsr.l   #1,d0
        move.l  #0x94009300, (a0)+
        move.l  #0x97009600, (a0)+
        move.w  #0x9500, (a0)+
        movep.l d0, -7(a0)
        movep.w d1, -9(a0)

        move.l  4(sp), d0 /* cmd */
        move.w  12(sp),d1 /* to */

        and.l   #0xFFFF,d1
        lsl.l   #2,d1
        lsr.w   #2,d1
        swap    d1
        or.l    d0,d1
        move.l  d1,(a0)+

        move.l  a0,(dmabuf_end)
        rts

/* void dma_pop() */
FUNC dma_pop
        sub.l   #16,(dmabuf_end)
        move.l  (dmabuf_end),a0
        lea     (VdpCtrl),a1
        move.w  #7,d0

        DisableInts
        FastPauseZ80

    dma_pop_loop:
        move.w  (a0)+,(a1)
        dbf     d0,dma_pop_loop

        ResumeZ80
        EnableInts

        rts

/* void dma_flush() */
FUNC dma_flush
        lea     (dmabuf),a0
        lea     (VdpCtrl),a1

        move.l  (dmabuf_end),d0
        sub.l   #dmabuf,d0
        beq.s   dma_clear
        lsr.w   #1,d0
        subq.w  #1,d0

        DisableInts
        FastPauseZ80

    dma_flush_loop:
        move.w  (a0)+,(a1)
        dbf     d0,dma_flush_loop

        move.w  #VDPREG_INCR+2,(a1)  /* autoinc = 2 */

        ResumeZ80
        EnableInts

/* void dma_clear() */
FUNC dma_clear
        move.l  #dmabuf,(dmabuf_end)
        rts

    .endif
