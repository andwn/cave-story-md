//***************************************************************************
// "bitmap.h"
// Header file for "bitmap.c"
//***************************************************************************
// mdtiler - Bitmap to tile conversion tool
// Copyright 2011, 2012 Javier Degirolmo
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

#ifndef BITMAP_H
#define BITMAP_H

// Required headers
#include <stdint.h>

// Definition of a bitmap
typedef struct {
   int width;              // Width in pixels
   int height;             // Height in pixels
   uint8_t *data;          // Bitmap data
   uint8_t **rows;         // Pointers to each row
} Bitmap;

// Function prototypes
void set_palette(const uint16_t *);
Bitmap *load_bitmap(const char *);
uint8_t get_pixel(const Bitmap *, int, int);
void destroy_bitmap(Bitmap *);

#endif
