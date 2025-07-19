	.include "macros.i"

/* TMSS */
EQU ConsoleVer, 0xA10001   /* Console Version */
EQU TMSS,       0xA14000   /* TMSS Register */

	.section .bss

	.align 2
/* Error Handling Stuff */
VAR v_err_reg,  l, 16
VAR v_err_pc,   l, 1
VAR v_err_addr, l, 1
VAR v_err_inst, w, 1
VAR v_err_sr,   w, 1
VAR v_err_type, b, 1

/* VSync */
VAR vblank,     b, 1

/* CPU Vectors */

	.section .text.vectors

Vectors:
		dc.l    __stack_top, _start
		dc.l    BusError, AddressError, IllegalInst, ZeroDivide
	.rept 22
		dc.l	NullInt
	.endr
		dc.l	HorizontalInt, NullInt, VerticalInt, NullInt
	.rept 32
		dc.l	NullInt
	.endr

/* ROM Header */

	.section .text.header

RomHeader:
		.ascii	"SEGA MEGA DRIVE "          /* Console Signature */
	.globl Date
Date:
		.ascii	"SKYCHASE 2025.07"          /* Company & Date */
		.ascii	"DOUKUTSU MONOGATARI MD  "  /* Domestic (JP) Title */
		.ascii  "                        "
		.ascii	"Cave Story MD           "  /* Overseas (EN) Title */
		.ascii  "                        "
		.ascii	"GM ANDYG002-B0"            /* ROM ID */
		dc.w	0                           /* Checksum (not used) */
		.ascii	"J6              "          /* Peripheral support */
		dc.l	__rom_start                 /* ROM Start Address */
		dc.l	__rom_end-1                 /* ROM End Address */
		dc.l	__ram_start                 /* RAM Start Address */
		dc.l	__ram_end-1                 /* RAM End Address */
		.ascii	"RA"                        /* SRAM Enable */
		dc.w	0xF820                      /* SRAM Type */
		dc.l	SramBase                    /* SRAM Start Address */
		dc.l	SramBase+0xFFFE             /* SRAM End Address */
	.globl Version
Version:
		.ascii	"Ver 0.8.5\0  "              /* Modem? */
	.globl Homepage
Homepage:
		.ascii	"https://github.com/andwn/cave-story-md\0\0" /* Free space for note */
		.ascii	"JUE             "          /* Region */

/* Blank data used for various things */

	.section .text.boot

	.globl BlankData
	.align 256
BlankData:
	.rept 16
		dc.l    0, 0, 0, 0
	.endr

/* System boot */

_start:
		DisableInts
		move.b	(ConsoleVer),d0         /* Check console version */
		andi.b  #0x0F,d0                /* Version 0 = skip TMSS */
		beq.s   SkipTMSS
		move.l  (RomHeader),(TMSS)      /* Write 'SEGA' to TMSS register */
SkipTMSS:
		/* Shut up PSG */
		lea		(0xC00011), a0
		move.b	#0x9F, d0  /* Channel 0 att 15 */
		move.b	#0x20, d1  /* Channel offset */
		move.b	d0, (a0)
		add.b	d1, d0
		move.b	d0, (a0)
		add.b	d1, d0
		move.b	d0, (a0)
		add.b	d1, d0
		move.b	d0, (a0)
	.globl 	_hard_reset
_hard_reset:
		lea     __text_end,a0           /* Start of .data segment init values in ROM */
		lea     __data_start,a1         /* Start of .data segment in RAM */
		move.l  #__data_size,d0         /* Size of .data segment */
		lsr.l   #1,d0
		beq     NoCopy
		subq.w  #1,d0                   /* sub extra iteration */
CopyVar:
		move.w  (a0)+,(a1)+             /* Copy initialized data to RAM */
		dbra    d0,CopyVar
NoCopy:
		/* .bss segment is directly after .data, so a1 is already there */
		move.l  #__bss_size,d0          /* Size of .bss segment in RAM */
		lsr.l   #1,d0
		beq     NoZero
		moveq   #0,d1
		subq.w  #1,d0                   /* sub extra iteration */
ZeroVar:
		move.w  d1,(a1)+                /* Copy zero data to RAM */
		dbra    d0,ZeroVar
NoZero:
		jmp     main

/* Error handling */

	.section .text.error

IllegalInst:
		move.b  #1,(v_err_type)
		bra.s   IllegalDump
ZeroDivide:
		move.b  #2,(v_err_type)
		bra.s   ZeroDump
BusError:
AddressError:
		move.b  #0,(v_err_type)
		move.l  2(sp),(v_err_addr)
		move.w  6(sp),(v_err_inst)
		move.w  8(sp),(v_err_sr)
		move.l  10(sp),(v_err_pc)
		bra.s   RegDump
IllegalDump:
		move.b  #1,(v_err_type)
		bra.s   pc_sr_dump
ZeroDump:
		move.b  #2,(v_err_type)
pc_sr_dump:
		move.w  (sp),(v_err_sr)
		move.l  2(sp),(v_err_pc)
RegDump:
		move.l  a6,(v_err_reg+12)
		lea		(v_err_reg),a6
		move.l  d7,(a6)+
		move.l  a7,(a6)+
		move.l  d6,(a6)+
		adda.w	#4,a6
		move.l  d5,(a6)+
		move.l  a5,(a6)+
		move.l  d4,(a6)+
		move.l  a4,(a6)+
		move.l  d3,(a6)+
		move.l  a3,(a6)+
		move.l  d2,(a6)+
		move.l  a2,(a6)+
		move.l  d1,(a6)+
		move.l  a1,(a6)+
		move.l  d0,(a6)+
		move.l  a0,(a6)+

		lea		(VdpCtrl).l,a5
		lea		(VdpData).l,a6
		moveq	#0,d7
		move.w	#0x8B00,(a5)		/* Plane scroll mode */
		move.w	#0x8F02,(a5)		/* Auto-increment = 2 */
		move.l	#0x7F000000,(a5)	/* VRAM 0xFC00 */
		move.w	d7,(a6)				/* Plane A H scroll = 0 */
		move.l	#0x40000010,(a5)	/* VSRAM 0x0000 */
		move.w	d7,(a6)				/* Plane A V scroll = 0 */
		move.l	#0x7E000000,(a5)	/* VRAM 0xF800 */
		move.l	d7,(a6)				/* Clear sprite table */
		move.l	#0xC0000000,(a5)	/* CRAM $0000 */
		move.l	d7,(a6)
		move.l	#0xC01E0000,(a5)	/* CRAM $001E */
		move.w	0xEEE,(a6)
		move.w	#0x9100,(a5)		/* Hide window plane */
		move.w	#0x9200,(a5)

		lea		(ErrFont).l,a0
		move.l	#0x40000000,(a5)	/* VRAM 0x0000 */
		moveq.l	#0,d5
		moveq.l	#30,d2
DecLoop:
		move.l	d5,(a6)
		moveq.l	#2,d3
DecLoop2:
		move.b	(a0)+,d0
		moveq.l	#1,d4
DecLoop3:
		moveq.l	#0,d1
		btst.b	#6,d0
		sne.b	d1
		lsl.l	#8,d1
		btst.b	#5,d0
		sne.b	d1
		lsl.l	#8,d1
		btst.b	#4,d0
		sne.b	d1
		btst.b	#7,d0
		beq.s	NoPostShift
		andi.l	#0xFF0FFFFF,d1
		lsl.l	#4,d1
NoPostShift:
		lsl.l	#4,d1
		move.l	d1,(a6)

		lsl.b	#4,d0
		dbf		d4,DecLoop3
		dbf		d3,DecLoop2
		move.l	d5,(a6)
		dbf		d2,DecLoop

		move.b	(v_err_type).l,d5
		lsl.w	#4,d5
		lea		StrError(pc, d5.w),a0

		move.l	#0x40000003+(0x82<<16),(a5)	/* VRAM 0xA082 */
		move.w	#0x8000,d0		/* Clear with priority bit set */
		moveq.l	#15,d4
DrawChar:
		move.b	(a0)+,d0
		addq.b	#1,d0
		move.w	d0,(a6)
		dbf		d4,DrawChar
		bra.s	DrawRegs

StrError:	dc.b	10,13,13,22,14,23,23,255,14,22,22,20,22,255,255,255
			dc.b	17,18,18,14,16,10,18,255,17,19,23,24,22,25,12,24
			dc.b	27,14,22,20,255,13,17,26,17,13,14,255,255,255,255,255

DrawRegs:
		lea		(v_err_reg),a0
		move.l	#0x40000003+(0x502<<16),d6	/* VRAM 0xA502 */
		move.w	#7,d5
DrawRegLoop:
		move.l	d6,(a5)		/* Set pos in Plane A */
		move.b	#14,d0		/* D */
		move.w	d0,(a6)
		move.b	d5,d0		/* # */
		addq.b	#1,d0
		move.w	d0,(a6)
		move.b	#29,d0		/* = */
		move.w	d0,(a6)
		move.l	(a0)+,d1
		bsr		PrintHex4
		move.b	#0,d0		/* _ */
		move.w	d0,(a6)
		move.b	#11,d0		/* A */
		move.w	d0,(a6)
		move.b	d5,d0		/* # */
		addq.b	#1,d0
		move.w	d0,(a6)
		move.b	#29,d0		/* = */
		move.w	d0,(a6)
		move.l	(a0)+,d1
		bsr		PrintHex4

		sub.l	#0x00800000,d6	/* Next row in nametable */
		dbf		d5,DrawRegLoop

		move.l	(v_err_reg+4),a0
		move.l	#0x40000003+(0xD32<<16),d6
		move.w	#23,d5
DrawStackLoop:
		move.l	d6,(a5)

		move.b	#24,d0	/* S */
		move.w	d0,(a6)
		move.b	#22,d0	/* P */
		move.w	d0,(a6)
		move.b	#30,d0	/* + */
		move.w	d0,(a6)
		move.b	d5,d1	/* # */
		add.b	d1,d1
		add.b	d1,d1
		bsr		PrintHex1
		move.b	#29,d0	/* = */
		move.w	d0,(a6)
		move.l	(a0)+,d1
		bsr		PrintHex4

		sub.l	#0x00800000,d6
		dbf		d5,DrawStackLoop

		move.l	#0x40000003+(0x602<<16),(a5)
		move.b	#22,d0	/* P */
		move.w	d0,(a6)
		move.b	#13,d0	/* C */
		move.w	d0,(a6)
		move.b	#29,d0	/* = */
		move.w	d0,(a6)
		move.l	(v_err_pc),d1
		bsr		PrintHex3
		
		move.l	#0x40000003+(0x682<<16),(a5)
		move.b	#24,d0	/* S */
		move.w	d0,(a6)
		move.b	#23,d0	/* R */
		move.w	d0,(a6)
		move.b	#29,d0	/* = */
		move.w	d0,(a6)
		move.w	(v_err_sr),d1
		bsr		PrintHex2
		
		cmp.b	#0,(v_err_type)		/* No more params */
		bne.s	Uwa
		
		move.l	#0x40000003+(0x782<<16),(a5)
		move.b	#21,d0	/* O */
		move.w	d0,(a6)
		move.b	#22,d0	/* P */
		move.w	d0,(a6)
		move.b	#29,d0	/* = */
		move.w	d0,(a6)
		move.w	(v_err_inst),d1
		bsr		PrintHex2
		
		move.l	#0x40000003+(0x802<<16),(a5)
		move.b	#11,d0	/* A */
		move.w	d0,(a6)
		move.b	#14,d0	/* D */
		move.w	d0,(a6)
		move.b	#29,d0	/* = */
		move.w	d0,(a6)
		move.l	(v_err_addr),d1
		bsr		PrintHex3
		
Uwa:
		bra.s	Uwa

/* d1 = integer */
/* breaks d0.b */
PrintHex4:
		moveq.l	#7,d2
		bra.s	PrintHexLoop
PrintHex3:
		rol.l	#8,d1
		moveq.l	#5,d2
		bra.s	PrintHexLoop
PrintHex2:
		swap	d1
		moveq.l	#3,d2
		bra.s	PrintHexLoop
PrintHex1:
		ror.l	#8,d1
		moveq.l	#1,d2
PrintHexLoop:
		rol.l	#4,d1
		move.b	d1,d0
		andi.b	#0xF,d0
		addq.b	#1,d0
		move.w	d0,(a6)
		dbf		d2,PrintHexLoop
		rts

ErrFont:
		dc.b	0,0,0								/* ' ' : 255 */
		dc.b	0b01110101, 0b01010101, 0b01010111	/* 0 : 00 */
		dc.b	0b00100010, 0b00100010, 0b00100010	/* 1 : 01 */
		dc.b	0b01110001, 0b01110100, 0b01000111	/* 2 : 02 */
		dc.b	0b01110001, 0b00110001, 0b00010111	/* 3 : 03 */
		dc.b	0b01010101, 0b01010111, 0b00010001	/* 4 : 04 */
		dc.b	0b01110100, 0b01110001, 0b00010111	/* 5 : 05 */
		dc.b	0b01110100, 0b01110101, 0b01010111	/* 6 : 06 */
		dc.b	0b01110001, 0b10010010, 0b00100010	/* 7 : 07 */
		dc.b	0b01110101, 0b01110101, 0b01010111	/* 8 : 08 */
		dc.b	0b01110101, 0b01010111, 0b00010111	/* 9 : 09 */
		dc.b	0b01110101, 0b01010111, 0b01010101	/* A : 10 */
		dc.b	0b11110101, 0b11110101, 0b01011111	/* B : 11 */
		dc.b	0b01110100, 0b01000100, 0b01000111	/* C : 12 */
		dc.b	0b11110101, 0b01010101, 0b01011111	/* D : 13 */
		dc.b	0b01110100, 0b01100100, 0b01000111	/* E : 14 */
		dc.b	0b01110100, 0b01100100, 0b01000100	/* F : 15 */
		dc.b	0b01110100, 0b01000101, 0b01010111	/* G : 16 */
		dc.b	0b10110010, 0b00100010, 0b00101011	/* I : 17 */
		dc.b	0b01000100, 0b01000100, 0b01000111	/* L : 18 */
		dc.b	0b11110101, 0b01010101, 0b01010101	/* N : 19 */
		dc.b	0b10110101, 0b01010101, 0b01011011	/* O : 20 */
		dc.b	0b11110101, 0b01011111, 0b01000100	/* P : 21 */
		dc.b	0b11110101, 0b01011111, 0b01010101	/* R : 22 */
		dc.b	0b10110101, 0b10101001, 0b01011011	/* S : 23 */
		dc.b	0b01110010, 0b00100010, 0b00100010	/* T : 24 */
		dc.b	0b01010101, 0b01010101, 0b01011011	/* U : 25 */
		dc.b	0b01010101, 0b01010101, 0b10110010	/* V : 26 */
		dc.b	0b01110001, 0b10010010, 0b10100111	/* Z : 27 */
		dc.b	0b00000111, 0b00000000, 0b01110000	/* = : 28 */
		dc.b	0b00100010, 0b01110111, 0b00100010	/* + : 29 */
		.even


FUNC klog
		movea.l	4(sp),a0
		move.w	#0x9e00,d0
		move.b	(a0)+,d0
		beq.s	1f
		movea.l	#0xC00004,a1
	0:
		move.w	d0,(a1)
		move.b	(a0)+,d0
		bne.s	0b

		move.w	d0,(a1)
	1:
		rts



/* Interrupts */

	.section .text.interrupts

VerticalInt:
		tst.w   (xgmTempo)
		beq.s   NoXGM
		movem.l d0-d1/a0-a1,-(sp)
		bsr.s   xgm_vblank_process
		movem.l (sp)+,d0-d1/a0-a1
NoXGM:
		/* Set VBlank complete flag */
		st.b	(vblank)
HorizontalInt:
NullInt:
		rte

	.section .text

FUNC xgm_vblank_process
		/* XGM VBlank Process */
		lea    (Z80BusReq),a0           /* a0 = Z80 Halt Port */
		lea    (Z80Reset),a1            /* a1 = Z80 Reset Port */
		clr.w  d1					    /* num = 0 */
		move.w xgmTempoCnt,d0		    /* cnt = xgmTempoCnt */
		bgt.s  XGM_CntSkip				/* cnt > 0 */
    XGM_CntLoop:
		addq.w #1,d1				    /* num++ */
		add.w  xgmTempoDef,d0		    /* cnt += xgmTempoDef */
		ble.s  XGM_CntLoop				/* cnt <= 0 */
    XGM_CntSkip:
		sub.w  xgmTempo,d0				/* xgmTempoCnt = cnt - xgmTempo */
		move.w d0,xgmTempoCnt
    XGM_WhileTRUE:
		move.w #0x0100,(a0)			    /* Take Z80 bus and reset */
		move.w #0x0100,(a1)
    XGM_WaitZ80Get:
		move.w (a0),d0				    /* Wait for bus to be taken */
		btst   #8,d0
		bne.s  XGM_WaitZ80Get
		move.b (Z80DrvParams+0xE),d0	/* if (!z80_drv_params[0x0E]) break */
		beq.s  XGM_VBlankEnd
		move.w #0,(a0)					/* Release Z80 bus */
		movm.l d0-d3,-(sp)		        /* Wait about 80 cycles */
		movm.l (sp)+,d0-d3
    XGM_WaitZ80Rel:
		move.w (a0),d0				    /* Wait for bus to be released */
		btst   #8,d0
		beq.s  XGM_WaitZ80Rel
		bra.s  XGM_WhileTRUE
    XGM_VBlankEnd:
		move.b (Z80DrvParams+0xF),d0	/* z80_drv_params[0x0F] += num */
		add.b  d0,d1
		move.b d1,(Z80DrvParams+0xF)
		move.w #0,(a0)					/* Release Z80 bus */
		rts
