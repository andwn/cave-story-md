#ifndef MD_COMP_H
#define MD_COMP_H

#include "types.h"

// comp.s routines
void slz_unpack(const void *in, void *out);
void uftc_unpack(const void *in, void *out, uint16_t start, uint16_t num);

#endif //MD_COMP_H
