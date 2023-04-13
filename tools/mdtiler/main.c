//***************************************************************************
// "main.c"
// Program entry point, parses command line and runs stuff as required
//***************************************************************************
// mdtiler - Bitmap to tile conversion tool
// Copyright 2011, 2012, 2016, 2017, 2018 Javier Degirolmo
//
// This file is part of mdtiler.
//
// mdtiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// mdtiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with mdtiler.  If not, see <http://www.gnu.org/licenses/>.
//***************************************************************************

// Required headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "batch.h"
#include "bitmap.h"
#include "tiles.h"

// Actions that may be performed
typedef enum {
   ACTION_DEFAULT,      // Default action

   ACTION_BATCH,        // Batch build
   ACTION_TILEMAP,      // Quick build (tilemap ordering)
   ACTION_SPRITE,       // Quick build (sprite ordering)

   ACTION_TOOMANY       // Too many actions specified
} Action;

// Default palette when no palette is given
// Bonus points to whoever can tell where this palette is from
static const uint16_t default_pal[] = {
   0x000, 0xA00, 0x0A0, 0xAA0, 0x00A, 0xA0A, 0x06A, 0xAAA,
   0x666, 0xE66, 0x6E6, 0xEE6, 0x66E, 0xE6E, 0x6EE, 0xEEE
};

// Function prototypes
int build_tilemap(const char *, const char *);
int build_sprite(const char *, const char *);

//***************************************************************************
// Program entry point
//***************************************************************************

int main(int argc, char **argv) {
   // Set default palette
   set_palette(default_pal);

   // To know if there was an error or not
   int errcode = 0;

   // Scan all arguments
   int show_help = 0;
   int show_ver = 0;
   Action action = ACTION_DEFAULT;
   Format format = FORMAT_DEFAULT;
   const char *infilename = NULL;
   const char *outfilename = NULL;

   int scan_ok = 1;
   int err_manyfiles = 0;

   for (int curr_arg = 1; curr_arg < argc; curr_arg++) {
      // Get pointer to argument, to make our lives easier
      const char *arg = argv[curr_arg];

      // If it's an option, parse it
      if (scan_ok && arg[0] == '-') {
         // Stop parsing options?
         if (!strcmp(arg, "--"))
            scan_ok = 0;

         // Show help or version?
         else if (!strcmp(arg, "-h") || !strcmp(arg, "--help"))
            show_help = 1;
         else if (!strcmp(arg, "-v") || !strcmp(arg, "--version"))
            show_ver = 1;

         // Batch build?
         else if (!strcmp(arg, "-b") || !strcmp(arg, "--batch"))
            action = action == ACTION_DEFAULT ?
                     ACTION_BATCH : ACTION_TOOMANY;

         // Quick build?
         else if (!strcmp(arg, "-t") || !strcmp(arg, "--tilemap"))
            action = action == ACTION_DEFAULT ?
                     ACTION_TILEMAP : ACTION_TOOMANY;
         else if (!strcmp(arg, "-s") || !strcmp(arg, "--sprite"))
            action = action == ACTION_DEFAULT ?
                     ACTION_SPRITE : ACTION_TOOMANY;

         // Format for quick build?
         else if (!strcmp(arg, "-4") || !strcmp(arg, "--4bpp"))
            format = format == FORMAT_DEFAULT ?
                     FORMAT_4BPP : FORMAT_TOOMANY;
         else if (!strcmp(arg, "-1") || !strcmp(arg, "--1bpp"))
            format = format == FORMAT_DEFAULT ?
                     FORMAT_1BPP : FORMAT_TOOMANY;

         // Unknown argument
         else {
            fprintf(stderr, "Error: unknown option \"%s\"\n", arg);
            errcode = 1;
         }
      }

      // Input filename?
      else if (infilename == NULL)
         infilename = arg;

      // Output filename?
      else if (outfilename == NULL)
         outfilename = arg;

      // Too many files specified?
      else
         err_manyfiles = 1;
   }

   // No action specified?
   if (action == ACTION_DEFAULT)
      action = ACTION_BATCH;

   // No format specified?
   if (format == FORMAT_DEFAULT)
      format = FORMAT_4BPP;

   // Batch building only takes an input file, so error out with a too many
   // files error if an output file is specified
   if (action == ACTION_BATCH && outfilename != NULL)
      err_manyfiles = 1;

   // Look for error conditions
   if (action == ACTION_TOOMANY) {
      errcode = 1;
      fprintf(stderr, "Error: can't specify more than one action\n");
   } else if (!show_help && !show_ver) {
      if (infilename == NULL) {
         errcode = 1;
         fprintf(stderr, "Error: input filename missing\n");
      } else if (outfilename == NULL && action != ACTION_BATCH) {
         errcode = 1;
         fprintf(stderr, "Error: output filename missing\n");
      } else if (err_manyfiles) {
         errcode = 1;
         fprintf(stderr, "Error: too many filenames specified\n");
      } else if (format == FORMAT_TOOMANY) {
         errcode = 1;
         fprintf(stderr, "Error: can't specify more than one format\n");
      }
   }

   // If there was an error then quit
   if (errcode)
      return EXIT_FAILURE;

   // Show tool version?
   if (show_ver) {
      puts("1.90");
      return EXIT_SUCCESS;
   }

   // Show tool usage?
   if (show_help) {
      printf("Usage:\n"
             "  %s -b <batchfile>\n"
             "  %s -t <infile> <outfile>\n"
             "  %s -s <infile> <outfile>\n"
             "\n"
             "Options:\n"
             "  -b or --batch ..... Batch build (default)\n"
             "  -t or --tilemap ... Quick build, tilemap tile order\n"
             "  -s or --sprite .... Quick build, sprite tile order\n"
             "  -4 or --4bpp ...... Output 4bpp tiles (quick build)\n"
             "  -1 or --1bpp ...... Output 1bpp tiles (quick build)\n"
             "  -h or --help ...... Show this help\n"
             "  -v or --version ... Show tool version\n",
             argv[0], argv[0], argv[0]);
      return EXIT_SUCCESS;
   }

   // Start the job!
   switch (action) {
      // Batch build
      case ACTION_BATCH:
         errcode = build_batch(infilename);
         break;

      // Quick build (tilemap ordering)
      case ACTION_TILEMAP:
         set_output_format(format);
         errcode = build_tilemap(infilename, outfilename);
         break;

      // Quick build (sprite ordering)
      case ACTION_SPRITE:
         set_output_format(format);
         errcode = build_sprite(infilename, outfilename);
         break;

      // Oops?!
      default:
         errcode = ERR_UNKNOWN;
         break;
   }

   // If there was an error, show a message
   if (errcode) {
      // Determine message to show
      const char *msg;
      const char *errfile = NULL;

      switch(errcode) {
         case ERR_OPENINPUT:
            msg = "can't open input file";
            errfile = infilename;
            break;
         case ERR_OPENOUTPUT:
            msg = "can't open output file";
            errfile = outfilename;
            break;
         case ERR_OPENBATCH:
            msg = "can't open batch file";
            errfile = infilename;
            break;
         case ERR_CANTREAD:
            msg = "can't read batch file";
            errfile = infilename;
            break;
         case ERR_CANTWRITE:
            msg = "can't write to output file";
            errfile = outfilename;
            break;
         case ERR_CANTWRITEGFX:
            msg = "can't write to tiles file";
            errfile = infilename;
            break;
         case ERR_CANTWRITEMAP:
            msg = "can't write to tilemap mappings file";
            errfile = infilename;
            break;
         case ERR_CANTWRITESPR:
            msg = "can't write to sprite mappings file";
            errfile = infilename;
            break;
         case ERR_MANYTILES:
            msg = "too many unique tiles";
            errfile = infilename;
            break;
         case ERR_NOMEMORY: msg = "ran out of memory"; break;
         case ERR_PARSE:
            msg = "unable to process batch file";
            errfile = infilename;
            break;
         default: msg = "unknown error"; break;
      }

      // Show message on screen
      if (errfile != NULL)
         fprintf(stderr, "Error[%s]: %s\n", errfile, msg);
      else
         fprintf(stderr, "Error: %s\n", msg);
   }

   // Quit program
   return errcode ? EXIT_FAILURE : EXIT_SUCCESS;
}

//***************************************************************************
// build_tilemap
// Does a quick build using tilemap arrangement
//---------------------------------------------------------------------------
// param infilename: name of input file
// param outfilename: name of output file
// return: error code
//***************************************************************************

int build_tilemap(const char *infilename, const char *outfilename) {
   // Load input bitmap
   Bitmap *in = load_bitmap(infilename);
   if (in == NULL)
      return ERR_OPENINPUT;

   // Open output file
   FILE *out = fopen(outfilename, "wb");
   if (out == NULL) {
      destroy_bitmap(in);
      return ERR_OPENOUTPUT;
   }

   // Parse the bitmap as a massive tilemap
   int errcode = write_tilemap(in, out, 0, 0,
      in->width >> 3, in->height >> 3);

   // We're done, return whatever happened
   fclose(out);
   destroy_bitmap(in);
   return errcode;
}

//***************************************************************************
// build_sprite
// Does a quick build using sprite arrangement
//---------------------------------------------------------------------------
// param infilename: name of input file
// param outfilename: name of output file
// return: error code
//***************************************************************************

int build_sprite(const char *infilename, const char *outfilename) {
   // Load input bitmap
   Bitmap *in = load_bitmap(infilename);
   if (in == NULL)
      return ERR_OPENINPUT;

   // Open output file
   FILE *out = fopen(outfilename, "wb");
   if (out == NULL) {
      destroy_bitmap(in);
      return ERR_OPENOUTPUT;
   }

   // Parse the bitmap as a massive sprite
   int errcode = write_sprite(in, out, 0, 0,
      in->width >> 3, in->height >> 3);

   // We're done, return whatever happened
   fclose(out);
   destroy_bitmap(in);
   return errcode;
}
