
/***************************************************************************** */
/* DecompressSlz */
/* Decompresses SLZ data into memory */
/*---------------------------------------------------------------------------- */
/* input a6.l .... Pointer to compressed data */
/* input a5.l .... Where to store decompressed data */
/* output a6.l ... Right after input buffer */
/* output a5.l ... Right after output buffer */
/* breaks ........ d5, d6, d7 */
/***************************************************************************** */
	.globl DecompressSlz
DecompressSlz:
    move.w  d3, -(sp)               /* Save registers */
    move.w  d4, -(sp)

    move.b  (a6)+, d7               /* Get uncompressed size */
    lsl.w   #8, d7
    move.b  (a6)+, d7

    moveq   #1, d6                  /* Cause code to fetch new token data */
                                    /* as soon as it starts */
slz_MainLoop:
    tst.w   d7                      /* Did we read all the data? */
    beq     slz_End                      /* If so, we're done with it! */

    subq.w  #1, d6                  /* Check if we need more tokens */
    bne.s   slz_HasTokens
    move.b  (a6)+, d5
    moveq   #8, d6
slz_HasTokens:

    add.b   d5, d5                  /* Get next token type */
    bcc.s   slz_Uncompressed             /* 0 = uncompressed, 1 = compressed */

    move.b  (a6)+, d3               /* Compressed? Read string info */
    lsl.w   #8, d3                    /* d3 = distance */
    move.b  (a6)+, d3                 /* d4 = length */
    move.b  d3, d4
    lsr.w   #4, d3
    and.w   #0x0F, d4

    subq.w  #3, d7                  /* Length is offset by 3 */
    sub.w   d4, d7                  /* Now that we know the string length, */
                                      /* discount it from the amount of data */
                                      /* to be read */

    addq.w  #3, d3                  /* Distance is offset by 3 */
    neg.w   d3                      /* Make distance go backwards */

    add.w   d4, d4                  /* Copy bytes using Duff's device */
    add.w   d4, d4                    /* MUCH faster than a loop, due to lack */
    eor.w   #0x0F<<2, d4               /* of iteration overhead */
    jmp     slz_Duff(pc,d4.w)
slz_Duff:
    .rept    0x10+2
    move.b  (a5,d3.w), (a5)+
    .endr

    bra     slz_MainLoop               /* Keep processing data */

slz_Uncompressed:
    move.b  (a6)+, (a5)+            /* Uncompressed? Read as is */
    subq.w  #1, d7                  /* It's always one byte long */
    bra     slz_MainLoop               /* Keep processing data */

slz_End:
    move.w  (sp)+, d4               /* Restore registers */
    move.w  (sp)+, d3
rts /* End of subroutine */
