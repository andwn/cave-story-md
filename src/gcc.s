* 32-bit multiplication and division routines from libgcc

	.globl	__mulsi3
__mulsi3:
	move.w	4(sp), d0	/* x0 -> d0 */
	mulu.w	10(sp), d0	/* x0*y1 */
	move.w	6(sp), d1	/* x1 -> d1 */
	mulu.w	8(sp), d1	/* x1*y0 */
	add.w	d1, d0
	swap	d0
	clr.w	d0
	move.w	6(sp), d1	/* x1 -> d1 */
	mulu.w	10(sp), d1	/* x1*y1 */
	add.l	d1, d0
	rts

	.globl	__udivsi3
__udivsi3:
	move.l	d2, -(sp)
	move.l	12(sp), d1	/* d1 = divisor */
	move.l	8(sp), d0	/* d0 = dividend */

	cmp.l	#0x10000, d1 /* divisor >= 2 ^ 16 ?   */
	bcc.s	1f		/* then try next algorithm */
	move.l	d0, d2
	clr.w	d2
	swap	d2
	divu	d1, d2          /* high quotient in lower word */
	move.w	d2, d0		/* save high quotient */
	swap	d0
	move.w	10(sp), d2	/* get low dividend + high rest */
	divu	d1, d2		/* low quotient */
	move.w	d2, d0
	bra.s	4f

1:	move.l	d1, d2		/* use d2 as divisor backup */
2:	lsr.l	#1, d1	/* shift divisor */
	lsr.l	#1, d0	/* shift dividend */
	cmp.l	#0x10000, d1 /* still divisor >= 2 ^ 16 ?  */
	bcc.s	2b
	divu	d1, d0		/* now we have 16-bit divisor */
	and.l	#0xFFFF, d0 /* mask out divisor, ignore remainder */

/* Multiply the 16-bit tentative quotient with the 32-bit divisor.  Because of
   the operand ranges, this might give a 33-bit product.  If this product is
   greater than the dividend, the tentative quotient was too large. */
	move.l	d2, d1
	mulu	d0, d1		/* low part, 32 bits */
	swap	d2
	mulu	d0, d2		/* high part, at most 17 bits */
	swap	d2		/* align high part with low part */
	tst.w	d2		/* high part 17 bits? */
	bne.s	3f		/* if 17 bits, quotient was too large */
	add.l	d2, d1		/* add parts */
	bcs.s	3f		/* if sum is 33 bits, quotient was too large */
	cmp.l	8(sp), d1	/* compare the sum with the dividend */
	bls.s	4f		/* if sum > dividend, quotient was too large */
3:	subq.l	#1, d0	/* adjust quotient */

4:	move.l	(sp)+, d2
	rts

	.globl	__divsi3
__divsi3:
	move.l	d2, -(sp)

	moveq	#1, d2	/* sign of result stored in d2 (=1 or =-1) */
	move.l	12(sp), d1	/* d1 = divisor */
	bpl.s	1f
	neg.l	d1
	neg.b	d2		/* change sign because divisor <0  */
1:	move.l	8(sp), d0	/* d0 = dividend */
	bpl.s	2f
	neg.l	d0
	neg.b	d2

2:	move.l	d1, -(sp)
	move.l	d0, -(sp)
	bsr.s	__udivsi3	/* divide abs(dividend) by abs(divisor) */
	addq.l	#8, sp

	tst.b	d2
	bpl.s	3f
	neg.l	d0

3:	move.l	(sp)+, d2
	rts

	.globl	__umodsi3
__umodsi3:
	move.l	8(sp), d1	/* d1 = divisor */
	move.l	4(sp), d0	/* d0 = dividend */
	move.l	d1, -(sp)
	move.l	d0, -(sp)
	bsr.w	__udivsi3
	addq.l	#8, sp
	move.l	8(sp), d1	/* d1 = divisor */
	move.l	d1, -(sp)
	move.l	d0, -(sp)
	bsr.w	__mulsi3	/* d0 = (a/b)*b */
	addq.l	#8, sp
	move.l	4(sp), d1	/* d1 = dividend */
	sub.l	d0, d1		/* d1 = a - (a/b)*b */
	move.l	d1, d0
	rts

	.globl	__modsi3
__modsi3:
	move.l	8(sp), d1	/* d1 = divisor */
	move.l	4(sp), d0	/* d0 = dividend */
	move.l	d1, -(sp)
	move.l	d0, -(sp)
	bsr.w	__divsi3
	addq.l	#8, sp
	move.l	8(sp), d1	/* d1 = divisor */
	move.l	d1, -(sp)
	move.l	d0, -(sp)
	bsr.w	__mulsi3	/* d0 = (a/b)*b */
	addq.l	#8, sp
	move.l	4(sp), d1	/* d1 = dividend */
	sub.l	d0, d1		/* d1 = a - (a/b)*b */
	move.l	d1, d0
	rts
