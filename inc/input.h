#ifndef INC_INPUT_H_
#define INC_INPUT_H_

#include "common.h"

// Ay yo here a bunch of parentheses thanks eclipse
#define joy_pressed(b) (((joystate&b)&&((~oldstate)&b)))
#define joy_released(b) ((((~joystate)&b)&&(oldstate&b)))
#define joy_down(b) ((joystate&b))

u16 joystate, oldstate;

void input_init();
void input_update();

#endif // INC_INPUT_H_
