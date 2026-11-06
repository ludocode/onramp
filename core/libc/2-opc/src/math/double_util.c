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

void ldexp(unsigned* out, unsigned* v, int exp) {
    __double_from_float(out, ldexpf(__double_to_float(v), exp));
}

void logb(unsigned* out, unsigned* v) {
    __double_from_float(out, logbf(__double_to_float(v)));
}

void frexp(unsigned* out, unsigned* x, int* exp) {
    __double_from_float(out, frexpf(__double_to_float(x), exp));
}

void fabs(unsigned* out, unsigned* v) {
    // TODO handle nan properly
    __double_from_float(out, __double_to_float(v) & ~FLOAT_SIGN_BIT);
}

void fmod(unsigned* out, unsigned* dividend, unsigned* divisor) {
    __double_from_float(out,
            fmodf(__double_to_float(dividend), __double_to_float(divisor)));
}

void remainder(unsigned* out, unsigned* dividend, unsigned* divisor) {
    __double_from_float(out,
            remainderf(__double_to_float(dividend), __double_to_float(divisor)));
}

void remquo(unsigned* out, unsigned* dividend, unsigned* divisor,
        unsigned* partial_quotient)
{
    unsigned pqf;
    unsigned val = remquof(__double_to_float(dividend), __double_to_float(divisor), &pqf);
    __double_from_float(partial_quotient, pqf);
    __double_from_float(out, val);
}

void fma(unsigned* out, unsigned* factor1, unsigned* factor2, unsigned* addend) {
    __double_from_float(out, fmaf(__double_to_float(factor1),
                __double_to_float(factor2), __double_to_float(addend)));
}

void fmax(unsigned* out, unsigned* x, unsigned* y) {
    __double_from_float(out, fmaxf(__double_to_float(x), __double_to_float(y)));
}

void fmin(unsigned* out, unsigned* x, unsigned* y) {
    __double_from_float(out, fminf(__double_to_float(x), __double_to_float(y)));
}

void fdim(unsigned* out, unsigned* x, unsigned* y) {
    __double_from_float(out, fdimf(__double_to_float(x), __double_to_float(y)));
}

void nan(unsigned* out, const char* __repr) {
    // TODO we should parse repr and stuff it in the low bits
    (void)__repr;
    __double_from_float(out, FLOAT_QUIET_NAN);
}

void modf(unsigned* out, unsigned* v, unsigned* integer_part) {
    unsigned ipf;
    unsigned val = modff(__double_to_float(v), &ipf);
    __double_from_float(integer_part, ipf);
    __double_from_float(out, val);
}

int ilogb(unsigned* v) {
    return ilogbf(__double_to_float(v));
}

void nextafter(unsigned* out, unsigned* from, unsigned* to) {
    __double_from_float(out,
            nextafterf(__double_to_float(from), __double_to_float(to)));
}

void nexttoward(unsigned* out, unsigned* from, unsigned* to) {
    __double_from_float(out,
            nexttowardf(__double_to_float(from), __double_to_float(to)));
}

void ceil(unsigned* out, unsigned* v) {
    __double_from_float(out, ceilf(__double_to_float(v)));
}

void floor(unsigned* out, unsigned* v) {
    __double_from_float(out, floorf(__double_to_float(v)));
}

void trunc(unsigned* out, unsigned* v) {
    __double_from_float(out, truncf(__double_to_float(v)));
}

void round(unsigned* out, unsigned* v) {
    __double_from_float(out, roundf(__double_to_float(v)));
}

long lround(unsigned* v) {
    return lroundf(__double_to_float(v));
}

void llround(unsigned* ll_out, unsigned* v) {
    llroundf(ll_out, __double_to_float(v));
}

void nearbyint(unsigned* out, unsigned* v) {
    __double_from_float(out, nearbyintf(__double_to_float(v)));
}

void rint(unsigned* out, unsigned* v) {
    __double_from_float(out, rintf(__double_to_float(v)));
}

long lrint(unsigned* v) {
    return lrintf(__double_to_float(v));
}

void llrint(unsigned* ll_out, unsigned* v) {
    llrintf(ll_out, __double_to_float(v));
}
