/*
 * tscdecrypt.c
 *
 *  Created on: Feb 22, 2015
 *      Author: aderosier
 */

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
	// Declarations
	FILE *infile, *outfile;
	char outfn[256], buf, key;
	int len;
	// Open in/out files
	if(argc != 2) return 1;
	infile = fopen(argv[1], "rb");
	if(!infile) return 2;
	strcpy(outfn, argv[1]);
	strcat(outfn, ".txt");
	outfile = fopen(outfn, "wb");
	if(!outfile) return 3;
	// Find the key at "file_length / 2"
	fseek(infile, 0, SEEK_END);
	len = ftell(infile);
	fseek(infile, len / 2, SEEK_SET);
	fread(&key, 1, 1, infile);
	// Output "decrypted" file into plain text
	fseek(infile, 0, SEEK_SET);
	while(fread(&buf, 1, 1, infile) > 0) {
		if(buf != key) buf -= key;
		fwrite(&buf, 1, 1, outfile);
	}
	fclose(infile);
	fclose(outfile);
	return 0;
}
