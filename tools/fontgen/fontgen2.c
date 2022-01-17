/**
 * fontgen - Generates an image based on a kanji list and ttf font
 * 
 * Compile:
 * cc fontgen.c -o fontgen
 * 
 * Usage:
 * ./fontgen <kanji list file> <output file>
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define min(a,b) ((a)>(b)?(b):(a))

uint8_t *kanji; // Increase if need more than 8KB
int kanjiSize;

int utf8_to_index(uint8_t *str, int *bytes) {
    int index = 0;
    if (str[0] < 0x80) {
        // U+0000
        *bytes = 1;
        index = str[0];
    } else if (str[0] < 0xE0) {
        // U+0080
        *bytes = 2;
        index = /*0x80 +*/ ((str[0] & 0x1F) << 6) + (str[1] & 0x3F);
    } else if (str[0] < 0xF0) {
        // U+0800
        *bytes = 3;
        index = /*0x800 +*/ ((str[0] & 0x0F) << 12) + ((str[1] & 0x3F) << 6) + (str[2] & 0x3F);
    } else {
        // U+10000 (Not implemented lol)
        *bytes = 4;
    }
    if(index == 0xFF5E || index == 0xFF0D) index = 19968; // Weird out of bounds dashes
    //if(index >= 0x100 + 0x1000) index -= 0x800;
    return index;
}

void load_kanji(const char *filename) {
	FILE *kfile = fopen(filename, "rb");
	if(!kfile) {
		printf("ERROR: Failed to open '%s'.\n", filename);
		exit(1);
	}
	fseek(kfile, 0, SEEK_END);
	kanjiSize = ftell(kfile); // Each iteration is 2 bytes
	fseek(kfile, 0, SEEK_SET);
	kanji = malloc(kanjiSize);
	fread(kanji, 1, kanjiSize, kfile);
	fclose(kfile);
}

size_t fwrite_be16(uint16_t *ptr, size_t nmemb, FILE *stream) {
    int x = 1;
    if ( *((char*)&x) == 1) {
        /* Little endian machine, swap bytes */
        uint8_t *buffer = (uint8_t*) ptr;
        for (size_t i = 0; i < nmemb; i++) {
            uint8_t temp = buffer[2*i];
            buffer[2*i]     = buffer[2*i + 1];
            buffer[2*i + 1] = temp;
        }
    }
    return fwrite(ptr, 2, nmemb, stream);
}

int main(int argc, char *argv[]) {
	if(argc != 3) {
		printf("Usage: fontgen <kanji list file> <output file>\n");
		return 1;
	}
	// Load font
    int font_w, font_h, cc;
    uint32_t *font = (uint32_t*) stbi_load("mplus.png", &font_w, &font_h, &cc, STBI_rgb_alpha);
    if(!font) {
        printf("ERROR: Failed to load '%s'\n", "mplus.png");
        exit(1);
    }
    int font_cols = font_w / 12;
    //int font_h = hh / 16;
    // Load kanji list
	load_kanji(argv[1]);
    // Init tileset (2 color bitmap)
    uint16_t *tileset = calloc(kanjiSize, 2 * 16);

    int realSize = 0;
    int dst_y = 0;
	for(int i = 0; i < kanjiSize;) {
		// Decode UTF-8 char and convert RGBA letter from font to 2 color BMP data
        int bytes;
        uint16_t index = utf8_to_index(&kanji[i], &bytes);
        int src_x = (index % font_cols) * 12;
        int src_y = (index / font_cols) * 16;
        printf("U+%04X : %d,%d\n", index, src_x, src_y);
        for(int y = src_y; y < src_y + 16; y++) {
            uint16_t *row = &tileset[dst_y + (y - src_y)];
            for(int x = src_x; x < src_x + 12; x++) {
                if(font[y * font_w + x]) {
                    *row |= 1 << (15 - (x - src_x));
                    printf("%X", 15 - (x - src_x));
                } else {
                    printf("-");
                }
            }
            printf(" -> %04X\n", *row);
        }

		i += bytes;
        realSize++;
        dst_y += 16;
	}
    stbi_image_free(font);
    free(kanji);

    FILE *outfile = fopen(argv[2], "wb");
    if(!outfile) {
        printf("ERROR: Failed to write '%s'\n", argv[2]);
        exit(1);
    }
    fwrite_be16(tileset, realSize * 16, outfile);
    fclose(outfile);

	free(tileset);
	
	printf("Success!\n");
	return 0;
}
