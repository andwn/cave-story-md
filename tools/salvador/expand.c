/*
 * expand.c - decompressor implementation
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

#include <stdlib.h>
#include <string.h>
#include "format.h"
#include "expand.h"
#include "libsalvador.h"

#ifdef _MSC_VER
#define FORCE_INLINE __forceinline
#else /* _MSC_VER */
#define FORCE_INLINE __attribute__((always_inline))
#endif /* _MSC_VER */

static inline FORCE_INLINE int salvador_read_bit(const unsigned char **ppInBlock, const unsigned char *pDataEnd, int *nCurBitMask, unsigned char *bits) {
   int nBit;

   const unsigned char* pInBlock = *ppInBlock;

   if ((*nCurBitMask) == 0) {
      if (pInBlock >= pDataEnd) return -1;
      (*bits) = *pInBlock++;
      (*nCurBitMask) = 128;
   }

   nBit = ((*bits) & 128) ? 1 : 0;

   (*bits) <<= 1;
   (*nCurBitMask) >>= 1;

   *ppInBlock = pInBlock;
   return nBit;
}

static inline FORCE_INLINE int salvador_read_elias(const unsigned char** ppInBlock, const unsigned char* pDataEnd, const int nInitialValue, const int nIsBackward, int* nCurBitMask, unsigned char* bits) {
   int nValue = nInitialValue;

   if (nIsBackward) {
      while (salvador_read_bit(ppInBlock, pDataEnd, nCurBitMask, bits) == 1) {
         nValue = (nValue << 1) | salvador_read_bit(ppInBlock, pDataEnd, nCurBitMask, bits);
      }
   }
   else {
      while (!salvador_read_bit(ppInBlock, pDataEnd, nCurBitMask, bits)) {
         nValue = (nValue << 1) | salvador_read_bit(ppInBlock, pDataEnd, nCurBitMask, bits);
      }
   }

   return nValue;
}

static inline FORCE_INLINE int salvador_read_elias_inverted(const unsigned char** ppInBlock, const unsigned char* pDataEnd, const int nInitialValue, int* nCurBitMask, unsigned char* bits) {
   int nValue = nInitialValue;

   while (!salvador_read_bit(ppInBlock, pDataEnd, nCurBitMask, bits)) {
      nValue = (nValue << 1) | (salvador_read_bit(ppInBlock, pDataEnd, nCurBitMask, bits) ^ 1);
   }

   return nValue;
}

static inline FORCE_INLINE int salvador_read_elias_prefix(const unsigned char** ppInBlock, const unsigned char* pDataEnd, const int nInitialValue, const int nIsBackward, int* nCurBitMask, unsigned char* bits, unsigned int nFirstBit) {
   int nValue = nInitialValue;

   if (nIsBackward) {
      if (nFirstBit) {
         nValue = (nValue << 1) | salvador_read_bit(ppInBlock, pDataEnd, nCurBitMask, bits);
         while (salvador_read_bit(ppInBlock, pDataEnd, nCurBitMask, bits) == 1) {
            nValue = (nValue << 1) | salvador_read_bit(ppInBlock, pDataEnd, nCurBitMask, bits);
         }
      }
   }
   else {
      if (!nFirstBit) {
         nValue = (nValue << 1) | salvador_read_bit(ppInBlock, pDataEnd, nCurBitMask, bits);
         while (!salvador_read_bit(ppInBlock, pDataEnd, nCurBitMask, bits)) {
            nValue = (nValue << 1) | salvador_read_bit(ppInBlock, pDataEnd, nCurBitMask, bits);
         }
      }
   }

   return nValue;
}

/**
 * Get maximum decompressed size of compressed data
 *
 * @param pInputData compressed data
 * @param nInputSize compressed size in bytes
 * @param nFlags compression flags (set to FLG_IS_INVERTED)
 *
 * @return maximum decompressed size
 */
size_t salvador_get_max_decompressed_size(const unsigned char *pInputData, size_t nInputSize, const unsigned int nFlags) {
   const unsigned char* pInputDataEnd = pInputData + nInputSize;
   int nCurBitMask = 0;
   unsigned char bits = 0;
   int nIsFirstCommand = 1;
   const int nIsInverted = (nFlags & FLG_IS_INVERTED) && !(nFlags & FLG_IS_BACKWARD);
   const int nIsBackward = (nFlags & FLG_IS_BACKWARD) ? 1 : 0;
   int nDecompressedSize = 0;

   if (pInputData >= pInputDataEnd)
      return -1;

   while (1) {
      unsigned int nIsMatchWithOffset;

      if (nIsFirstCommand) {
         /* The first command is always literals */
         nIsFirstCommand = 0;
         nIsMatchWithOffset = 0;
      }
      else {
         /* Read match with offset / literals bit */
         nIsMatchWithOffset = salvador_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
         if (nIsMatchWithOffset == -1)
            return -1;
      }

      if (nIsMatchWithOffset == 0) {
         unsigned int nLiterals = salvador_read_elias(&pInputData, pInputDataEnd, 1, nIsBackward, &nCurBitMask, &bits);

         /* Count literals */

         if ((pInputData + nLiterals) <= pInputDataEnd) {
            pInputData += nLiterals;
            nDecompressedSize += nLiterals;
         }
         else {
            return -1;
         }

         /* Read match with offset / rep match bit */

         nIsMatchWithOffset = salvador_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
         if (nIsMatchWithOffset == -1)
            return -1;
      }

      unsigned int nMatchLen;

      if (nIsMatchWithOffset) {
         /* Match with offset */

         unsigned int nMatchOffsetHighByte;

         if (nIsInverted)
            nMatchOffsetHighByte = salvador_read_elias_inverted(&pInputData, pInputDataEnd, 1, &nCurBitMask, &bits);
         else
            nMatchOffsetHighByte = salvador_read_elias(&pInputData, pInputDataEnd, 1, nIsBackward, &nCurBitMask, &bits);

         if (nMatchOffsetHighByte == 256)
            break;

         if (pInputData >= pInputDataEnd)
            return -1;

         unsigned int nMatchOffsetLowByte = (unsigned int)(*pInputData++);

         nMatchLen = salvador_read_elias_prefix(&pInputData, pInputDataEnd, 1, nIsBackward, &nCurBitMask, &bits, nMatchOffsetLowByte & 1);

         nMatchLen += (2 - 1);
      }
      else {
         /* Rep-match */

         nMatchLen = salvador_read_elias(&pInputData, pInputDataEnd, 1, nIsBackward, &nCurBitMask, &bits);
      }

      /* Count matched bytes */
      nDecompressedSize += nMatchLen;
   }

   return nDecompressedSize;
}

/**
 * Decompress data in memory
 *
 * @param pInputData compressed data
 * @param pOutData buffer for decompressed data
 * @param nInputSize compressed size in bytes
 * @param nMaxOutBufferSize maximum capacity of decompression buffer
 * @param nDictionarySize size of dictionary in front of input data (0 for none)
 * @param nFlags compression flags (set to FLG_IS_INVERTED)
 *
 * @return actual decompressed size, or -1 for error
 */
size_t salvador_decompress(const unsigned char *pInputData, unsigned char *pOutData, size_t nInputSize, size_t nMaxOutBufferSize, size_t nDictionarySize, const unsigned int nFlags) {
   const unsigned char *pInputDataEnd = pInputData + nInputSize;
   unsigned char *pCurOutData = pOutData + nDictionarySize;
   const unsigned char *pOutDataEnd = pCurOutData + nMaxOutBufferSize;
   int nCurBitMask = 0;
   unsigned char bits = 0;
   int nMatchOffset = 1;
   int nIsFirstCommand = 1;
   const int nIsInverted = (nFlags & FLG_IS_INVERTED) && !(nFlags & FLG_IS_BACKWARD);
   const int nIsBackward = (nFlags & FLG_IS_BACKWARD) ? 1 : 0;

   if (pInputData >= pInputDataEnd && pCurOutData < pOutDataEnd)
      return -1;

   while (1) {
      unsigned int nIsMatchWithOffset;

      if (nIsFirstCommand) {
         /* The first command is always literals */
         nIsFirstCommand = 0;
         nIsMatchWithOffset = 0;
      }
      else {
         /* Read match with offset / literals bit */
         nIsMatchWithOffset = salvador_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
         if (nIsMatchWithOffset == -1)
            return -1;
      }

      if (nIsMatchWithOffset == 0) {
         unsigned int nLiterals = salvador_read_elias(&pInputData, pInputDataEnd, 1, nIsBackward, &nCurBitMask, &bits);

         /* Copy literals */

         if ((pInputData + nLiterals) <= pInputDataEnd &&
            (pCurOutData + nLiterals) <= pOutDataEnd) {
            memcpy(pCurOutData, pInputData, nLiterals);
            pInputData += nLiterals;
            pCurOutData += nLiterals;
         }
         else {
            return -1;
         }

         /* Read match with offset / rep match bit */

         nIsMatchWithOffset = salvador_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
         if (nIsMatchWithOffset == -1)
            return -1;
      }

      unsigned int nMatchLen;

      if (nIsMatchWithOffset) {
         /* Match with offset */

         unsigned int nMatchOffsetHighByte;

         if (nIsInverted)
            nMatchOffsetHighByte = salvador_read_elias_inverted(&pInputData, pInputDataEnd, 1, &nCurBitMask, &bits);
         else
            nMatchOffsetHighByte = salvador_read_elias(&pInputData, pInputDataEnd, 1, nIsBackward, &nCurBitMask, &bits);

         if (nMatchOffsetHighByte == 256)
            break;
         nMatchOffsetHighByte--;

         if (pInputData >= pInputDataEnd)
            return -1;

         unsigned int nMatchOffsetLowByte = (unsigned int)(*pInputData++);
         if (nIsBackward)
            nMatchOffset = (nMatchOffsetHighByte << 7) | (nMatchOffsetLowByte >> 1);
         else
            nMatchOffset = (nMatchOffsetHighByte << 7) | (127 - (nMatchOffsetLowByte >> 1));
         nMatchOffset++;

         nMatchLen = salvador_read_elias_prefix(&pInputData, pInputDataEnd, 1, nIsBackward, &nCurBitMask, &bits, nMatchOffsetLowByte & 1);

         nMatchLen += (2 - 1);
      }
      else {
         /* Rep-match */

         nMatchLen = salvador_read_elias(&pInputData, pInputDataEnd, 1, nIsBackward, &nCurBitMask, &bits);
      }

      /* Copy matched bytes */
      const unsigned char* pSrc = pCurOutData - nMatchOffset;
      if (pSrc >= pOutData) {
         if ((pSrc + nMatchLen) <= pOutDataEnd) {
            if ((pCurOutData + nMatchLen) <= pOutDataEnd) {
               while (nMatchLen) {
                  *pCurOutData++ = *pSrc++;
                  nMatchLen--;
               }
            }
            else {
               return -1;
            }
         }
         else {
            return -1;
         }
      }
      else {
         return -1;
      }
   }

   return (size_t)(pCurOutData - pOutData) - nDictionarySize;
}
