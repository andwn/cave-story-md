#ifndef INC_INPUT_H_
#define INC_INPUT_H_

#include "common.h"

#define joy_pressed(b) (((joystate&b)&&((~oldstate)&b)))
#define joy_released(b) ((((~joystate)&b)&&(oldstate&b)))
#define joy_down(b) ((joystate&b))

#define input_update() {                                                                       \
	oldstate = joystate;                                                                       \
	joystate = JOY_readJoypad(JOY_1);                                                          \
}

// Current and previous states, so the moment a button is pressed or released can be detected
u16 joystate, oldstate;

u8 controllerType;

// Just sets the 2 states to 0
void input_init();

#endif // INC_INPUT_H_
