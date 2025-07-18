// Converts some M68K asm symbols for GAS (;, $, %)
// Prints output to stdout
// Written because the awk scripts were stripping double quotes
//
// Build:
//  gcc gasify.c -o gasify
//
// Example uses in Makefile:
//  ASFLAGS = -m68000 --register-prefix-optional --bitwise-or
//  gasify $< | as $(ASFLAGS) -c -o $@ -
//
//  ASFLAGS = -m68000 -x assembler-with-cpp -Wa,--register-prefix-optional,--bitwise-or
//  gasify $< | gcc $(ASFLAGS) -c -o $@ -

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_LEN 1024

int main(int argc, char *argv[]) {
	if(argc != 2) {
		puts("Usage: gasify <input file>");
		return EXIT_FAILURE;
	}
	FILE *f = fopen(argv[1], "r");
	if(!f) {
		printf("ERROR: Failed to read '%s'\n", argv[1]);
		return EXIT_FAILURE;
	}

    char line[LINE_LEN], out[LINE_LEN];
    while(fgets(line, LINE_LEN, f)) {
        bool in_comment = false;
        bool in_quote = false;
        int oi = 0;
        for(int i = 0; i < strlen(line); i++) {
            if(line[i] == '\r' || line[i] == '\n') {
                continue;
            } else if(in_comment) {
                out[oi++] = line[i];
            } else if(in_quote) {
                out[oi++] = line[i];
                if(line[i] == '"') in_quote = false;
            } else if(line[i] == '"') {
                out[oi++] = line[i];
                in_quote = true;
            } else if(line[i] == ';') {
                out[oi++] = line[i];
                out[oi++] = '#';
                in_comment = true;
            } else if(line[i] == '$') {
                out[oi++] = '0';
                out[oi++] = 'x';
            } else if(line[i] == '%') {
                out[oi++] = '0';
                out[oi++] = 'b';
            } else {
                out[oi++] = line[i];
            }
        }
        out[oi] = 0;
        puts(out);
    }
    fclose(f);

    return EXIT_SUCCESS;
}
