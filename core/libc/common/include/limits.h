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

#ifndef __ONRAMP_LIBC_LIMITS_H_INCLUDED
#define __ONRAMP_LIBC_LIMITS_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

#define BOOL_WIDTH 8

#define CHAR_BIT 8
#define CHAR_MAX SCHAR_MAX
#define CHAR_MIN SCHAR_MIN
#define CHAR_WIDTH CHAR_BIT

#define SCHAR_MAX 127
#define SCHAR_MIN (-127-1)
#define SCHAR_WIDTH CHAR_BIT
#define UCHAR_MAX 255U
#define UCHAR_WIDTH CHAR_BIT

#define SHRT_MAX 32767
#define SHRT_MIN (-32767-1)
#define SHRT_WIDTH 16
#define USHRT_MAX 65535U
#define USHRT_WIDTH 16

#define INT_MAX 2147483647
#define INT_MIN (-2147483647-1)
#define INT_WIDTH 32
#define UINT_MAX 4294967295U
#define UINT_WIDTH 32

#define LONG_MAX INT_MAX
#define LONG_MIN INT_MIN
#define LONG_WIDTH 32
#define ULONG_MAX UINT_MAX
#define ULONG_WIDTH 32

#define LLONG_MAX 9223372036854775807LL
#define LLONG_MIN (-9223372036854775807LL-1)
#define LLONG_WIDTH 64
#define ULLONG_MAX 18446744073709551615ULL
#define ULLONG_WIDTH 64

#define MB_LEN_MAX 4
#define MB_CUR_MAX 4

#define BITINT_MAXWIDTH 64

#endif
