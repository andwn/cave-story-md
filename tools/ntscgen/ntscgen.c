#include <math.h>
#include <stdio.h>

// Rather than truncating the values we want them rounded
// otherwise low values (like less than 30) will be really off

#define TABLE_SIZE 0x400

int main() {
	for(int i = 0; i < TABLE_SIZE; i++) {
		double acc = ((double)i) * 50 / 60;
		double vel = ((double)(i << 4)) * 50 / 60;
		double time = ((double)i) * 60 / 50;
		printf("\t");
		printf("0x%04x, ", i);
		printf("0x%04x, ", (int)(round(acc)));
		printf("0x%04x, ", (int)(round(vel)));
		printf("0x%04x, ", (int)(round(time)));
		printf("\n");
	}
	return 0;
}
