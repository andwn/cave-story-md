/* 
 * Table & macros to help adjust acceleration/velocity/timer values for 60 FPS,
 * because (n * 60 / 50), IS_PALSYSTEM branching, etc is too expensive for the CPU
 */

// Acceleration, range 0x000-0x3FF
#define ACC(n) (ntsctab[n][ntsc_acc])

// Velocity, range 0x0000-0x3FF0, multiples of 0x10
#define VEL(n) (ntsctab[(n)>>4][ntsc_vel])

// Time, range 0x000-0x3FF
#define TIME(n) (ntsctab[n][ntsc_time])

#define CHECKNTSC() {                                                                          \
	ntsc_acc  = IS_PALSYSTEM ? 1 : 0;                                                          \
	ntsc_vel  = IS_PALSYSTEM ? 2 : 0;                                                          \
	ntsc_time = IS_PALSYSTEM ? 3 : 0;                                                          \
}

uint8_t ntsc_acc;
uint8_t ntsc_vel;
uint8_t ntsc_time;

extern const uint16_t ntsctab[0x400][4];
