//***************************************************************************
// "main.h"
// Some common definitions and such
//***************************************************************************
// mdtiler - Bitmap to tile conversion tool
// Copyright 2011, 2012, 2018 Javier Degirolmo
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

#ifndef MAIN_H
#define MAIN_H

// Error codes
enum {
   ERR_NONE,            // No error
   ERR_OPENINPUT,       // Can't open input file
   ERR_OPENOUTPUT,      // Can't open output file
   ERR_OPENBATCH,       // Can't open batch file
   ERR_CANTREAD,        // Can't read from batch file
   ERR_CANTWRITE,       // Can't write to output file
   ERR_CANTWRITEGFX,    // Can't write to output file (tiles)
   ERR_CANTWRITEMAP,    // Can't write to output file (tilemap mappings)
   ERR_CANTWRITESPR,    // Can't write to output file (sprite mappings)
   ERR_MANYTILES,       // Too many unique tiles
   ERR_NOMEMORY,        // Ran out of memory
   ERR_PARSE,           // Parsing error
   ERR_BADQUOTE,        // Quote inside non-quoted token
   ERR_NOQUOTE,         // Missing ending quote
   ERR_UNKNOWN          // Unknown error
};

#endif
