/*
 * kanjicount - Counts the number of unique kanji used in TSC files
 * 
 * Compile:
 * gcc kanjicount.c -o kanjicount
 * 
 * Usage:
 * ./kanjicount <tsc file> [more tsc files ...]
 * 
 * Make sure you open the output txt files as Microsoft Code Page 932
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t *tsc = NULL;
int tscSize = 0;

uint16_t kanji[6000] = {};
int count = 0;

FILE *out = NULL;
FILE *debug = NULL;

void tsc_open(const char *filename) {
	FILE *tscFile = fopen(filename, "rb");
	fseek(tscFile, 0, SEEK_END);
	tscSize = ftell(tscFile);
	tsc = malloc(tscSize);
	fseek(tscFile, 0, SEEK_SET);
	fread(tsc, 1, tscSize, tscFile);
	// Obtain the key from the center of the file
	uint8_t key = tsc[tscSize / 2];
	// Apply key to all bytes except where the key itself was
	for(int i = 0; i < tscSize; i++) {
		if(i != tscSize / 2) tsc[i] -= key;
		fputc(tsc[i], debug);
	}
	fclose(tscFile);
}

void tsc_close() {
	if(tsc) {
		free(tsc);
		tsc = NULL;
	}
}

void tsc_read() {
	for(int i = 0; i < tscSize; i++) {
		// Check if this is a double byte char
		if((tsc[i] >= 0x81 && tsc[i] <= 0x9F) || (tsc[i] >= 0xE0 && tsc[i] <= 0xEF)) {
			// Check if the double byte represents a kanji
			if(tsc[i] >= 0x88 && tsc[i] <= 0xEE) {
				uint16_t wc = (tsc[i] << 8) + tsc[i+1];
				// Ok, it's a kanji, compare against the list
				bool alreadyCounted = false;
				for(int k = 0; k < count; k++) {
					if(kanji[k] == wc) {
						alreadyCounted = true;
						break;
					}
				}
				// If it wasn't in the list yet, it's unique
				if(!alreadyCounted) {
					fprintf(out, "Kanji: ");
					
					uint8_t b = wc >> 8; // First byte
					fwrite(&b, 1, 1, out);
					b = wc & 0xFF; // Second byte
					fwrite(&b, 1, 1, out);
					
					fprintf(out, " (0x%hx)\n", wc);
					kanji[count] = wc;
					count++;
				}
			}
			i++; // Increment i a second time, since this char was 2 bytes
		}
	}
}

int main(int argc, char *argv[]) {
	if(argc <= 1) {
		printf("Not enough args.\n");
		return EXIT_SUCCESS;
	}
	
	out = fopen("out.txt", "wb");
	debug = fopen("debug.txt", "wb");
	
	for(int i = 1; i < argc; i++) {
		tsc_open(argv[i]);
		tsc_read();
		tsc_close();
	}
	
	fclose(out);
	fclose(debug);
	
	printf("Total: %d\n", count);
	return EXIT_SUCCESS;
}
