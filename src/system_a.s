    .align  2
    .globl	SYS_assertReset
    .type   SYS_assertReset, @function
SYS_assertReset:

    reset
    rts


    .align  2
    .globl 	SYS_hardReset
    .type   SYS_hardReset, @function
SYS_hardReset:

    move   #0x2700,%sr
    move.l (0),%a7
    jmp    _hard_reset
