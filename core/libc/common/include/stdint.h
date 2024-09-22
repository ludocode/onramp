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

#ifndef __ONRAMP_LIBC_STDINT_H_INCLUDED
#define __ONRAMP_LIBC_STDINT_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

#include <__onramp/__wchar_t.h>
#include <__onramp/__wchar_limits.h>



// these types exist in omC under cci/0

#ifdef __onramp_cci_omc__
typedef char int8_t;
typedef int int32_t;
#endif
#ifndef __onramp_cci_omc__
typedef signed char int8_t;
typedef signed int int32_t;
#endif

#define INT8_MIN (-127-1)
#define INT8_MAX 127
#define INT32_MIN (-2147483647-1)
#define INT32_MAX 2147483647

typedef int8_t int_least8_t;
typedef int32_t int_least32_t;

#define INT_LEAST8_MIN INT8_MIN
#define INT_LEAST8_MAX INT8_MAX
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST32_MAX INT32_MAX

typedef int8_t int_fast8_t;
typedef int32_t int_fast32_t;

#define INT_FAST8_MIN INT8_MIN
#define INT_FAST8_MAX INT8_MAX
#define INT_FAST32_MIN INT32_MIN
#define INT_FAST32_MAX INT32_MAX

typedef int32_t intptr_t;

#define INTPTR_MIN INT32_MIN
#define INTPTR_MAX INT32_MAX



// these types are available in opC under cci/1

#ifndef __onramp_cci_omc__

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef signed short int16_t;

#define UINT8_MAX 255U
#define INT16_MIN (-32767-1)
#define INT16_MAX 32767
#define UINT16_MAX 65535U
#define UINT32_MAX 4294967295U

typedef uint8_t uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef int16_t int_least16_t;

#define UINT_LEAST8_MAX UINT8_MAX
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST16_MAX INT16_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define UINT_LEAST32_MAX UINT32_MAX

typedef uint8_t uint_fast8_t;
typedef uint16_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef int16_t int_fast16_t;

#define UINT_FAST8_MAX UINT8_MAX
#define INT_FAST16_MIN INT16_MIN
#define INT_FAST16_MAX INT16_MAX
#define UINT_FAST16_MAX UINT16_MAX
#define UINT_FAST32_MAX UINT32_MAX

typedef uint32_t uintptr_t;

#define UINTPTR_MAX UINT32_MAX

#endif



// `long long` is only available on cci/2

#ifndef __onramp_cci_omc__
#ifndef __onramp_cci_opc__

typedef unsigned long long uint64_t;
typedef signed long long int64_t;

#define INT64_MIN (-9223372036854775807LL-1LL)
#define INT64_MAX 9223372036854775807LL
#define UINT64_MAX 18446744073709551615UL

typedef uint64_t uint_least64_t;
typedef int64_t int_least64_t;

#define INT_LEAST64_MIN INT64_MIN
#define INT_LEAST64_MAX INT64_MAX
#define UINT_LEAST64_MAX UINT64_MAX

typedef uint64_t uint_fast64_t;
typedef int64_t int_fast64_t;

#define INT_FAST64_MIN INT64_MIN
#define INT_FAST64_MAX INT64_MAX
#define UINT_FAST64_MAX UINT64_MAX

#endif
#endif



// `intmax_t` is normally `long long` which is 64 bits.
#ifndef __onramp_cci_opc__
#ifndef __onramp_cci_omc__
#ifdef INTMAX_64_DISABLED // TODO llong is not supported by cci/2 yet
    typedef int64_t intmax_t;
    #define INTMAX_MIN INT64_MIN
    #define INTMAX_MAX INT64_MAX
    #define INTMAX_C INT64_C

    typedef uint64_t uintmax_t;
    #define UINTMAX_MAX UINT64_MAX
    #define UINTMAX_C UINT64_C

    #define __INTMAX_IS_64_BITS
#endif
#endif
#endif

// During bootstrapping, opC doesn't have `long long` so when
// compiling with cci/1, `intmax_t` is 32 bits.
//
// (Note that this is an ABI break, so `intmax_t` can't be passed across
// library boundaries during bootstrapping. That's fine; it's just used
// internally in cpp/2 and in individual sources in libc/2 and libc/3.)
#ifndef __INTMAX_IS_64_BITS
    typedef int32_t intmax_t;
    #define INTMAX_MIN INT32_MIN
    #define INTMAX_MAX INT32_MAX
    #define INTMAX_C INT32_C

    #ifndef __onramp_cci_omc__
        typedef uint32_t uintmax_t;
        #define UINTMAX_MAX UINT32_MAX
        #define UINTMAX_C UINT32_C
    #endif
#endif

#ifndef __onramp_cpp_omc__
#undef __INTMAX_IS_64_BITS
#endif



// Apparently this file doesn't actually need to define size_t or ptrdiff_t.
#define PTRDIFF_WIDTH 32
#define PTRDIFF_MIN INT32_MIN
#define PTRDIFF_MAX INT32_MAX
#define SIZE_WIDTH 32
#define SIZE_MAX UINT32_MAX

// TODO: SIG_ATOMIC_MIN and SIG_ATOMIC_MAX

#ifndef __onramp_cpp_omc__
    #define INT8_C(x) x
    #define INT16_C(x) x
    #define INT32_C(x) x
/* TODO cpp/1 breaks on ## even in untaken conditionals
    #define INT64_C(x) x ## LL
    #define UINT8_C(x) x ## U
    #define UINT16_C(x) x ## U
    #define UINT32_C(x) x ## U
    #define UINT64_C(x) x ## ULL
    */
#endif



#define WINT_WIDTH 32
#define WINT_MIN INT32_MIN
#define WINT_MAX INT32_MAX

#endif
