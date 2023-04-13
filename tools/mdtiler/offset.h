//***************************************************************************
// "offset.h"
// Header file for "offset.c"
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

#ifndef OFFSET_H
#define OFFSET_H

// Required headers
#include <stdint.h>

// Function prototypes
uint16_t get_map_offset(void);
void set_map_offset(uint16_t);
void increment_offset(uint16_t);
void set_continuous_offset(int);
int is_continuous_offset(void);

#endif
