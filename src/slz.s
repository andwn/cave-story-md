#****************************************************************************
# void DecompressSlzC(const void *indata, void *outdata)
# C safe version of DecompressSlz
#----------------------------------------------------------------------------
# indata .... Pointer to compressed data ........ -> a1
# outdata ... Where to store decompressed data .. -> a0
# returns ... nothing
#****************************************************************************

    .globl DecompressSlzC
DecompressSlzC:
    move.l  4(sp), a1               /* Get parameters from stack */
    move.l  8(sp), a0
    movem.l d2-d4, -(sp)            /* Save registers */

    move.b  (a1)+, d2               /* Get uncompressed size */
    lsl.w   #8, d2
    move.b  (a1)+, d2

    moveq   #1, d1                  /* Cause code to fetch new token data */
                                    /* as soon as it starts */
SLZ_MainLoop:
    tst.w   d2                      /* Did we read all the data? */
    beq     SLZ_End                   /* If so, we're done with it! */

    subq.w  #1, d1                  /* Check if we need more tokens */
    bne.s   SLZ_HasTokens
    move.b  (a1)+, d0
    moveq   #8, d1
SLZ_HasTokens:

    add.b   d0, d0                  /* Get next token type */
    bcc.s   SLZ_Uncompressed          /* 0 = uncompressed, 1 = compressed */

    move.b  (a1)+, d3               /* Compressed? Read string info */
    lsl.w   #8, d3                    /* d3 = distance */
    move.b  (a1)+, d3                 /* d4 = length */
    move.b  d3, d4
    lsr.w   #4, d3
    and.w   #0x0F, d4

    subq.w  #3, d2                  /* Length is offset by 3 */
    sub.w   d4, d2                  /* Now that we know the string length, */
                                      /* discount it from the amount of data */
                                      /* to be read */

    addq.w  #3, d3                  /* Distance is offset by 3 */
    neg.w   d3                      /* Make distance go backwards */

    add.w   d4, d4                  /* Copy bytes using Duff's device */
    add.w   d4, d4                    /* MUCH faster than a loop, due to lack */
    eor.w   #0x0F<<2, d4              /* of iteration overhead */
    jmp     SLZ_Duff(pc,d4.w)
SLZ_Duff:
    .rept   0x12
    move.b  (a0,d3.w), (a0)+
    .endr

    bra     SLZ_MainLoop            /* Keep processing data */

SLZ_Uncompressed:
    move.b  (a1)+, (a0)+            /* Uncompressed? Read as is */
    subq.w  #1, d2                  /* It's always one byte long */
    bra     SLZ_MainLoop              /* Keep processing data */

SLZ_End:
    movem.l (sp)+, d2-d4            /* Restore registers */

    rts                             /* End of subroutine */
