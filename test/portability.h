/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2026 Fraser Heavy Software
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

#ifndef PORTABILITY_H_INCLUDED
#define PORTABILITY_H_INCLUDED

/*
 * This header helps us test our C code in native C compilers. It is
 * force-included by the test Makefiles.
 */

/*
 * On Windows, environ is called _environ.
 *
 * We otherwise assume environ is available. It's not standard C, but it is in
 * POSIX and there's no other way to get a list of all environment variables.
 */
#ifdef _WIN32
    #define environ _environ
#endif

/*
 * Clang warns about the use of bitwise `|` on boolean value because it wants
 * us to use `||`, but `||` does not exist in omC.
 *
 * We need to silence this warning in omC source files. We do want the warning
 * on in other C source files though. The test Makefiles of programs written in
 * omC define __onramp_omc__.
 */
#ifdef __onramp_omc__
    #ifdef __has_warning
        #if __has_warning("-Wbitwise-instead-of-logical")
            #pragma GCC diagnostic ignored "-Wbitwise-instead-of-logical"
        #endif
    #endif
#endif

/**
 * -Wall -Wextra warns about unused parameters. We have lots of those because
 * our early stage compilers don't allow us to omit them or cast them to void.
 */
#ifdef __GNUC__
    #ifdef __has_warning
        #if __has_warning("-Wunused-parameter")
            #pragma GCC diagnostic ignored "-Wunused-parameter"
        #endif
    #else
        #pragma GCC diagnostic ignored "-Wunused-parameter"
    #endif
#endif

/**
 * We need stdc_leading_zeros() but it's a C23 function. We still want to be
 * able to test with earlier compilers and libcs so we work around it here.
 * Note that we don't fallback to __builtin_clz() because its behaviour is
 * undefined on 0.
 */
#ifndef ONRAMP_LIBC_TEST
    #ifdef __has_include
        #if __has_include(<stdbit.h>)
            #include <stdbit.h>
        #endif
    #endif
    #if __has_builtin(__builtin_stdc_leading_zeros)
        #ifndef stdc_leading_zeros_uc
            #define stdc_leading_zeros_uc(x) (__builtin_stdc_leading_zeros((unsigned char)(x)))
        #endif
        #ifndef stdc_leading_zeros_us
            #define stdc_leading_zeros_us(x) (__builtin_stdc_leading_zeros((unsigned short)(x)))
        #endif
        #ifndef stdc_leading_zeros_ui
            #define stdc_leading_zeros_ui(x) (__builtin_stdc_leading_zeros((unsigned int)(x)))
        #endif
        #ifndef stdc_leading_zeros_ul
            #define stdc_leading_zeros_ul(x) (__builtin_stdc_leading_zeros((unsigned long)(x)))
        #endif
        #ifndef stdc_leading_zeros_ull
            #define stdc_leading_zeros_ull(x) (__builtin_stdc_leading_zeros((unsigned long long)(x)))
        #endif
    #endif
    #if !defined(stdc_leading_zeros_uc) \
            || !defined(stdc_leading_zeros_us) \
            || !defined(stdc_leading_zeros_ui) \
            || !defined(stdc_leading_zeros_ul) \
            || !defined(stdc_leading_zeros_ull)
        #include <limits.h>

        static inline int onramp_test_stdc_leading_zeros_ull(unsigned long long x) {
            int ret = CHAR_BIT * sizeof(x);
            while (x) {
                --ret;
                x >>= 1;
            }
            return ret;
        }

        #ifndef stdc_leading_zeros_uc
            #define stdc_leading_zeros_uc(x) (onramp_test_stdc_leading_zeros_ull((unsigned char)(x)) \
                    - CHAR_BIT * (sizeof(unsigned long long) - sizeof(unsigned char)))
        #endif
        #ifndef stdc_leading_zeros_us
            #define stdc_leading_zeros_us(x) (onramp_test_stdc_leading_zeros_ull((unsigned short)(x)) \
                    - CHAR_BIT * (sizeof(unsigned long long) - sizeof(unsigned short)))
        #endif
        #ifndef stdc_leading_zeros_ui
            #define stdc_leading_zeros_ui(x) (onramp_test_stdc_leading_zeros_ull((unsigned int)(x)) \
                    - CHAR_BIT * (sizeof(unsigned long long) - sizeof(unsigned int)))
        #endif
        #ifndef stdc_leading_zeros_ul
            #define stdc_leading_zeros_ul(x) (onramp_test_stdc_leading_zeros_ull((unsigned long)(x)) \
                    - CHAR_BIT * (sizeof(unsigned long long) - sizeof(unsigned long)))
        #endif
        #ifndef stdc_leading_zeros_ull
            #define stdc_leading_zeros_ull onramp_test_stdc_leading_zeros_ull
        #endif
    #endif
#endif

/*
 * Disable a few extra Clang warnings. These occur due to compiling parts of
 * our libc with a non-Onramp compiler. It's not really designed for this; some
 * changes are needed to make the tests compile better with a native compiler
 * and libc.
 */
#ifdef __GNUC__
    #ifdef __has_warning
        #if __has_warning("-Wbuiltin-macro-redefined")
            #pragma GCC diagnostic ignored "-Wbuiltin-macro-redefined"
        #endif
        #if __has_warning("-Wc23-extensions")
            #pragma GCC diagnostic ignored "-Wc23-extensions"
        #endif
    #endif
#endif

#endif
