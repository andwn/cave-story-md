/*
 * Prints various lookup tables for math functions as C arrays
 *
 * gcc lutgen.c -o lutgen -lm
 *
 * ./lutgen > mathtabs.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

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
    printf("TAB div%d", val);
    for(int i = 0; i < size; i++) {
        if ((i % (val*2)) == 0) printf("\n\tdc.b ");
        printf("%3d,", i / val);
    }
    printf("\n\n");
}

void do_modtable(int val, int size) {
    printf("const uint8_t mod%d[%d] = {", val, size);
    for(int i = 0; i < size; i++) {
        if ((i % (val*2)) == 0) printf("\n\t");
        printf("%d,", i % val);
    }
    printf("\n};\n\n");
}

void do_sintable(int scale) {
    int32_t table[0x100];
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

    printf("TAB sin");
    for (int i = 0; i < 0x140; i++) {
        if (i == 0x40) printf("\nTAB cos");
        if ((i % 8) == 0) printf("\n\tdc.l ");
        printf("$%08X", table[i % 0x100]);
        if ((i % 8) != 7) printf(",");
    }
    printf("\n\n");

    //printf("\t.align 2\n\t.globl cos\ncos:", size);
    //for (int i = 0x40; i < 0x140; i++) {
    //    if ((i % 16) == 0) printf("\n\t");
    //    printf("$%04hX,", table[i & 0xFF]);
    //}
    //printf("\n\n");

    //table[0x00] = 0;
    //table[0x40] = scale * MULTIPLIER;
    //table[0x80] = 0;
    //table[0xC0] = -scale * MULTIPLIER;
//
    //for (int i = 1; i < 0x40; i++) {
    //    table[i] = (sin(i * 3.1415926 / 0x80) * scale + 0.5 * MULTIPLIER);
    //    table[0x80-i] = table[i];
    //    table[0x80+i] = -table[i];
    //    table[0x100-i] = -table[i];
    //}
//
    //printf("const int16_t sin2[0x%X] = {", size);
    //for (int i = 0; i < 0x100; i++) {
    //    if ((i % 8) == 0) printf("\n\t");
    //    printf("$%04hX,", table[i]);
    //}
    //printf("\n};\n\n");
//
    //printf("const int16_t cos2[0x%X] = {", size);
    //for (int i = 0x40; i < 0x140; i++) {
    //    if ((i % 8) == 0) printf("\n\t");
    //    printf("$%04hX,", table[i & 0xFF]);
    //}
    //printf("\n};\n\n");
}

void do_atantable(void) {
    double rad2deg = 0x80 / 3.14159265;
    printf("TAB atan");
    for(int y = 0; y < 64; y++) {
        printf("\n\tdc.b ");
        for(int x = 0; x < 64; x++) {
            printf("$%02hX", (uint8_t)(atan2(31.5 - y, x - 31.5) * rad2deg));
            if ((x % 64) != 63) printf(",");
        }
    }
    printf("\n\n");
}

void do_squaretable(int size) {
    printf("TAB pow2");
    for(int i = 0; i < size; i++) {
        if(i % 8 == 0) printf("\n\tdc.w ");
        printf("$%04X,", MIN(i*i, UINT16_MAX));
    }
    printf("\n\n");
}

void do_fuzztable(int x_scale, int y_scale, int size) {
    double rad2deg = 3.14159265 / (size / 2);
    printf("static const int8_t fuzz_xy[%d*2] = {", size);
    for(int i = 0; i < size; i++) {
        if(i % 8 == 0) printf("\n\t");
        printf("%3d,%3d, ", (int)(sin(i * rad2deg) * x_scale), (int)(cos(i * rad2deg) * y_scale));
    }
    printf("\n};\n");
}

void do_projectile_test(int num, int range, int speed) {
    static float var_per_best = 100;
    static float var_per_worst = 0;
    static float var_per_avg = 0;
    for(int i = 0; i < num; i++) {
        float src_x = rand() % range;
        float src_y = rand() % range;
        float tgt_x = rand() % range;
        float tgt_y = rand() % range;

        // Start with absolute value of x and y distance
        float x = tgt_x - src_x;
        float y = tgt_y - src_y;
        float x_spd = fabsf(x);
        float y_spd = fabsf(y);
        
        // Shrink via shifts until less than the desired speed (usually 4-6 iterations)
        // (TODO: Increase the shift in the first iteration if point-blanking is prohibited?)
        int iter1 = 0;
        while(x_spd > speed || y_spd > speed) {
            x_spd /= 2;
            y_spd /= 2;
            iter1++;
        }

        // Increases threshold for how much larger than speed x_spd + y_spd can be,
        // based on how much closer to 45 degrees they are precieved to be
        float diag_speed = speed;
        if(x_spd < y_spd * 8 && y_spd < x_spd * 8) {
            float speed_step = speed / 6;
            diag_speed += speed_step;
            if(x_spd < y_spd * 4 && y_spd < x_spd * 4) {
                diag_speed += speed_step;
                if(x_spd < y_spd * 2 && y_spd < x_spd * 2) {
                    diag_speed += speed_step;
                }
            }
        }

        // Grow and shrink x_spd and y_spd to get them closer to the desired speed
        // More iterations = more fine grained result
        int iter2 = 0;
        float x_up = x_spd / 2;
        float y_up = y_spd / 2;
        for(int j = 0; j < 4; j++) {
            if(x_spd + y_spd < diag_speed) {
                x_spd += x_up;
                y_spd += y_up;
            }
            x_up /= 2;
            y_up /= 2;
            if(x_spd + y_spd > diag_speed) {
                x_spd -= x_up;
                y_spd -= y_up;
            }
            iter2++;
        }
        
        // Restore sign
        if(y < 0) y_spd = -y_spd;
        if(x < 0) x_spd = -x_spd;

        // Accurate calculation to compare
        float angle = atan2f(y, x);
        float acc_x = cosf(angle) * speed;
        float acc_y = sinf(angle) * speed;
        float var_x = fabsf(acc_x - x_spd);
        float var_y = fabsf(acc_y - y_spd);
        float var_per = var_x + var_y / 2 / speed * 100;
        var_per_avg += var_per;
        if(var_per_best > var_per) var_per_best = var_per;
        if(var_per_worst < var_per) var_per_worst = var_per;
        
        printf("(%3.0f,%3.0f) -> (%3.0f,%3.0f) = (%4.0f,%4.0f) @ (%3.3f,%3.3f)\t// %3.3f%%\t// %d + %d\n", 
            src_x, src_y, tgt_x, tgt_y, x, y, x_spd, y_spd, var_per, iter1, iter2);
    }
    var_per_avg /= num;
    printf("\nAverage = %3.3f%%, Best = %3.3f%%, Worst = %3.3f%%\n", var_per_avg, var_per_best, var_per_worst);
}

//static inline div10(uint32_t n) {
//    uint32_t q = (n >> 1) + (n >> 2);
//    q = q + (q >> 4);
//    q = q + (q >> 8);
//    q = q + (q >> 16);
//    q = q >> 3;
//    uint32_t r = n - (((q << 2) + q) << 1);
//    return q + (r > 9);
//}
//
//static inline uint16_t div60(uint32_t n) {
//	uint16_t n_64 = n >> 6;
//	uint16_t n_64_2 = n >> 7;
//	return n_64 + n_128;
//}

// Really fucking slow divide
#define SHIFT_NUM 16
void divu32_slow(uint32_t n/*umerator*/, uint16_t d/*enominator*/, uint32_t *q/*uotient*/, uint16_t *r/*emainder*/) {
    int iters[SHIFT_NUM] = {0};
    int iter_ind = 0;
    *q = 0;
    uint32_t n_up = (d << SHIFT_NUM);
    uint32_t q_up = (1 << SHIFT_NUM);
    while(q_up) {
        while(n >= n_up) {
            n -= n_up;
            *q += q_up;
            iters[iter_ind]++;
        }
        n_up >>= 1;
        q_up >>= 1;
        iter_ind++;
    }
    *r = n;
    printf("Iterations: [");
    for(int i = 0; i < SHIFT_NUM; i++) printf("%d ", iters[i]);
    printf("]\n");
}

void do_div_test(void) {
    for(int i = 0; i < 64; i++) {
        uint32_t r = ((rand() % 99) + 1) * 60 * 60 * 50 - (rand() % (60 * 60 * 50));
        uint32_t r50 = r / 50;
        uint32_t r60 = r / 60;

        uint32_t r50_q, r60_q;
        uint16_t r50_r, r60_r;
        divu32_slow(r, 50, &r50_q, &r50_r);
        divu32_slow(r, 60, &r60_q, &r60_r);
        printf("%8u | %6u %6u | %6u %6u [%hu %hu]\n", r, r50, r60, r50_q, r60_q, r50_r, r60_r);
    }
}

int main() {
    //printf("#include \"md/md.h\"\n\n");

    // Multiplication Tables
    //do_multable(3, MULTAB_SIZE);
    //do_multable(6, MULTAB_SIZE);
    //do_multable(9, MULTAB_SIZE);
    //do_multable(12, MULTAB_SIZE);

    // Division Tables
    //do_divtable(10, DIVTAB_SIZE);

    // Modulus Tables
    //do_modtable(10, DIVTAB_SIZE);

    // Sine / Cosine
    do_sintable(0x07FFF); // 0.5
    do_sintable(0x0FFFF); // 1.0
    do_sintable(0x17FFF); // 1.5
    do_sintable(0x3FFFF); // 4.0
    do_sintable(0x5FFFF); // 6.0

    do_atantable();

    //do_fuzztable(20, 32, 128);

    //do_squaretable(384);

    // Tangent
    //printf("const uint32_t tan_table[0x%X] = {\n", TANTAB_SIZE + 1);
    //for(int i = 0; i < TANTAB_SIZE; i++) {
    //    printf("\t0x%08X,\n", (int)(tan((double)i * PIBT) * (1 << 13)));
    //}
    //printf("\t0xFFFFFFFF,\n");
    //printf("};\n\n");

    //do_projectile_test(1024, 512, 8.0f);

    //do_div_test();
    // NTSC Time/Speeds
    //printf("const uint16_t time_tab_ntsc[0x%X] = {\n", NTSCTAB_SIZE);
    //for(int i = 0; i < NTSCTAB_SIZE; i += COLUMNS) {
    //    printf("\t");
    //    for(int j = i; j < i + COLUMNS; j++) {
    //        double time = ((double)j) * 6.0 / 5.0;
    //        printf("0x%04x,", truncate(time));
    //    }
    //    printf("\n");
    //}
    //printf("};\n\n");
    //printf("const uint16_t speed_tab_ntsc[0x%X] = {\n", NTSCTAB_SIZE);
    //for(int i = 0; i < NTSCTAB_SIZE; i += COLUMNS) {
    //    printf("\t");
    //    for(int j = i; j < i + COLUMNS; j++) {
    //        double speed = ((double)j) * 5.0 / 6.0;
    //        printf("0x%04x,", truncate(speed));
    //    }
    //    printf("\n");
    //}
    //printf("};\n\n");

    // PAL Time/Speeds (same as index)

    //printf("const uint16_t time_tab_pal[0x%X] = {\n", NTSCTAB_SIZE);
    //for(int i = 0; i < NTSCTAB_SIZE; i += COLUMNS) {
    //    printf("\t");
    //    for(int j = i; j < i + COLUMNS; j++) printf("0x%04X,", j);
    //    printf("\n");
    //}
    //printf("};\n\n");
    //printf("const uint16_t speed_tab_pal[0x%X] = {\n", NTSCTAB_SIZE);
    //for(int i = 0; i < NTSCTAB_SIZE; i += COLUMNS) {
    //    printf("\t");
    //    for(int j = i; j < i + COLUMNS; j++) printf("0x%04x,", j);
    //    printf("\n");
    //}
    //printf("};\n\n");
}
