/*
 * shrink.h - compressor definitions
 *
 * Copyright (C) 2021 Emmanuel Marty
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*
 * Uses the libdivsufsort library Copyright (c) 2003-2008 Yuta Mori
 *
 * Implements the ZX0 encoding designed by Einar Saukas. https://github.com/einar-saukas/ZX0
 * Also inspired by Charles Bloom's compression blog. http://cbloomrants.blogspot.com/
 *
 */

#ifndef _SHRINK_H
#define _SHRINK_H

#include "divsufsort.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LCP_BITS 18
#define TAG_BITS 4
#define LCP_MAX ((1U<<(LCP_BITS - TAG_BITS)) - 1)
#define LCP_AND_TAG_MAX ((1U<<LCP_BITS) - 1)
#define LCP_SHIFT (63-LCP_BITS)
#define LCP_MASK (((1ULL<<LCP_BITS) - 1) << LCP_SHIFT)
#define POS_MASK ((1ULL<<LCP_SHIFT) - 1)
#define VISITED_FLAG 0x8000000000000000ULL
#define EXCL_VISITED_MASK  0x7fffffffffffffffULL

#define NINITIAL_ARRIVALS_PER_POSITION 40
#define NMAX_ARRIVALS_PER_POSITION 109
#define NMATCHES_PER_INDEX 78

#define LEAVE_ALONE_MATCH_SIZE 340

/** One match option */
typedef struct _salvador_match {
   unsigned short length;
   unsigned short offset;
} salvador_match;

/** Forward arrival slot */
typedef struct _salvador_arrival {
   int cost;

   unsigned int from_pos:17;
   int from_slot:11;

   unsigned int rep_offset;

   unsigned int rep_pos:17;
   unsigned int match_len:14;

   int num_literals;
   int score;
} salvador_arrival;

/** Visited match */
typedef int salvador_visited;

/** Compression statistics */
typedef struct _salvador_stats {
   int num_literals;
   int num_normal_matches;
   int num_rep_matches;
   int num_eod;

   int safe_dist;

   int min_literals;
   int max_literals;
   int total_literals;

   int min_offset;
   int max_offset;
   long long total_offsets;

   int min_match_len;
   int max_match_len;
   int total_match_lens;

   int min_rle1_len;
   int max_rle1_len;
   int total_rle1_lens;

   int min_rle2_len;
   int max_rle2_len;
   int total_rle2_lens;

   int commands_divisor;
   int literals_divisor;
   int match_divisor;
   int rle1_divisor;
   int rle2_divisor;
} salvador_stats;

/** Compression context */
typedef struct _salvador_compressor {
   divsufsort_ctx_t divsufsort_context;
   unsigned long long *intervals;
   unsigned long long *pos_data;
   unsigned long long *open_intervals;
   salvador_match *match;
   unsigned short *match_depth;
   salvador_match *best_match;
   salvador_arrival *arrival;
   int *first_offset_for_byte;
   int *next_offset_for_pos;
   int *offset_cache;
   int flags;
   int block_size;
   int max_offset;
   int max_arrivals_per_position;
   salvador_stats stats;
} salvador_compressor;

/**
 * Get maximum compressed size of input(source) data
 *
 * @param nInputSize input(source) size in bytes
 *
 * @return maximum compressed size
 */
size_t salvador_get_max_compressed_size(const size_t nInputSize);

/**
 * Compress memory
 *
 * @param pInputData pointer to input(source) data to compress
 * @param pOutBuffer buffer for compressed data
 * @param nInputSize input(source) size in bytes
 * @param nMaxOutBufferSize maximum capacity of compression buffer
 * @param nFlags compression flags (set to FLG_IS_INVERTED)
 * @param nMaxOffset maximum match offset to use (0 for default)
 * @param nDictionarySize size of dictionary in front of input data (0 for none)
 * @param progress progress function, called after compressing each block, or NULL for none
 * @param pStats pointer to compression stats that are filled if this function is successful, or NULL
 *
 * @return actual compressed size, or -1 for error
 */
size_t salvador_compress(const unsigned char *pInputData, unsigned char *pOutBuffer, const size_t nInputSize, const size_t nMaxOutBufferSize,
   const unsigned int nFlags, const size_t nMaxOffset, const size_t nDictionarySize, void(*progress)(long long nOriginalSize, long long nCompressedSize), salvador_stats *pStats);

#ifdef __cplusplus
}
#endif

#endif /* _SHRINK_H */
