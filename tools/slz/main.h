//***************************************************************************
// "main.h"
// Some common definitions and such
//***************************************************************************
// Slz compression tool
// Copyright 2011 Javier Degirolmo
//
// This file is part of the slz tool.
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//***************************************************************************

#ifndef MAIN_H
#define MAIN_H

// Required headers
#include <stdint.h>

// Error codes
enum {
   ERR_NONE,            // No error
   ERR_CANTREAD,        // Can't read from input file
   ERR_CANTWRITE,       // Can't write into output file
   ERR_TOOLARGE16,      // File is too large for SLZ16
   ERR_TOOLARGE24,      // File is too large for SLZ24
   ERR_CORRUPT,         // File is corrupt?
   ERR_NOMEMORY,        // Ran out of memory
   ERR_UNKNOWN          // Unknown error
};

// Possible formats
enum {
   FORMAT_DEFAULT,      // No format specified
   FORMAT_SLZ16,        // SLZ16 (16-bit size)
   FORMAT_SLZ24,        // SLZ24 (24-bit size)
   FORMAT_TOOMANY       // Too many formats specified
};

// Function prototypes
int read_word(FILE *, uint16_t *);
int read_tribyte(FILE *, uint32_t *);
int write_word(FILE *, const uint16_t);
int write_tribyte(FILE *, const uint32_t);

#endif
