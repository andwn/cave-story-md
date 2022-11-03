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

#define COLUMNS		    12

#define MULTAB_SIZE     64
#define DIVTAB_SIZE     1024
#define MODTAB_SIZE     1024

#define SINTAB_SIZE     0x100
#define MULTIPLIER      1.5

#define TANTAB_SIZE     0x20
#define PIBT            ((360.00f / (float)(TANTAB_SIZE * 4)) * (3.14159265f / 180.00f))

#define NTSCTAB_SIZE    0x400

int truncate(double x) {
    return (x - (int)x) < 0.75 ? (int)x : (int)(x + 1);
}

void do_multable(int val, int size) {
    printf("const int16_t mul%d[%d] = {", val, size);
    for(int i = 0; i < size; i++) {
        if ((i % COLUMNS) == 0) printf("\n\t");
        printf("%d,", i * val);
    }
    printf("\n};\n\n");
}

void do_divtable(int val, int size) {
    printf("const uint8_t div%d[%d] = {", val, size);
    for(int i = 0; i < size; i++) {
        if ((i % (val*2)) == 0) printf("\n\t");
        printf("%d,", i / val);
    }
    printf("\n};\n\n");
}

void do_modtable(int val, int size) {
    printf("const uint8_t mod%d[%d] = {", val, size);
    for(int i = 0; i < size; i++) {
        if ((i % (val*2)) == 0) printf("\n\t");
        printf("%d,", i % val);
    }
    printf("\n};\n\n");
}

void do_sintable(int scale, int size) {
    int16_t table[0x100];
    table[0x00] = 0;
    table[0x40] = scale;
    table[0x80] = 0;
    table[0xC0] = -scale;

    for (int i = 1; i < 0x40; i++) {
        table[i] = (sin(i * 3.1415926 / 0x80) * scale + 0.5);
        table[0x80-i] = table[i];
        table[0x80+i] = -table[i];
        table[0x100-i] = -table[i];
    }

    printf("const int16_t sin[0x%X] = {", size);
    for (int i = 0; i < 0x100; i++) {
        if ((i % 8) == 0) printf("\n\t");
        printf("0x%04hX,", table[i]);
    }
    printf("\n};\n\n");

    printf("const int16_t cos[0x%X] = {", size);
    for (int i = 0x40; i < 0x140; i++) {
        if ((i % 8) == 0) printf("\n\t");
        printf("0x%04hX,", table[i & 0xFF]);
    }
    printf("\n};\n\n");

    table[0x00] = 0;
    table[0x40] = scale * MULTIPLIER;
    table[0x80] = 0;
    table[0xC0] = -scale * MULTIPLIER;

    for (int i = 1; i < 0x40; i++) {
        table[i] = (sin(i * 3.1415926 / 0x80) * scale + 0.5 * MULTIPLIER);
        table[0x80-i] = table[i];
        table[0x80+i] = -table[i];
        table[0x100-i] = -table[i];
    }

    printf("const int16_t sin2[0x%X] = {", size);
    for (int i = 0; i < 0x100; i++) {
        if ((i % 8) == 0) printf("\n\t");
        printf("0x%04hX,", table[i]);
    }
    printf("\n};\n\n");

    printf("const int16_t cos2[0x%X] = {", size);
    for (int i = 0x40; i < 0x140; i++) {
        if ((i % 8) == 0) printf("\n\t");
        printf("0x%04hX,", table[i & 0xFF]);
    }
    printf("\n};\n\n");
}

int main() {
    printf("#include \"common.h\"\n\n");

    // Multiplication Tables
    do_multable(3, MULTAB_SIZE);
    do_multable(6, MULTAB_SIZE);
    do_multable(9, MULTAB_SIZE);
    do_multable(12, MULTAB_SIZE);

    // Division Tables
    do_divtable(10, DIVTAB_SIZE);

    // Modulus Tables
    do_modtable(10, DIVTAB_SIZE);

    // Sine / Cosine
    do_sintable(512, SINTAB_SIZE);

    // Tangent
    printf("const uint32_t tan_table[0x%X] = {\n", TANTAB_SIZE + 1);
    for(int i = 0; i < TANTAB_SIZE; i++) {
        printf("\t0x%08X,\n", (int)(tan((double)i * PIBT) * (1 << 13)));
    }
    printf("\t0xFFFFFFFF,\n");
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
