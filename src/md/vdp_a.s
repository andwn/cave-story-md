    .include "macros.i"

/* VDP Ports*/
EQU VdpCtrl,       0xC00004  /* VDP control port */
EQU VdpData,       0xC00000  /* VDP data port */
EQU HvCounter,     0xC00008  /* H/V counter */

/* VDP Registers */
EQU VDPREG_MODE1,  0x8000  /* Mode register #1 */
EQU VDPREG_MODE2,  0x8100  /* Mode register #2 */
EQU VDPREG_MODE3,  0x8B00  /* Mode register #3 */
EQU VDPREG_MODE4,  0x8C00  /* Mode register #4 */

EQU VDPREG_PLANEA, 0x8200  /* Plane A table address */
EQU VDPREG_PLANEB, 0x8400  /* Plane B table address */
EQU VDPREG_SPRITE, 0x8500  /* Sprite table address */
EQU VDPREG_WINDOW, 0x8300  /* Window table address */
EQU VDPREG_HSCROLL,0x8D00  /* HScroll table address */

EQU VDPREG_SIZE,   0x9000  /* Plane A and B size */
EQU VDPREG_WINX,   0x9100  /* Window X split position */
EQU VDPREG_WINY,   0x9200  /* Window Y split position */
EQU VDPREG_INCR,   0x8F00  /* Autoincrement */
EQU VDPREG_BGCOL,  0x8700  /* Background color */
EQU VDPREG_HRATE,  0x8A00  /* HBlank interrupt rate */

/* VDP FIFO Commands */
EQU VRAM_ADDR_CMD,     0x40000000
EQU CRAM_ADDR_CMD,     0xC0000000
EQU VSRAM_ADDR_CMD,    0x40000010

/* Screen boundaries */
EQU ScreenWidth,    320
EQU ScreenHalfW,    160

    .section .bss

VAR ScreenHeight,   w, 1
VAR ScreenHalfH,    w, 1
#VAR FPS,            b, 1
VAR pal_mode,       b, 1

    .section .text

#FUNC vdp_init_a
#        lea     (VdpCtrl),a0
#        lea     (VdpData),a1
#        move.w  #0x8174,d0      /* Mode set command */
#        move.w  #224,(ScreenHeight)
#        move.w  #112,(ScreenHalfH)
#        move.b  #60,(FPS)
#        move.b  (a0),(PalMode)
#        and.b   #1,(PalMode)
#        beq.s   vdp_init_notpal
#        add.w   #16,(ScreenHeight)
#        addq.w  #8,(ScreenHalfH)
#        sub.b   #10,(FPS)
#        or.w    #8,d0           /* 30H flag */
#    vdp_init_notpal:
#        move.w  #32,(SpriteYMax)
#        add.w   (ScreenHeight),(SpriteYMax)
#        /* Set the wacky regies */
#        move.w  #0x8004,(a0)
#        move.w  d0,(a0)
#        move.w  #0x8200 | (VDP_PLANE_A >> 10),(a0)
#        move.w  #0x8300 | (VDP_PLANE_W >> 10),(a0)
#        move.w  #0x8400 | (VDP_PLANE_B >> 13),(a0)
#        move.w  #0x8500 | (VDP_SPRITE_TABLE >> 9),(a0)
#        move.w  #0x8600,(a0)
#        move.w  #0x8700,(a0)
#        move.w  #0x8800,(a0)
#        move.w  #0x8900,(a0)
#        move.w  #0x8A01,(a0)
#        move.w  #0x8B00 | (VSCROLL_PLANE << 2) | HSCROLL_PLANE,(a0)
#        move.w  #0x8C81,(a0)
#        move.w  #0x8D00 | (VDP_HSCROLL_TABLE >> 10),(a0)
#        move.w  #0x8E00,(a0)
#        move.w  #0x8F02,(a0)
#        move.w  #0x9001,(a0)
#        move.w  #0x9100,(a0)
#        move.w  #0x9200,(a0)
#        jsr     dma_clear
#        move.w  #VDP_PLANE_A,-(sp)
#        jsr     vdp_map_clear
#        move.w  #0,-(sp)
#        jsr     vdp_hscroll
#        jsr     vdp_vscroll
#        addq.w  #2,sp
#        move.w  #VDP_PLANE_B,(sp)
#        jsr     vdp_map_clear
#        move.w  #0,-(sp)
#        jsr     vdp_hscroll
#        jsr     vdp_vscroll
#        addq.w  #4,sp
#        jsr     vdp_sprites_clear
#        jsr     vdp_sprites_update

#FUNC vdp_vsync
#        clr.b   (vblank)
#    vdp_vsync_wait:
#        tst.b   (vblank)
#        beq.s   vdp_vsync_wait
#        rts
