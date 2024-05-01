	.include "macros.i"

FUNC memclr
		move.w	8(sp),d0			/* d0 = len */
		beq		memset_ret
		swap	d0					/* keep len in upper word for later */

		moveq.l	#0,d1				/* d1 = value */
		bra.s	memset_clrval

FUNC memset
		move.w	10(sp),d0			/* d0 = len */
		beq		memset_ret
		swap	d0					/* keep len in upper word for later */

		move.b	9(sp),d1			/* d1 = value */
		move.b	d1,d0
		lsl.w	#8,d1
		move.b	d0,d1
		move.w	d1,d0
		swap	d1
		move.w	d0,d1				/* d1 = value | (value << 8) | (value << 16) | (value << 24) */

memset_clrval:
		move.l	4(sp),a0			/* a0 = to */
		move.w	a0,d0
		btst	#0,d0				/* dst & 1 ? (word aligned) */
		beq		memset_aligned_2
		move.b	d1,(a0)+

memset_aligned_2:
		btst	#1,d0				/* dst & 2 ? (long aligned) */
		beq		memset_aligned_4
		move.w	d1,(a0)+

memset_aligned_4:
		swap	d0					/* restore length value */
		lsr.w	#2,d0				/* convert to long copy iterations */
		subq.w	#1,d0
memset_loop:
		move.l	d1,(a0)+
		dbf		d0,memset_loop

memset_ret:
		rts


FUNC memcpy
		move.w	12(sp),d0			/* d0 = len */
		beq.s	memset_ret

		move.l	4(sp),a1			/* a1 = dst */
		move.l	8(sp),a0			/* a0 = src */

		move.w	a0,d1
		btst	#0,d1
		beq.s	memcpy_unaligned
		move.w	a1,d1
		btst	#0,d1
		beq.s	memcpy_unaligned

		btst	#0,d0
		bne.s	memcpy_len_2
		move.b	(a0)+,(a1)+

memcpy_len_2:
		btst	#1,d0
		bne.s	memcpy_len_4
		move.w	(a0)+,(a1)+

memcpy_len_4:
		lsr.w	#2,d0
		subq.w	#1,d0
memcpy_long_loop:
		move.l	(a0)+,(a1)+
		dbf		d0,memcpy_long_loop
		rts

memcpy_unaligned:
		subq.w	#1,d0
memcpy_byte_loop:
		move.b	(a0)+,(a1)+
		dbf		d0,memcpy_byte_loop
		rts
