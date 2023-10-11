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

#define STB_C_LEX_C_DECIMAL_INTS    Y   //  "0|[1-9][0-9]*"                        CLEX_intlit
#define STB_C_LEX_C_HEX_INTS        Y   //  "0x[0-9a-fA-F]+"                       CLEX_intlit
#define STB_C_LEX_C_OCTAL_INTS      Y   //  "[0-7]+"                               CLEX_intlit
#define STB_C_LEX_C_DECIMAL_FLOATS  Y   //  "[0-9]*(.[0-9]*([eE][-+]?[0-9]+)?)     CLEX_floatlit
#define STB_C_LEX_C99_HEX_FLOATS    N   //  "0x{hex}+(.{hex}*)?[pP][-+]?{hex}+     CLEX_floatlit
#define STB_C_LEX_C_IDENTIFIERS     Y   //  "[_a-zA-Z][_a-zA-Z0-9]*"               CLEX_id
#define STB_C_LEX_C_DQ_STRINGS      Y   //  double-quote-delimited strings with escapes  CLEX_dqstring
#define STB_C_LEX_C_SQ_STRINGS      N   //  single-quote-delimited strings with escapes  CLEX_ssstring
#define STB_C_LEX_C_CHARS           Y   //  single-quote-delimited character with escape CLEX_charlits
#define STB_C_LEX_C_COMMENTS        Y   //  "/* comment */"
#define STB_C_LEX_CPP_COMMENTS      Y   //  "// comment to end of line\n"
#define STB_C_LEX_C_COMPARISONS     Y   //  "==" CLEX_eq  "!=" CLEX_noteq   "<=" CLEX_lesseq  ">=" CLEX_greatereq
#define STB_C_LEX_C_LOGICAL         Y   //  "&&"  CLEX_andand   "||"  CLEX_oror
#define STB_C_LEX_C_SHIFTS          Y   //  "<<"  CLEX_shl      ">>"  CLEX_shr
#define STB_C_LEX_C_INCREMENTS      Y   //  "++"  CLEX_plusplus "--"  CLEX_minusminus
#define STB_C_LEX_C_ARROW           Y   //  "->"  CLEX_arrow
#define STB_C_LEX_EQUAL_ARROW       N   //  "=>"  CLEX_eqarrow
#define STB_C_LEX_C_BITWISEEQ       Y   //  "&="  CLEX_andeq    "|="  CLEX_oreq     "^="  CLEX_xoreq
#define STB_C_LEX_C_ARITHEQ         Y   //  "+="  CLEX_pluseq   "-="  CLEX_minuseq
                                        //  "*="  CLEX_muleq    "/="  CLEX_diveq    "%=" CLEX_modeq
                                        //  if both STB_C_LEX_SHIFTS & STB_C_LEX_ARITHEQ:
                                        //                      "<<=" CLEX_shleq    ">>=" CLEX_shreq

#define STB_C_LEX_PARSE_SUFFIXES    Y   // letters after numbers are parsed as part of those numbers, and must be in suffix list below
#define STB_C_LEX_DECIMAL_SUFFIXES  "uUlL"  // decimal integer suffixes e.g. "uUlL" -- these are returned as-is in string storage
#define STB_C_LEX_HEX_SUFFIXES      "uUlL"  // e.g. "uUlL"
#define STB_C_LEX_OCTAL_SUFFIXES    "uUlL"  // e.g. "uUlL"
#define STB_C_LEX_FLOAT_SUFFIXES    "fF"  //

#define STB_C_LEX_0_IS_EOF             Y  // if Y, ends parsing at '\0'; if N, returns '\0' as token
#define STB_C_LEX_INTEGERS_AS_DOUBLES  N  // parses integers as doubles so they can be larger than 'int', but only if STB_C_LEX_STDLIB==N
#define STB_C_LEX_MULTILINE_DSTRINGS   N  // allow newlines in double-quoted strings
#define STB_C_LEX_MULTILINE_SSTRINGS   N  // allow newlines in single-quoted strings
#define STB_C_LEX_USE_STDLIB           Y  // use strtod,strtol for parsing #s; otherwise inaccurate hack
#define STB_C_LEX_DOLLAR_IDENTIFIER    Y  // allow $ as an identifier character
#define STB_C_LEX_FLOAT_NO_DECIMAL     Y  // allow floats that have no decimal point if they have an exponent

#define STB_C_LEX_DEFINE_ALL_TOKEN_NAMES  N   // if Y, all CLEX_ token names are defined, even if never returned
                                              // leaving it as N should help you catch config bugs

#define STB_C_LEX_DISCARD_PREPROCESSOR    Y   // discard C-preprocessor directives (e.g. after prepocess
                                              // still have #line, #pragma, etc)

//#define STB_C_LEX_ISWHITE(str)    ... // return length in bytes of whitespace characters if first char is whitespace

#define STB_C_LEXER_DEFINITIONS         // This line prevents the header file from replacing your definitions

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
char* slurp(const char* fn, size_t* size) {
    FILE* file = fopen(fn, "rb");
    if (!file) {
        printf("ERROR: Failed to open '%s'.\n", fn);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buf = malloc(*size + 1);
    if (!buf) {
        printf("ERROR: Failed to allocate '%lu' bytes.\n", *size);
        fclose(file);
        return NULL;
    }
    fread(buf, 1, *size, file);
    buf[*size] = 0;
    fclose(file);
    return buf;
}

int main(int argc,char *argv[]) {
    if (argc < 3) {
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
            char c = argv[1][i];
            guard[i] = isalnum(c) ? toupper(c) : '_';
        }
        fprintf(outfile, "#ifndef %s\n#define %s\n\n", guard, guard);
    }

    for(int i = 2; i < argc; i++) {
        size_t size;
        char *srcfile = slurp(argv[i], &size);
        if(!srcfile) return EXIT_FAILURE;

        fprintf(outfile, "/* ---- %s ---- */\n", argv[i]);
        
        stb_lexer lex;
        stb_c_lexer_init(&lex, srcfile, srcfile + size, lexer_storage, STORAGE_SIZE);
        int scope_level = 0;        // Current scope level, parsing only happens at level 0
        int paren_level = 0;        // Same for parentheses
        int state = PS_DECL;        // Parser state
        char def_line[256] = {};    // Line to write in header for forward definition
        while(stb_c_lexer_get_token(&lex)) {
            if(lex.token == '{') scope_level++;
            if(lex.token == '}') {
                scope_level--;
                if(scope_level == 0) {
                    def_line[0] = 0;
                    state = PS_DECL;
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
                state = PS_DECL;
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
                            state = PS_IGNORE;
                            break;
                        }
                        if(def_line[0] && isalnum(def_line[strlen(def_line)-1])) {
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
                        }
                        break;
                    }
                } break;

                case PS_FUNC_PARAMS: {
                    switch(lex.token) {
                        case CLEX_id: // Identifier
                        if(isalnum(def_line[strlen(def_line)-1])) {
                            strcat(def_line, " ");
                        }
                        strcat(def_line, lex.string);
                        break;
                        
                        case ')': // All done?
                        strcat(def_line, ")");
                        if(paren_level == 0) {
                            if(def_line[strlen(def_line)-1] == '(') {
                                strcat(def_line, "void"); // Moffitt might whine if I don't do this
                            }
                            fprintf(outfile, "%s;\n", def_line);
                            def_line[0] = 0;
                            state = PS_IGNORE;
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
