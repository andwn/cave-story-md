/* Convert extracted list of Windows-1252 encoded names to CSMD printable format */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* copied from tscomp */
bool is_ascii(char c) {
    static const char *ValidChars =
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
            " /,.?=!@$%^&*()[]{}|_-+:;'~\"\n";
    for(int i = 0; i < strlen(ValidChars); i++) {
        if(c == ValidChars[i]) return true;
    }
    return false;
}
bool is_extended(uint8_t c) {
    static const uint8_t ExtendedChars[] = {
            0xA1, 0xBF, /* Upside-down exclamation/question */
            0xC0, 0xC8, 0xCC, 0xD2, 0xD9, 0xE0, 0xE8, 0xEC, 0xF2, 0xF9,  /* Grave */
            0xC7, 0xE7, /* Cedilla */
            0xC1, 0xC9, 0xCD, 0xD3, 0xDA, 0xE1, 0xE9, 0xED, 0xF3, 0xFA,  /* Acute */
            0xC2, 0xCA, 0xCE, 0xD4, 0xDB, 0xE2, 0xEA, 0xEE, 0xF4, 0xFB,  /* Circumflex */
            0xC4, 0xCB, 0xCF, 0xD6, 0xDC, 0xE4, 0xEB, 0xEF, 0xF6, 0xFC,  /* Umlaut */
            0xC3, 0xD5, 0xE3, 0xF5, /* Squiggly vowels */
            0xD1, 0xF1, /* Squiggly N */
            0x8C, 0x9C, /* OE */
            0xC6, 0xE6, /* AE */
            0xBA, /* Ordinal */
            0xDF, /* Sharp S */
    };
    for(int i = 0; i < 56; i++) {
        if(c == ExtendedChars[i]) return true;
    }
    return false;
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Usage: namestr <in txt> <out dat>\n");
        return 1;
    }
    FILE *fin = fopen(argv[1], "rb");
    if(!fin) {
        printf("Failed to read '%s'.\n", argv[1]);
        return 1;
    }
    FILE *fout = fopen(argv[2], "wb");
    if(!fout) {
        fclose(fin);
        printf("Failed to create or replace '%s'.\n", argv[2]);
        return 1;
    }
    for(int i = 0; i < 95; i++) {
        static const uint8_t zero = 0;
        uint8_t in_name[32];
        fgets(in_name, 32, fin);
        int oc = 0;
        for(int ic = 0; ic < 32; ic++) {
            if(in_name[ic] == 0 || in_name[ic] == '\n' || in_name[ic] == '\r') {
                while(oc < 32) {
                    fwrite(&zero, 1, 1, fout);
                    oc++;
                }
                break;
            }
            if(in_name[ic] >= 0xC0) in_name[ic] -= 0x80;
            if(in_name[ic] < 0x80) {
                fwrite(&in_name[ic], 1, 1, fout);
                oc++;
            } else {
                static const char mark = 0x01;
                static const char extmap[] = {
                        0, 8, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0,10,11, 0, 0, 8, 0, 0, 0, 0, 2, 0, 0, 0, 0, 6,
                        0, 0, 4, 5, 9, 0, 0, 0, 1,12, 3, 0, 0, 0, 0, 7,
                };
                char mychr = extmap[in_name[ic] - 0x80];
                printf("%c (%02x) -> %d\n", in_name[ic], in_name[ic], mychr);
                fwrite(&mark, 1, 1, fout);
                fwrite(&mychr, 1, 1, fout);
                oc += 2;
            }
            #if 0
            if(is_ascii(in_name[ic])) {
                fwrite(&in_name[ic], 1, 1, fout);
                oc++;
            } else if(is_extended(in_name[ic])){
                /* copied from tscomp */
                if(in_name[ic] == 0x8C) {
                    static const char mystr[2] = "OE";
                    fwrite(&mystr, 1, 2, fout);
                } else if(in_name[ic] == 0x9C) {
                    static const char mystr[2] = "oe";
                    fwrite(&mystr, 1, 2, fout);
                } else if(in_name[ic] == 0xC6) {
                    static const char mystr[2] = "AE";
                    fwrite(&mystr, 1, 2, fout);
                } else if(in_name[ic] == 0xE6) {
                    static const char mystr[2] = "ae";
                    fwrite(&mystr, 1, 2, fout);
                } else {
                    static const char mark = 0x01;
                    static const char extmap[] = {
                            0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,27, 0, 0, 0, 0, 2,
                            3, 3, 3, 3, 3, 3, 0,15, 4, 4, 4, 4, 5, 5, 5, 5,
                            0, 6, 7, 7, 7, 7, 7, 0, 0, 8, 8, 8, 8, 0, 0,30,
                            9, 9,17,28,22,22, 0,16,10,10,18,23,11,11,19,24,
                            0,12,13,13,20,29,25, 0, 0,14,14,21,26, 0, 0, 0,
                    };
                    char mychr = extmap[in_name[ic] - 160];
                    printf("%c (%02x) -> %d\n", in_name[ic], in_name[ic], mychr);
                    fwrite(&mark, 1, 1, fout);
                    fwrite(&mychr, 1, 1, fout);
                }
                oc += 2;
            } else {
                printf("WARN: Invalid char '%c' (0x%02x)\n", in_name[ic], in_name[ic]);
            }
            #endif
        }
    }
    fclose(fin);
    fclose(fout);
    return 0;
}
