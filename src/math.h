#ifndef MD_MATH_H
#define MD_MATH_H

#include "md/types.h"

#define floor(x) ((x)&~0x1FF)
#define round(x) (((x)+0x100)&~0x1FF)
#define ceil(x)  (((x)+0x1FF)&~0x1FF)

#define min(X, Y)   (((X) < (Y))?(X):(Y))
#define max(X, Y)   (((X) > (Y))?(X):(Y))
#define abs(X)      (((X) < 0)?-(X):(X))

// Div/mod tables to help math when displaying digits
extern const uint8_t div10[0x400];
extern const uint8_t mod10[0x400];

// Angles
#define A_RIGHT	0x00
#define A_DOWN	0x40
#define A_LEFT	0x80
#define A_UP	0xC0

// Sine & cosine lookup tables
extern const int16_t sin[0x100];
extern const int16_t cos[0x100];
// Above tables but every value multiplied by 1.5, quick reference:
// <<1 == *3, <<2 == *6, <<3 == *12, <<4 == *24, <<5 == *48
extern const int16_t sin2[0x100];
extern const int16_t cos2[0x100];

// Get angle between two points using arctan table
uint8_t get_angle(int32_t curx, int32_t cury, int32_t tgtx, int32_t tgty);

#endif //MD_MATH_H
