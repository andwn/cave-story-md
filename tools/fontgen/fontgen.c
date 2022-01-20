/**
 * fontgen - Generates an image based on a kanji list and ttf font
 * 
 * Compile:
 * gcc fontgen.c -lSDL2 -lSDL2_ttf -o fontgen
 * 
 * Usage:
 * ./fontgen <ttf file> <kanji list file> <output file>
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <ctype.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

//#define FONT_SIZE 16

#define min(a,b) ((a)>(b)?(b):(a))

uint8_t *kanji; // Increase if need more than 8KB
uint16_t kanjiSize;

TTF_Font *font;
SDL_Surface *tileset;

uint8_t char_bytes(uint8_t val) {
    if (val < 128) {
        return 1;
    } else if (val < 224) {
        return 2;
    } else if (val < 240) {
        return 3;
    } else {
        return 4;
    }
}

void load_font(const char *filename) {
	font = TTF_OpenFont(filename, 12);
	if(!font) {
		printf("%s\n", TTF_GetError());
		exit(EXIT_FAILURE);
	}
	//TTF_SetFontHinting(font, TTF_HINTING_NORMAL);
	//TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
}

void load_kanji(const char *filename) {
	FILE *kfile = fopen(filename, "rb");
	if(!kfile) {
		printf("ERROR: Failed to open '%s'.\n", filename);
		exit(EXIT_FAILURE);
	}
	fseek(kfile, 0, SEEK_END);
	kanjiSize = ftell(kfile); // Each iteration is 2 bytes
	fseek(kfile, 0, SEEK_SET);
	kanji = malloc(kanjiSize);
	fread(kanji, 1, kanjiSize, kfile);
	fclose(kfile);
}

int main(int argc, char *argv[]) {
	if(argc != 4) {
		printf("Usage: fontgen <ttf file> <kanji list file> <output file>\n");
		return EXIT_SUCCESS;
	}
	// Init SDL
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	// Load font and kanji list
	load_font(argv[1]);
	load_kanji(argv[2]);
	
	tileset = SDL_CreateRGBSurface(0, 16, kanjiSize * 16, 32, 0,0,0,0);
	
	// This is PAL_Main, for the message box
	const SDL_Color color[16] = {
		{ 0x00, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0xFF },
		{ 0x3b, 0x41, 0x94, 0xFF },
		{ 0x64, 0x72, 0xcf, 0xFF },
		{ 0xca, 0xca, 0xFF, 0xFF },
		{ 0x00, 0x00, 0x00, 0xFF },
		{ 0x00, 0x00, 0x00, 0xFF },
		{ 0x00, 0x00, 0x00, 0xFF },
		{ 0x00, 0x00, 0x00, 0xFF },
		{ 0x00, 0x00, 0x00, 0xFF },
		{ 0x00, 0x00, 0x00, 0xFF },
		{ 0x00, 0x00, 0x00, 0xFF },
		{ 0xe6, 0xef, 0xda, 0xFF },
		{ 0x8c, 0xa5, 0xbd, 0xFF },
		{ 0x00, 0x00, 0x00, 0xFF },
		{ 0xFF, 0xFF, 0xFF, 0xFF },
	};
	//SDL_Palette *pal = SDL_AllocPalette(16);
	//SDL_SetPaletteColors(pal, color, 0, 16);
	//SDL_SetSurfacePalette(tileset, pal);
	
	// Change the Y value for srect if the glyphs are cut off in the output
	SDL_Rect srect  = { 0, 0, 12, 16 };
	SDL_Rect drect  = { 0, 0, 12, 16 };
	
	for(uint16_t i = 0; i < kanjiSize;) {
		// Put the next UTF-8 char into a string and render it
		uint8_t bytes = char_bytes(kanji[i]);
		char str[6] = {};
		for(uint8_t c = 0; c < bytes; c++) {
			str[c] = kanji[i+c];
		}
		int w;
		TTF_SizeUTF8(font, str, &w, NULL);
		//drect.x = 0;//8 - w / 2;
		//SDL_Surface *text = TTF_RenderUTF8_Shaded(font, str, color[15], color[2]);
		SDL_Surface *text = TTF_RenderUTF8_Solid(font, str, color[15]);
		SDL_BlitSurface(text, &srect, tileset, &drect);
		SDL_FreeSurface(text);
		i += bytes;
		drect.y += 16;
	}
	
	SDL_SaveBMP(tileset, argv[3]);
	
	SDL_FreeSurface(tileset);
	//SDL_FreePalette(pal);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	
	printf("Success!\n");
	return EXIT_SUCCESS;
}
