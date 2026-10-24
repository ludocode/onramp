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

#ifndef FLOAT_DEFS_H_INCLUDED
#define FLOAT_DEFS_H_INCLUDED

#include <stdint.h>

#define FLOAT_BITS 32u

#define FLOAT_EXPONENT_BITS 8u
#define FLOAT_EXPONENT_SHIFT FLOAT_SIGNIFICAND_BITS
#define FLOAT_EXPONENT_MASK (((uint32_t)1u << FLOAT_EXPONENT_BITS) - 1u)
#define FLOAT_EXPONENT_BIAS (((uint32_t)1u << (FLOAT_EXPONENT_BITS - 1u)) - 1u)

#define FLOAT_SIGNIFICAND_BITS 23u
#define FLOAT_SIGNIFICAND_MASK (((uint32_t)1u << FLOAT_SIGNIFICAND_BITS) - 1u)
#define FLOAT_SIGNIFICAND_IMPLICIT_BIT ((uint32_t)1u << FLOAT_SIGNIFICAND_BITS)

#define FLOAT_SIGN_SHIFT (FLOAT_SIGNIFICAND_BITS + FLOAT_EXPONENT_BITS)

#define FLOAT_SIGN_BIT ((uint32_t)1u << FLOAT_SIGN_SHIFT)  // 1 is negative, 0 is positive
#define FLOAT_HIDDEN_BIT ((uint32_t)1u << FLOAT_SIGNIFICAND_BITS)
#define FLOAT_QUIET_BIT ((uint32_t)1u << (FLOAT_SIGNIFICAND_BITS - 1u))

#define FLOAT_INFINITY (FLOAT_EXPONENT_MASK << FLOAT_EXPONENT_SHIFT)

// Our canonical NaN has only the quiet bit set.
#define FLOAT_QUIET_NAN (FLOAT_INFINITY | FLOAT_QUIET_BIT)

/**
 * Returns the biased exponent field of the given float.
 */
#ifdef __onramp_cpp_omc__
    #define FLOAT_EXPONENT __float_exponent
    static uint32_t __float_exponent(uint32_t x) {
        return (x >> FLOAT_EXPONENT_SHIFT) & FLOAT_EXPONENT_MASK;
    }
#endif
#ifndef __onramp_cpp_omc__
    #define FLOAT_EXPONENT(x) (((x) >> FLOAT_EXPONENT_SHIFT) & FLOAT_EXPONENT_MASK)
#endif

/**
 * Returns the significand of the given float, not including the hidden bit.
 */
#ifdef __onramp_cpp_omc__
    #define FLOAT_SIGNIFICAND __float_significand
    static uint32_t __float_significand(uint32_t x) {
        return x & FLOAT_SIGNIFICAND_MASK;
    }
#endif
#ifndef __onramp_cpp_omc__
    #define FLOAT_SIGNIFICAND(x) ((x) & FLOAT_SIGNIFICAND_MASK)
#endif

#endif
