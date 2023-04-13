//***************************************************************************
// "batch.c"
// Batch file processing
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
#include <ctype.h>
#include "main.h"
#include "bitmap.h"
#include "offset.h"
#include "map.h"
#include "palette.h"
#include "sprite.h"
#include "tiles.h"

// Possible layout formats
typedef enum {
   LAYOUT_TILEMAP,
   LAYOUT_SPRITE
} Layout;

// Structure for storing token information
typedef struct {
   char **tokens;             // Pointer to token list
   unsigned num_tokens;       // Number of tokens
} TokenList;

// Function prototypes
static int read_line(FILE *, char **);
static int split_tokens(const char *, TokenList *);
static void free_tokens(TokenList *);
static void print_error_line(size_t, const char *);
static int is_integer(const char *);
static int is_color(const char *);
static unsigned string_to_integer(const char *);
static char *make_path(const char *, const char *);

//***************************************************************************
// build_batch
// Performs a batch build
//---------------------------------------------------------------------------
// param infilename: name of batch file
// return: error code
//***************************************************************************

int build_batch(const char *infilename) {
   // To store error codes
   int errcode;

   // If there was any parsing error, this variable gets set
   int failed = 0;

   // Open batch file
   FILE *file = fopen(infilename, "r");
   if (file == NULL)
      return ERR_OPENBATCH;

   // Current state
   Bitmap *in = NULL;                  // Input bitmap
   FILE *out[2] = { NULL, NULL };      // Output blobs
   Layout layout = LAYOUT_TILEMAP;     // Tile ordering

   // Default base directory
   char *basedir = malloc(2);
   if (basedir == NULL) {
      fclose(file);
      return ERR_NOMEMORY;
   }
   strcpy(basedir, ".");

   // Go through all lines
   for (size_t curr_line = 1; !feof(file); curr_line++) {
      // Get next line
      char *line;
      errcode = read_line(file, &line);
      if (errcode) {
         fclose(file);
         return errcode;
      }

      // Get a list of the arguments in this line
      TokenList args;
      errcode = split_tokens(line, &args);
      if (errcode) {
         // Syntax errors?
         if (errcode == ERR_BADQUOTE) {
            print_error_line(curr_line, infilename);
            fputs("quote inside non-quoted token\n", stderr);
         } else if (errcode == ERR_NOQUOTE) {
            print_error_line(curr_line, infilename);
            fputs("missing ending quote\n", stderr);
         }

         // Nope, something more serious, give up
         else {
            free(line);
            fclose(file);
            free(basedir);
            return errcode;
         }

         // Can't continue with this line, move on...
         failed = 1;
         free(line);
         continue;
      }

      // Done with the line
      free(line);

      // Get number of arguments in this line
      size_t num_args = args.num_tokens;

      // Empty sentence? (ignore)
      if (num_args == 0)
         continue;

      // Retrieve what command is it
      const char *command = args.tokens[0];

      // Set input file?
      if (!strcmp(command, "input")) {
         // Check number of arguments
         if (num_args != 2) {
            // Determine error message
            const char *msg = num_args == 1 ?
               "input filename not specified\n" :
               "too many parameters\n";

            // Show message on screen
            print_error_line(curr_line, infilename);
            fputs(msg, stderr);
            failed = 1;
         }

         // Set input file if arguments are valid
         else {
            // Close old bitmap if needed
            if (in != NULL)
               destroy_bitmap(in);

            // Attempt to load input bitmap
            char *filename = make_path(basedir, args.tokens[1]);
            if (filename == NULL) {
               errcode = ERR_NOMEMORY;
               goto panic;
            }
            in = load_bitmap(filename);

            // Oops?
            if (in == NULL) {
               print_error_line(curr_line, infilename);
               fprintf(stderr, "can't load input bitmap \"%s\"\n", filename);
               failed = 1;
            }

            free(filename);
         }
      }

      // Set main output files?
      else if (!strcmp(command, "output") || !strcmp(command, "output2")) {
         // Check number of arguments
         if (num_args != 2) {
            // Determine error message
            const char *msg = num_args == 1 ?
               "output filename not specified\n" :
               "too many parameters\n";

            // Show message on screen
            print_error_line(curr_line, infilename);
            fputs(msg, stderr);
            failed = 1;
         }

         // Set output file if arguments are valid
         else {
            // Determine which output file to mess with
            // Yeah, this looks hackish XD but it's correct
            int which = (command[6] == '2') ? 1 : 0;

            // Close old file if needed
            if (out[which] != NULL)
               fclose(out[which]);

            // Attempt to open output file
            char *filename = make_path(basedir, args.tokens[1]);
            if (filename == NULL) {
               errcode = ERR_NOMEMORY;
               goto panic;
            }
            out[which] = fopen(filename, "wb");

            // Oops?
            if (out[which] == NULL) {
               print_error_line(curr_line, infilename);
               fprintf(stderr, "can't open output bitmap \"%s\"\n",
                  filename);
               failed = 1;
            }

            free(filename);
         }
      }

      // Set palette?
      else if (!strcmp(command, "palette")) {
         // Check number of arguments
         if (num_args != 17) {
            // Determine error message
            const char *msg;
            if (num_args == 1)
               msg = "no colors specified\n";
            else if (num_args > 17)
               msg = "too many parameters\n";
            else if (num_args == 16)
               msg = "1 color missing\n";
            else
               msg = "%d colors missing\n";

            // Show message on screen
            print_error_line(curr_line, infilename);
            fprintf(stderr, msg, 17 - num_args);
            failed = 1;
         }

         // Parse palette
         else {
            // Where we store the palette
            uint16_t palette[0x10];

            // Read entire palette
            for (unsigned i = 0; i < 0x10; i++) {
               // Get token for this entry
               const char *arg = args.tokens[i + 1];

               // Check that it's a valid color value
               if (!is_color(arg)) {
                  print_error_line(curr_line, infilename);
                  fprintf(stderr, "\"%s\" is not a valid color\n", arg);
                  failed = 1;
               }

               // Parse value
               palette[i] = strtoul(arg, NULL, 0x10);
            }

            // Set new palette
            set_palette(palette);
         }
      }

      // Set tile ordering?
      else if (!strcmp(command, "layout")) {
         // Check number of arguments
         if (num_args != 2) {
            // Determine error message
            const char *msg = num_args == 1 ?
               "layout not specified\n" :
               "too many parameters\n";

            // Show message on screen
            print_error_line(curr_line, infilename);
            fputs(msg, stderr);
            failed = 1;
         }

         // Determine output layout
         else {
            // Get layout
            const char *param = args.tokens[1];

            // Check if it's a supported layout
            if (!strcmp(param, "tilemap"))
               layout = LAYOUT_TILEMAP;
            else if (!strcmp(param, "sprite"))
               layout = LAYOUT_SPRITE;

            // Nope
            else {
               print_error_line(curr_line, infilename);
               fprintf(stderr, "unknown layout type \"%s\"\n", param);
               failed = 1;
            }
         }
      }

      // Set output format?
      else if (!strcmp(command, "format")) {
         // Check number of arguments
         if (num_args != 2) {
            // Determine error message
            const char *msg = num_args == 1 ?
               "format not specified\n" :
               "too many parameters\n";

            // Show message on screen
            print_error_line(curr_line, infilename);
            fputs(msg, stderr);
            failed = 1;
         }

         // Determine output format
         else {
            // Get format
            const char *param = args.tokens[1];

            // Check if it's a supported format
            if (!strcmp(param, "4bpp"))
               set_output_format(FORMAT_4BPP);
            else if (!strcmp(param, "1bpp"))
               set_output_format(FORMAT_1BPP);

            // Nope
            else {
               print_error_line(curr_line, infilename);
               fprintf(stderr, "unknown format \"%s\"\n", param);
               failed = 1;
            }
         }
      }

      // Write tiles?
      else if (!strcmp(command, "tiles")) {
         // Check number of arguments
         if (num_args != 5) {
            // Determine error message
            const char *msg;
            switch (num_args) {
               case 1: msg = "missing coordinates and dimensions\n"; break;
               case 2: msg = "missing Y coordinate and dimensions\n"; break;
               case 3: msg = "missing dimensions\n"; break;
               case 4: msg = "missing height\n"; break;
               default: msg = "too many parameters\n"; break;
            }

            // Show message on screen
            print_error_line(curr_line, infilename);
            fputs(msg, stderr);
            failed = 1;
         }

         // Make sure there's a bitmap to read from...
         else if (in == NULL) {
            print_error_line(curr_line, infilename);
            fputs("no input file to read from\n", stderr);
            failed = 1;
         }

         // Make sure there's a file to write into...
         else if (out[0] == NULL) {
            print_error_line(curr_line, infilename);
            fputs("no output file to write into\n", stderr);
            failed = 1;
         }

         // Everything is seemingly OK, process command
         else {
            // Retrieve parameters
            // To-do: check that they're indeed integers, but for now it
            // isn't much of an issue because at worst atoi will return 0
            int x = string_to_integer(args.tokens[1]) << 3;
            int y = string_to_integer(args.tokens[2]) << 3;
            int width = string_to_integer(args.tokens[3]);
            int height = string_to_integer(args.tokens[4]);

            // Process tiles and write them in the output file
            switch (layout) {
               // Tilemap ordering
               case LAYOUT_TILEMAP:
                  errcode = write_tilemap(in, out[0], x, y, width, height);
                  break;

               // Sprite ordering
               case LAYOUT_SPRITE:
                  errcode = write_sprite(in, out[0], x, y, width, height);
                  break;
            }

            // Gah!
            if (errcode) {
               free_tokens(&args);
               goto panic;
            }
         }
      }

      // Generate map?
      else if (!strcmp(command, "map")) {
         // Check number of arguments
         if (num_args != 5) {
            // Determine error message
            const char *msg;
            switch (num_args) {
               case 1: msg = "missing coordinates and dimensions\n"; break;
               case 2: msg = "missing Y coordinate and dimensions\n"; break;
               case 3: msg = "missing dimensions\n"; break;
               case 4: msg = "missing height\n"; break;
               default: msg = "too many parameters\n"; break;
            }

            // Show message on screen
            print_error_line(curr_line, infilename);
            fputs(msg, stderr);
            failed = 1;
         }

         // Make sure there's a bitmap to read from...
         else if (in == NULL) {
            print_error_line(curr_line, infilename);
            fputs("no input file to read from\n", stderr);
            failed = 1;
         }

         // Make sure there's a file to write tiles into...
         else if (out[0] == NULL) {
            print_error_line(curr_line, infilename);
            fputs("no output file to write tiles\n", stderr);
            failed = 1;
         }

         // Make sure there's a file to write mappings into...
         else if (out[1] == NULL) {
            print_error_line(curr_line, infilename);
            fputs("no output file to write tilemap mappings\n", stderr);
            failed = 1;
         }

         // Everything is seemingly OK, process command
         else {
            // Retrieve parameters
            // To-do: check that they're indeed integers, but for now it
            // isn't much of an issue because at worst atoi will return 0
            int x = string_to_integer(args.tokens[1]) << 3;
            int y = string_to_integer(args.tokens[2]) << 3;
            int width = string_to_integer(args.tokens[3]);
            int height = string_to_integer(args.tokens[4]);

            // Generate map
            errcode = generate_map(in, out[0], out[1], x, y, width, height,
               layout == LAYOUT_SPRITE);

            // Gah!
            if (errcode) {
               free_tokens(&args);
               goto panic;
            }
         }
      }

      // Generate sprite mapping?
      else if (!strcmp(command, "sprite")) {
         // This command only makes sense with sprite ordering!
         if (layout != LAYOUT_SPRITE) {
            print_error_line(curr_line, infilename);
            fputs("\"sprite\" command only works with sprite ordering\n",
               stderr);
            failed = 1;
         }

         // End of sprite?
         else if (num_args == 2 && strcmp(args.tokens[1], "end") == 0) {
            errcode = generate_sprite_end(out[1]);
            if (errcode) {
               free_tokens(&args);
               goto panic;
            }
         }

         // Check number of arguments then
         // Should be identical to "tiles"
         else if (num_args != 5) {
            // Determine error message
            const char *msg;
            switch (num_args) {
               case 1: msg = "missing coordinates and dimensions\n"; break;
               case 2: msg = "missing Y coordinate and dimensions\n"; break;
               case 3: msg = "missing dimensions\n"; break;
               case 4: msg = "missing height\n"; break;
               default: msg = "too many parameters\n"; break;
            }

            // Show message on screen
            print_error_line(curr_line, infilename);
            fputs(msg, stderr);
            failed = 1;
         }

         // Make sure there's a bitmap to read from...
         else if (in == NULL) {
            print_error_line(curr_line, infilename);
            fputs("no input file to read from\n", stderr);
            failed = 1;
         }

         // Make sure there's a file to write into...
         else if (out[0] == NULL) {
            print_error_line(curr_line, infilename);
            fputs("no output file to write into\n", stderr);
            failed = 1;
         }

         // Make sure there's a file to write mappings into...
         else if (out[1] == NULL) {
            print_error_line(curr_line, infilename);
            fputs("no output file to write sprite mappings\n", stderr);
            failed = 1;
         }

         // Everything is seemingly OK, process command
         else {
            // Retrieve parameters
            // To-do: check that they're indeed integers, but for now it
            // isn't much of an issue because at worst atoi will return 0
            int x = string_to_integer(args.tokens[1]) << 3;
            int y = string_to_integer(args.tokens[2]) << 3;
            int width = string_to_integer(args.tokens[3]);
            int height = string_to_integer(args.tokens[4]);

            // Generate sprite mapping
            errcode = generate_sprite(in, out[0], out[1],
               x, y, width, height);
            if (errcode) {
               free_tokens(&args);
               goto panic;
            }
         }
      }

      // Set offset for map/sprite?
      else if (!strcmp(command, "offset")) {
         // Check number of arguments
         if (num_args != 2) {
            // Determine error message
            const char *msg;
            switch (num_args) {
               case 1: msg = "missing offset\n"; break;
               default: msg = "too many parameters\n"; break;
            }

            // Show message on screen
            print_error_line(curr_line, infilename);
            fputs(msg, stderr);
            failed = 1;
         }

         // Check if the argument is to toggle continuous mode
         int done = 0;
         if (!failed) {
            if (strcmp(args.tokens[1], "continuous") == 0) {
               set_continuous_offset(1);
               done = 1;
            } else if (strcmp(args.tokens[1], "restarting") == 0) {
               set_continuous_offset(0);
               done = 1;
            }
         }

         // If not check that the offset is indeed an integer
         if (!failed && !done) {
            if (!is_integer(args.tokens[1])) {
               print_error_line(curr_line, infilename);
               fputs("offset must be an integer\n", stderr);
               failed = 1;
            }
         }

         // All OK, set new offset
         if (!failed && !done) {
            set_map_offset(string_to_integer(args.tokens[1]));
         }
      }

      // Set origin for sprite mappings?
      else if (!strcmp(command, "origin")) {
         // Check number of arguments
         if (num_args != 3) {
            // Determine error message
            const char *msg;
            switch (num_args) {
               case 1: msg = "missing coordinates\n"; break;
               case 2: msg = "missing Y coordinate\n"; break;
               default: msg = "too many parameters\n"; break;
            }

            // Show message on screen
            print_error_line(curr_line, infilename);
            fputs(msg, stderr);
            failed = 1;
         }

         // Change sprite origin
         if (!failed) {
            int x = string_to_integer(args.tokens[1]);
            int y = string_to_integer(args.tokens[2]);
            set_sprite_origin(x, y);
         }
      }

      // Remap palettes?
      else if (!strcmp(command, "remappal")) {
         // group -> palette notation?
         if (num_args == 4 && strcmp(args.tokens[2], "->") == 0) {
            int group = string_to_integer(args.tokens[1]);
            int palette = string_to_integer(args.tokens[3]);

            if (group < 0 || group > 15) {
               print_error_line(curr_line, infilename);
               fputs("color group must be between 0 and 15\n", stderr);
               failed = 1;
            }
            if (palette < 0 || palette > 7) {
               print_error_line(curr_line, infilename);
               fputs("palette must be between 0 and 7\n", stderr);
               failed = 1;
            }

            remap_palette(group, palette);
         }

         // Other notations not implemented yet
         else {
           print_error_line(curr_line, infilename);
           fputs("invalid arguments\n", stderr);
           failed = 1;
         }
      }

      // Dump bitmap palette?
      else if (!strcmp(command, "dumppal")) {
         // Check number of arguments
         if (num_args != 2) {
            // Determine error message
            const char *msg = num_args == 1 ?
               "palette filename not specified\n" :
               "too many parameters\n";

            // Show message on screen
            print_error_line(curr_line, infilename);
            fputs(msg, stderr);
            failed = 1;
         }

         else {
            // Attempt to open output file
            char *filename = make_path(basedir, args.tokens[1]);
            if (filename == NULL) {
               errcode = ERR_NOMEMORY;
               goto panic;
            }

            FILE *file = fopen(filename, "wb");
            if (file == NULL) {
               print_error_line(curr_line, infilename);
               fprintf(stderr, "can't open output palette \"%s\"\n",
                  filename);
               failed = 1;
            }

            // Try to write it
            int success = 1;
            if (!failed) {
               success = dump_bitmap_palette(file);
               fclose(file);
            }
            if (!success) {
               print_error_line(curr_line, infilename);
               fprintf(stderr, "can't write output palette \"%s\"\n",
                  filename);
               failed = 1;
            }

            free(filename);
         }
      }

      // Change base directory?
      else if (!strcmp(command, "basedir")) {
         if (num_args != 2) {
            // Determine error message
            const char *msg;
            switch (num_args) {
               case 1: msg = "missing directory\n"; break;
               default: msg = "too many parameters\n"; break;
            }

            // Show message on screen
            print_error_line(curr_line, infilename);
            fputs(msg, stderr);
            failed = 1;
         }

         // Change base directory
         if (!failed) {
            free(basedir);
            basedir = malloc(strlen(args.tokens[1]) + 1);
            if (basedir == NULL) {
               errcode = ERR_NOMEMORY;
               goto panic;
            }
            strcpy(basedir, args.tokens[1]);
         }
      }

      // Unknown command?
      else {
         print_error_line(curr_line, infilename);
         fprintf(stderr, "unknown command \"%s\"\n", command);
         failed = 1;
      }

      // Get rid of arguments
      free_tokens(&args);
   }

   // Done with the resources
   if (in) destroy_bitmap(in);
   if (out[0]) fclose(out[0]);
   if (out[1]) fclose(out[1]);

   // We're done
   free(basedir);
   fclose(file);
   return failed ? ERR_PARSE : ERR_NONE;

   // If we get here then something went SERIOUSLY wrong
panic:
   if (in) destroy_bitmap(in);
   if (out[0]) fclose(out[0]);
   if (out[1]) fclose(out[1]);
   free(basedir);
   fclose(file);
   return errcode;
}

//***************************************************************************
// read_line
// Reads a line from a text file. The pointer to the buffer should be freed
// once it isn't needed anymore.
//---------------------------------------------------------------------------
// param file: input file
// param where: where to store a pointer to the line
// return: error code
//***************************************************************************

#define BUFLINE_SIZE 0x80
#define BUFLINE_STEP 0x20

static int read_line(FILE *file, char **where) {
   // Set the pointer to null for now (if there's an error, this is what the
   // caller will see)
   *where = NULL;

   // Initial size of the buffer
   size_t bufsize = BUFLINE_SIZE;
   size_t bufpos = 0;

   // Allocate initial buffer for the line
   // Allocate one extra char for the terminating null
   char *buffer = (char *) malloc(bufsize+1);
   if (buffer == NULL)
      return ERR_NOMEMORY;

   // Keep reading until the end of the line
   for (;;) {
      // Get next character
      int c = fgetc(file);

      // End of file? Error?
      if (c == EOF) {
         if (feof(file))
            break;
         else {
            free(buffer);
            return ERR_CANTREAD;
         }
      }

      // End of line? (take backslash-newline combination into account)
      if (c == '\n' || c == '\r') {
         if (bufpos > 0 && buffer[bufpos-1] == '\\')
            bufpos--;
         else
            break;
      }

      // Null character? (skip!)
      if (c == '\0')
         continue;

      // Put character into the buffer
      buffer[bufpos] = c;
      bufpos++;

      // Do we need a larger buffer?
      if (bufpos == bufsize) {
         bufsize += BUFLINE_STEP;
         char *temp = (char *) malloc(bufsize+1);
         if (temp == NULL) {
            free(buffer);
            return ERR_NOMEMORY;
         } else
            buffer = temp;
      }
   }

   // Success!
   buffer[bufpos] = '\0';
   *where = buffer;
   return ERR_NONE;
}

//***************************************************************************
// split_tokens [internal]
// Parses a line and splits it into tokens (generating a token list).
//---------------------------------------------------------------------------
// param str: line to split into tokens
// param list: where to store token information
// return: error code
//***************************************************************************

static int split_tokens(const char *str, TokenList *list) {
   // Reset list
   list->tokens = NULL;
   list->num_tokens = 0;

   // Go through entire line
   while (*str) {
      // Skip whitespace
      if (isspace(*str)) {
         str++;
         continue;
      }

      // Comment? End of line?
      if (*str == '#' || *str == '\0')
         break;

      // Unquoted token?
      if (*str != '\"') {
         // Make room for a new token in the list
         char **temp_list = (char **) realloc(list->tokens,
            sizeof(char *) * (list->num_tokens + 1));
         if (temp_list == NULL) {
            free_tokens(list);
            return ERR_NOMEMORY;
         } else
            list->tokens = temp_list;

         // Token starts here
         const char *start = str;

         // Try to find the end of the token
         // Also make sure there aren't any quotes inside...
         do {
            str++;
            if (*str == '\"') {
               free_tokens(list);
               return ERR_BADQUOTE;
            }
         } while (!(isspace(*str) || *str == '\0'));

         // Get length of the token
         size_t length = str - start;

         // Allocate memory for this token
         char *buffer = (char *) malloc(length + 1);
         if (buffer == NULL) {
            free_tokens(list);
            return ERR_NOMEMORY;
         } else {
            memcpy(buffer, start, length);
            buffer[length] = '\0';
         }

         // Put token inside the token list
         list->tokens[list->num_tokens] = buffer;
         list->num_tokens++;
      }

      // Quoted token?
      else {
         // Make room for a new token in the list
         char **temp_list = (char **) realloc(list->tokens,
            sizeof(char *) * (list->num_tokens + 1));
         if (temp_list == NULL) {
            free_tokens(list);
            return ERR_NOMEMORY;
         } else
            list->tokens = temp_list;

         // Skip quote
         str++;

         // Mark where the token starts
         // Also we need to count this character by character, since escaped
         // quotes take up two bytes in the string but they will be kept as
         // a single one.
         const char *start = str;
         size_t length = 0;

         // Look for the ending quote
         for (;;) {
            // Uh oh, missing quote...
            if (*str == '\0') {
               free_tokens(list);
               return ERR_NOQUOTE;
            }

            // Quote? Check if it's an escaped quote or the end of the token
            else if (*str == '\"') {
               str++;
               if (*str == '\"') {
                  length++;
                  str++;
               } else
                  break;
            }

            // Normal character, count it as usual
            else {
               length++;
               str++;
            }
         }

         // Allocate memory for this token
         char *buffer = (char *) malloc(length + 1);
         if (buffer == NULL) {
            free_tokens(list);
            return ERR_NOMEMORY;
         }

         // Copy token into buffer, taking into account escaped quotes (which
         // will be stored as a single quote)
         char *ptr = buffer;
         for (size_t i = 0; i < length; i++) {
            if (*start == '\"') {
               *ptr++ = '\"';
               start += 2;
            } else
               *ptr++ = *start++;
         }
         *ptr = '\0';

         // Put token inside the token list
         list->tokens[list->num_tokens] = buffer;
         list->num_tokens++;
      }
   }

   // Success!
   return ERR_NONE;
}

//***************************************************************************
// free_tokens [internal]
// Deallocates the information in a token list.
//---------------------------------------------------------------------------
// param list: pointer to tokens list
//***************************************************************************

static void free_tokens(TokenList *list) {
   // Any tokens?
   if (list->tokens != NULL) {
      // Get rid of the tokens themselves
      for (unsigned i = 0; i < list->num_tokens; i++)
         free(list->tokens[i]);

      // Get rid of pointer list
      free(list->tokens);
   }

   // Reset list
   list->tokens = NULL;
   list->num_tokens = 0;
}

//***************************************************************************
// print_error_line
// Prints the header for syntax error messages
//---------------------------------------------------------------------------
// param line: current line ID
// param filename: name of file with error (can be NULL)
//***************************************************************************

static void print_error_line(size_t line, const char *filename) {
   if (filename == NULL) filename = "";

   if (filename[0] == '\0')
      fprintf(stderr, "Error[%zu]: ", line);
   else
      fprintf(stderr, "Error[%s:%zu]: ", filename, line);
}

//***************************************************************************
// is_integer
// Checks if a string is a valid integer value
//---------------------------------------------------------------------------
// param str: string to check
// return: non-zero if valid, zero if not valid
//***************************************************************************

static int is_integer(const char *str) {
   // Er...
   if (str == NULL)
      return 0;
   if (*str == '\0')
      return 0;

   // Determine which characters are allowed
   const char *allowed;
   if (*str == '$') {
      allowed = "0123456789ABCDEFabcdef";
      str++;
   } else {
      allowed = "0123456789";
   }

   // Check that only allowed characters are inside
   for (; *str != '\0'; str++) {
      if (strchr(allowed, *str) == NULL)
         return 0;
   }

   // Valid integer!
   return 1;
}

//***************************************************************************
// is_color
// Checks if a string is a valid Mega Drive color value
//---------------------------------------------------------------------------
// param str: string to check
// return: non-zero if valid, zero if not valid
//***************************************************************************

static int is_color(const char *str) {
   // Er...
   if (str == NULL)
      return 0;
   if (*str == '\0')
      return 0;

   // Allowed characters
   static const char *valid = "02468ACEace";

   // Check that it only consists of valid characters
   for (unsigned i = 0; i < 3; i++) {
      if (strchr(valid, *str) == NULL)
         return 0;
   }

   // Colors are always three characters long
   if (str[3] != '\0')
      return 0;

   // Valid color!
   return 1;
}

//***************************************************************************
// string_to_integer
// Converts a string into an integer. Handles both decimal and hexadecimal.
//---------------------------------------------------------------------------
// param str: string to convert
// return: integer value in string
//***************************************************************************

static unsigned string_to_integer(const char *str) {
   // Hexadecimal?
   if (*str == '$') {
      str++;
      return strtoul(str, NULL, 16);
   }

   // Decimal?
   else {
      return strtoul(str, NULL, 10);
   }
}

//***************************************************************************
// make_path
// Takes a base directory and a filename (which may itself contain
// subdirectories) and returns a string with them combined (literally
// "basedir/filename"). You must call free() once you're done with the
// string.
//---------------------------------------------------------------------------
// param basedir: base directory
// param filename: filename
// return: pointer to string (NULL if failure)
//***************************************************************************

static char *make_path(const char *basedir, const char *filename)
{
   size_t len = strlen(basedir) + strlen(filename) + 2;
   char *buffer = malloc(len);
   if (buffer == NULL) return NULL;

   sprintf(buffer, "%s/%s", basedir, filename);
   return buffer;
}
