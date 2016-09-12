#include "input.h"

#include <genesis.h>

void input_init() {
	JOY_init();
	joystate = 0;
	oldstate = 0;
}
