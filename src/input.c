#include "input.h"

#include <genesis.h>

void input_init() {
	JOY_init();
	joystate = 0;
	oldstate = 0;
}

void input_update() {
	oldstate = joystate;
	joystate = JOY_readJoypad(JOY_1);
}
