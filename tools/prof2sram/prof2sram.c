#include <stdio.h>
#include <string.h>

#define FORMAT_PROFILE 0
#define FORMAT_SRAM 1
#define FORMAT_INVALID -1

#define fskip(f, n) fseek(f, n, SEEK_CUR)

const char *ext_profile = ".dat";
const char *ext_sram = ".srm";

const char *header_ver = "Do041220";
const char *flag_str = "FLAG";

struct {
	char header[8];
	unsigned char current_map;
	unsigned char current_song;
	int x_position;
	int y_position;
	unsigned char direction;
	unsigned short max_health;
	unsigned char whimsical_stars;
	unsigned short current_health;
	unsigned char current_weapon;
	unsigned short equipment;
	unsigned int time;
	struct {
		unsigned char type;
		unsigned char level;
		unsigned short energy;
		unsigned short max_ammo;
		unsigned short ammo;
	} weapon[8];
	unsigned short item[32];
	struct {
		unsigned short menu;
		unsigned short location;
	} teleport[8];
	char flag_header[4];
	unsigned int flag[250];
} ProfileData;

int read_profile_data(const char *filename);
int read_sram_data(const char *filename);
int write_profile_data(const char *filename);
int write_sram_data(const char *filename);

int get_format(const char *filename) {
	const char *ext = &filename[strlen(filename) - 4];
	if(strcmp(ext, ext_profile) == 0) {
		return FORMAT_PROFILE;
	} else if(strcmp(ext, ext_sram) == 0) {
		return FORMAT_SRAM;
	}
	return FORMAT_INVALID;
}

int main(int argc, char *argv[]) {
	if(argc != 3) {
		printf("Usage: %s <infile> <outfile>\n", argv[0]);
		return 0;
	}
	int inType = get_format(argv[1]), outType = get_format(argv[2]);
	printf("Hello!!\n");
	if(inType == FORMAT_PROFILE && outType == FORMAT_SRAM) {
		if(read_profile_data(argv[1])) {
			printf("Issue reading profile data, aborting.\n");
			return 1;
		}
		if(write_sram_data(argv[2])) {
			printf("Issue writing SRAM data, aborting.\n");
			return 1;
		}
	} else if(inType == FORMAT_SRAM && outType == FORMAT_PROFILE) {
		read_sram_data(argv[1]);
		write_profile_data(argv[2]);
	} else {
		printf("Invalid file extensions.\n");
		return 1;
	}
	return 0;
}

int read_profile_data(const char* filename) {
	FILE *infile = fopen(filename, "rb");
	if(infile == NULL) {
		printf("Can't open input file.\n");
		return 1;
	}
	// Verify header
	fread(ProfileData.header, 1, 8, infile);
	if(strncmp(ProfileData.header, header_ver, 8)) {
		printf("Invalid header.\n");
		return 1;
	}
	// Current Map
	fread(&ProfileData.current_map, 1, 1, infile);
	fskip(infile, 3);
	// Current Song
	fread(&ProfileData.current_song, 1, 1, infile);
	fskip(infile, 3);
	// Player Position
	fread(&ProfileData.x_position, 4, 1, infile);
	fread(&ProfileData.y_position, 4, 1, infile);
	// Player Direction
	fread(&ProfileData.direction, 1, 1, infile);
	fskip(infile, 3);
	// Health
	fread(&ProfileData.max_health, 2, 1, infile);
	fread(&ProfileData.whimsical_stars, 2, 1, infile);
	fread(&ProfileData.current_health, 2, 1, infile);
	fskip(infile, 2);
	// Current Weapon, Equipment
	fread(&ProfileData.current_weapon, 1, 1, infile);
	fskip(infile, 7);
	fread(&ProfileData.equipment, 2, 1, infile);
	fskip(infile, 6);
	// Play Time
	fread(&ProfileData.time, 4, 1, infile);
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
	}
	// Items
	for(int i = 0; i < 32; i++) {
		fread(&ProfileData.item[i], 2, 1, infile);
		fskip(infile, 2);
	}
	// Teleporter Locations
	for(int i = 0; i < 8; i++) {
		fread(&ProfileData.teleport[i].menu, 2, 1, infile);
		fskip(infile, 2);
		fread(&ProfileData.teleport[i].location, 2, 1, infile);
		fskip(infile, 2);
	}
	// Flags
	fseek(infile, 0x218, SEEK_SET);
	fread(ProfileData.flag_header, 1, 4, infile);
	if(strncmp(ProfileData.flag_header, flag_str, 4)) {
		printf("Error: Expected 'FLAG'.\n");
		return 1;
	}
	for(int i = 0; i < 250; i++) {
		fread(&ProfileData.flag[i], 4, 1, infile);
	}
	// All done
	fclose(infile);
	return 0;
}

int read_sram_data(const char *filename) {
	
}

int write_profile_data(const char *filename) {
	
}

void sram_write_byte(FILE *file, unsigned char value) {
	fskip(file, 1);
	fwrite(&value, 1, 1, file);
}

void sram_write_word(FILE *file, unsigned short value) {
	unsigned char high = value >> 8, low = value & 0xFF;
	fskip(file, 1);
	fwrite(&high, 1, 1, file);
	fskip(file, 1);
	fwrite(&low, 1, 1, file);
}

void sram_write_dword(FILE *file, unsigned int value) {
	unsigned char byte1 = value >> 24, byte2 = (value >> 16) & 0xFF;
	unsigned char byte3 = (value >> 8) & 0xFF, byte4 = value & 0xFF;
	fskip(file, 1);
	fwrite(&byte1, 1, 1, file);
	fskip(file, 1);
	fwrite(&byte2, 1, 1, file);
	fskip(file, 1);
	fwrite(&byte3, 1, 1, file);
	fskip(file, 1);
	fwrite(&byte4, 1, 1, file);
}

int write_sram_data(const char *filename) {
	FILE *outfile = fopen(filename, "wb");
	if(outfile == NULL) {
		printf("Can't open output file.\n");
		return 1;
	}
	// Current Map
	sram_write_word(outfile, ProfileData.current_map);
	// Current Song
	sram_write_word(outfile, ProfileData.current_song);
	// Player Position
	sram_write_word(outfile, (ProfileData.x_position + 0x1000) >> 13);
	sram_write_word(outfile, (ProfileData.y_position + 0x1000) >> 13);
	// Player Direction
	//sram_write_byte(outfile ProfileData.direction > 0);
	// Health
	sram_write_word(outfile, ProfileData.max_health);
	sram_write_word(outfile, ProfileData.current_health);
	// Current Weapon, Equipment
	sram_write_word(outfile, ProfileData.current_weapon);
	sram_write_word(outfile, ProfileData.equipment);
	// Play Time
	unsigned char hour, min, sec, frame = 0;
	frame = ProfileData.time % 50;
	sec = (ProfileData.time / 50) % 60;
	min = ((ProfileData.time / 50) / 60) % 60;
	hour = ((ProfileData.time / 50) / 60) / 60;
	sram_write_byte(outfile, hour);
	sram_write_byte(outfile, min);
	sram_write_byte(outfile, sec);
	sram_write_byte(outfile, frame);
	// Weapons
	fseek(outfile, 0x20 * 2, SEEK_SET);
	for(int i = 0; i < 8; i++) {
		sram_write_byte(outfile, ProfileData.weapon[i].type);
		sram_write_byte(outfile, ProfileData.weapon[i].level);
		sram_write_word(outfile, ProfileData.weapon[i].energy);
		sram_write_word(outfile, ProfileData.weapon[i].max_ammo);
		sram_write_word(outfile, ProfileData.weapon[i].ammo);
	}
	// Items
	for(int i = 0; i < 32; i++) {
		sram_write_byte(outfile, ProfileData.item[i]);
	}
	// Teleporter Locations
	for(int i = 0; i < 8; i++) {
		sram_write_word(outfile, ProfileData.teleport[i].location);
	}
	// Flags
	fseek(outfile, 0x100 * 2, SEEK_SET);
	for(int i = 0; i < 250; i++) {
		sram_write_dword(outfile, ProfileData.flag[i]);
	}
	// All done
	fclose(outfile);
}

