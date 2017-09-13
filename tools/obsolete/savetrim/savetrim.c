/*
 * Strip padding at the end of SRAM
 * 
 * Compile:
 * gcc savetrim.c -o savetrim
 * 
 * Usage:
 * ./prof2sram <file1> [file2] [...]
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SRAM_SIZE 0x500

int main(int argc, char *argv[]) {
	for(int i = 1; i < argc; i++) {
		FILE *in = fopen(argv[i], "rb");
		if(!in) {
			printf("E: Couldn't open %s.\n", argv[i]);
			continue;
		}
		char outfn[256];
		sprintf(outfn, "%s-trim", argv[i]);
		FILE *out = fopen(outfn, "wb");
		if(!out) {
			printf("E: Couldn't create %s.\n", outfn);
			continue;
			fclose(in);
		}
		uint8_t sram[SRAM_SIZE];
		if(fread(sram, 1, SRAM_SIZE, in) < SRAM_SIZE) {
			printf("E: %s shorter than %d bytes.\n", argv[i], SRAM_SIZE);
		}
		fclose(in);
		if(fwrite(sram, 1, SRAM_SIZE, out) < SRAM_SIZE) {
			printf("E: Wrote less than %d bytes to %s.\n", SRAM_SIZE, outfn);
		}
		fclose(out);
	}
}
