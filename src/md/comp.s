    .include "macros.i"

/* extern void slz_unpack(const void *in, void *out) */
FUNC slz_unpack
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
        beq     SLZ_End                 /* If so, we're done with it! */

        subq.w  #1, d1                  /* Check if we need more tokens */
        bne.s   SLZ_HasTokens
        move.b  (a1)+, d0
        moveq   #8, d1
    SLZ_HasTokens:

        add.b   d0, d0                  /* Get next token type */
        bcc.s   SLZ_Uncompressed        /* 0 = uncompressed, 1 = compressed */

        move.b  (a1)+, d3               /* Compressed? Read string info */
        lsl.w   #8, d3                  /* d3 = distance */
        move.b  (a1)+, d3               /* d4 = length */
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
        add.w   d4, d4                  /* MUCH faster than a loop, due to lack */
        eor.w   #0x0F<<2, d4            /* of iteration overhead */
        jmp     SLZ_Duff(pc,d4.w)
    SLZ_Duff:
        .rept   0x12
        move.b  (a0,d3.w), (a0)+
        .endr

        bra     SLZ_MainLoop            /* Keep processing data */

    SLZ_Uncompressed:
        move.b  (a1)+, (a0)+            /* Uncompressed? Read as is */
        subq.w  #1, d2                  /* It's always one byte long */
        bra     SLZ_MainLoop            /* Keep processing data */

    SLZ_End:
        movem.l (sp)+, d2-d4            /* Restore registers */

        rts                             /* End of subroutine */


/* extern void uftc_unpack(const void *in, void *out, uint16_t start, uint16_t num) */
FUNC uftc_unpack
        move.l  4(sp),a0                /* in */
        move.l  8(sp),a1                /* out */
        /* -mshort */
        move.w  12(sp),d0               /* start */
        move.w  14(sp),d1               /* num */
        /* no -mshort */
#        move.w  14(sp),d0              /* start */
#        move.w  18(sp),d1              /* num */
        movem.l d2/a2-a4, -(sp)         /* Save registers */

        moveq   #0, d2                  /* Get size of dictionary */
        move.w  (a0)+, d2

        lea     (a0,d2.l), a4           /* Get address of data with first tile */
        and.l   #0xFFFF, d0               /* to be decompressed (using a dword */
        lsl.l   #3, d0                    /* so we can have up to 8192 tiles) */
        lea     (a4,d0.l), a4

        bra.s   UFTC_LoopEnd            /* Start decompressing */
    UFTC_Loop:
        move.w  (a4)+, d2               /* Fetch addresses of dictionary */
        lea     (a0,d2.l), a3             /* entries for the first two 4x4 */
        move.w  (a4)+, d2                 /* blocks of this tile */
        lea     (a0,d2.l), a2

        move.w  (a3)+, (a1)+            /* Decompress first pair of 4x4 blocks */
        move.w  (a2)+, (a1)+              /* into the output buffer */
        move.w  (a3)+, (a1)+
        move.w  (a2)+, (a1)+
        move.w  (a3)+, (a1)+
        move.w  (a2)+, (a1)+
        move.w  (a3)+, (a1)+
        move.w  (a2)+, (a1)+

        move.w  (a4)+, d2               /* Fetch addresses of dictionary */
        lea     (a0,d2.l), a3             /* entries for the last two 4x4 */
        move.w  (a4)+, d2                 /* blocks of this tile */
        lea     (a0,d2.l), a2

        move.w  (a3)+, (a1)+            /* Decompress last pair of 4x4 blocks */
        move.w  (a2)+, (a1)+              /* into the output buffer */
        move.w  (a3)+, (a1)+
        move.w  (a2)+, (a1)+
        move.w  (a3)+, (a1)+
        move.w  (a2)+, (a1)+
        move.w  (a3)+, (a1)+
        move.w  (a2)+, (a1)+

    UFTC_LoopEnd:
        dbf     d1, UFTC_Loop           /* Go for next tile */

        movem.l  (sp)+, d2/a2-a4        /* Restore registers */
        rts                             /* End of subroutine */
