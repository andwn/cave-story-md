#ifndef MD_JOY_H
#define MD_JOY_H

#include "types.h"

#define JOY_UP       0x0001
#define JOY_DOWN     0x0002
#define JOY_LEFT     0x0004
#define JOY_RIGHT    0x0008
#define JOY_B        0x0010
#define JOY_C        0x0020
#define JOY_A        0x0040
#define JOY_START    0x0080
#define JOY_Z        0x0100
#define JOY_Y        0x0200
#define JOY_X        0x0400
#define JOY_MODE     0x0800

#define JOY_ANYDIR      0x000F
#define JOY_ANYBTN      0x0FF0

#define JOY_TYPE_PAD3           0x00
#define JOY_TYPE_PAD6           0x01

extern const uint16_t btn[12];
extern const char btnName[12][4];

#define joy_pressed(b) (((joystate&(b))&&((~joystate_old)&(b))))
#define joy_released(b) ((((~joystate)&(b))&&(oldstate&(b))))
#define joy_down(b) ((joystate&(b)))

extern uint8_t joytype;
extern uint16_t joystate, joystate_old;

void joy_init();
void joy_update();

#endif //MD_JOY_H
