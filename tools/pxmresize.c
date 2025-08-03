#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t *slurp(const char *filename, size_t *size) {
    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    uint8_t *data = malloc(*size);
    fseek(f, 0, SEEK_SET);
    fread(data, 1, *size, f);
    fclose(f);
    return data;
}

void dump(const char *filename, uint8_t *data, size_t size) {
    FILE *f = fopen(filename, "wb");
    fwrite(data, 1, size, f);
    fclose(f);
}

int main(int argc, char *argv[]) {
    if(argc != 4) return 1;
    // Load source PXM
    size_t size;
    uint8_t *pxm = slurp(argv[1], &size);
    int w = pxm[4] | (pxm[5] << 8);
    int h = pxm[6] | (pxm[7] << 8);
    // Load new PXM
    int new_w = atoi(argv[2]);
    int new_h = atoi(argv[3]);
    uint8_t *new_pxm = malloc(8 + new_w * new_h);
    new_pxm[0] = 'P';
    new_pxm[1] = 'X';
    new_pxm[2] = 'M';
    new_pxm[3] = pxm[3];
    new_pxm[4] = new_w & 0xFF;
    new_pxm[5] = new_w >> 8;
    new_pxm[6] = new_h & 0xFF;
    new_pxm[7] = new_h >> 8;
    uint8_t *cur = pxm + 8;
    uint8_t *new_cur = new_pxm + 8;
    for(int y = 0; y < new_h; y++) {
        for(int x = 0; x < new_w; x++) {
            if(y >= h || x >= w) {
                *new_cur++ = 0;
            } else {
                *new_cur++ = *cur++;
            }
        }
        cur += w - new_w;
    }
    int ext = strlen(argv[1]);
    argv[1][ext-1] = 'o';
    dump(argv[1], new_pxm, 8 + new_w * new_h);
    free(pxm);
    free(new_pxm);
    return 0;
}
