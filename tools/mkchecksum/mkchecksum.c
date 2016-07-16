#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("No ROM file specified.\n");
		return EXIT_SUCCESS;
	}
	FILE *rom = fopen(argv[1], "r+b");
	if(rom == NULL) {
		printf("Failed to open %s.\n", argv[1]);
		return EXIT_FAILURE;
	}
	unsigned char buf[4];
	unsigned char chk[4] = { 0, 0, 0, 0 };
	fseek(rom, 0x200, SEEK_SET);
	for(int i = 0; i < (0x100000 - 0x200) >> 2; i++) {
		// Big endian
		fread(buf, 1, 4, rom);
		chk[0] ^= buf[0];
		chk[1] ^= buf[1];
		chk[2] ^= buf[2];
		chk[3] ^= buf[3];
	}
	chk[0] ^= chk[2];
	chk[1] ^= chk[3];
	printf("Checksum: %hx%hx\n", chk[0], chk[1]);
	// Write checksum to ROM header
	fseek(rom, 0x18E, SEEK_SET);
	fwrite(chk, 1, 2, rom);
	fclose(rom);
	return EXIT_SUCCESS;
}
