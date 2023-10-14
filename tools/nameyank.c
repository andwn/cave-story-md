/* Output map names from a Doukutsu.exe */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Give me a filename!\n");
        return 1;
    }
    FILE *fin = fopen(argv[1], "rb");
    if(!fin) {
        printf("Failed to read '%s'.\n", argv[1]);
        return 1;
    }
    //int addr = 0x9A0A5;
	int addr = 0x93855; // Chinese version
    for(int i = 0; i < 95; i++) {
        char name[32];
        fseek(fin, addr, SEEK_SET);
        fgets(name, 32, fin);
        printf("%s\n", name);
        addr += 200;
    }
    fclose(fin);
    return 0;
}
