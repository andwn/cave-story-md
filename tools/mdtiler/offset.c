//***************************************************************************
// "offset.c"
// Handles tile ID offsets for map and sprite commands
//***************************************************************************
// mdtiler - Bitmap to tile conversion tool
// Copyright 2018 Javier Degirolmo
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
#include "offset.h"

// Offset for the tiles in the map
static uint16_t offset = 0;

// If set, the offset changes after each call to 'map'
// This allows to allocate tiles for multiple maps without knowing ahead of
// time how many tiles will be actually needed
static int continuous = 0;

//***************************************************************************
// get_map_offset
// Retrieves the current tile ID offset
//---------------------------------------------------------------------------
// return: current tile ID offset
//***************************************************************************

uint16_t get_map_offset(void) {
   return offset;
}

//***************************************************************************
// set_map_offset
// Sets the tile ID offset
//---------------------------------------------------------------------------
// param new_offset: new tile ID offset
//***************************************************************************

void set_map_offset(uint16_t new_offset) {
   offset = new_offset;
}

//***************************************************************************
// increment_offset
// Increments the tile ID offset
//---------------------------------------------------------------------------
// param amount: how much to increment tile ID offset
//***************************************************************************

void increment_offset(uint16_t amount) {
   offset += amount;
}

//***************************************************************************
// set_continuous_offset
// Sets whether offset should be continuous or not
//---------------------------------------------------------------------------
// param enable: 1 for continuous, 0 otherwise
//***************************************************************************

void set_continuous_offset(int enable) {
   continuous = !!enable;
}

//***************************************************************************
// is_continuous_offset
// Returns whether offset is continuous or not
//---------------------------------------------------------------------------
// return: 1 if continuous, 0 if resets
//***************************************************************************

int is_continuous_offset(void) {
   return continuous;
}
