//***************************************************************************
// "map.h"
// Header file for "map.c"
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

#ifndef MAP_H
#define MAP_H

// Required headers
#include <stdio.h>
#include "bitmap.h"

// Function prototypes
int generate_map(const Bitmap *, FILE *, FILE *, int, int, int, int, int);

#endif
