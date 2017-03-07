#include "common.h"
#include "joy.h"

#include "input.h"

void input_init() {
	joystate = 0;
	oldstate = 0;
	controllerType = JOY_getJoypadType(JOY_1);
}
