.section .data

	.globl vblank
		
vblank:	ds.b 1

.section .text.keepboot

*-------------------------------------------------------
*
*       Sega startup code for the GNU Assembler
*       Translated from:
*       Sega startup code for the Sozobon C compiler
*       Written by Paul W. Lee
*       Modified by Charles Coty
*       Modified by Stephane Dallongeville
*
*-------------------------------------------------------

    .globl 	_hard_reset

    .org    0x00000000

_Start_Of_Rom:
_Vecteurs_68K:
        dc.l    0x00000000              /* Stack address */
        dc.l    _Entry_Point            /* Program start address */
        dc.l    _Bus_Error
        dc.l    _Address_Error
        dc.l    _Illegal_Instruction
        dc.l    _Zero_Divide
        dc.l    _Chk_Instruction
        dc.l    _Trapv_Instruction
        dc.l    _Privilege_Violation
        dc.l    _Trace
        dc.l    _Line_1010_Emulation
        dc.l    _Line_1111_Emulation
        dc.l     _Error_Exception, _Error_Exception, _Error_Exception, _Error_Exception
        dc.l     _Error_Exception, _Error_Exception, _Error_Exception, _Error_Exception
        dc.l     _Error_Exception, _Error_Exception, _Error_Exception, _Error_Exception
        dc.l    _Error_Exception, _INT, _EXTINT, _INT
        dc.l    _HINT
        dc.l    _INT
        dc.l    _VINT
        dc.l    _INT
        dc.l    _INT,_INT,_INT,_INT,_INT,_INT,_INT,_INT
        dc.l    _INT,_INT,_INT,_INT,_INT,_INT,_INT,_INT
        dc.l    _INT,_INT,_INT,_INT,_INT,_INT,_INT,_INT
        dc.l    _INT,_INT,_INT,_INT,_INT,_INT,_INT,_INT

        .incbin "src/boot/rom_head.bin", 0x10, 0x100

_Entry_Point:
        move    #0x2700,%sr
        tst.l   0xa10008
        bne.s   SkipJoyDetect
        tst.w   0xa1000c
SkipJoyDetect:
        lea     Table,%a5
        movem.w (%a5)+,%d5-%d7
        movem.l (%a5)+,%a0-%a4
* Check Version Number
        move.b  -0x10ff(%a1),%d0
        andi.b  #0x0f,%d0
        beq.s   WrongVersion
* Sega Security Code (SEGA)
        move.l  #0x53454741,0x2f00(%a1)
WrongVersion:
        move.w  (%a4),%d0
        moveq   #0x00,%d0
        movea.l %d0,%a6
        move    %a6,%usp
        move.w  %d7,(%a1)
        move.w  %d7,(%a2)
        jmp     _hard_reset

Table:
        dc.w    0x8000,0x3fff,0x0100
        dc.l    0xA00000,0xA11100,0xA11200,0xC00000,0xC00004

_hard_reset:
* clear Genesis RAM
        lea     0xff0000,%a0
        moveq   #0,%d0
        move.w  #0x3FFF,%d1

ClearRam:
        move.l  %d0,(%a0)+
        dbra    %d1,ClearRam

* copy initialized variables from ROM to Work RAM
        lea     _stext,%a0
        lea     0xFF0000,%a1
        move.l  #_sdata,%d0
        lsr.l   #1,%d0
        beq     NoCopy

        subq.w  #1,%d0
CopyVar:
        move.w  (%a0)+,(%a1)+
        dbra    %d0,CopyVar

NoCopy:

* Jump to initialisation process...

        jmp     _start_entry


*------------------------------------------------
*
*       interrupt functions
*
*------------------------------------------------

registersDump:
        move.l %d0,registerState+0
        move.l %d1,registerState+4
        move.l %d2,registerState+8
        move.l %d3,registerState+12
        move.l %d4,registerState+16
        move.l %d5,registerState+20
        move.l %d6,registerState+24
        move.l %d7,registerState+28
        move.l %a0,registerState+32
        move.l %a1,registerState+36
        move.l %a2,registerState+40
        move.l %a3,registerState+44
        move.l %a4,registerState+48
        move.l %a5,registerState+52
        move.l %a6,registerState+56
        move.l %a7,registerState+60
        rts

busAddressErrorDump:
        move.w 4(%sp),ext1State
        move.l 6(%sp),addrState
        move.w 10(%sp),ext2State
        move.w 12(%sp),srState
        move.l 14(%sp),pcState
        jmp registersDump

exception4WDump:
        move.w 4(%sp),srState
        move.l 6(%sp),pcState
        move.w 10(%sp),ext1State
        jmp registersDump

exceptionDump:
        move.w 4(%sp),srState
        move.l 6(%sp),pcState
        jmp registersDump


_Bus_Error:
        jsr busAddressErrorDump
        movem.l %d0-%d1/%a0-%a1,-(%sp)
        jsr    _bus_error_cb
        movem.l (%sp)+,%d0-%d1/%a0-%a1
        rte

_Address_Error:
        jsr busAddressErrorDump
        movem.l %d0-%d1/%a0-%a1,-(%sp)
        jsr    _address_error_cb
        movem.l (%sp)+,%d0-%d1/%a0-%a1
        rte

_Illegal_Instruction:
        jsr exception4WDump
        movem.l %d0-%d1/%a0-%a1,-(%sp)
        jsr    _illegal_instruction_cb
        movem.l (%sp)+,%d0-%d1/%a0-%a1
        rte

_Zero_Divide:
        jsr exceptionDump
        movem.l %d0-%d1/%a0-%a1,-(%sp)
        jsr    _zero_divide_cb
        movem.l (%sp)+,%d0-%d1/%a0-%a1
        rte

_Chk_Instruction:
        jsr exception4WDump
        movem.l %d0-%d1/%a0-%a1,-(%sp)
        jsr    _chk_instruction_cb
        movem.l (%sp)+,%d0-%d1/%a0-%a1
        rte

_Trapv_Instruction:
        jsr exception4WDump
        movem.l %d0-%d1/%a0-%a1,-(%sp)
        jsr    _trapv_instruction_cb
        movem.l (%sp)+,%d0-%d1/%a0-%a1
        rte

_Privilege_Violation:
        jsr exceptionDump
        movem.l %d0-%d1/%a0-%a1,-(%sp)
        jsr    _privilege_violation_cb
        movem.l (%sp)+,%d0-%d1/%a0-%a1
        rte

_Trace:
        jsr exceptionDump
        movem.l %d0-%d1/%a0-%a1,-(%sp)
        jsr    _trace_cb
        movem.l (%sp)+,%d0-%d1/%a0-%a1
        rte

_Error_Exception:
        jsr exceptionDump
        movem.l %d0-%d1/%a0-%a1,-(%sp)
        jsr    _exception_cb
        movem.l (%sp)+,%d0-%d1/%a0-%a1
        rte

_Line_1010_Emulation:
_Line_1111_Emulation:
_INT:
_HINT:
_EXTINT:
        rte

_VINT:
        move.b	#1,(vblank)
        rte
