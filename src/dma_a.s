/* Based on Miniplanets DMA routines */

.section .bss

    .globl dmabuf
    .globl dmabuf_end
dmabuf:     ds.w 8*48
dmabuf_end: ds.l 1

.section .text

/* void dma_queue(uint32_t cmd, uint32_t from, uint16_t to, uint16_t len, uint16_t inc) */
#    .globl dma_queue
#dma_queue:
#        move.l  (dmabuf_end),a0
#
#        move.w  16(sp),d0 /* inc */
#
#        or.w    #0x8F00,d0
#        move.w  d0,(a0)+
#
#        move.l  8(sp), d0 /* from */
#        move.w  14(sp),d1 /* len */
#
#        lsr.l   #1,d0
#        move.l  #0x94009300, (a0)+
#        move.l  #0x97009600, (a0)+
#        move.w  #0x9500, (a0)+
#        movep.l d0, -7(a0)
#        movep.w d1, -9(a0)
#
#        move.l  4(sp), d0 /* cmd */
#        move.w  12(sp),d1 /* to */
#
#        and.l   #0xFFFF,d1
#        lsl.l   #2,d1
#        lsr.w   #2,d1
#        swap    d1
#        or.l    d0,d1
#        move.l  d1,(a0)+
#
#        move.l  a0,(dmabuf_end)
#        rts

/* void dma_pop() */
#    .globl dma_pop
#dma_pop:
#        sub.l   #16,(dmabuf_end)
#        move.l  (dmabuf_end),a0
#        lea     (0xC00004),a1
#        move.w  #7,d0
#
#    dma_pop_loop:
#        move.w  (a0)+,(a1)
#        dbf     d0,dma_pop_loop
#
#        rts

/* void dma_flush() */
    .globl dma_flush
dma_flush:
        lea     (dmabuf),a0
        lea     (0xC00004),a1

        move.l  (dmabuf_end),d0
        sub.l   #dmabuf,d0
        beq.s   dma_clear
        lsr.w   #1,d0
        subq.w  #1,d0

    dma_flush_loop:
        move.w  (a0)+,(a1)
        dbf     d0,dma_flush_loop

        move.w  #0x8F02,(a1)  /* autoinc = 2 */

/* void dma_clear() */
    .globl dma_clear
dma_clear:
        move.l  #dmabuf,(dmabuf_end)
        rts
