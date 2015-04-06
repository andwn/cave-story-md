/*
 * tscomp.c
 *
 *  Created on: Feb 22, 2015
 *      Author: aderosier
 *
 *  Converts Cave Story TSC files to a smaller binary format
 *  This is horribly written and needs an overhaul
 */

#include <stdio.h>
#include <string.h>

#define true 1
#define false 0

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;

typedef unsigned char bool;
//enum { false, true };

struct {
	char text[4];
	u8 args;
} instructions[92] = {
	// Message related (0-11)
	{ "MSG", 0 }, // Normal message window bottom
	{ "MS2", 0 }, // Invisible message window top
	{ "MS3", 0 }, // Normal message window top
	{ "CLO", 0 }, // Close message window
	{ "CLR", 0 }, // Clear message window
	{ "NUM", 1 }, // Print a number to the message window
	{ "GIT", 1 }, // Show item graphic
	{ "FAC", 1 }, // Show face
	{ "CAT", 0 }, // Show message immediately
	{ "SAT", 0 }, // Same as CAT
	{ "TUR", 0 }, // Show remainder of message immediately
	{ "YNJ", 1 }, // Prompts Yes/No, jump if no is selected
	// Commands that end an event (12-17)
	{ "END", 0 }, // End event, return control, close msg window
	{ "EVE", 1 }, // Jump to another event
	{ "TRA", 4 }, // Teleport player to another map
	{ "INI", 0 }, // Reset memory and restart game (0x10)
	{ "LDP", 0 }, // Load save data
	{ "ESC", 0 }, // Return to title screen
	// Music / Sound (18-25)
	{ "CMU", 1 }, // Change music
	{ "FMU", 0 }, // Fade music
	{ "RMU", 0 }, // Resume music
	{ "SOU", 1 }, // Play a sound
	{ "SPS", 0 }, // Start the propeller sound
	{ "CPS", 0 }, // Stop the propeller sound
	{ "SSS", 1 }, // Start the stream sound
	{ "CSS", 0 }, // Stop the stream sound
	// Wait Commands
	{ "NOD", 0 }, // Wait for player input before resuming
	{ "WAI", 1 }, // Wait an amount of time
	{ "WAS", 0 }, // Wait for player to be on the ground
	// Player Control
	{ "MM0", 0 }, // Stop player movement
	{ "MOV", 2 }, // Move the player
	{ "MYB", 1 }, // Makes the player hop (0x20)
	{ "MYD", 1 }, // Set player direction
	{ "UNI", 1 }, // Change movement type
	{ "UNJ", 2 }, // Jump on movement type
	{ "KEY", 0 }, // Lock player controls and hide HUD
	{ "PRI", 0 }, // Lock player controls until KEY or END
	{ "FRE", 0 }, // Give control back to the player
	{ "HMC", 0 }, // Hide the player
	{ "SMC", 0 }, // Show the player
	{ "LI+", 1 }, // Restore health
	{ "ML+", 1 }, // Increase max health
	// NPC / Entities / Boss
	{ "ANP", 3 }, // Animate NPC
	{ "CNP", 3 }, // Change NPC
	{ "MNP", 4 }, // Move NPC
	{ "DNA", 1 }, // Delete all of a certain NPC
	{ "DNP", 1 }, // Delete specific NPC
	{ "INP", 3 }, // Initialize / change entity (0x30)
	{ "SNP", 4 }, // Also creates an entity
	{ "BOA", 1 }, // Change boss animation / state
	{ "BSL", 1 }, // Start boss fight with entity
	{ "NCJ", 2 }, // Jump to event if NPC exists
	{ "ECJ", 2 }, // Jump to event if any entity exists
	// Arms
	{ "AE+", 0 }, // Refill all ammo
	{ "ZAM", 0 }, // Takes away all ammo
	{ "AM+", 2 }, // Give player weapon and/or ammo
	{ "AM-", 1 }, // Remove player weapon
	{ "TAM", 3 }, // Trade weapons
	{ "AMJ", 2 }, // Jump to event if have weapon
	// Equps
	{ "EQ+", 1 }, // Equip item
	{ "EQ-", 1 }, // Unequip item
	// Items
	{ "IT+", 1 }, // Give item
	{ "IT-", 1 }, // Remove item
	{ "ITJ", 2 }, // Jump to event if have item (0x40)
	// Flags
	{ "FL+", 1 }, // Set flag
	{ "FL-", 1 }, // Clear flag
	{ "FLJ", 2 }, // Jump to event if flag is true
	{ "SK+", 1 }, // Enable skipflag
	{ "SK-", 1 }, // Disable skipflag
	{ "SKJ", 2 }, // Jump on skipflag (boss rematch)
	// Camera
	{ "FOB", 2 }, // Focus on boss
	{ "FOM", 1 }, // Focus on me
	{ "FON", 2 }, // Focus on NPC
	{ "QUA", 1 }, // Shake camera for quake effect
	// Screen Effects
	{ "FAI", 1 }, // Fade in
	{ "FAO", 1 }, // Fade out
	{ "FLA", 0 }, // Flash screen white
	// Room
	{ "MLP", 0 }, // Show the map
	{ "MNA", 0 }, // Display room name
	{ "CMP", 3 }, // Change room tile (0x50)
	{ "MP+", 1 }, // Enable map flag
	{ "MPJ", 1 }, // Jump if map flag enabled
	// Misc
	{ "CRE", 0 }, // Rolls the credits
	{ "SIL", 1 }, // Show illustration (credits)
	{ "CIL", 0 }, // Clear illustration (credits)
	{ "SLP", 0 }, // Show teleporter menu
	{ "PS+", 2 }, // Portal slot +
	{ "SVP", 0 }, // Saves the game
	{ "STC", 0 }, // Save time counter
	{ "XX1", 1 }, // Island control
};

// Declarations
FILE *infile, *outfile;
char outfn[256], buf[18000], key;
int len, cursor, mode;
bool msgWindowOpen;

void skip() {
	cursor++;
}

void write(u8 c) {
	fwrite(&c, 1, 1, outfile);
	cursor++;
}

void writenum() {
	u16 num =
			(buf[cursor+0] - 0x30) * 1000 +
			(buf[cursor+1] - 0x30) * 100 +
			(buf[cursor+2] - 0x30) * 10 +
			(buf[cursor+3] - 0x30);
	// Moto CPUs use big endian, reverse lines if there are problems
	fwrite(&num, 1, 2, outfile);
	//fwrite(&num+1, 1, 1, outfile);
	cursor += 4;
}

void writecmd(u8 c) {
	fwrite(&c, 1, 1, outfile);
	cursor += 3;
}

bool is_text(u8 c) {
	return (c >= 0x20 && c < 0x7F);
}

void parse_text() {
	printf("Text: ");
	while(true) {
		if(buf[cursor] == '<') {
			printf("\n");
			break;
		} else if(buf[cursor] == '\r') {
			skip();
		} else if(is_text(buf[cursor]) || buf[cursor] == '\n') {
			printf("%c", buf[cursor]);
			write(buf[cursor]);
		} else {
			printf("Aw hell no what is \"%c\"?", buf[cursor]);
			skip();
		}
	}
}
// Found a '<', look in the instructions table for how to convert
// the following instruction
bool parse_command() {
	// Figure out which command this is
	char cmd[4] = { buf[cursor], buf[cursor+1], buf[cursor+2], '\0' };
	for(int i = 0; i < 92; i++) {
		if(strcmp(cmd, instructions[i].text) == 0) {
			// Found a match! write the ID
			// There are 91 commands and extended ascii is never used
			// by the game, so we use those for commands instead
			writecmd(i + 0x80);
			// Take care of any args
			for(int a = 0; a < instructions[i].args; a++) {
				writenum();
				if(buf[cursor] == ':') skip();
			}
			if(i >= 0 && i < 3) {
				msgWindowOpen = true;
			} else if(i == 3) {
				msgWindowOpen = false;
			} else if(i >= 12 && i < 18) {
				msgWindowOpen = false;
				return true;
			}
			return false;
		}
	}
	// Didn't find anything. That shouldn't happen
	printf("Don't know how to \"%s\" something.\n", cmd);
	return false;
}

// Found an event, default to display message until encountering
// a '<' for commands
void find_commands() {
	while(true) {
		if(buf[cursor] == '#') {
			break;
		} else if(buf[cursor] == '<') {
			printf("Found a command! %.4s\n", &buf[cursor]);
			skip(); //write('<');
			if(parse_command()) break;
		} else if(cursor >= len) {
			return;
		} else if(msgWindowOpen) {
			parse_text();
		} else {
			skip();
		}
	}
}

// Looking for an event or global instruction
void find_events() {
	switch(buf[cursor]) {
	case '#':
		printf("Found an event! %.5s\n", &buf[cursor]);
		// Hash (#), Event number (4 digits)
		write(0xFF);
		write(0xFF);
		cursor--;
		writenum();
		//skip(); // Newline char
		find_commands();
		break;
	default:
		skip();
		break;
	}
}

void parse() {
	// Count the number of events first
	u8 eventCount = 0;
	for(int i = cursor; i < len; i++) {
		if(buf[i] == '#') eventCount++;
	}
	write(eventCount);
	cursor = 0; // Thanks Obama
	// Then parse through them
	printf("Begin parsing\n");
	while(cursor < len) {
		find_events();
	}
	printf("All done!\n");
}

int main(int argc, char *argv[]) {
	// Open in/out files
	if(argc != 2) return 1;
	infile = fopen(argv[1], "rb");
	if(!infile) return 2;
	strcpy(outfn, argv[1]);
	strcat(outfn, ".mds");
	outfile = fopen(outfn, "wb");
	if(!outfile) return 3;
	// Find the key at "file_length / 2"
	fseek(infile, 0, SEEK_END);
	len = ftell(infile);
	fseek(infile, len / 2, SEEK_SET);
	fread(&key, 1, 1, infile);
	// Read "decrypted" file into buf
	fseek(infile, 0, SEEK_SET);
	fread(buf, 1, len, infile);
	for(int i = 0; i < len; i++) {
		if(buf[i] != key) buf[i] -= key;
	}
	buf[len] = '\0';
	fclose(infile);
	// Do shit
	cursor = 0;
	msgWindowOpen = false;
	parse();
	fclose(outfile);
	//getc(stdin);
	return 0;
}
