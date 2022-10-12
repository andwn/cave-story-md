/*
 * kanjistr - Converts a newline separated list of Shift JIS strings into CSMD format
 * 
 * Compile:
 * gcc kanjistr.c -o kanjistr
 * 
 * Usage:
 * ./kanjistr <kanji list file> <string file> <out file> [linelen]
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Chars starting with 0xE0-0xFE are a 2 byte sequence
#define MULTIBYTE_BEGIN	0xE0
#define MULTIBYTE_END	0xFE

enum {
	CT_ASCII,
	CT_KANJI,
	CT_SKIP
};

const char *ValidChars = 
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz ,.?=!@$%^&*()[]{}|_-+:;'\"\n";

uint16_t *kanji = NULL;
uint16_t kanjiCount = 0;

uint8_t get_char_type(uint8_t c) {
	// First check the valid ASCII chars list
	for(uint16_t i = 0; i < strlen(ValidChars); i++) {
		if(c == ValidChars[i]) return CT_ASCII;
	}
	// Double byte char?
	if((c >= 0x81 /*&& c <= 0x9F) || (c >= 0xE0 */&& c <= 0xFE)) {
		return CT_KANJI;
	}
	return CT_SKIP;
}

int main(int argc,char *argv[]) {
	if(argc < 4) {
		printf("Usage: kanjistr <kanji list file> <string file> <out file> [linelen]\n");
		return EXIT_SUCCESS;
	}
	
	// Load the kanji list
	FILE *kfile = fopen(argv[1], "rb");
	if(!kfile) {
		printf("ERROR: Failed to open '%s'.\n", argv[1]);
		return EXIT_FAILURE;
	}
	fseek(kfile, 0, SEEK_END);
	kanjiCount = ftell(kfile) / 2; // Each iteration is 2 bytes
	fseek(kfile, 0, SEEK_SET);
	kanji = malloc(kanjiCount * 2);
	fread(kanji, 1, kanjiCount * 2, kfile);
	fclose(kfile);
	// Endianness is a bitch
	for(uint16_t i = 0; i < kanjiCount; i++) {
		uint8_t hi = kanji[i] >> 8;
		uint8_t lo = kanji[i] & 0xFF;
		kanji[i] = (lo << 8) | hi;
	}
	printf("INFO: Loaded kanji list.\n");
	
	// Do the thing
	FILE *strfile = fopen(argv[2], "rb");
	FILE *outfile = fopen(argv[3], "wb");
	
	uint16_t line = 1;
	uint8_t linelen = 16, linex = 0;
	if(argc > 4) linelen = atoi(argv[4]);
	
	while(!feof(strfile)) {
		uint8_t c;
		fread(&c, 1, 1, strfile);
		uint8_t ct = get_char_type(c);
		switch(ct) {
			case CT_ASCII:
			if(c == '\n') {
				// End of line
				c = '\0';
				while(linex < linelen) {
					fwrite(&c, 1, 1, outfile);
					linex++;
				}
				linex = 0;
				line++;
			} else {
				fwrite(&c, 1, 1, outfile);
				linex++;
			}
			break;
			case CT_KANJI: {
				uint8_t c2;
				fread(&c2, 1, 1, strfile);
				uint16_t wc = (c << 8) | c2;
				uint16_t k;
				for(k = 0; k < kanjiCount; k++) {
					if(wc == kanji[k]) break;
				}
				if(k == kanjiCount) {
					printf("WARN: %hu,%hhu Unknown kanji: 0x%04hx\n", line, linex, wc);
					c = '?';
					fwrite(&c, 1, 1, outfile);
					linex++;
				} else {
					// Index by appearance in the list, and fit that number into banks
					// of 0x60 for each 'page'
					uint8_t page = k / 0x60;
					uint8_t word = k % 0x60;
					uint8_t b = MULTIBYTE_BEGIN + page; // First byte
					fwrite(&b, 1, 1, outfile);
					linex++;
					b = word + 0x20; // Second byte
					fwrite(&b, 1, 1, outfile);
					linex++;
				}
			}
			break;
		}
		if(linex > linelen) {
			printf("WARN: Line %hu is %hhu, longer than max %hhu\n", line, linex, linelen);
		}
	}
	
	fclose(outfile);
	fclose(strfile);
	
	free(kanji);
	
	return EXIT_SUCCESS;
}
