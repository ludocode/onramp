/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Fraser Heavy Software
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

#ifndef ONRAMP_LIBO_FLOAT_H_INCLUDED
#define ONRAMP_LIBO_FLOAT_H_INCLUDED

/**
 * @file
 *
 * Functions for bootstrapping floating point arithmetic.
 *
 * Our bootstrapping compilers (cci/0 and cci/1) don't support floats. This
 * file therefore contains wrappers for functions so that they may consume and
 * return integers during bootstrapping and floats in the final ABI.
 *
 * Consider a function like strtof():
 *
 * - When compiling programs with cci/2, i.e. when using the fully bootstrapped
 *   Onramp to compile other software, it must return float, as in normal C.
 *
 * - When compiling the libc, it must return unsigned int. It must be
 *   compilable by both cci/1 (without float support) and by cci/2; it
 *   performs floating-point arithmetic in software and manually composes the
 *   floating point return value into an integer.
 *
 * - When compiling cci/2 itself, we need a version of strtof() that returns
 *   unsigned int, because cci/2 is compiled with cci/1.
 *
 * - We also need to be able to compile cci/2 with a native compiler for
 *   testing. In this case we need a wrapper for the native strtof() that
 *   performs a bit cast to unsigned int.
 *
 * We'd prefer the assembly name of the function be simply "strtof", but cci/1
 * doesn't support __asm__ names, so we can't use a different declaration for
 * it. So when compiling the libc, and when compiling with cci/1, a version
 * that returns `unsigned` is called simply "strtof".
 *
 * We provide an alternate name __strtof_u() which returns unsigned int and
 * simply redirects to the real strtof(). This is called in the implementation
 * of cci/2. Under cci/1, it is a simple #define; under cci/2, it uses an asm
 * name redirection; and under GCC it uses a statement expression to perform
 * union type punning.
 */

#include <stdlib.h>

#ifdef __onramp__
    #ifdef __onramp_cci_opc__
        unsigned strtof(const char* restrict nptr, char** restrict endptr);
        #define __strtof_u strtof
    #endif
    #ifndef __onramp_cci_opc__
        unsigned __strtof_u(const char* restrict nptr, char** restrict endptr) __asm__("strtof");
    #endif
#endif
#ifndef __onramp__
    #define __strtof_u(...) ((union {float f; unsigned u;}){.f = strtof(__VA_ARGS__)}.u)
#endif

#endif
