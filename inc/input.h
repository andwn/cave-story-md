#define joy_pressed(b) (((joystate&(b))&&((~oldstate)&(b))))
#define joy_released(b) ((((~joystate)&(b))&&(oldstate&(b))))
#define joy_down(b) ((joystate&(b)))

#define input_update() {                                                                       \
	oldstate = joystate;                                                                       \
	joystate = JOY_readJoypad(JOY_1);                                                          \
}

// Current and previous states, so the moment a button is pressed or released can be detected
uint16_t joystate, oldstate;

uint8_t controllerType;

// Just sets the 2 states to 0
void input_init();
