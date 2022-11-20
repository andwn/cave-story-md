/* cc tools/patchrom/patchrom.c -o bin/patchrom */
/* patchrom <source rom> <patch file> <output rom> */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint8_t rom[0x400000]; // 4MB

int main(int argc, char *argv[]) {
	if(argc != 4) {
		printf("Usage: patchrom <source rom> <patch file> <output rom>\n");
		return 1;
	}
	// Slurp the source ROM into the rom array
	FILE *source = fopen(argv[1], "rb");
	if(!source) {
		printf("Failed to read source ROM '%s'.\n", argv[1]);
		return 1;
	}
	fseek(source, 0, SEEK_END);
	int rom_size = ftell(source);
	fseek(source, 0, SEEK_SET);
	fread(rom, 1, rom_size, source);
	fclose(source);
	// Find the address of "LANGDAT\0" in ROM
	int address = 0;
	for(int i = 0x300000 /* 3.0MB */; i < 0x400000-8; i++) {
		if(strncmp("LANGDAT", (char*)&rom[i], 8) == 0) {
			address = i;
			break;
		}
	}
	if(!address) {
		printf("ERROR: Unable to find LANGDAT location in source ROM.\n");
		return 1;
	}
	// Load up the patch file and inject it at the LANGDAT address
	FILE *patch = fopen(argv[2], "rb");
	if(!patch) {
		printf("Failed to read patch file '%s'.\n", argv[2]);
		return 1;
	}
	fseek(patch, 0, SEEK_END);
	int patch_size = ftell(patch);
	fseek(patch, 0, SEEK_SET);
	fread(&rom[address], 1, patch_size, patch);
	fclose(patch);
	// Need to relocate the addresses in the pointer table by adding the start address
    // of the patch to them
	for(int i = address + 12; i < 0x400000 - 4; i += 4) {
		// The pointers are big endian, so gotta do this
		//uint32_t p[4] = { rom[i+3], rom[i+2], rom[i+1], rom[i+0] };
		//printf("%02x%02x%02x%02x\n", p[0], p[1], p[2], p[3]);
		uint32_t ptr = rom[i+3] | (rom[i+2]<<8) | (rom[i+1]<<16) | (rom[i]<<24);
        if(ptr == 0xFFFFFFFF) {
            break; // End of pointer table
        }
		//printf("%u -> ", ptr);
		ptr += address;
		//printf("%u\n", ptr);
		rom[i+3] = ptr & 0xFF;
		rom[i+2] = (ptr>>8) & 0xFF;
		rom[i+1] = (ptr>>16) & 0xFF;
		rom[i+0] = (ptr>>24);
	}
	// Finally, save the output ROM
	FILE *out = fopen(argv[3], "wb");
	if(!out) {
		printf("Failed to read source ROM '%s'.\n", argv[3]);
		return 1;
	}
	fwrite(rom, 1, rom_size, out);
	fclose(out);
	return 0;
}
