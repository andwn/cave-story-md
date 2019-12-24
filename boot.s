.section .data

	.globl v_err_reg
	.globl v_err_pc
	.globl v_err_addr
	.globl v_err_ext1
	.globl v_err_ext2
	.globl v_err_sr
	.globl v_err_type
	.globl vblank
	
	.globl xgmTempo
	.globl xgmTempoCnt
	.globl xgmTempoDef
	
v_err_reg:	ds.l 16
v_err_pc:	ds.l 1
v_err_addr:	ds.l 1
v_err_ext1:	ds.w 1
v_err_ext2: ds.w 1
v_err_sr:	ds.w 1
v_err_type:	ds.b 1
vblank:		ds.b 1

xgmTempo:   ds.w 1
xgmTempoCnt:ds.w 1
xgmTempoDef:ds.w 1

.section .text

    .globl 	_hard_reset

    .org    0x00000000

RomStart:
        dc.l    0x000000, _start
        dc.l    BusError, AddressError, IllegalInst, ZeroDivide
		dc.l	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		dc.l	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		dc.l	ExtInt, 0, HBlank, 0, VBlank, 0
	.rept 8
		dc.l	0, 0, 0, 0
	.endr

RomHeader:
        .ascii	"SEGA MEGA DRIVE "
		.ascii	"GRIND   2019.DEC"
		.ascii	"Doukutsu Monogatari MD                          "
		.ascii	"Cave Story MD                                   "
		.ascii	"GM ANDYG002-A5"
		dc.w	0
		.ascii	"J               "
		dc.l	0x000000
		dc.l	0x3FFFFF
		dc.l	0xFF0000
		dc.l	0xFFFFFF
		.ascii	"RA"
		dc.w	0xF820
		dc.l	0x200001
		dc.l	0x20FFFF
		.ascii	"            "
		.ascii	"https://github.com/andwn/cave-story-md  "
		.ascii	"JUE             "

_start:
        move    #0x2700,sr              /* Disable interrupts */
		move.b	(0xA10001),d0           /* Check console version */
        andi.b  #0x0F,d0                /* Version 0 = skip TMSS */
        beq.s   NoTMSS
        move.l  (0x100),0xA14000        /* Write 'SEGA' to TMSS register */
NoTMSS:
        move.w  (0xC00004),d0           /* Read VDP status */
        move.w  #0x0100,(0xA11100)      /* Halt / Reset Z80 */
        move.w  #0x0100,(0xA11200)
_hard_reset:                            /* SYS_HardReset() resets sp and jumps here */
        lea     0xFF0000,a0             /* First RAM address */
        moveq   #0,d0
        move.w  #0x3FFF,d1              /* (Size of RAM - 1) / Size of long */
ClearRam:
        move.l  d0,(a0)+
        dbra    d1,ClearRam
        lea     _stext,a0               /* Start of initialized data (BSS) in ROM */
        lea     0xFF0000,a1             /* First RAM address */
        move.l  #_sdata,d0              /* (Size of BSS + 1) / 2 */
        addq.l  #1,d0
        lsr.l   #1,d0
        beq     NoCopy
        subq.w  #1,d0                   /* sub extra iteration */
CopyVar:
        move.w  (a0)+,(a1)+             /* Copy initialized data to RAM */
        dbra    d0,CopyVar
NoCopy:
        jsr     main                    /* IT BEGINS */
        beq.s   _hard_reset             /* main returned, reset */

/* Error handling */

BusError:
		move.b #0,(v_err_type)
        bra.s  AddressDump

AddressError:
		move.b #1,(v_err_type)
        bra.s  AddressDump

IllegalInst:
		move.b #2,(v_err_type)
        bra.s  IllegalDump

ZeroDivide:
		move.b #3,(v_err_type)
        bra.s  ZeroDump

AddressDump:
        move.w 4(sp),v_err_ext1
        move.l 6(sp),v_err_addr
        move.w 10(sp),v_err_ext2
        move.w 12(sp),v_err_sr
        move.l 14(sp),v_err_pc
        bra.s  RegDump
IllegalDump:
        move.w 10(sp),v_err_ext1
ZeroDump:
        move.w 4(sp),v_err_sr
        move.l 6(sp),v_err_pc
RegDump:
        move.l d0,v_err_reg+0
        move.l d1,v_err_reg+4
        move.l d2,v_err_reg+8
        move.l d3,v_err_reg+12
        move.l d4,v_err_reg+16
        move.l d5,v_err_reg+20
        move.l d6,v_err_reg+24
        move.l d7,v_err_reg+28
        move.l a0,v_err_reg+32
        move.l a1,v_err_reg+36
        move.l a2,v_err_reg+40
        move.l a3,v_err_reg+44
        move.l a4,v_err_reg+48
        move.l a5,v_err_reg+52
        move.l a6,v_err_reg+56
        move.l a7,v_err_reg+60
        jmp    _error

/* Standard interrupts */

HBlank:
ExtInt:
        rte

VBlank:
		movem.l d0-d1/a0-a1,-(sp)

		/* Wait for DMA */
XGM_DMAWait:
        move.w 0xC00004,d0
        andi.w #2,d0
        bgt.s  XGM_DMAWait
		
		/* XGM VBlank Process */
		lea    0xA11100,%a0             /* a0 = Z80 Halt Port */
		lea    0xA11200,%a1             /* a1 = Z80 Reset Port */
		clr.w  %d1					    /* num = 0 */
		move.w xgmTempoCnt,%d0		    /* cnt = xgmTempoCnt */
		bgt.s  XGM_CntSkip				/* cnt > 0 */
XGM_CntLoop:
		addq.w #1,%d1				    /* num++ */
		add.w  xgmTempoDef,%d0		    /* cnt += xgmTempoDef */
		ble.s  XGM_CntLoop				/* cnt <= 0 */
XGM_CntSkip:
		sub.w  xgmTempo,%d0				/* xgmTempoCnt = cnt - xgmTempo */
		move.w %d0,xgmTempoCnt
XGM_WhileTRUE:
		move.w #0x0100,(%a0)			/* Take Z80 bus and reset */
		move.w #0x0100,(%a1)
XGM_WaitZ80Get:
		move.w (%a0),%d0				/* Wait for bus to be taken */
		btst   #8,%d0
		bne.s  XGM_WaitZ80Get
		move.b 0xA00112,%d0				/* if (!z80_drv_params[0x0E]) break */
		beq.s  XGM_VBlankEnd
		move.w #0,(%a0)					/* Release Z80 bus */
		movm.l %d0-%d3,-(%sp)		    /* Wait about 80 cycles */
		movm.l (%sp)+,%d0-%d3
XGM_WaitZ80Rel:
		move.w (%a0),%d0				/* Wait for bus to be released */
		btst   #8,%d0
		beq.s  XGM_WaitZ80Rel
		bra.s  XGM_WhileTRUE
XGM_VBlankEnd:
		move.b (0xA00113),%d0			/* z80_drv_params[0x0F] += num */
		add.b  %d0,%d1
		move.b %d1,(0xA00113)
		move.w #0,(%a0)					/* Release Z80 bus */
		
		movem.l (sp)+,d0-d1/a0-a1

		/* Set VBlank complete flag */
        move.b	#1,(vblank)
        rte
