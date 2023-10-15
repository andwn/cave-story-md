/* C compatible variables & functions */
.macro VAR _name, _size, _num
    .globl \_name
\_name: ds.\_size \_num
.endm

.macro EQU _name, _val
    .globl \_name
.equ \_name, \_val
.endm

.macro FUNC _name, _align=2
    .section .text.\_name
    .globl \_name
    .type  \_name, @function
    .align \_align
\_name:
.endm

/* For resources */
.macro BIN _name, _file, _align=2
    .globl \_name
    .align \_align
\_name:
    .incbin "\_file"
.endm

.macro PCM _name, _file, _align=256
    .globl \_name
    .globl \_name\()_end
    .align \_align
\_name:
    .incbin "\_file"
\_name\()_end:
.endm

.macro PTR _name, _dest, _align=2
    .globl  \_name
    .align  \_align
\_name:
    dc.l    \_dest
.endm

.macro SPRITE _name, _file, _fwidth, _fheight, _nframes, _align=2
    .globl  \_name
    .align  \_align
\_name:
    dc.w    \_fwidth, \_fheight, \_nframes, \_fwidth * \_fheight
    .incbin "\_file"
\_name\()_end:
.endm

/* Interrupts */
.macro DisableInts
        move #0x2700,sr
.endm

.macro EnableInts
        move #0x2300,sr
.endm

/* Z80 */
.macro PauseZ80
        move.w  #0x100,(Z80BusReq)
1:
        btst    #0,(Z80BusReq)
        bne.s   1b
.endm

.macro FastPauseZ80
        move.w  #0x100,(Z80BusReq)
.endm

.macro ResumeZ80
        move.w  #0x000,(Z80BusReq)
.endm
