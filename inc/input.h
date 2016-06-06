#ifndef INC_INPUT_H_
#define INC_INPUT_H_

#include "common.h"

// Will give >0 when true (do not use ==1, use >0)
#define joy_pressed(b) (((joystate&b)&&((~oldstate)&b)))
#define joy_released(b) ((((~joystate)&b)&&(oldstate&b)))
#define joy_down(b) ((joystate&b))

// Current and previous states, so the moment a button is pressed or released can be detected
u16 joystate, oldstate;

// Just sets the 2 states to 0
void input_init();

// Updates joystate to current button presses
void input_update();

#endif // INC_INPUT_H_
