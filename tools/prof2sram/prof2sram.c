/*
 * Simple tool to convert Cave Story Profile.dat to SRAM data for Cave Story MD
 * 
 * Compile:
 * gcc prof2sram.c -o prof2sram
 * 
 * Usage:
 * ./prof2sram [-v] [-g] [-n <290.rec>] <input Profile.dat> <output SRAM>
 * -g will skip every other byte in the output file (so it'll work with Gens)
 * -v will print verbose information about the profile
 * 
 * Example:
 * Grab a Profile.dat and optionally 290.rec from http://www.cavestory.org/download/saves.php
 * Extract with your archive manager of choice, then run
 * ./prof2sram /path/to/Profile.dat doukutsu.srm
 * Or to include 290.rec
 * ./prof2sram -n /path/to/290.rec /path/to/Profile.dat doukutsu.srm
 * 
 * Where does the SRAM go? Depends on your emulator.
 * Gens-GS: $HOME/.gens/doukutsu.srm
 * BlastEm: $HOME/.local/share/blastem/doukutsu/save.sram
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define fskip(f, n) fseek(f, n, SEEK_CUR)

const char *header_ver = "Do041220";
const char *flag_str = "FLAG";
const char *wepname[14] = {
	"N/A", "Snake", "Polar Star", "Fireball", "Machine Gun", "Missiles", "N/A", 
	"Bubbler", "N/A", "Blade", "Super Missiles", "N/A", "Nemesis", "Spur"
};

static uint8_t oddbytes = 0;
static uint8_t verbose = 0;

struct {
	char header[8];
	uint8_t current_map;
	uint8_t current_song;
	int32_t x_position;
	int32_t y_position;
	uint8_t direction;
	uint16_t max_health;
	uint8_t whimsical_stars;
	uint16_t current_health;
	uint8_t current_weapon;
	uint16_t equipment;
	uint32_t time;
	struct {
		uint8_t type;
		uint8_t level;
		uint16_t energy;
		uint16_t max_ammo;
		uint16_t ammo;
	} weapon[8];
	uint16_t item[32];
	struct {
		uint16_t menu;
		uint16_t location;
	} teleport[8];
	char flag_header[4];
	uint32_t flag[250];
} ProfileData;

struct {
	uint32_t ticks[4];
	uint8_t key[4];
} NikuCounter;

int read_290rec(const char *filename);
int read_profile_data(const char *filename);
int write_sram_data(const char *filename);

void sram_write_u8(FILE *file, uint8_t value);
void sram_write_u16(FILE *file, uint16_t value);
void sram_write_u32(FILE *file, uint32_t value);
void sram_seek(FILE *file, uint32_t pos);

void usage() {
	printf("Usage: prof2sram [options] <input Profile.dat> <output SRAM>\n");
	exit(0);
}

int main(int argc, char *argv[]) {
	if(argc < 3) usage();
	// Make sure we will write all zeroes if no 290.rec is specified
	memset(&NikuCounter, 0, sizeof(NikuCounter));
	// Look for any optional args
	for(int i = 1; i < argc - 2; i++) {
		if(argv[i][0] != '-') usage();
		switch(argv[i][1]) {
			case 'n': // 290.rec
			{
				i++; // next arg is the filename
				if(read_290rec(argv[i])) {
					perror("Issue reading 290.rec file.\n");
					return 1;
				}
			}
			break;
			case 'g': // Output in Gens SRAM format
			{
				oddbytes = 1;
			}
			break;
			case 'v':
			{
				verbose = 1;
			}
			break;
			default: usage();
		}
	}
	if(read_profile_data(argv[argc - 2])) {
		printf("Issue reading profile data, aborting.\n");
		return 1;
	}
	if(write_sram_data(argv[argc - 1])) {
		printf("Issue writing SRAM data, aborting.\n");
		return 1;
	}
}

int read_290rec(const char *filename) {
	FILE *infile = fopen(filename, "rb");
	if(!infile) {
		perror("Failed to load 290.rec.");
		return 1;
	}
	fread(&NikuCounter, 1, 20, infile);
	// All done
	fclose(infile);
	return 0;
}

int read_profile_data(const char* filename) {
	FILE *infile = fopen(filename, "rb");
	if(!infile) {
		perror("Failed to load input file.");
		return 1;
	}
	// Verify header
	fread(ProfileData.header, 1, 8, infile);
	if(strncmp(ProfileData.header, header_ver, 8)) {
		printf("Invalid header '%.8s'. Expected '%.8s'.\n", ProfileData.header, header_ver);
		return 1;
	}
	// Current Map
	fread(&ProfileData.current_map, 1, 1, infile);
	fskip(infile, 3);
	if(verbose) printf("Map ID: %hhu\n", ProfileData.current_map);
	// Current Song
	fread(&ProfileData.current_song, 1, 1, infile);
	fskip(infile, 3);
	if(verbose) printf("Song ID: %hhu\n", ProfileData.current_song);
	// Player Position
	fread(&ProfileData.x_position, 4, 1, infile);
	fread(&ProfileData.y_position, 4, 1, infile);
	if(verbose) printf("Location X: 0x%x Y: 0x%x\n", 
						ProfileData.x_position, ProfileData.y_position);
	// Player Direction
	fread(&ProfileData.direction, 1, 1, infile);
	fskip(infile, 3);
	if(verbose) printf("Facing %s.\n", ProfileData.direction == 0 ? "left" : 
									   ProfileData.direction == 2 ? "right" : "???");
	// Health
	fread(&ProfileData.max_health, 2, 1, infile);
	fread(&ProfileData.whimsical_stars, 2, 1, infile);
	fread(&ProfileData.current_health, 2, 1, infile);
	fskip(infile, 2);
	if(verbose) printf("Health: %hu/%hu\n", ProfileData.current_health, ProfileData.max_health);
	// Current Weapon, Equipment
	fread(&ProfileData.current_weapon, 1, 1, infile);
	fskip(infile, 7);
	if(verbose) printf("Current Weapon: %hhu\n", ProfileData.current_weapon);
	fread(&ProfileData.equipment, 2, 1, infile);
	fskip(infile, 6);
	if(verbose) printf("Equipment: 0x%hx\n", ProfileData.current_weapon);
	// Play Time
	fread(&ProfileData.time, 4, 1, infile);
	if(verbose) {
		uint8_t hour, min, sec, frame = 0;
		frame = ProfileData.time % 50;
		sec = (ProfileData.time / 50) % 60;
		min = ((ProfileData.time / 50) / 60) % 60;
		hour = ((ProfileData.time / 50) / 60) / 60;
		printf("Time: %hhu:%hhu:%hhu.%hhu\n", hour, min, sec, frame);
	}
	// Weapons
	for(int i = 0; i < 8; i++) {
		fread(&ProfileData.weapon[i].type, 2, 1, infile);
		fskip(infile, 2);
		fread(&ProfileData.weapon[i].level, 2, 1, infile);
		fskip(infile, 2);
		fread(&ProfileData.weapon[i].energy, 2, 1, infile);
		fskip(infile, 2);
		fread(&ProfileData.weapon[i].max_ammo, 2, 1, infile);
		fskip(infile, 2);
		fread(&ProfileData.weapon[i].ammo, 2, 1, infile);
		fskip(infile, 2);
		if(verbose) {
			printf("Weapon %d: %s level %hu\n", i, 
					wepname[ProfileData.weapon[i].type], ProfileData.weapon[i].level);
		}
	}
	// Items
	if(verbose) printf("Items: ");
	for(int i = 0; i < 32; i++) {
		fread(&ProfileData.item[i], 2, 1, infile);
		fskip(infile, 2);
		if(verbose) printf("%hu,", ProfileData.item[i]);
	}
	if(verbose) printf("\n");
	// Teleporter Locations
	if(verbose) printf("Warps: ");
	for(int i = 0; i < 8; i++) {
		fread(&ProfileData.teleport[i].menu, 2, 1, infile);
		fskip(infile, 2);
		fread(&ProfileData.teleport[i].location, 2, 1, infile);
		fskip(infile, 2);
		if(verbose) printf("%hu:%hu, ", ProfileData.item[i]);
	}
	if(verbose) printf("\n");
	// Flags
	fseek(infile, 0x218, SEEK_SET);
	fread(ProfileData.flag_header, 1, 4, infile);
	if(strncmp(ProfileData.flag_header, flag_str, 4)) {
		printf("Error: Expected 'FLAG', found '%.4s'.\n", ProfileData.flag_header);
		return 1;
	}
	if(verbose) printf("Flags:\n");
	for(int i = 0; i < 250; i++) {
		fread(&ProfileData.flag[i], 4, 1, infile);
		if(verbose) printf("%x", ProfileData.flag[i]);
	}
	if(verbose) printf("\n");
	// All done
	fclose(infile);
	return 0;
}

void sram_write_u8(FILE *file, uint8_t value) {
	if(oddbytes) fskip(file, 1);
	fwrite(&value, 1, 1, file);
}

void sram_write_u16(FILE *file, uint16_t value) {
	sram_write_u8(file, value >> 8); // High byte
	sram_write_u8(file, value & 0xFF); // Low byte
}

void sram_write_u32(FILE *file, uint32_t value) {
	sram_write_u16(file, value >> 16); // High word
	sram_write_u16(file, value & 0xFFFF); // Low word
}

void sram_seek(FILE *file, uint32_t pos) {
	if(oddbytes) pos *= 2;
	fseek(file, pos, SEEK_SET);
}

int write_sram_data(const char *filename) {
	FILE *outfile = fopen(filename, "wb");
	if(!outfile) {
		printf("Can't open output file.\n");
		return 1;
	}
	// Current Map
	sram_write_u16(outfile, ProfileData.current_map);
	// Current Song
	sram_write_u16(outfile, ProfileData.current_song);
	// Player Position
	sram_write_u16(outfile, (ProfileData.x_position + 0x1000) >> 13);
	sram_write_u16(outfile, (ProfileData.y_position + 0x1000) >> 13);
	// Health
	sram_write_u16(outfile, ProfileData.max_health);
	sram_write_u16(outfile, ProfileData.current_health);
	// Current Weapon, Equipment
	sram_write_u16(outfile, ProfileData.current_weapon);
	sram_write_u16(outfile, ProfileData.equipment);
	// Play Time
	uint8_t hour, min, sec, frame = 0;
	frame = ProfileData.time % 50;
	sec = (ProfileData.time / 50) % 60;
	min = ((ProfileData.time / 50) / 60) % 60;
	hour = ((ProfileData.time / 50) / 60) / 60;
	sram_write_u8(outfile, hour);
	sram_write_u8(outfile, min);
	sram_write_u8(outfile, sec);
	sram_write_u8(outfile, frame);
	// Weapons
	sram_seek(outfile, 0x20);
	for(int i = 0; i < 5; i++) {
		sram_write_u8(outfile, ProfileData.weapon[i].type);
		sram_write_u8(outfile, ProfileData.weapon[i].level);
		sram_write_u16(outfile, ProfileData.weapon[i].energy);
		sram_write_u16(outfile, ProfileData.weapon[i].max_ammo);
		sram_write_u16(outfile, ProfileData.weapon[i].ammo);
	}
	// Check the last 3 weapons -- CSMD will only use the first 5 slots
	for(int i = 5; i < 8; i++) {
		if(ProfileData.weapon[i].type) {
			printf("Warning: Weapon of type '%hhu' at index '%d' ignored.\n",
					ProfileData.weapon[i].type, i);
		}
	}
	// Nikumaru Counter
	for(int i = 0; i < 4; i++) sram_write_u32(outfile, NikuCounter.ticks[i]);
	for(int i = 0; i < 4; i++) sram_write_u8(outfile, NikuCounter.key[i]);
	// "CSMD" checksum
	sram_write_u32(outfile, ('C'<<24)|('S'<<16)|('M'<<8)|('D'));
	// Items
	for(int i = 0; i < 32; i++) {
		sram_write_u8(outfile, ProfileData.item[i]);
	}
	// Teleporter Locations
	for(int i = 0; i < 8; i++) {
		sram_write_u16(outfile, ProfileData.teleport[i].location);
	}
	// Flags
	sram_seek(outfile, 0x100);
	for(int i = 0; i < 250; i++) {
		sram_write_u32(outfile, ProfileData.flag[i]);
	}
	// All done
	fclose(outfile);
	return 0;
}
