/*******************************************************************************
 * hppgen - Generates a header for a collection of C/C++ files
 *   As this program is currently only meant to generate function definitions
 *   for the NPC act/ai code in CSMD, it does not contain a full C parser.
 *   So don't use it in other projects unless you plan on messing with that.
 *
 * Compile:
 * cc hppgen.c -o hppgen
 *
 * Usage:
 * ./hppgen <out h file> <in c file> [more c files...]
 ******************************************************************************/

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_C_LEX_C_DECIMAL_INTS   Y
#define STB_C_LEX_C_HEX_INTS       Y
#define STB_C_LEX_C_OCTAL_INTS     Y
#define STB_C_LEX_C_DECIMAL_FLOATS Y
#define STB_C_LEX_C99_HEX_FLOATS   N //  "0x{hex}+(.{hex}*)?[pP][-+]?{hex}+     CLEX_floatlit
#define STB_C_LEX_C_IDENTIFIERS    Y
#define STB_C_LEX_C_DQ_STRINGS     Y
#define STB_C_LEX_C_SQ_STRINGS     N //  single-quote-delimited strings with escapes  CLEX_ssstring
#define STB_C_LEX_C_CHARS          Y
#define STB_C_LEX_C_COMMENTS       Y
#define STB_C_LEX_CPP_COMMENTS     Y
#define STB_C_LEX_C_COMPARISONS    Y
#define STB_C_LEX_C_LOGICAL        Y
#define STB_C_LEX_C_SHIFTS         Y
#define STB_C_LEX_C_INCREMENTS     Y
#define STB_C_LEX_C_ARROW          Y
#define STB_C_LEX_EQUAL_ARROW      N //  "=>"  CLEX_eqarrow
#define STB_C_LEX_C_BITWISEEQ      Y
#define STB_C_LEX_C_ARITHEQ        Y

#define STB_C_LEX_PARSE_SUFFIXES   Y
#define STB_C_LEX_DECIMAL_SUFFIXES "uUlL"
#define STB_C_LEX_HEX_SUFFIXES     "uUlL"
#define STB_C_LEX_OCTAL_SUFFIXES   "uUlL"
#define STB_C_LEX_FLOAT_SUFFIXES   "fF"

#define STB_C_LEX_0_IS_EOF            Y
#define STB_C_LEX_INTEGERS_AS_DOUBLES N
#define STB_C_LEX_MULTILINE_DSTRINGS  N // allow newlines in double-quoted strings
#define STB_C_LEX_MULTILINE_SSTRINGS  N // allow newlines in single-quoted strings
#define STB_C_LEX_USE_STDLIB          N
#define STB_C_LEX_DOLLAR_IDENTIFIER   Y
#define STB_C_LEX_FLOAT_NO_DECIMAL    Y

#define STB_C_LEX_DEFINE_ALL_TOKEN_NAMES N

#define STB_C_LEX_DISCARD_PREPROCESSOR Y

#define STB_C_LEXER_DEFINITIONS

#define STB_C_LEXER_IMPLEMENTATION
#include "inc/stb_c_lexer.h"

// Parser state
enum {
	PS_DECL,
	PS_FUNC_PARAMS,
	PS_IGNORE,
	PS_ASSIGNMENT,
};

#define STORAGE_SIZE 0x40000 // 256K
char lexer_storage[STORAGE_SIZE];

// カレーうどんは好きなのか？
char *slurp(const char *fn, size_t *size) {
	FILE *file = fopen(fn, "rb");
	if(!file) {
		printf("ERROR: Failed to open '%s'.\n", fn);
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char *buf = malloc(*size + 1);
	if(!buf) {
		printf("ERROR: Failed to allocate '%lu' bytes.\n", *size);
		fclose(file);
		return NULL;
	}
	fread(buf, 1, *size, file);
	buf[*size] = 0;
	fclose(file);
	return buf;
}

int main(int argc, char *argv[]) {
	if(argc < 3) {
		puts("Usage: hppgen <out h file> <in c file> [more c files...]");
		return EXIT_FAILURE;
	}
	FILE *outfile = fopen(argv[1], "w");
	if(!outfile) {
		printf("ERROR: Failed to open '%s' for writing\n", argv[1]);
		return EXIT_FAILURE;
	}

	// Header guard
	char guard[256] = {};
	{
		for(size_t i = 0; i < strlen(argv[1]); i++) {
			char c   = argv[1][i];
			guard[i] = isalnum(c) ? toupper(c) : '_';
		}
		fprintf(outfile, "#ifndef %s\n#define %s\n\n", guard, guard);
	}

	for(int i = 2; i < argc; i++) {
		size_t size;
		char  *srcfile = slurp(argv[i], &size);
		if(!srcfile) return EXIT_FAILURE;

		fprintf(outfile, "/* ---- %s ---- */\n", argv[i]);

		stb_lexer lex;
		stb_c_lexer_init(&lex, srcfile, srcfile + size, lexer_storage, STORAGE_SIZE);
		int  scope_level   = 0;       // Current scope level, parsing only happens at level 0
		int  paren_level   = 0;       // Same for parentheses
		int  state         = PS_DECL; // Parser state
		char def_line[256] = {};      // Line to write in header for forward definition
		while(stb_c_lexer_get_token(&lex)) {
			if(lex.token == '{') scope_level++;
			if(lex.token == '}') {
				scope_level--;
				if(scope_level == 0) {
					def_line[0] = 0;
					state       = PS_DECL;
					continue;
				}
			}
			if(scope_level > 0) continue;

			if(lex.token == '(') paren_level++;
			if(lex.token == ')') paren_level--;

			if(lex.token == ';') {
				if(def_line[0] && state != PS_IGNORE) {
					fprintf(outfile, "extern %s;\n", def_line);
				}
				def_line[0] = 0;
				state       = PS_DECL;
				continue;
			}
			if(state == PS_IGNORE) continue;

			switch(lex.token) {
			case '*': // Pointers leave the type name in lex.string
				strcat(def_line, " *");
				continue;
			case '[': // Array brackets
				strcat(def_line, "[");
				continue;
			case ']': // Array brackets
				strcat(def_line, "]");
				continue;
			case ',': // Another parameter, or declaration
				strcat(def_line, ", ");
				continue;
			}
			if(state == PS_ASSIGNMENT) continue;

			switch(state) {
			case PS_DECL: {
				switch(lex.token) {
				case CLEX_id: // Identifier
					if(strcmp(lex.string, "static") == 0 || strcmp(lex.string, "extern") == 0) {
						def_line[0] = 0;
						state       = PS_IGNORE;
						break;
					}
					if(def_line[0] && isalnum(def_line[strlen(def_line) - 1])) {
						strcat(def_line, " ");
					}
					strcat(def_line, lex.string);
					break;
				case '(': // Start of function parameters
					strcat(def_line, "(");
					state = PS_FUNC_PARAMS;
					break;
				case '=': // Assignment, parse left side only
					state = PS_ASSIGNMENT;
					break;
				case CLEX_intlit: // Integer literal
				{
					char num[16];
					snprintf(num, 16, "%ld", lex.int_number);
					strcat(def_line, num);
				} break;
				}
			} break;

			case PS_FUNC_PARAMS: {
				switch(lex.token) {
				case CLEX_id: // Identifier
					if(isalnum(def_line[strlen(def_line) - 1])) { strcat(def_line, " "); }
					strcat(def_line, lex.string);
					break;

				case ')': // All done?
					strcat(def_line, ")");
					if(paren_level == 0) {
						if(def_line[strlen(def_line) - 1] == '(') {
							strcat(def_line, "void"); // Moffitt might whine if I don't do this
						}
						fprintf(outfile, "%s;\n", def_line);
						def_line[0] = 0;
						state       = PS_IGNORE;
					}
					break;
				}
			} break;
			}
		}
		free(srcfile);
		fprintf(outfile, "\n");
	}

	fprintf(outfile, "#endif // %s\n", guard);
	fclose(outfile);
	return EXIT_SUCCESS;
}
