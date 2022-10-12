/*******************************************************************************
 * hppgen - Generates a header for a collection of C/C++ files
 *
 * Compile:
 * gcc hppgen.c -o hppgen
 *
 * Usage:
 * ./hppgen [-gnot] <c file> [more files]
 ******************************************************************************/

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 512KB
#define MAX_SIZE 0x80000
#define min(a,b) ((a)>(b)?(b):(a))

bool guard;
bool trace;
FILE *outfile;
char *srcfile[MAX_SIZE];

void usage() {
    printf("Usage: hppgen [-ot] <c file> [more files...]\n");
    printf("  -g:    Generate header guard\n");
    printf("  -n:    Don't generate header guard\n");
    printf("  -o=fn: Output to file instead of stdout\n");
    printf("  -t:    Show tracing info (debug)\n");
    printf("\n");
    printf("Parameters are executed in order, so it is possible to apply\n");
    printf("different options per file. For example,\n");
    printf("-go=header1.h source1.c -no=header2.h source2.c\n");
    printf("header1 will have guards but not header2.\n");
}

int process_file(const char *fn) {
    FILE *src = fopen(fn, "rb");
    if(!src) {
        printf("ERROR: Failed to open '%s' for reading.\n", fn);
        return EXIT_FAILURE;
    }
    fseek(src, SEEK_END, 0);
    size_t size = ftell(src);
    fread(srcfile, 1, min(size, MAX_SIZE), src);
    fclose(src);

    return EXIT_SUCCESS;
}

int main(int argc,char *argv[]) {
    if (argc < 2) {
        usage();
        return EXIT_FAILURE;
    }
    guard = false;
    trace = false;
    outfile = stdout;
    for (int i = 1; i < argc - 1; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j] != '\0'; j++) {
                switch (argv[i][j]) {
                    case 'g':
                        guard = true;
                        break;
                    case 'n':
                        guard = false;
                        break;
                    case 't':
                        trace = true;
                        break;
                    case 'o': {
                        if (strlen(&argv[i][j]) >= 4) {
                            j += 2;
                        } else { // If they put a space instead of =
                            i++;
                            j = 0;
                        }
                        if(outfile != stdout) fclose(outfile);
                        outfile = fopen(&argv[i][j], "wb");
                        if(!outfile) {
                            printf("ERROR: Failed to open '%s' for writing\n", &argv[i][j]);
                        }
                        j = strlen(argv[i]) - 1;
                        break;
                    }
                }
            }
        } else {
            if(process_file(argv[i]) != EXIT_SUCCESS) {
                if(outfile != stdout) fclose(outfile);
                return EXIT_FAILURE;
            }
        }
    }
    if(outfile != stdout) fclose(outfile);
    return EXIT_SUCCESS;
}
