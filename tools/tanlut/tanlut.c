/* 
 * Tangent lookup table
 * 
 * Based on the way NXEngine did it, see:
 * https://github.com/libretro/nxengine-libretro/blob/master/nxengine/trig.cpp
 */

#include <stdio.h>
#include <math.h>

#define TABLE_SIZE 32

#define PIBT ((360.00f / (float)(TABLE_SIZE * 4)) * (3.14159265f / 180.00f))

int main() {
	printf("const u32 tan_table[%u] = {\n", TABLE_SIZE);
	for(int i = 0; i < TABLE_SIZE; i++) {
		printf("\t%d,\n", (int)(tan((double)i * PIBT) * (1 << 13)));
	}
	printf("};\n");
}
