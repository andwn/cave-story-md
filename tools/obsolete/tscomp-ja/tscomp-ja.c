/*
 * tscomp-ja - Converts a TSC file into bytecode usable by CSMD
 * 
 * Compile:
 * gcc tscomp-ja.c -o tscomp-ja
 * 
 * Usage:
 * ./tscomp-ja [-t] <tsc file[s ...]> <kanji list file>
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EVENTS 		106
#define COMMAND_COUNT 	93

// Chars starting with 0xE0-0xFE are a 2 byte sequence
#define MULTIBYTE_BEGIN	0xE0
#define MULTIBYTE_END	0xFE

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

#define OP_EVENT		0xFFFF

enum {
	CT_COMMAND,
	CT_EVENT,
	CT_ASCII,
	CT_KANJI,
	CT_SKIP,
	CT_SKIP2BYTE,
	CT_INVALID,
	CT_INVALID2BYTE,
};

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

uint8_t *tsc = NULL;
uint16_t tscSize = 0;
uint16_t pc = 0;

uint16_t *kanji = NULL;
uint16_t kanjiCount = 0;

bool trace = false;

void tsc_open(const char *filename) {
	if(trace) printf("TRACE: Opening '%s'\n", filename);
	
	FILE *tscFile = fopen(filename, "rb");
	fseek(tscFile, 0, SEEK_END);
	tscSize = ftell(tscFile);
	tsc = malloc(tscSize);
	fseek(tscFile, 0, SEEK_SET);
	fread(tsc, 1, tscSize, tscFile);
	fclose(tscFile);
	// Obtain the key from the center of the file
	uint8_t key = tsc[tscSize / 2];
	// Apply key to all bytes except where the key itself was
	for(uint16_t i = 0; i < tscSize; i++) {
		if(i != tscSize / 2) tsc[i] -= key;
		if(trace) fputc(tsc[i], stdout);
	}
	pc = 0;
}

void tsc_close() {
	if(trace) printf("TRACE: Closing TSC\n");
	
	if(tsc) {
		free(tsc);
		tsc = NULL;
	}
}

uint8_t get_char_type() {
	// Command symbol '<'
	if(tsc[pc] == SYM_COMMAND) return CT_COMMAND;
	// Event symbol '#'
	if(tsc[pc] == SYM_EVENT) return CT_EVENT;
	// First check the valid ASCII chars list
	for(uint16_t i = 0; i < strlen(ValidChars); i++) {
		if(tsc[pc] == ValidChars[i]) return CT_ASCII;
	}
	// Skip '\r'
	if(tsc[pc] == '\r') return CT_SKIP;
	// Double byte char?
	if((tsc[pc] >= 0x81 && tsc[pc] <= 0x9F) || (tsc[pc] >= 0xE0 && tsc[pc] <= 0xFC)) {
		//uint16_t wc = (tsc[pc] << 8) | tsc[pc+1];
		// Kanji
		//for(int i = 0; i < kanjiCount; i++) {
		//	if(wc == kanji[i]) return CT_KANJI;
		//}
		//return CT_INVALID2BYTE;
		return CT_KANJI;
	}
	return CT_INVALID;
}

uint16_t read_number() {
	char str[LEN_NUMBER + 1];
	for(int i = 0; i < 4; i++) {
		char c = tsc[pc++];
		if(!isdigit(c)) {
			printf("WARN: Parameter should be 4 digits but is only %hu.\n", i);
			str[i] = '\0';
			pc--;
			break;
		}
		str[i] = c;
	}
	str[LEN_NUMBER] = '\0';
	
	return atoi(str);
}

uint16_t do_command(FILE *fout) {
	uint8_t opcode = 0xDB; // NOP
	uint8_t params = 0;
	uint16_t flags = 0;
	char str[LEN_COMMAND + 1];
	// Find command from 3 character string
	for(int i = 0; i < LEN_COMMAND; i++) str[i] = tsc[pc++];
	str[LEN_COMMAND] = '\0';
	for(int i = 0; i < COMMAND_COUNT; i++) {
		if(strcmp(str, command_table[i].name) == 0) {
			opcode = command_table[i].opcode;
			params = command_table[i].params;
			flags = command_table[i].flags;
			break;
		}
	}
	if(opcode == 0xDB) {
		printf("ERROR: Bad command '%s'\n", str);
		return flags;
	}
	
	if(trace) printf("TRACE: Command: '<%s", str);
	
	//printf("TRACE: Command %s matches %hhu.\n", str, opcode);
	fwrite(&opcode, 1, 1, fout);
	// Parse parameters
	for(int i = 0; i < params; i++) {
		uint16_t val = read_number();
		
		if(trace) printf("%04hu", val);
		
		fwrite(&val, 1, 2, fout);
		// Parameters should be separated by ':', CS doesn't actually check though
		if(i != params - 1) {
			if(tsc[pc++] != ':') {
				if(trace) printf("\n");
				printf("WARN: No ':' between parameters.\n");
			} else {
				if(trace) printf(":");
			}
		}
	}
	
	if(trace) printf("'\n");
	
	return flags;
}

void do_event(FILE *fout) {
	bool msgWindowOpen = false;
	uint16_t id = read_number();
	uint16_t commandCount = 0;
	
	if(trace) printf("TRACE: Event: #%04hu\n", id);
	
	fwrite(&id, 1, 2, fout);
	while(pc < tscSize) {
		uint8_t ct = get_char_type();
		switch(ct) {
			case CT_COMMAND: {
				pc++;
				commandCount++;
				uint16_t flags = do_command(fout);
				if(flags & CFLAG_MSGOPEN) msgWindowOpen = true;
				else if(flags & CFLAG_MSGCLOSE) msgWindowOpen = false;
				if(flags & CFLAG_END) return;
			}
			break;
			case CT_EVENT: {
				if(commandCount != 0) {
					printf("WARN: Non-empty event #%04hu has no ending!\n", id);
				}
				//pc++;
				return;
			}
			break;
			case CT_ASCII: {
				if(msgWindowOpen) {
					fwrite(&tsc[pc], 1, 1, fout);
				} else if(tsc[pc] != '\n') {
					printf("WARN: Printable text char '%c' is never displayed.\n", tsc[pc]);
				}
				pc++;
			}
			break;
			case CT_KANJI: {
				uint16_t wc = (tsc[pc] << 8) | tsc[pc+1];
				int k;
				for(k = 0; k < kanjiCount; k++) {
					if(wc == kanji[k]) break;
				}
				if(k == kanjiCount) {
					printf("WARN: Unknown kanji: 0x%04hx\n", wc);
				} else {
					// Index by appearance in the list, and fit that number into banks
					// of 0x60 for each 'page'
					uint8_t page = k / 0x60;
					uint8_t word = k % 0x60;
					uint8_t b = MULTIBYTE_BEGIN + page; // First byte
					fwrite(&b, 1, 1, fout);
					b = word + 0x20; // Second byte
					fwrite(&b, 1, 1, fout);
				}
				pc += 2;
			}
			break;
			case CT_SKIP: pc++; break;
			case CT_SKIP2BYTE: pc += 2; break;
			case CT_INVALID: {
				printf("WARN: Invalid character: '%c' (0x%02hx)\n", tsc[pc], tsc[pc]);
				pc++;
			}
			break;
			case CT_INVALID2BYTE: {
				uint16_t wc = (tsc[pc] << 8) | tsc[pc+1];
				printf("WARN: Invalid double byte character: 0x%04hx\n", wc);
				pc += 2;
			}
			break;
		}
	}
}

void do_script(FILE *fout) {
	uint8_t eventCount = 0;
	// Place holder to store the real count when finished
	fwrite(&eventCount, 1, 1, fout);
	while(pc < tscSize && eventCount < MAX_EVENTS) {
		uint8_t c = tsc[pc++];
		if(c == SYM_EVENT) {
			uint16_t sym = OP_EVENT;
			fwrite(&sym, 1, 2, fout);
			do_event(fout);
			eventCount++;
		} else if(c != '\n' && c != '\r') {
			//printf("Debug: Char '%c' while looking for events.\n", c);
		}
	}
	// Event count at beginning of file
	fseek(fout, 0, SEEK_SET);
	fwrite(&eventCount, 1, 1, fout);
}

int main(int argc,char *argv[]) {
	if(argc < 3) {
		printf("Usage: tscomp-ja [-t] <tsc file [more tsc files ...]> <kanji list file>\n");
		return 0;
	}
	if(argv[1][0] == '-' && argv[1][1] == 't' && argv[1][2] == '\0') {
		trace = true;
	}
	
	// Load the kanji list
	FILE *kfile = fopen(argv[argc-1], "rb");
	if(!kfile) {
		printf("ERROR: Failed to open '%s'.\n", argv[argc-1]);
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
	
	// Go through each of the TSC files given
	char outstr[256];
	for(int i = 1 + trace; i < argc-1; i++) {
		tsc_open(argv[i]);
		
		sprintf(outstr, "%s", argv[i]);
		outstr[strlen(outstr)-1] = 'b';
		FILE *outfile = fopen(outstr, "wb");
		do_script(outfile);
		fclose(outfile);
		
		tsc_close();
	}
	
	free(kanji);
	
	return EXIT_SUCCESS;
}
