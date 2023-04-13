//***************************************************************************
// "tiles.h"
// Header file for "tiles.c"
//***************************************************************************
// mdtiler - Bitmap to tile conversion tool
// Copyright 2011, 2012, 2015 Javier Degirolmo
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

#ifndef TILES_H
#define TILES_H

// Required headers
#include <stdint.h>
#include <stdio.h>
#include "bitmap.h"

// Possible tile output formats
typedef enum {
   FORMAT_DEFAULT,      // Use default format
   FORMAT_4BPP,         // 4bpp tiles
   FORMAT_1BPP,         // 1bpp tiles
   FORMAT_TOOMANY       // Too many formats specified
} Format;

// Structure to hold data for a tile
typedef struct {
   uint32_t normal[8];     // Row data (not flipped)
   uint32_t flipped[8];    // Row data (flipped horizontally)
   uint8_t flags;          // Palette and priority
} Tile;

// Function prototypes
void get_tile(const Bitmap *, Tile *, int, int);
Format get_output_format(void);
void set_output_format(Format);
int write_tilemap(const Bitmap *, FILE *, int, int, int, int);
int write_sprite(const Bitmap *, FILE *, int, int, int, int);

#endif
