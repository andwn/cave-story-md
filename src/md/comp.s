    .include "macros.i"

FUNC zx0_unpack
		move.l	4(sp),a0
		move.l	8(sp),a1
zx0_decompress:
		movem.l a2/d2,-(sp)  /* preserve registers */
		moveq #-128,d1       /* initialize empty bit queue */
							/* plus bit to roll into carry */
		moveq #-1,d2         /* initialize rep-offset to 1 */

	.literals:     
        bsr.s .get_elias     /* read number of literals to copy */
		subq.l #1,d0         /* dbf will loop until d0 is -1, not 0 */
	.copy_lits:    
        move.b (a0)+,(a1)+   /* copy literal byte */
		dbf d0,.copy_lits    /* loop for all literal bytes */
		
		add.b d1,d1          /* read 'match or rep-match' bit */
		bcs.s .get_offset    /* if 1: read offset, if 0: rep-match */

	.rep_match:    
        bsr.s .get_elias     /* read match length (starts at 1) */
	.do_copy:      
        subq.l #1,d0         /* dbf will loop until d0 is -1, not 0 */
	.do_copy_offs: 
        move.l a1,a2         /* calculate backreference address */
		add.l d2,a2          /* (dest + negative match offset)      */
	.copy_match:   
        move.b (a2)+,(a1)+   /* copy matched byte */
		dbf d0,.copy_match   /* loop for all matched bytes */

		add.b d1,d1          /* read 'literal or match' bit */
		bcc.s .literals      /* if 0: go copy literals */

	.get_offset:   
        moveq #-2,d0         /* initialize value to $fe */
		bsr.s .elias_loop    /* read high byte of match offset */
		addq.b #1,d0         /* obtain negative offset high byte */
		beq.s .done          /* exit if EOD marker */
		move.w d0,d2         /* transfer negative high byte into d2 */
		lsl.w #8,d2          /* shift it to make room for low byte */

		moveq #1,d0          /* initialize length value to 1 */
		move.b (a0)+,d2      /* read low byte of offset + 1 bit of len */
		asr.l #1,d2          /* shift len bit into carry/offset in place */
		bcs.s .do_copy_offs  /* if len bit is set, no need for more */
		bsr.s .elias_bt      /* read rest of elias-encoded match length */
		bra.s .do_copy_offs  /* go copy match */

	.get_elias:    
        moveq #1,d0          /* initialize value to 1 */
	.elias_loop:   
        add.b d1,d1          /*shift bit queue, high bit into carry */
		bne.s .got_bit       /*queue not empty, bits remain */
		move.b (a0)+,d1      /*read 8 new bits */
		addx.b d1,d1         /*shift bit queue, high bit into carry */
							 /*and shift 1 from carry into bit queue */

	.got_bit:      
        bcs.s .got_elias     /* done if control bit is 1 */
	.elias_bt:     
        add.b d1,d1          /* read data bit */
		addx.l d0,d0         /* shift data bit into value in d0 */
		bra.s .elias_loop    /* keep reading */

    .done:         
        movem.l (sp)+,a2/d2  /* restore preserved registers */
    .got_elias:    
        rts


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
