#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int widest_index = 0;
    int widest_size = 0;
    int tallest_index = 0;
    int tallest_size = 0;
    for(int i = 1; i < argc; i++) {
        FILE *f = fopen(argv[i], "rb");
        if(!f) {
            printf("Failed to open '%s', skipping...\n", argv[1]);
            continue;
        }
        uint8_t pxm[4];
        fread(pxm, 1, 4, f);
        if(pxm[0] == 'P' && pxm[1] == 'X' && pxm[2] == 'M') {
            uint16_t size[2];
            fread(size, 2, 2, f);
            if(size[0] > widest_size) {
                widest_index = i;
                widest_size = size[0];
            }
            if(size[1] > tallest_size) {
                tallest_index = i;
                tallest_size = size[1];
            }
        } else {
            printf("'%s' is not a PXM, skipping...\n", argv[i]);
        }
        fclose(f);
    }
    printf("Widest:  %s - %d\n", argv[widest_index], widest_size);
    printf("Tallest: %s - %d\n", argv[tallest_index], tallest_size);
    return 0;
}
