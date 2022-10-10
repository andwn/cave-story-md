.section .text

#/* 0x200000 */
#    .org 0x200000

        .align 256
        .globl smp_null
        .globl TILE_BLANK
        .globl BLANK_DATA
smp_null:
TILE_BLANK:
BLANK_DATA:
    .rept 16
        dc.l    0, 0, 0, 0
    .endr

#    .ascii  "SEGA MEGA DRIVE "
#    .ascii  "                "
#    .ascii  "https://www.youtube.com/watch?v=y-kIvItmQMc     "
#    .ascii  "https://www.youtube.com/watch?v=HnyGSl3K-IE     "
#    .ascii  "GM 00001009-00"
#    dc.w    0
