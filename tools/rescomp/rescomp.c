#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

#include "rescomp.h"
#include "tools.h"
#include "plugin.h"

// add your plugin include here
#include "palette.h"
#include "tileset.h"
#include "sprite.h"
#include "xgmmusic.h"
#include "wav.h"
#include "bin.h"

// forward
static int doComp(char *fileName, char *fileNameOut, int header);
static int execute(char *info, FILE *fs, FILE *fh);

// shared directory informations
//char *currentDirSystem;
char *currentDir;
//char *resDirSystem;
char *resDir;

// add your plugin declaration here
Plugin *plugins[] =
{
    &palette,
    &tileset,
    &sprite,
    &xgm,
    &wav,
    &bin,
    NULL,
};


int main(int argc, char **argv)
{
    char fileName[MAX_PATH_LEN];
    char fileNameOut[MAX_PATH_LEN];
    int header;
    int ii;

    // default
    header = 1;
    fileName[0] = 0;
    fileNameOut[0] = 0;

    // save rescomp directory
//    currentDirSystem = getDirectorySystem(argv[0]);
    currentDir = getDirectory(argv[0]);

    // parse parmeters
    for (ii = 1; ii < argc; ii++)
    {
        char *arg = argv[ii];

        if (!strcmp(arg, "-noheader")) header = 0;
        else if (!fileName[0]) strcpy(fileName, arg);
        else if (!fileNameOut[0]) strcpy(fileNameOut, arg);
    }

    // strcpy(fileName, "gfx.res");

    //printf("rescomp v1.5\n");

    if (!fileName[0])
    {
        printf("Error: missing the input file.\n");
        printf("\n");
        printf("Usage 1 - compile resource:\n");
        printf("  rescomp input [output] [-noheader]\n");
        printf("    input: the input resource file (.res)\n");
        printf("    output: the asm output filename (same name is used for the include file)\n");
        printf("    -noheader: specify that we don't want to generate the header file (.h)\n");
        printf("  Ex: rescomp resources.res outres.s\n");

        return 1;
    }

    return doComp(fileName, fileNameOut, header);
}

static int doComp(char *fileName, char *fileNameOut, int header)
{
    char tempName[MAX_PATH_LEN];
    char headerName[MAX_PATH_LEN];
    char line[MAX_LINE_LEN];
    FILE *fileInput;
    FILE *fileOutputS;
    FILE *fileOutputH;

    tempName[0] = 0;

    // save input file directory
    resDir = getDirectory(fileName);

    if (!fileNameOut[0]) strcpy(fileNameOut, fileName);

    fileInput = fopen(fileName, "rb");

    if (!fileInput)
    {
        printf("Couldn't open input file %s\n", fileName);
        return 1;
    }

    // remove extension
    removeExtension(fileNameOut);

    // create output .s file
    strcpy(tempName, fileNameOut);
    strcat(tempName, ".s");
    fileOutputS = fopen(tempName, "w");

    if (!fileOutputS)
    {
        fclose(fileInput);
        printf("Couldn't open output file %s\n", tempName);
        return 1;
    }

    // create output .h file
    strcpy(tempName, fileNameOut);
    strcat(tempName, ".h");
    fileOutputH = fopen(tempName, "w");

    if (!fileOutputH)
    {
        fclose(fileInput);
        fclose(fileOutputS);
        printf("Couldn't open output file %s\n", tempName);
        return 1;
    }

    fprintf(fileOutputS, ".section .rodata\n\n");
//    fprintf(fileOutputS, ".text\n\n");

    // build header name
    strcpy(tempName, getFilename(fileNameOut));
    strcpy(headerName, resDir);
    strcat(headerName, "_");
    strcat(headerName, tempName);
    strreplace(headerName, ':', '_');
    strreplace(headerName, '/', '_');
    strupper(headerName);

    fprintf(fileOutputH, "#ifndef _%s_H_\n", headerName);
    fprintf(fileOutputH, "#define _%s_H_\n\n", headerName);

    // process line by line
    while (fgets(line, sizeof(line), fileInput))
    {
        // error while executing --> return code 1
        if (!execute(line, fileOutputS, fileOutputH))
        {
            fclose(fileInput);
            fclose(fileOutputS);
            fclose(fileOutputH);

            return 1;
        }
    }

    fprintf(fileOutputH, "\n");
    fprintf(fileOutputH, "#endif // _%s_H_\n", headerName);

    fclose(fileInput);
    fclose(fileOutputS);
    fclose(fileOutputH);

    if (!header)
    {
        // remove unwanted header file
        strcpy(tempName, fileNameOut);
        strcat(tempName, ".h");
        remove(tempName);
    }

    return 0;
}

static int execute(char *info, FILE *fs, FILE *fh)
{
    Plugin **plugin;

    char type[MAX_NAME_LEN];

    // pass empty line
    if (sscanf(info, "%s", type) < 1)
        return TRUE;
    // pass comment
    if (!strncasecmp(type, "//", 2))
        return TRUE;
    if (!strncasecmp(type, "#", 1))
        return TRUE;

    plugin = plugins;

    while(*plugin != NULL)
    {
        if ((*plugin)->isSupported(type))
        {
			char cmd[MAX_NAME_LEN], name[MAX_NAME_LEN];
			sscanf(info, "%s %s", cmd, name);
            printf("Resource: %s\n", name);
            //printf("--> executing plugin %s...\n", type);
            return (*plugin)->execute(info, fs, fh);
        }

        // try next plugin
        plugin++;
    }

    printf("Error: Unknown resource '%s'", info);
    printf("Accepted resource types are:\n");
    printf("  PALETTE\n");
    printf("  TILESET\n");
    printf("  SPRITE\n");
    printf("  XGM\n");
    printf("  WAV\n");
    printf("  BIN\n");

    return FALSE;
}
