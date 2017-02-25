/* 
 * Prints a sine and cosine lookup table to stdout. Based on a thing Sik sent me
 *
 * gcc lutgen.c -o lutgen -lm
 * 
 * ./lutgen > trig.c
 */

#include <stdint.h>
#include <stdio.h>
#include <math.h>

int main() {
   int16_t table[0x100];
   
   table[0x00] = 0;
   table[0x40] = 0x200;
   table[0x80] = 0;
   table[0xC0] = -0x200;
   
   for (int i = 1; i < 0x40; i++) {
      table[i] = sin(i * 3.1415926 / 0x80) * 0x200 + 0.5;
      table[0x80-i] = table[i];
      table[0x80+i] = -table[i];
      table[0x100-i] = -table[i];
   }
   
   printf("#include \"common.h\"\n\n");
   
   printf("const s16 sin[0x100] = {\n\t");
   for (int i = 0; i < 0x100; i++) {
      printf("0x%04hx, ", table[i]);
      if ((i & 7) == 7) printf("\n\t");
   }
   printf("};\n\n");
   
   printf("const s16 cos[0x100] = {\n\t");
   for (int i = 0x40; i < 0x140; i++) {
      printf("0x%04hx, ", table[i & 0xFF]);
      if ((i & 7) == 7) printf("\n\t");
   }
   printf("};\n");
}
