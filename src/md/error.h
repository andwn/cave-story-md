#ifndef MD_ERROR_H
#define MD_ERROR_H

#include "types.h"

#define error_oom() _error_oom(__FILE__, __LINE__)
#define error_other(err) _error_other(err, __FILE__, __LINE__)

__attribute__((noreturn))
void _error_oom(const char* file, const uint16_t line);
__attribute__((noreturn))
void _error_other(const char *err, const char *file, const uint16_t line);

#endif //MD_ERROR_H
