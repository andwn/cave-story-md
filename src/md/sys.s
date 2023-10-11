    .include "macros.i"

/* TMSS */
EQU ConsoleVer, 0xA10001   /* Console Version */
EQU TMSS,       0xA14000   /* TMSS Register */

    .section .bss.system

/* Error Handling Stuff */
VAR v_err_reg,  l, 16
VAR v_err_pc,   l, 1
VAR v_err_addr, l, 1
VAR v_err_ext1, w, 1
VAR v_err_ext2, w, 1
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
		.ascii	"SKYCHASE 2023.10"          /* Company & Date */
		.ascii	"DOUKUTSU MONOGATARI MD  "  /* Domestic (JP) Title */
		.ascii  "                        "
		.ascii	"Cave Story MD           "  /* Overseas (EN) Title */
		.ascii  "                        "
		.ascii	"GM ANDYG002-A8"            /* ROM ID */
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
		.ascii	"Ver 0.8.1\0  "              /* Modem? */
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
_hard_reset:                            /* SYS_HardReset() resets sp and jumps here */
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
        jsr     main                    /* IT BEGINS */
        beq.s   _hard_reset             /* main returned, reset */

/* Error handling */

IllegalInst:
		move.b  #2,(v_err_type)
        bra.s   IllegalDump
ZeroDivide:
		move.b  #3,(v_err_type)
        bra.s   ZeroDump
BusError:
AddressError:
		move.b  #1,(v_err_type)
        move.w  4(sp),v_err_ext1
        move.l  6(sp),v_err_addr
        move.w  10(sp),v_err_ext2
        move.w  12(sp),v_err_sr
        move.l  14(sp),v_err_pc
        bra.s   RegDump
IllegalDump:
        move.w  10(sp),v_err_ext1
ZeroDump:
        move.w  4(sp),v_err_sr
        move.l  6(sp),v_err_pc
RegDump:
        move.l  d0,v_err_reg+0
        move.l  d1,v_err_reg+4
        move.l  d2,v_err_reg+8
        move.l  d3,v_err_reg+12
        move.l  d4,v_err_reg+16
        move.l  d5,v_err_reg+20
        move.l  d6,v_err_reg+24
        move.l  d7,v_err_reg+28
        move.l  a0,v_err_reg+32
        move.l  a1,v_err_reg+36
        move.l  a2,v_err_reg+40
        move.l  a3,v_err_reg+44
        move.l  a4,v_err_reg+48
        move.l  a5,v_err_reg+52
        move.l  a6,v_err_reg+56
        move.l  a7,v_err_reg+60
        jmp     _error

/* Interrupts */

VerticalInt:
        tst.w   (xgmTempo)
        beq.s   NoXGM
		movem.l d0-d1/a0-a1,-(sp)
		jsr     xgm_vblank_process
		movem.l (sp)+,d0-d1/a0-a1
NoXGM:
		/* Set VBlank complete flag */
        move.b	#1,(vblank)
HorizontalInt:
NullInt:
        rte
