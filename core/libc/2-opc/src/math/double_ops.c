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

/*
 * Functions for `double` math.
 *
 * For now most functions just wrap the `float` equivalents since we are
 * compiling with a 32-bit double.
 *
 * The only trick is that double is passed indirectly (by pointer) even though
 * it is 32 bits. The compiler is already written to handle a 64-bit double;
 * eventually we'll switch it to 64 bits, then start upgrading these functions
 * to maintain full precision.
 */

#define __ONRAMP_LIBC_FLOATING_IMPL

#include "double_defs.h"

#include <stdlib.h>
#include <stdint.h>

#ifdef __onramp__
    #include <__onramp/__arithmetic.h>
    uint32_t strtof(const char* restrict str, char** /*nullable*/ restrict out_end);
    unsigned ldexpf(unsigned x, int exp);
#endif
#ifndef __onramp__
    #include "float_funcs.c"
    #include "float_ops.c"
    #include "float_util.c"
#endif



/*
 * double <-> float conversions
 */

// These functions are called everywhere here for now to do the conversion.
// They do nothing when double is 32 bits.

unsigned* __double_from_float(unsigned* d, unsigned f) {
    *d = f;
    return d;
}

unsigned __double_to_float(const unsigned* d) {
    return *d;
}



/*
 * double <-> integer conversions
 */

unsigned __double_to_u32(const unsigned* d) {
    return __float_to_u32(__double_to_float(d));
}

int __double_to_i32(const unsigned* d) {
    return __float_to_i32(__double_to_float(d));
}

unsigned* __double_to_u64(unsigned* u, const unsigned* d) {
    return __float_to_u64(u, __double_to_float(d));
}

unsigned* __double_to_i64(unsigned* i, const unsigned* d) {
    return __float_to_i64(i, __double_to_float(d));
}

unsigned* __double_from_u32(unsigned* d, unsigned u) {
    return __double_from_float(d, __float_from_u32(u));
}

unsigned* __double_from_i32(unsigned* d, int i) {
    return __double_from_float(d, __float_from_i32(i));
}

unsigned* __double_from_u64(unsigned* d, const unsigned* u) {
    return __double_from_float(d, __float_from_u64(u));
}

unsigned* __double_from_i64(unsigned* d, const unsigned* i) {
    return __double_from_float(d, __float_from_i64(i));
}



/*
 * Arithmetic
 */

unsigned* __double_add(unsigned* d, const unsigned* l, const unsigned* r) {
    return __double_from_float(d, __float_add(
                __double_to_float(l), __double_to_float(r)));
}

unsigned* __double_sub(unsigned* d, const unsigned* l, const unsigned* r) {
    return __double_from_float(d, __float_sub(
                __double_to_float(l), __double_to_float(r)));
}

unsigned* __double_mul(unsigned* d, const unsigned* l, const unsigned* r) {
    return __double_from_float(d, __float_mul(
                __double_to_float(l), __double_to_float(r)));
}

unsigned* __double_div(unsigned* d, const unsigned* l, const unsigned* r) {
    return __double_from_float(d, __float_div(
                __double_to_float(l), __double_to_float(r)));
}

unsigned* __double_negate(unsigned* out, const unsigned* in) {
    return __double_from_float(out, __float_negate(__double_to_float(in)));
}



/*
 * Comparisons
 */

_Bool __double_ne(const unsigned* l, const unsigned* r) {
    return __float_ne(__double_to_float(l), __double_to_float(r));
}

_Bool __double_lt(const unsigned* l, const unsigned* r) {
    return __float_lt(__double_to_float(l), __double_to_float(r));
}

_Bool __double_lte(const unsigned* l, const unsigned* r) {
    return __float_lte(__double_to_float(l), __double_to_float(r));
}



/*
 * Misc
 */

void strtod(unsigned* out, const char* restrict str, char** /*nullable*/ restrict out_end) {
    __double_from_float(out, strtof(str, out_end));
}

void ldexp(unsigned* out, unsigned x, int exp) {
    __double_from_float(out, ldexpf(x, exp));
}
