/**
 * bmp2dat - Convert BMP output from fontgen to 16x16 1bpp tile data
 * 
 * Compile:
 * cc bmp2dat.c -o bmp2dat
 * 
 * Usage:
 * ./bmp2dat <input bmp> <output dat>
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define min(a,b) ((a)>(b)?(b):(a))

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
		printf("Usage: bmp2dat <input bmp> <output dat>\n");
		return 1;
	}
	// Load font
    int font_w, font_h, cc;
    uint32_t *font = (uint32_t*) stbi_load(argv[1], &font_w, &font_h, &cc, STBI_rgb_alpha);
    if(!font) {
        printf("ERROR: Failed to load '%s'\n", argv[1]);
        exit(1);
    }
    // Init tileset (2 color bitmap)
    uint16_t *tileset = calloc(font_h, 2);

    int lastChar = 0;
    for(int c = 0; c < font_h / 16; c++) {
        int filledBits = 0;
        for(int y = 0; y < 16; y++) {
            int yy = c * 16 + y;
            uint16_t *row = &tileset[yy];
            for(int x = 0; x < 16; x++) {
                if (font[yy * font_w + x] & 0xFF) {
                    filledBits++;
                    *row |= 1 << (15 - x);
                    printf("%X", 15 - x);
                } else {
                    printf("-");
                }
            }
            printf(" -> %04X\n", *row);
        }
        if(filledBits) lastChar = c;
    }
    stbi_image_free(font);

    FILE *outfile = fopen(argv[2], "wb");
    if(!outfile) {
        printf("ERROR: Failed to write '%s'\n", argv[2]);
        exit(1);
    }
    fwrite_be16(tileset, (lastChar + 1) * 16, outfile);
    fclose(outfile);

	free(tileset);
	
	printf("Success!\n");
	return 0;
}
