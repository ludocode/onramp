/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Fraser Heavy Software
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

#ifndef __ONRAMP_LIBC_MATH_H_INCLUDED
#define __ONRAMP_LIBC_MATH_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

#define FP_INFINITE 1
#define FP_NAN 2
#define FP_NORMAL 3
#define FP_SUBNORMAL 4
#define FP_ZERO 5 // not zero

// TODO we declare these as taking float but the implementation takes unsigned.
// We need to sort out a better solution.
#ifndef __ONRAMP_LIBC_FLOAT_IMPL
int __float_fpclassify(float __x);
int __float_signbit(float __x);
int __float_signbit(float __x);
int __float_issignaling(float __x);
float copysignf(float __magnitude, float __sign);
#define copysign copysignf
#endif

/*
 * These are supposed to be macros. For now we only support floats;
 * eventually this should use _Generic or it should be a built-in.
 *
 * They are required to evaluate to ints where non-zero means true. On Onramp
 * they always return 0 or 1.
 */

#ifndef __onramp_cpp_omc__

// TODO these are C99

#define fpclassify(x) __float_fpclassify(x)
#define signbit(x) __float_signbit(x)

#define isinf(x) ((int)(__float_fpclassify(x) == FP_INFINITE))
#define isnan(x) ((int)(__float_fpclassify(x) == FP_NAN))
#define isnormal(x) ((int)(__float_fpclassify(x) == FP_NORMAL)) // does not include zero

#define isfinite(x) ((int) __extension__ ({ \
            int __c = float_fpclassify(x); __c != FP_INFINITE && __c != FP_NAN }))

// TODO these are C23

#define issignaling(x) __float_issignaling(x)
#define iszero(x) ((int)(__float_fpclassify(x) == FP_ZERO))
#define issubnormal(x) ((int)(__float_fpclassify(x) == FP_SUBNORMAL))

#endif

// TODO sin, cos, etc

// TODO C99 quiet macros isless(), isgreater(), etc.

#endif
