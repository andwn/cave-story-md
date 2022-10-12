/*
 * Prints various lookup tables for math functions as C arrays
 *
 * gcc lutgen.c -o lutgen -lm
 *
 * ./lutgen > mathtabs.c
 */

#include <stdint.h>
#include <stdio.h>
#include <math.h>

#define COLUMNS		    16

#define SINTAB_SIZE     0x100
#define MULTIPLIER      1.5

#define TANTAB_SIZE     0x20
#define PIBT            ((360.00f / (float)(TANTAB_SIZE * 4)) * (3.14159265f / 180.00f))

#define NTSCTAB_SIZE    0x400

int truncate(double x) {
    return (x - (int)x) < 0.75 ? (int)x : (int)(x + 1);
}

int main() {
    printf("#include \"common.h\"\n\n");

    // Sine / Cosine

    int16_t table[0x100];
    table[0x00] = 0;
    table[0x40] = 0x200 * MULTIPLIER;
    table[0x80] = 0;
    table[0xC0] = -0x200 * MULTIPLIER;

    for (int i = 1; i < 0x40; i++) {
        table[i] = (sin(i * 3.1415926 / 0x80) * 0x200 + 0.5) * MULTIPLIER;
        table[0x80-i] = table[i];
        table[0x80+i] = -table[i];
        table[0x100-i] = -table[i];
    }



    printf("const int16_t sin[0x%X] = {", SINTAB_SIZE);
    for (int i = 0; i < 0x100; i++) {
        if ((i % COLUMNS) == 0) printf("\n\t");
        printf("0x%04hx,", table[i]);
    }
    printf("\n};\n\n");

    printf("const int16_t cos[0x%X] = {", SINTAB_SIZE);
    for (int i = 0x40; i < 0x140; i++) {
        if ((i % COLUMNS) == 0) printf("\n\t");
        printf("0x%04hx,", table[i & 0xFF]);
    }
    printf("\n};\n\n");

    // Tangent

    printf("const uint32_t tan_table[0x%X] = {\n", TANTAB_SIZE);
    for(int i = 0; i < TANTAB_SIZE; i++) {
        printf("\t0x%08X,\n", (int)(tan((double)i * PIBT) * (1 << 13)));
    }
    printf("};\n\n");

    // NTSC Time/Speeds

    printf("const uint16_t time_tab_ntsc[0x%X] = {\n", NTSCTAB_SIZE);
    for(int i = 0; i < NTSCTAB_SIZE; i += COLUMNS) {
        printf("\t");
        for(int j = i; j < i + COLUMNS; j++) {
            double time = ((double)j) * 6.0 / 5.0;
            printf("0x%04x,", truncate(time));
        }
        printf("\n");
    }
    printf("};\n\n");
    printf("const uint16_t speed_tab_ntsc[0x%X] = {\n", NTSCTAB_SIZE);
    for(int i = 0; i < NTSCTAB_SIZE; i += COLUMNS) {
        printf("\t");
        for(int j = i; j < i + COLUMNS; j++) {
            double speed = ((double)j) * 5.0 / 6.0;
            printf("0x%04x,", truncate(speed));
        }
        printf("\n");
    }
    printf("};\n\n");

    // PAL Time/Speeds (same as index)

    printf("const uint16_t time_tab_pal[0x%X] = {\n", NTSCTAB_SIZE);
    for(int i = 0; i < NTSCTAB_SIZE; i += COLUMNS) {
        printf("\t");
        for(int j = i; j < i + COLUMNS; j++) printf("0x%04X,", j);
        printf("\n");
    }
    printf("};\n\n");
    printf("const uint16_t speed_tab_pal[0x%X] = {\n", NTSCTAB_SIZE);
    for(int i = 0; i < NTSCTAB_SIZE; i += COLUMNS) {
        printf("\t");
        for(int j = i; j < i + COLUMNS; j++) printf("0x%04x,", j);
        printf("\n");
    }
    printf("};\n\n");
}
