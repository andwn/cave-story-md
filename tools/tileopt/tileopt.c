/**
 * tileopt - Optimizes a tileset by removing unused tiles
 * 
 * Compile:
 * gcc tileopt.c -lSDL2 -lSDL2_image -o tileopt
 * 
 * Usage:
 * ./tileopt <tileset file> <tile attribute file> <map file, [more map files, ...]>
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tileset and attributes
int tileCount;
unsigned char tileAttr[256];
int tileUsage[256];
SDL_Surface *tileset;

// Tilemaps
int mapCount;
unsigned short *mapWidth;
unsigned short *mapHeight;
unsigned char **mapTiles;

void load_tileset(const char *filename) {
	SDL_Surface *tileset = IMG_Load(filename);
	if(tileset == NULL) {
		printf("%s\n", SDL_GetError());
		exit(0);
	}
}

void load_attr(const char *filename) {
	FILE *file = fopen(filename, "rb");
	tileCount = fread(tileAttr, 1, 256, file);
	printf("Bytes read from attribute file: %d\n", tileCount);
	fclose(file);
	if(tileCount <= 0) exit(0);
}

void load_map(const char *filename, int index) {
	FILE *file = fopen(filename, "rb");
	fseek(file, 4, SEEK_SET);
	fread(&mapWidth[index], 2, 1, file);
	fread(&mapHeight[index], 2, 1, file);
	mapTiles[index] = malloc(sizeof(unsigned char) * mapWidth[index] * mapHeight[index]);
	fread(mapTiles[index], 1, mapWidth[index] * mapHeight[index], file);
	fclose(file);
}

void save_tileset(const char *filename) {
	char newfn[256];
	sprintf(newfn, "%s_opt.bmp", filename);
	SDL_SaveBMP(tileset, newfn);
}

void save_attr(const char *filename) {
	char newfn[256];
	sprintf(newfn, "%s_opt.pxa", filename);
	FILE *file = fopen(newfn, "wb");
	fwrite(tileAttr, 1, tileCount, file);
	fclose(file);
}

void save_map(const char *filename, int index) {
	char newfn[256];
	sprintf(newfn, "%s_opt.pxm", filename);
	FILE *file = fopen(newfn, "wb");
	fprintf(file, "PXM");
	fseek(file, 4, SEEK_SET);
	fwrite(&mapWidth[index], 2, 1, file);
	fwrite(&mapHeight[index], 2, 1, file);
	fwrite(mapTiles[index], 1, mapWidth[index] * mapHeight[index], file);
	fclose(file);
}

// Count how many times each tile has been used in the open maps
void calculate_usage() {
	// Make sure all values start at 0
	memset(tileUsage, 0, 256 * sizeof(int));
	for(int map = 0; map < mapCount; map++) {
		for(int tile = 0; tile < mapWidth[map] * mapHeight[map]; tile++) {
			tileUsage[mapTiles[map][tile]]++;
		}
	}
}

// Decrement any tile indexes larger than the one specified
void decrement_map_tiles(unsigned char tileIndex) {
	for(int map = 0; map < mapCount; map++) {
		for(int tile = 0; tile < mapWidth[map] * mapHeight[map]; tile++) {
			if(mapTiles[map][tile] > tileIndex) mapTiles[map][tile]--;
		}
	}
}

void shift_tileset(unsigned char fromIndex, unsigned char toIndex) {
	int x1 = (fromIndex % 16) * 16;
	int y1 = (fromIndex / 16) * 16;
	int x2 = (toIndex % 16) * 16;
	int y2 = (toIndex / 16) * 16;
	unsigned char bpp = tileset->format->BytesPerPixel;
	SDL_LockSurface(tileset);
	for(int y = 0; y < 16; y++) {
		memcpy(tileset->pixels + ((y2 + y) * 256 + x2) * bpp, 
			tileset->pixels + ((y1 + y) * 256 + x1) * bpp, bpp * 16);
	}
	SDL_UnlockSurface(tileset);
}

// Remove any tile that has been used 0 times
void trim_unused() {
	for(int tile = 0; tile < tileCount; tile++) {
		if(tileUsage[tile] != 0) continue;
		tileCount--;
		// Shift all tile attributes after this one down to previous array element
		for(int i = tile; i < tileCount; i++) {
			tileAttr[i] = tileAttr[i + 1];
			tileUsage[i] = tileUsage[i + 1];
			shift_tileset(i + 1, i);
		}
		tileAttr[tileCount] = 0;
		tileUsage[tileCount] = 0;
		// Go into each map and decrement tile index values larger than this one
		decrement_map_tiles(tile);
	}
}

int main(int argc, char *argv[]) {
	if(argc < 4) return 1;
	load_tileset(argv[1]);
	load_attr(argv[2]);
	mapCount = argc - 3;
	mapWidth = malloc(sizeof(unsigned short) * mapCount);
	mapHeight = malloc(sizeof(unsigned short) * mapCount);
	mapTiles = malloc(sizeof(unsigned char*) * mapCount);
	for(int i = 0; i < mapCount; i++) {
		load_map(argv[i + 3], i);
	}
	calculate_usage();
	trim_unused();
	save_tileset(argv[1]);
	save_attr(argv[2]);
	for(int i = 0; i < mapCount; i++) {
		save_map(argv[i + 3], i);
	}
	return 0;
}
