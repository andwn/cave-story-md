#include <math.h>
#include <stdio.h>

// Rather than truncating the values we want them rounded
// otherwise low values (like less than 30) will be really off

#define TABLE_SIZE	0x80
#define COLUMNS		8

int main() {
	printf("#include \"stdint.h\"\n\n");
	printf("uint8_t time_tab_ntsc[0x%x] = {\n", TABLE_SIZE);
	for(int i = 0; i < TABLE_SIZE; i += 8) {
		printf("\t");
		for(int j = i; j < i + 8; j++) {
			double time = ((double)j) * 6.0 / 5.0;
			printf("0x%02x, ", (int)(round(time)));
		}
		printf("\n");
	}
	printf("};\n\n");
	printf("uint8_t speed_tab_ntsc[0x%x] = {\n", TABLE_SIZE);
	for(int i = 0; i < TABLE_SIZE; i += 8) {
		printf("\t");
		for(int j = i; j < i + 8; j++) {
			double speed = ((double)j) * 5.0 / 6.0;
			printf("0x%02x, ", (int)(round(speed)));
		}
		printf("\n");
	}
	printf("};\n\n");
	return 0;
}
