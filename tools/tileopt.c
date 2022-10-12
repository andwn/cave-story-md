/**
 * tileopt - Removes unused tiles from a Cave Story tileset by analyzing PXM files, 
 * then adjusts the values in all related files accordingly
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

#define min(a,b) ((a)>(b)?(b):(a))

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
	tileset = IMG_Load(filename);
	if(tileset == NULL) {
		printf("%s\n", SDL_GetError());
		exit(0);
	}
}

void load_attr(const char *filename) {
	FILE *file = fopen(filename, "rb");
	fseek(file, 0, SEEK_END);
	tileCount = min(ftell(file), (tileset->w / 16) * (tileset->h / 16));
	fseek(file, 0, SEEK_SET);
	fread(tileAttr, 1, tileCount, file);
	printf("Bytes read from attribute file: %d\n", tileCount);
	fclose(file);
	if(tileCount <= 0) exit(0);
}

void load_map(const char *filename, int index) {
	FILE *file = fopen(filename, "rb");
	fseek(file, 4, SEEK_SET);
	fread(&mapWidth[index], 2, 1, file);
	fread(&mapHeight[index], 2, 1, file);
	printf("Map Index %d - %hu, %hu\n", index, mapWidth[index], mapHeight[index]);
	mapTiles[index] = malloc(mapWidth[index] * mapHeight[index]);
	fread(mapTiles[index], 1, mapWidth[index] * mapHeight[index], file);
	fclose(file);
}

void save_tileset(const char *filename) {
	char newfn[300];
	sprintf(newfn, "%s_opt.bmp", filename);
	SDL_SaveBMP(tileset, newfn);
}

void save_attr(const char *filename) {
	char newfn[300];
	sprintf(newfn, "%s_opt.pxa", filename);
	FILE *file = fopen(newfn, "wb");
	fwrite(tileAttr, 1, tileCount, file);
	fclose(file);
}

void save_map(const char *filename, int index) {
	char newfn[300];
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
	memset(tileUsage, 0, tileCount * sizeof(int));
	for(int map = 0; map < mapCount; map++) {
		for(int tile = 0; tile < mapWidth[map] * mapHeight[map]; tile++) {
			tileUsage[mapTiles[map][tile]]++;
		}
	}
	printf("Total Usage:\n");
	for(int y = 0; y < tileCount / 16; y++) {
		for(int x = 0; x < 16; x++) {
			printf("%04d ", tileUsage[y * 16 + x]);
		}
		printf("\n");
	}
}

// Decrement any tile indexes larger than the one specified
void decrement_map_tiles(int tileIndex) {
	for(int map = 0; map < mapCount; map++) {
		for(int tile = 0; tile < mapWidth[map] * mapHeight[map]; tile++) {
			if(mapTiles[map][tile] > tileIndex) mapTiles[map][tile]--;
		}
	}
}

void shift_tileset(int fromIndex, int toIndex) {
	int x1 = (fromIndex % 16) * 16;
	int y1 = (fromIndex / 16) * 16;
	int x2 = (toIndex % 16) * 16;
	int y2 = (toIndex / 16) * 16;
	Uint8 *pixels = (Uint8*)tileset->pixels;
	for(int y = 0; y < 16; y++) {
		memcpy(&pixels[((y2 + y) * tileset->pitch + x2)], 
			&pixels[((y1 + y) * tileset->pitch + x1)], 16);
	}
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
		for(int y = 0; y < 16; y++) {
			Uint8 *pixels = (Uint8*)tileset->pixels;
			memset(&pixels[(tileCount/16*16 + y) * tileset->pitch + (tileCount%16)*16], 0, 16);
		}
		// Go into each map and decrement tile index values larger than this one
		decrement_map_tiles(tile);
		tile--;
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
	SDL_LockSurface(tileset);
	calculate_usage();
	trim_unused();
	printf("Saving tileset\n");
	save_tileset(argv[1]);
	SDL_UnlockSurface(tileset);
	SDL_FreeSurface(tileset);
	printf("Saving PXA\n");
	save_attr(argv[2]);
	for(int i = 0; i < mapCount; i++) {
		printf("Saving map %d\n", i);
		save_map(argv[i + 3], i);
		free(mapTiles[i]);
	}
	free(mapWidth);
	free(mapHeight);
	free(mapTiles);
	printf("All operations successful!\n");
	return 0;
}
