/*
 * tscomp - Converts a TSC file into bytecode usable by CSMD
 * 
 * Compile:
 * gcc tscomp.c -o tscomp
 * 
 * Usage:
 * ./tscomp <tsc file> <output file>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bool unsigned char
#define TRUE 1
#define FALSE 0

#define MAX_EVENTS 106
#define COMMAND_COUNT 93

#define SYM_EVENT		'#'
#define SYM_COMMENT		'-'
#define SYM_COMMAND		'<'
#define SYM_PARAM_SEP	':'

#define LEN_COMMAND		3
#define LEN_NUMBER		4

#define CFLAG_MSGOPEN	0x01
#define CFLAG_MSGCLOSE	0x02
#define CFLAG_JUMP		0x04
#define CFLAG_END		0x08

#define OP_EVENT	0xFFFF

// TSC Command Definition
typedef struct {
	char name[4]; // All commands are 3 uppercase characters
	unsigned char opcode; // Byte value that represents the instruction
	unsigned char params; // Number of parameters 0-4
	unsigned short flags; // Other properties of the instruction
} CommandDef;

const CommandDef command_table[COMMAND_COUNT] = {
	// Message Related
	{ "MSG", 0x80, 0, CFLAG_MSGOPEN },
	{ "MS2", 0x81, 0, CFLAG_MSGOPEN },
	{ "MS3", 0x82, 0, CFLAG_MSGOPEN },
	{ "CLO", 0x83, 0, CFLAG_MSGCLOSE },
	{ "CLR", 0x84, 0, CFLAG_MSGOPEN },
	{ "NUM", 0x85, 1, CFLAG_MSGOPEN },
	{ "GIT", 0x86, 1, CFLAG_MSGOPEN },
	{ "FAC", 0x87, 1, CFLAG_MSGOPEN },
	{ "CAT", 0x88, 0, 0 },
	{ "SAT", 0x89, 0, 0 },
	{ "TUR", 0x8A, 0, 0 },
	{ "YNJ", 0x8B, 1, CFLAG_JUMP },
	// Commands that are at the end of an event block
	{ "END", 0x8C, 0, CFLAG_MSGCLOSE | CFLAG_END },
	{ "EVE", 0x8D, 1, CFLAG_MSGCLOSE | CFLAG_JUMP | CFLAG_END },
	{ "TRA", 0x8E, 4, CFLAG_MSGCLOSE | CFLAG_END },
	{ "INI", 0x8F, 0, CFLAG_MSGCLOSE | CFLAG_END },
	{ "LDP", 0x90, 0, CFLAG_MSGCLOSE | CFLAG_END },
	{ "ESC", 0x91, 0, CFLAG_MSGCLOSE | CFLAG_END },
	// Music / Sound (18-25)
	{ "CMU", 0x92, 1, 0 }, // Change music
	{ "FMU", 0x93, 0, 0 }, // Fade music
	{ "RMU", 0x94, 0, 0 }, // Resume music
	{ "SOU", 0x95, 1, 0 }, // Play a sound
	{ "SPS", 0x96, 0, 0 }, // Start the propeller sound
	{ "CPS", 0x97, 0, 0 }, // Stop the propeller sound
	{ "SSS", 0x98, 1, 0 }, // Start the stream sound
	{ "CSS", 0x99, 0, 0 }, // Stop the stream sound
	// Wait Commands
	{ "NOD", 0x9A, 0, 0 }, // Wait for player input before resuming
	{ "WAI", 0x9B, 1, 0 }, // Wait an amount of time
	{ "WAS", 0x9C, 0, 0 }, // Wait for player to be on the ground
	// Player Control
	{ "MM0", 0x9D, 0, 0 }, // Stop player movement
	{ "MOV", 0x9E, 2, 0 }, // Move the player
	{ "MYB", 0x9F, 1, 0 }, // Makes the player hop (0x20)
	{ "MYD", 0xA0, 1, 0 }, // Set player direction
	{ "UNI", 0xA1, 1, 0 }, // Change movement type
	{ "UNJ", 0xA2, 2, CFLAG_JUMP }, // Jump on movement type
	{ "KEY", 0xA3, 0, 0 }, // Lock player controls and hide HUD
	{ "PRI", 0xA4, 0, 0 }, // Lock player controls until KEY or END
	{ "FRE", 0xA5, 0, 0 }, // Give control back to the player
	{ "HMC", 0xA6, 0, 0 }, // Hide the player
	{ "SMC", 0xA7, 0, 0 }, // Show the player
	{ "LI+", 0xA8, 1, 0 }, // Restore health
	{ "ML+", 0xA9, 1, 0 }, // Increase max health
	// NPC / Entities / Boss
	{ "ANP", 0xAA, 3, 0 }, // Animate NPC
	{ "CNP", 0xAB, 3, 0 }, // Change NPC
	{ "MNP", 0xAC, 4, 0 }, // Move NPC
	{ "DNA", 0xAD, 1, 0 }, // Delete all of a certain NPC
	{ "DNP", 0xAE, 1, 0 }, // Delete specific NPC
	{ "INP", 0xAF, 3, 0 }, // Initialize / change entity (0x30)
	{ "SNP", 0xB0, 4, 0 }, // Also creates an entity
	{ "BOA", 0xB1, 1, 0 }, // Change boss animation / state
	{ "BSL", 0xB2, 1, 0 }, // Start boss fight with entity
	{ "NCJ", 0xB3, 2, CFLAG_JUMP }, // Jump to event if NPC exists
	{ "ECJ", 0xB4, 2, CFLAG_JUMP }, // Jump to event if any entity exists
	// Arms
	{ "AE+", 0xB5, 0, 0 }, // Refill all ammo
	{ "ZAM", 0xB6, 0, 0 }, // Takes away all ammo
	{ "AM+", 0xB7, 2, 0 }, // Give player weapon and/or ammo
	{ "AM-", 0xB8, 1, 0 }, // Remove player weapon
	{ "TAM", 0xB9, 3, 0 }, // Trade weapons
	{ "AMJ", 0xBA, 2, CFLAG_JUMP }, // Jump to event if have weapon
	// Equps
	{ "EQ+", 0xBB, 1, 0 }, // Equip item
	{ "EQ-", 0xBC, 1, 0 }, // Unequip item
	// Items
	{ "IT+", 0xBD, 1, 0 }, // Give item
	{ "IT-", 0xBE, 1, 0 }, // Remove item
	{ "ITJ", 0xBF, 2, CFLAG_JUMP }, // Jump to event if have item (0x40)
	// Flags
	{ "FL+", 0xC0, 1, 0 }, // Set flag
	{ "FL-", 0xC1, 1, 0 }, // Clear flag
	{ "FLJ", 0xC2, 2, CFLAG_JUMP }, // Jump to event if flag is true
	{ "SK+", 0xC3, 1, 0 }, // Enable skipflag
	{ "SK-", 0xC4, 1, 0 }, // Disable skipflag
	{ "SKJ", 0xC5, 2, CFLAG_JUMP }, // Jump on skipflag (boss rematch)
	// Camera
	{ "FOB", 0xC6, 2, 0 }, // Focus on boss
	{ "FOM", 0xC7, 1, 0 }, // Focus on me
	{ "FON", 0xC8, 2, 0 }, // Focus on NPC
	{ "QUA", 0xC9, 1, 0 }, // Shake camera for quake effect
	// Screen Effects
	{ "FAI", 0xCA, 1, 0 }, // Fade in
	{ "FAO", 0xCB, 1, 0 }, // Fade out
	{ "FLA", 0xCC, 0, 0 }, // Flash screen white
	// Room
	{ "MLP", 0xCD, 0, 0 }, // Show the map
	{ "MNA", 0xCE, 0, 0 }, // Display room name
	{ "CMP", 0xCF, 3, 0 }, // Change room tile (0x50)
	{ "MP+", 0xD0, 1, 0 }, // Enable map flag
	{ "MPJ", 0xD1, 1, CFLAG_JUMP }, // Jump if map flag enabled
	// Misc
	{ "CRE", 0xD2, 0, 0 }, // Rolls the credits
	{ "SIL", 0xD3, 1, 0 }, // Show illustration (credits)
	{ "CIL", 0xD4, 0, 0 }, // Clear illustration (credits)
	{ "SLP", 0xD5, 0, 0 }, // Show teleporter menu
	{ "PS+", 0xD6, 2, 0 }, // Portal slot +
	{ "SVP", 0xD7, 0, 0 }, // Saves the game
	{ "STC", 0xD8, 0, 0 }, // Save time counter
	{ "XX1", 0xD9, 1, 0 }, // Island control
	{ "SMP", 0xDA, 2, 0 },
	{ "NOP", 0xDB, 0, 0 },
};

const char *ValidChars = 
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz ,.?=!@$%^&*()[]{}|_-+:;'\"\n";

FILE* decrypt_tsc(const char *filename);

void do_script(FILE *fin, FILE *fout);
void do_event(FILE *fin, FILE *fout);
unsigned short do_command(FILE *fin, FILE *fout);
unsigned short read_number(FILE *file);

bool is_valid_char(char c);

int main(int argc,char *argv[]) {
	if(argc != 3) {
		printf("Usage: tscomp <tsc file> <output file>\n");
		return 0;
	}
	FILE *infile, *outfile;
	if((infile = decrypt_tsc(argv[1])) == NULL) {
		printf("Failed to open decrypted file.\n");
		return 1;
	}
	if((outfile = fopen(argv[2], "wb")) == NULL) {
		printf("Failed to open output file.\n");
		return 1;
	}
	do_script(infile, outfile);
	fclose(infile);
	fclose(outfile);
	return 0;
}

FILE* decrypt_tsc(const char *filename) {
	long fileSize;
	unsigned char key;
	FILE *infile, *outfile;
	if((infile = fopen(filename, "rb")) == NULL) {
		printf("Failed to open input file.\n");
		exit(1);
	}
	if((outfile = fopen("tscomp.tmp", "wb")) == NULL) {
		printf("Failed to create temporary file.\n");
		exit(1);
	}
	// Get the size of the file
	fseek(infile, 0, SEEK_END);
	fileSize = ftell(infile);
	// Obtain the key from the center of the file
	fseek(infile, fileSize / 2, SEEK_SET);
	fread(&key, 1, 1, infile);
	// Copy over bytes subtracting the key
	fseek(infile, 0, SEEK_SET);
	for(long i = 0; i < fileSize; i++) {
		unsigned char c;
		fread(&c, 1, 1, infile);
		if(i != fileSize / 2) c -= key;
		fwrite(&c, 1, 1, outfile);
	}
	// Close the files
	fclose(infile);
	fclose(outfile);
	// Reopen decrypted file in read mode and return it
	return fopen("tscomp.tmp", "rb");
}

void do_script(FILE *fin, FILE * fout) {
	unsigned char eventCount = 0;
	// Place holder to store the real count when finished
	fwrite(&eventCount, 1, 1, fout);
	while(!feof(fin) && eventCount < MAX_EVENTS) {
		char c = fgetc(fin);
		if(c == SYM_EVENT) {
			unsigned short sym = OP_EVENT;
			fwrite(&sym, 1, 2, fout);
			do_event(fin, fout);
			eventCount++;
		} else if(c == EOF) {
			break;
		} else if(c != '\n' && c != '\r') {
			//printf("Debug: Char '%c' while looking for events.\n", c);
		}
	}
	// Event count at beginning of file
	fseek(fout, 0, SEEK_SET);
	fwrite(&eventCount, 1, 1, fout);
}

void do_event(FILE *fin, FILE *fout) {
	bool msgWindowOpen;
	unsigned short id, commandCount;
	msgWindowOpen = FALSE;
	id = read_number(fin);
	commandCount = 0;
	fwrite(&id, 1, 2, fout);
	while(!feof(fin)) {
		char c = fgetc(fin);
		if(c == SYM_COMMAND) {
			commandCount++;
			unsigned short flags = do_command(fin, fout);
			if(flags & CFLAG_MSGOPEN) msgWindowOpen = TRUE;
			else if(flags & CFLAG_MSGCLOSE) msgWindowOpen = FALSE;
			if(flags & CFLAG_END) break;
		} else if(is_valid_char(c)) {
			if(msgWindowOpen) {
				fwrite(&c, 1, 1, fout);
			} else if(c != '\n') {
				printf("Warning: Printable text char '%c' is never displayed.\n", c);
			}
		} else if(c == SYM_EVENT) {
			if(commandCount != 0) {
				printf("Warning: Non-empty event #%04hu has no ending!\n", id);
			}
			fseek(fin, -1, SEEK_CUR);
			break;
		} else {
			if(c == '\r') continue;
			printf("Warning: Invalid char '%c' in event #%04hu.\n", c, id);
		}
	}
}

unsigned short do_command(FILE *fin, FILE *fout) {
	unsigned char opcode, params;
	unsigned short flags;
	char str[LEN_COMMAND + 1];
	// Find command from 3 character string
	fread(&str, 1, LEN_COMMAND, fin);
	str[LEN_COMMAND] = '\0';
	for(int i = 0; i < COMMAND_COUNT; i++) {
		if(strcmp(str, command_table[i].name) == 0) {
			opcode = command_table[i].opcode;
			params = command_table[i].params;
			flags = command_table[i].flags;
			break;
		}
	}
	//printf("TRACE: Command %s matches %hhu.\n", str, opcode);
	fwrite(&opcode, 1, 1, fout);
	// Parse parameters
	for(int i = 0; i < params; i++) {
		short val = read_number(fin);
		fwrite(&val, 1, 2, fout);
		// Parameters should be separated by ':', CS doesn't actually check though
		if(i != params - 1) {
			if(fgetc(fin) != ':') {
				printf("Warning: No ':' between parameters.\n");
			}
		}
	}
	return flags;
}

unsigned short read_number(FILE *file) {
	char str[LEN_NUMBER + 1];
	fread(&str, 1, LEN_NUMBER, file);
	str[LEN_NUMBER] = '\0';
	// Make sure the string is 4 digits
	for(int i = 0; i < LEN_NUMBER; i++) {
		if(str[i] < '0' || str[i] > '9') {
			// Parameters should be 4 digits, but sometimes they are less
			if(i < 4 && str[i] == ':') {
				printf("Warning: Parameter is less than 4 digits.\n");
				str[i] = '\0';
				fseek(file, -1, SEEK_CUR);
			} else {
				printf("Error: Expected number.\n");
				exit(1);
			}
		}
	}
	return atoi(str);
}

bool is_valid_char(char c) {
	for(int i = 0; i < strlen(ValidChars); i++) {
		if(c == ValidChars[i]) return TRUE;
	}
	return FALSE;
}
