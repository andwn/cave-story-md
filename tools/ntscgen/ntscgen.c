#include <math.h>
#include <stdio.h>

#define TABLE_SIZE	0x400
#define COLUMNS		8

int truncate(double x) {
	return (x - (int)x) < 0.75 ? (int)x : (int)(x + 1);
}

int main() {
	printf("#include <stdint.h>\n\n");
	// NTSC Time/Speeds
	printf("const uint16_t time_tab_ntsc[0x%X] = {\n", TABLE_SIZE);
	for(int i = 0; i < TABLE_SIZE; i += 8) {
		printf("\t");
		for(int j = i; j < i + 8; j++) {
			double time = ((double)j) * 6.0 / 5.0;
			printf("0x%04x, ", truncate(time));
		}
		printf("\n");
	}
	printf("};\n\n");
	printf("const uint16_t speed_tab_ntsc[0x%X] = {\n", TABLE_SIZE);
	for(int i = 0; i < TABLE_SIZE; i += 8) {
		printf("\t");
		for(int j = i; j < i + 8; j++) {
			double speed = ((double)j) * 5.0 / 6.0;
			printf("0x%04x, ", truncate(speed));
		}
		printf("\n");
	}
	printf("};\n\n");
	// PAL Time/Speeds (same as index)
	printf("const uint16_t time_tab_pal[0x%X] = {\n", TABLE_SIZE);
	for(int i = 0; i < TABLE_SIZE; i += 8) {
		printf("\t");
		for(int j = i; j < i + 8; j++) printf("0x%04X, ", j);
		printf("\n");
	}
	printf("};\n\n");
	printf("const uint16_t speed_tab_pal[0x%X] = {\n", TABLE_SIZE);
	for(int i = 0; i < TABLE_SIZE; i += 8) {
		printf("\t");
		for(int j = i; j < i + 8; j++) printf("0x%04x, ", j);
		printf("\n");
	}
	printf("};\n\n");
	return 0;
}
