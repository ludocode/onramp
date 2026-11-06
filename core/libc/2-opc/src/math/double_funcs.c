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

#define __ONRAMP_LIBC_FLOATING_IMPL

#include "double_defs.h"

#include <math.h>

#include "float_defs.h"

#ifdef __onramp__
    #include <__onramp/__arithmetic.h>
#endif
#ifndef __onramp__
    #include "float_funcs.c"
    #include "float_ops.c"
    #include "float_util.c"
#endif

void sqrt(unsigned* out, unsigned* v) {
    __double_from_float(out, sqrtf(__double_to_float(v)));
}

void cbrt(unsigned* out, unsigned* v) {
    __double_from_float(out, cbrtf(__double_to_float(v)));
}

void hypot(unsigned* out, unsigned* x, unsigned* y) {
    __double_from_float(out, hypotf(__double_to_float(x), __double_to_float(y)));
}

void pow(unsigned* out, unsigned* x, unsigned* y) {
    __double_from_float(out, powf(__double_to_float(x), __double_to_float(y)));
}

void log(unsigned* out, unsigned* v) {
    __double_from_float(out, logf(__double_to_float(v)));
}

void log1p(unsigned* out, unsigned* v) {
    __double_from_float(out, log1pf(__double_to_float(v)));
}

void log2(unsigned* out, unsigned* v) {
    __double_from_float(out, log2f(__double_to_float(v)));
}

void log10(unsigned* out, unsigned* v) {
    __double_from_float(out, log10f(__double_to_float(v)));
}

void exp(unsigned* out, unsigned* v) {
    __double_from_float(out, expf(__double_to_float(v)));
}

void expm1(unsigned* out, unsigned* v) {
    __double_from_float(out, expm1f(__double_to_float(v)));
}

void exp2(unsigned* out, unsigned* v) {
    __double_from_float(out, exp2f(__double_to_float(v)));
}

void sin(unsigned* out, unsigned* v) {
    __double_from_float(out, sinf(__double_to_float(v)));
}

void cos(unsigned* out, unsigned* v) {
    __double_from_float(out, cosf(__double_to_float(v)));
}

void tan(unsigned* out, unsigned* v) {
    __double_from_float(out, tanf(__double_to_float(v)));
}

void asin(unsigned* out, unsigned* v) {
    __double_from_float(out, asinf(__double_to_float(v)));
}

void acos(unsigned* out, unsigned* v) {
    __double_from_float(out, acosf(__double_to_float(v)));
}

void atan(unsigned* out, unsigned* v) {
    __double_from_float(out, atanf(__double_to_float(v)));
}

void atan2(unsigned* out, unsigned* x, unsigned* y) {
    __double_from_float(out, atan2f(__double_to_float(x), __double_to_float(y)));
}

void sinh(unsigned* out, unsigned* v) {
    __double_from_float(out, sinhf(__double_to_float(v)));
}

void cosh(unsigned* out, unsigned* v) {
    __double_from_float(out, coshf(__double_to_float(v)));
}

void tanh(unsigned* out, unsigned* v) {
    __double_from_float(out, tanhf(__double_to_float(v)));
}

void asinh(unsigned* out, unsigned* v) {
    __double_from_float(out, asinhf(__double_to_float(v)));
}

void acosh(unsigned* out, unsigned* v) {
    __double_from_float(out, acoshf(__double_to_float(v)));
}

void atanh(unsigned* out, unsigned* v) {
    __double_from_float(out, atanhf(__double_to_float(v)));
}

void erf(unsigned* out, unsigned* v) {
    __double_from_float(out, erff(__double_to_float(v)));
}

void erfc(unsigned* out, unsigned* v) {
    __double_from_float(out, erfcf(__double_to_float(v)));
}

void tgamma(unsigned* out, unsigned* v) {
    __double_from_float(out, tgammaf(__double_to_float(v)));
}

void lgamma(unsigned* out, unsigned* v) {
    __double_from_float(out, lgammaf(__double_to_float(v)));
}
