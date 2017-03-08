    .align	2
    .globl  VDP_loadTileData
    .type   VDP_loadTileData, @function
VDP_loadTileData:
    movm.l #0x3e00,-(%sp)

    move.l 24(%sp),%d2              | d2 = data
    move.l 32(%sp),%d3              | d3 = num
    jeq .L1

    movq  #0,%d6
    move.w 30(%sp),%d6              | d6 = ind
    lsl.w #5,%d6                    | d6 = ind * 32 = VRAM address

    tst.b 39(%sp)
    jeq .L2

    lsl.w #4,%d3                    | d3 = num * 16 (size of DMA in word)
    move.l #2,-(%sp)
    move.l %d3,-(%sp)               | prepare parameters for VDP_doDMA
    move.l %d6,-(%sp)
    move.l %d2,-(%sp)
    clr.l -(%sp)
    jsr VDP_waitDMACompletion
    jsr DMA_doDma
    lea (20,%sp),%sp
    jra .L1

    .align  2
.L2:
    pea 2.w
    jsr VDP_setAutoInc
    addq.l #4,%sp

    lsl.l #2,%d6
    lsr.w #2,%d6
    andi.w #0x3FFF,%d6
    ori.w #0x4000,%d6
    swap %d6                            | d6 = formated VRAM address for VDP command write

    move.l %d6,0xC00004                 | set destination address in VDP Ctrl command

    move.l %d2,%a0                      | a0 = data
    move.l #0xC00000,%a1
    subq.w #1,%d3

.L6:
    move.l (%a0)+,(%a1)
    move.l (%a0)+,(%a1)
    move.l (%a0)+,(%a1)
    move.l (%a0)+,(%a1)
    move.l (%a0)+,(%a1)
    move.l (%a0)+,(%a1)
    move.l (%a0)+,(%a1)
    move.l (%a0)+,(%a1)
    dbra %d3,.L6

.L1:
    movm.l (%sp)+,#0x7c
    rts
