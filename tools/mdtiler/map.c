//***************************************************************************
// "map.c"
// Generates map data out of a bitmap
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "bitmap.h"
#include "offset.h"
#include "tiles.h"

//***************************************************************************
// generate_map
// Takes care of the "map" command in mdtiler. Generates the tilemap and the
// tiles out of it.
//---------------------------------------------------------------------------
// param in: input bitmap
// param outgfx: output file where tiles are stored
// param outmap: output file where mappings are stored
// param x: base X coordinate (leftmost tile)
// param y: base Y coordinate (topmost tile)
// param width: width in tiles
// param height: height in tiles
// param order: zero for tilemap order, non-zero for sprite order
// return: error code
//***************************************************************************

int generate_map(const Bitmap *in, FILE *outgfx, FILE *outmap,
int x, int y, int width, int height, int order) {
   // Um...
   if (width <= 0 || height <= 0)
      return ERR_PARSE;

   // To store the mappings
   uint16_t *mappings = (uint16_t *) malloc(sizeof(uint16_t) *
      width * height);
   if (mappings == NULL)
      return ERR_NOMEMORY;

   // Get current offset
   uint16_t offset = get_map_offset();

   // To store each tile
   Tile *tiles = NULL;
   uint16_t num_tiles = 0;

   // To store the tile we're just checking
   Tile curr_tile;

   // Use a mask to filter out bits we don't care about depending on the
   // format (this is used to ensure palette indices in 1bpp are treated
   // as normally expected)
   uint32_t mask = (get_output_format() == FORMAT_1BPP) ?
      0x11111111 : 0xFFFFFFFF;

   // Scan all tiles in the tilemap
   int limit1 = order ? height : width;
   int limit2 = order ? width : height;

   for (int pos2 = 0; pos2 < limit2; pos2++)
   for (int pos1 = 0; pos1 < limit1; pos1++) {
      // Retrieve tile
      if (order)
         get_tile(in, &curr_tile, x + (pos2 << 3), y + (pos1 << 3));
      else
         get_tile(in, &curr_tile, x + (pos1 << 3), y + (pos2 << 3));

      for (unsigned i = 0; i < 8; i++) {
         curr_tile.normal[i] &= mask;
         curr_tile.flipped[i] &= mask;
      }

      // Where we store the ID of this tile
      uint16_t this_id = num_tiles;

      // Compare against all other tiles
      uint16_t match = 0;
      for (; match < num_tiles; match++) {
         // To make code more readable
         Tile *other = &tiles[match];

         // Is it this tile, non flipped?
         if (curr_tile.normal[0] == other->normal[0] &&
             curr_tile.normal[1] == other->normal[1] &&
             curr_tile.normal[2] == other->normal[2] &&
             curr_tile.normal[3] == other->normal[3] &&
             curr_tile.normal[4] == other->normal[4] &&
             curr_tile.normal[5] == other->normal[5] &&
             curr_tile.normal[6] == other->normal[6] &&
             curr_tile.normal[7] == other->normal[7])
         {
            this_id = match;
            break;
         }

         // Is it this tile, flipped horizontally?
         if (curr_tile.normal[0] == other->flipped[0] &&
             curr_tile.normal[1] == other->flipped[1] &&
             curr_tile.normal[2] == other->flipped[2] &&
             curr_tile.normal[3] == other->flipped[3] &&
             curr_tile.normal[4] == other->flipped[4] &&
             curr_tile.normal[5] == other->flipped[5] &&
             curr_tile.normal[6] == other->flipped[6] &&
             curr_tile.normal[7] == other->flipped[7])
         {
            this_id = match | 0x0800;
            break;
         }

         // Is it this tile, flipped vertically?
         if (curr_tile.normal[0] == other->normal[7] &&
             curr_tile.normal[1] == other->normal[6] &&
             curr_tile.normal[2] == other->normal[5] &&
             curr_tile.normal[3] == other->normal[4] &&
             curr_tile.normal[4] == other->normal[3] &&
             curr_tile.normal[5] == other->normal[2] &&
             curr_tile.normal[6] == other->normal[1] &&
             curr_tile.normal[7] == other->normal[0])
         {
            this_id = match | 0x1000;
            break;
         }

         // Is it this tile, flipped both ways?
         if (curr_tile.normal[0] == other->flipped[7] &&
             curr_tile.normal[1] == other->flipped[6] &&
             curr_tile.normal[2] == other->flipped[5] &&
             curr_tile.normal[3] == other->flipped[4] &&
             curr_tile.normal[4] == other->flipped[3] &&
             curr_tile.normal[5] == other->flipped[2] &&
             curr_tile.normal[6] == other->flipped[1] &&
             curr_tile.normal[7] == other->flipped[0])
         {
            this_id = match | 0x1800;
            break;
         }
      }

      // Unique tile?
      if (match == num_tiles) {
         // Increment tile count
         num_tiles++;
         if (num_tiles > 0x0800) {
            free(tiles);
            free(mappings);
            return ERR_MANYTILES;
         }

         // Allocate memory for new tile
         tiles = (Tile *) realloc(tiles, sizeof(Tile) * num_tiles);
         if (tiles == NULL) {
            free(tiles);
            free(mappings);
            return ERR_NOMEMORY;
         }

         // Store tile in the list
         memcpy(&tiles[num_tiles - 1], &curr_tile, sizeof(Tile));
      }

      // Add palette and priority
      this_id |= curr_tile.flags << 13;

      // Write tile in the mappings
      mappings[pos2 * limit1 + pos1] = this_id;
   }

   // Write all the tiles
   for (size_t i = 0; i < num_tiles; i++) {
      for (unsigned row = 0; row < 8; row++) {
         // 4bpp format?
         if (get_output_format() == FORMAT_4BPP) {
            // Split each row into bytes
            // We need to do this due to endianess shenanigans :P
            uint8_t buffer[4];
            buffer[0] = tiles[i].normal[row] >> 24;
            buffer[1] = tiles[i].normal[row] >> 16;
            buffer[2] = tiles[i].normal[row] >> 8;
            buffer[3] = tiles[i].normal[row];

            // Write row into file
            if (fwrite(buffer, 1, 4, outgfx) < 4) {
               free(tiles);
               free(mappings);
               return ERR_CANTWRITEGFX;
            }
         }

         // 1bpp format?
         else {
            // Each row is just one byte, but so far we've been storing the
            // tiles as 4bpp to make our life easier, so we need to convert
            // it to 1bpp first
            uint8_t buffer =
               (tiles[i].normal[row] & 0x10000000 ? 0x80 : 0x00) |
               (tiles[i].normal[row] & 0x01000000 ? 0x40 : 0x00) |
               (tiles[i].normal[row] & 0x00100000 ? 0x20 : 0x00) |
               (tiles[i].normal[row] & 0x00010000 ? 0x10 : 0x00) |
               (tiles[i].normal[row] & 0x00001000 ? 0x08 : 0x00) |
               (tiles[i].normal[row] & 0x00000100 ? 0x04 : 0x00) |
               (tiles[i].normal[row] & 0x00000010 ? 0x02 : 0x00) |
               (tiles[i].normal[row] & 0x00000001 ? 0x01 : 0x00);

            // Write row into file
            if (fwrite(&buffer, 1, 1, outgfx) < 1) {
               free(tiles);
               free(mappings);
               return ERR_CANTWRITEGFX;
            }
         }
      }
   }

   // Write the mappings
   size_t mapsize = width * height;
   for (size_t i = 0; i < mapsize; i++) {
      // Get ID of tile, offset included
      uint16_t tile = mappings[i] + offset;

      // Split each word into two bytes
      // We need to do this due to endianess shenanigans :P
      uint8_t buffer[2] = {
         tile >> 8,
         tile
      };

      // Write word into file
      if (fwrite(buffer, 1, 2, outmap) < 2) {
         free(tiles);
         free(mappings);
         return ERR_CANTWRITEMAP;
      }
   }

   // If continuous then adjust the offset
   if (is_continuous_offset())
      increment_offset(num_tiles);

   // Success!
   free(tiles);
   free(mappings);
   return ERR_NONE;
}
