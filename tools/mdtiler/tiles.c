//***************************************************************************
// "tiles.c"
// Tile fetching stuff
//***************************************************************************
// mdtiler - Bitmap to tile conversion tool
// Copyright 2011, 2012, 2015, 2018 Javier Degirolmo
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
#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "bitmap.h"
#include "palette.h"
#include "tiles.h"

// Prototype for functions used to fetch tiles
typedef int TileFunc(const Bitmap *, FILE *, int, int);

// Current tile output format
static Format format = FORMAT_4BPP;

//***************************************************************************
// get_tile
// Retrieves the contents of a tile. Format is always 4bpp.
//---------------------------------------------------------------------------
// param in: input bitmap
// param out: where to store data
// param xr: base X coordinate (leftmost pixel of tile)
// param yr: base Y coordinate (topmost pixel of tile)
//***************************************************************************

void get_tile(const Bitmap *in, Tile *out, int bx, int by) {
   // To keep track of the flags (palette and priority)
   uint8_t flags = 0;

   // Scan all rows
   for (int y = 0; y < 8; y++) {
      // To store the pixels as we fetch them
      uint32_t normal = 0;    // Left to right
      uint32_t flipped = 0;   // Right to left

      // Retrieve pixels of this row
      for (int x = 0; x < 8; x++) {
         uint8_t pixel = get_pixel(in, bx + x, by + y);
         flags |= (pixel >> 4) & 0x0F;
         pixel &= 0x0F;
         normal = normal << 4 | pixel;
         flipped = flipped >> 4 | pixel << 28;
      }

      // Store rows in tile data
      out->normal[y] = normal;
      out->flipped[y] = flipped;
   }

   // Store flags
   out->flags = get_palette_mapping(flags);
}

//***************************************************************************
// get_output_format
// Retrieves what's the current output format for tiles
//---------------------------------------------------------------------------
// return: current format
//***************************************************************************

Format get_output_format(void) {
   return format;
}

//***************************************************************************
// set_output_format
// Changes the output format for tiles
//---------------------------------------------------------------------------
// param value: new format
//***************************************************************************

void set_output_format(Format value) {
   format = value;
}

//***************************************************************************
// write_tile_1bpp
// Takes a tile from the bitmap and outputs a 1bpp tile
//---------------------------------------------------------------------------
// param in: input bitmap
// param out: output file
// param xr: base X coordinate (leftmost pixel of tile)
// param yr: base Y coordinate (topmost pixel of tile)
// return: error code
//***************************************************************************

static int write_tile_1bpp(const Bitmap *in, FILE *out, int bx, int by)
{
   // To store the tile data
   uint8_t data[8];

   // Read the tile from the bitmap to generate the 4bpp data
   uint8_t *ptr = data;
   for (int y = 0; y < 8; y++, ptr++) {
      uint8_t temp = 0;
      for (int x = 0; x < 8; x++) {
         temp <<= 1;
         temp |= get_pixel(in, bx + x, by + y) & 0x01;
      }
      *ptr = temp;
   }

   // Write tile blob into output file
   if (fwrite(data, 1, 8, out) < 8)
      return ERR_CANTWRITE;

   // Success!
   return ERR_NONE;
}

//***************************************************************************
// write_tile_4bpp
// Takes a tile from the bitmap and outputs a 4bpp tile
//---------------------------------------------------------------------------
// param in: input bitmap
// param out: output file
// param xr: base X coordinate (leftmost pixel of tile)
// param yr: base Y coordinate (topmost pixel of tile)
// return: error code
//***************************************************************************

static int write_tile_4bpp(const Bitmap *in, FILE *out, int bx, int by)
{
   // To store the tile data
   uint8_t data[32];

   // Read the tile from the bitmap to generate the 4bpp data
   uint8_t *ptr = data;
   for (int y = 0; y < 8; y++)
   for (int x = 0; x < 8; x += 2) {
      *ptr++ = (get_pixel(in, bx + x, by + y) << 4) |
               (get_pixel(in, bx + x + 1, by + y) & 0x0F);
   }

   // Write tile blob into output file
   if (fwrite(data, 1, 32, out) < 32)
      return ERR_CANTWRITE;

   // Success!
   return ERR_NONE;
}

//***************************************************************************
// write_tile_error
// Tile writing function to use if there was an error with the format
//---------------------------------------------------------------------------
// param in: (ignored)
// param out: (ignored)
// param xr: (ignored)
// param yr: (ignored)
// return: ERR_UNKNOWN
//***************************************************************************

static int write_tile_error(const Bitmap *in, FILE *out, int bx, int by)
{
   // To shut up the compiler
   (void) in;
   (void) out;
   (void) bx;
   (void) by;

   // Don't do anything, just panic
   return ERR_UNKNOWN;
}

//***************************************************************************
// get_write_func
// Returns which function to use to write tiles in the current format
//---------------------------------------------------------------------------
// return: pointer to function
//***************************************************************************

static inline TileFunc *get_write_func(void) {
   // Return the adequate function for this format
   // write_tile_error is used if there's a bug in the program...
   switch (format) {
      case FORMAT_4BPP: return write_tile_4bpp;
      case FORMAT_1BPP: return write_tile_1bpp;
      default: return write_tile_error;
   }
}

//***************************************************************************
// write_tilemap
// Outputs a block of tiles using tilemap ordering
//---------------------------------------------------------------------------
// param in: input bitmap
// param out: output file
// param xr: base X coordinate (leftmost pixel of tile)
// param yr: base Y coordinate (topmost pixel of tile)
// param width: width in tiles
// param height: height in tiles
// return: error code
//***************************************************************************

int write_tilemap(const Bitmap *in, FILE *out, int bx, int by,
int width, int height) {
   // Determine function we're going to use to fetch tiles
   TileFunc *func = get_write_func();

   // Traverse through all tiles in tilemap ordering
   // (left-to-right, then top-to-bottom)
   for (int y = 0; y < height; y++)
   for (int x = 0; x < width; x++) {
      int errcode = func(in, out, bx + (x << 3), by + (y << 3));
      if (errcode) return errcode;
   }

   // Success!
   return ERR_NONE;
}

//***************************************************************************
// write_sprite
// Outputs a block of tiles using sprite ordering
//---------------------------------------------------------------------------
// param in: input bitmap
// param out: output file
// param xr: base X coordinate (rightmost pixel of tile)
// param yr: base Y coordinate (topmost pixel of tile)
// param width: width in tiles
// param height: height in tiles
// return: error code
//***************************************************************************

int write_sprite(const Bitmap *in, FILE *out, int bx, int by,
int width, int height) {
   // Determine function we're going to use to fetch tiles
   TileFunc *func = get_write_func();

   // Sprites are at most 4 tiles high, so split sprites into strips that
   // have at most that length
   while (height > 0) {
      // Determine height for this strip
      int strip_height = height > 4 ? 4 : height;

      // Traverse through all tiles in sprite ordering
      // (top-to-bottom, then left-to-right)
      for (int x = 0; x < width; x++)
      for (int y = 0; y < strip_height; y++) {
         int errcode = func(in, out, bx + (x << 3), by + (y << 3));
         if (errcode) return errcode;
      }

      // Move onto the next strip
      height -= strip_height;
      by += strip_height << 3;
   }

   // Success!
   return ERR_NONE;
}
