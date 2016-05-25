#ifndef VDP_EXT_H_
#define VDP_EXT_H_

#include <genesis.h>

#define PUT_TILE(d, s) d[0]=s[0];d[1]=s[1];d[2]=s[2];d[3]=s[3];

// Window plane functions
void VDP_setWindowPos(u8 x, u8 y);
u8 VDP_getWindowPosX();
u8 VDP_getWindowPosY();
void VDP_drawTextWindow(const char *str, u16 x, u16 y);
void VDP_clearTextWindow(u16 x, u16 y, u16 w);

#endif
