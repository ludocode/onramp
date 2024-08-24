/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ONRAMP_LIBC_INTTYPES_H_INCLUDED
#define __ONRAMP_LIBC_INTTYPES_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

// 7.8.1 in C17 spec: <inttypes.h> includes <stdint.h>.
#include <stdint.h>



/* PRI */

/* 8 */
#define PRId8 "hhd"
#define PRIdLEAST8 PRId8
#define PRIdFAST8 PRId8
#define PRIi8 PRId8
#define PRIiLEAST8 PRIi8
#define PRIiFAST8 PRIi8
#define PRIo8 "hho"
#define PRIoLEAST8 PRIo8
#define PRIoFAST8 PRIo8
#define PRIu8 "hhu"
#define PRIuLEAST8 PRIu8
#define PRIuFAST8 PRIu8
#define PRIx8 "hhx"
#define PRIxLEAST8 PRIx8
#define PRIxFAST8 PRIx8
#define PRIX8 "hhX"
#define PRIXLEAST8 PRIX8
#define PRIXFAST8 PRIX8

/* 16 */
#define PRId16 "hd"
#define PRIdLEAST16 PRId16
#define PRIdFAST16 PRId16
#define PRIi16 PRId16
#define PRIiLEAST16 PRIi16
#define PRIiFAST16 PRIi16
#define PRIo16 "ho"
#define PRIoLEAST16 PRIo16
#define PRIoFAST16 PRIo16
#define PRIu16 "hu"
#define PRIuLEAST16 PRIu16
#define PRIuFAST16 PRIu16
#define PRIx16 "hx"
#define PRIxLEAST16 PRIx16
#define PRIxFAST16 PRIx16
#define PRIX16 "hX"
#define PRIXLEAST16 PRIX16
#define PRIXFAST16 PRIX16

/* 32 */
#define PRId32 "d"
#define PRIdLEAST32 PRId32
#define PRIdFAST32 PRId32
#define PRIi32 PRId32
#define PRIiLEAST32 PRIi32
#define PRIiFAST32 PRIi32
#define PRIo32 "o"
#define PRIoLEAST32 PRIo32
#define PRIoFAST32 PRIo32
#define PRIu32 "u"
#define PRIuLEAST32 PRIu32
#define PRIuFAST32 PRIu32
#define PRIx32 "x"
#define PRIxLEAST32 PRIx32
#define PRIxFAST32 PRIx32
#define PRIX32 "X"
#define PRIXLEAST32 PRIX32
#define PRIXFAST32 PRIX32

/* 64 */
#define PRId64 "ld"
#define PRIdLEAST64 PRId64
#define PRIdFAST64 PRId64
#define PRIi64 PRId64
#define PRIiLEAST64 PRIi64
#define PRIiFAST64 PRIi64
#define PRIo64 "lo"
#define PRIoLEAST64 PRIo64
#define PRIoFAST64 PRIo64
#define PRIu64 "lu"
#define PRIuLEAST64 PRIu64
#define PRIuFAST64 PRIu64
#define PRIx64 "lx"
#define PRIxLEAST64 PRIx64
#define PRIxFAST64 PRIx64
#define PRIX64 "lX"
#define PRIXLEAST64 PRIX64
#define PRIXFAST64 PRIX64

/* MAX */
#define PRIdMAX PRId64
#define PRIiMAX PRIi64
#define PRIoMAX PRIo64
#define PRIuMAX PRIu64
#define PRIxMAX PRIx64
#define PRIXMAX PRIX64

/* PTR */
#define PRIdPTR PRId32
#define PRIiPTR PRIi32
#define PRIoPTR PRIo32
#define PRIuPTR PRIu32
#define PRIxPTR PRIx32
#define PRIXPTR PRIX32



/* SCN */

/* 8 */
#define SCNd8 PRId8
#define SCNdLEAST8 PRIdLEAST8
#define SCNdFAST8 PRIdFAST8
#define SCNi8 PRIi8
#define SCNiLEAST8 PRIiLEAST8
#define SCNiFAST8 PRIiFAST8
#define SCNo8 PRIo8
#define SCNoLEAST8 PRIoLEAST8
#define SCNoFAST8 PRIoFAST8
#define SCNu8 PRIu8
#define SCNuLEAST8 PRIuLEAST8
#define SCNuFAST8 PRIuFAST8
#define SCNx8 PRIx8
#define SCNxLEAST8 PRIxLEAST8
#define SCNxFAST8 PRIxFAST8

/* 16 */
#define SCNd16 PRId16
#define SCNdLEAST16 PRIdLEAST16
#define SCNdFAST16 PRIdFAST16
#define SCNi16 PRIi16
#define SCNiLEAST16 PRIiLEAST16
#define SCNiFAST16 PRIiFAST16
#define SCNo16 PRIo16
#define SCNoLEAST16 PRIoLEAST16
#define SCNoFAST16 PRIoFAST16
#define SCNu16 PRIu16
#define SCNuLEAST16 PRIuLEAST16
#define SCNuFAST16 PRIuFAST16
#define SCNx16 PRIx16
#define SCNxLEAST16 PRIxLEAST16
#define SCNxFAST16 PRIxFAST16

/* 32 */
#define SCNd32 PRId32
#define SCNdLEAST32 PRIdLEAST32
#define SCNdFAST32 PRIdFAST32
#define SCNi32 PRIi32
#define SCNiLEAST32 PRIiLEAST32
#define SCNiFAST32 PRIiFAST32
#define SCNo32 PRIo32
#define SCNoLEAST32 PRIoLEAST32
#define SCNoFAST32 PRIoFAST32
#define SCNu32 PRIu32
#define SCNuLEAST32 PRIuLEAST32
#define SCNuFAST32 PRIuFAST32
#define SCNx32 PRIx32
#define SCNxLEAST32 PRIxLEAST32
#define SCNxFAST32 PRIxFAST32

/* 64 */
#define SCNd64 PRId64
#define SCNdLEAST64 PRIdLEAST64
#define SCNdFAST64 PRIdFAST64
#define SCNi64 PRIi64
#define SCNiLEAST64 PRIiLEAST64
#define SCNiFAST64 PRIiFAST64
#define SCNo64 PRIo64
#define SCNoLEAST64 PRIoLEAST64
#define SCNoFAST64 PRIoFAST64
#define SCNu64 PRIu64
#define SCNuLEAST64 PRIuLEAST64
#define SCNuFAST64 PRIuFAST64
#define SCNx64 PRIx64
#define SCNxLEAST64 PRIxLEAST64
#define SCNxFAST64 PRIxFAST64

/* MAX */
#define SCNdMAX PRIdMAX
#define SCNiMAX PRIiMAX
#define SCNoMAX PRIoMAX
#define SCNuMAX PRIuMAX
#define SCNxMAX PRIxMAX

/* PTR */
#define SCNdPTR PRIdPTR
#define SCNiPTR PRIiPTR
#define SCNoPTR PRIoPTR
#define SCNuPTR PRIuPTR
#define SCNxPTR PRIxPTR



#endif
