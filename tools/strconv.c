/* Program to convert localized text strings into CSMD format */

#include "tscomp/tscomp.h"

int main(int argc, char *argv[]) {
    const char *fname_in = NULL, *fname_out = NULL;
    if(argc < 3) {
        printf("Usage: namestr [-l=XX] <in txt> <out dat>\n");
        return EXIT_FAILURE;
    }
    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-' && argv[i][1] == 'l') {
            int j = 3;
            if(argv[i][2] == 0) {
                j = 0;
                i++;
            }
            language = read_langcode(&argv[i][j]);
        } else if(fname_in) {
            fname_out = argv[i];
        } else {
            fname_in = argv[i];
        }
    }

    //printf("Langcode: %d\n", language);

    if(language >= LANG_JA && language < LANG_RU) {
		if(!load_kanji_list()) return EXIT_FAILURE;
	}

    FILE *fin = fopen(fname_in, "rb");
    if(!fin) {
        printf("Failed to read '%s'.\n", fname_in);
        return 1;
    }
    FILE *fout = fopen(fname_out, "wb");
    if(!fout) {
        fclose(fin);
        printf("Failed to create or replace '%s'.\n", fname_out);
        return 1;
    }

    const char zero[40] = {0};
    char str[40];
    int line = 0;
    while(fgets(str, 40, fin)) {
        line++;
        if(str[0] == '#' || str[0] == 0) {
            //printf("[%d] Skipping comment\n", line);
            continue; // Comment or empty line
        }
        int oc = 0;
        for(int ic = 0; ic < 32; ic++) {
            if(str[ic] == '#' || str[ic] == '\r' || str[ic] == '\n' || str[ic] == 0
                || (str[ic] == ' ' && str[ic+1] == '#')) {
                //printf("[%d:%d] End of line\n", line, ic);
                fwrite(zero, 1, 32-oc, fout); // End of line, pad zeroes
                break;
            }
            int ct = get_char_type(str[ic]);
            switch(ct) {
                case CT_ASCII:
                    fwrite(&str[ic], 1, 1, fout);
                    oc++;
                    break;
                case CT_EXTEND:
                    decode_extended(fout, str[ic]);
                    oc++;
                    break;
                case CT_KANJI:
                    uint16_t wc = ((str[ic] << 8) & 0xFF00) | (str[ic + 1] & 0xFF);
                    //printf("[%d:%d] Kanji: %02X,%02X = %04X\n", line, ic, str[ic], str[ic+1], wc);
                    decode_kanji(fout, wc);
                    oc += 2;
                    ic++;
                    break;
                default:
                    printf("[%d:%d] Invalid character: '%c' (0x%02hx)\n", line, ic, str[ic], str[ic]);
                    fwrite("x", 1, 1, fout);
                    oc++;
                    break;
            }
        }
        if(oc > 32) {
            printf("ERROR: '%s' is too long.\n", str);
        }
    }
    fclose(fin);
    fclose(fout);
    return 0;
}
