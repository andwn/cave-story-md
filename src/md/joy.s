    .include "macros.i"

/* IO */
EQU IoCtrl1,    0xA10009   /* 1P control port */
EQU IoCtrl2,    0xA1000B   /* 2P control port */
EQU IoData1,    0xA10003   /* 1P data port */
EQU IoData2,    0xA10005   /* 2P data port */

    .section .bss

    .align 2
VAR joystate,       w, 1
VAR joystate_old,   w, 1
VAR joytype,        b, 1

    .section .text

FUNC joy_init
        DisableInts
        FastPauseZ80
        move.b  #0x40,(IoCtrl1)
        move.b  #0x40,(IoCtrl2)
        move.b  #0x40,(IoData1)
        move.b  #0x40,(IoData2)
        ResumeZ80
        EnableInts
        rts

FUNC joy_update
        move.b  #0,(joytype)
        DisableInts
        FastPauseZ80

        move.l  #IoData1,a0
        move.b  #0x40,(a0)  /* Step 1: **CB.RLDU */
        move.w  (joystate),(joystate_old)
        move.b  (a0),d0

        move.b  #0x00,(a0)  /* Step 2: **SA.00DU */
        and.b   #0x3F,d0
        move.b  (a0),d1

        move.b  #0x40,(a0)  /* Step 3: Ignore */
        and.b   #0x30,d1    /* Rearrange bits */

        move.b  #0x00,(a0)  /* Step 4: Ignore */
        lsl.b   #2,d1       /* into SACB.RLDU */

        move.b  #0x40,(a0)  /* Step 5: Ignore */
        or.b    d1,d0

        move.b  #0x00,(a0)  /* Step 6: Last 4 bits are 0 if 6 buttons */
        not.b   d0
        move.b  (a0),d1

        and.b   #0x0F,d1
        bne.s   joy_update_3btn

        move.b  #0x40,(a0)  /* Step 7: *CBS.MXYZ */
        move.b  #1,(joytype)
        move.b  (a0),d1

        not.b   d1
        and.b   #0x0F,d1
        move.b  d1,(joystate)

    joy_update_3btn:
        ResumeZ80
        EnableInts
        /* MXYZ.SACB.RLDU */
        move.b  d0,(joystate+1)
        rts
