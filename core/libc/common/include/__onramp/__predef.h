/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2026 Fraser Heavy Software
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

#ifndef __ONRAMP_LIBC_ONRAMP_PREDEF_H_INCLUDED
#define __ONRAMP_LIBC_ONRAMP_PREDEF_H_INCLUDED

/*
 * This file must be included implicitly by the compiler at the start of
 * compilation.
 */

#define __onramp_libc__ 1

#include <__onramp/__bool.h>

#define __STDC_NO_THREADS__
#define __STDC_NO_ATOMICS__
#define __STDC_NO_COMPLEX__
#define __STDC_NO_VLA__

#define __STDC_DEFER_TS25755__ 1

// These are here for lack of a better place to put them. We want functions
// declared in our libc headers (even those of earlier stages) to match the
// standards but we don't want to bother recognizing these keywords in cci/0.
#ifdef __onramp_cci_omc__
    #define _Noreturn
    #define restrict
    typedef int long;
#endif

// TODO some builtins will be defined here as well, at least until we implement
// them properly in cci/2
#ifdef __onramp_cci_opc__
    #define __builtin_unreachable abort
#endif
#ifndef __onramp_cci_opc__
    #ifndef __onramp_cci_omc__
        _Noreturn void __builtin_unreachable(void) __asm__("abort");
    #endif
#endif

// This is a bit inelegant: We can't pass -mabi=bootstrap to cc when compiling
// with cci/0 because it won't understand the option, but cc doesn't define
// __onramp_abi_bootstrap__ without it. Instead we define it for cci/0 here.
#ifdef __onramp_cci_omc__
    #define __onramp_abi_bootstrap__ 1
#endif
// We also need to define it for cci/1 for unit tests.
#ifdef __onramp_cci_opc__
    #define __onramp_abi_bootstrap__ 1
#endif

#endif
