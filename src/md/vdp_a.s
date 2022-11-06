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
VAR pal_mode,       b, 1
#VAR FPS,            b, 1

    .section .text

