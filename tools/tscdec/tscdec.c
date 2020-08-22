/*
 * cc tscdec.c -o tscdec
 * Decodes (or encodes) all TSC files provided as parameters
 */

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
	if(argc < 2) {
	    printf("Usage: tscdec <tsc file> [more tsc files ...]\n");
	    return 1;
	}
	for(int i = 1; i < argc; i++) {
        FILE *infile = fopen(argv[i], "rb");
        if(!infile) return 2;
        char outfn[512];
        strcpy(outfn, argv[i]);
        outfn[strlen(outfn)-2] = 'x';
        outfn[strlen(outfn)-1] = 't';
        FILE *outfile = fopen(outfn, "wb");
        if(!outfile) return 3;
        // Find the key at "file_length / 2"
        fseek(infile, 0, SEEK_END);
        int len = ftell(infile);
        fseek(infile, len / 2, SEEK_SET);
        char key, buf;
        fread(&key, 1, 1, infile);
        // Output "decrypted" file into plain text
        fseek(infile, 0, SEEK_SET);
        while(fread(&buf, 1, 1, infile) > 0) {
            if(buf != key) buf -= key;
            fwrite(&buf, 1, 1, outfile);
        }
        fclose(infile);
        fclose(outfile);
	}
	return 0;
}
