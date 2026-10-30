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

#include "float_defs.h"

#include <fenv.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

#ifndef ONRAMP_STANDALONE_TEST
#include <__onramp/__arithmetic.h>
#include "internal.h"
#endif

// TODO these should be constants
#define ONE __float_from_u32(1)
#define TWO __float_from_u32(2)
#define THREE __float_from_u32(3)
#define TEN __float_from_u32(10)
#define HALF __float_div(ONE, TWO)
#define ONE_THIRD __float_div(ONE, THREE)
#define THREE_HALFS __float_div(THREE, TWO)
#define LOG_TWO logf(TWO)
#define LOG_TEN logf(TEN)

unsigned expf(unsigned x);

unsigned sqrtf(unsigned x) {
    //printf("----------------------------------------\n");
    //printf("sqrtf\n");
    //printf("%.9g == %#08x\n", u2f(x), x);

    // If the value is zero, the result is zero (with the sign unchanged.)
    if ((x & ~FLOAT_SIGN_BIT) == 0) {
        //printf("zero\n");
        return x;
    }

    // Unpack the float
    uint32_t xs = x & FLOAT_SIGN_BIT;
    uint32_t xe = FLOAT_EXPONENT(x);
    uint32_t xm = FLOAT_SIGNIFICAND(x);

    // Handle infinity and NaN
    if (xe == FLOAT_EXPONENT_MASK) {

        // Check for infinity
        if (xm == 0) {
            if (xs) {
                // Negative infinity results in NaN
                feraiseexcept(FE_INVALID);
                return FLOAT_QUIET_NAN;
            } else {
                // Positive infinity stays the same
                return x;
            }
        }

        // NaN. Signal if it's signaling.
        if (!(xm & FLOAT_QUIET_BIT)) {
            __float_sigfpe();
        }

        // Return the NaN quieted.
        return x | FLOAT_QUIET_BIT;
    }

    // If the value is negative, the result is NaN.
    if (xs) {
        feraiseexcept(FE_INVALID);
        return FLOAT_QUIET_NAN;
    }

    // We use the classic Newton's method (or Babylonian method.) We start with
    // an initial guess, then iterate: each time we divide the number through
    // and average the current guess with the result. This is very slow because,
    // among other things, we have a division in each iteration.

    // TODO: sqrtf() is required by IEEE 754 to have perfect rounding, i.e. it
    // should exactly match the infinite precision result. This isn't supported
    // yet so the answer may be off by one bit (or perhaps two; I haven't
    // exhaustively tested it yet.) We will need to do the below math in higher
    // precision to get that last bit right. (At the same time it would be
    // worth switching to a faster algorithm. This is the simplest thing that
    // could possibly work but it's incredibly slow.)

    // For our initial guess, we ignore the mantissa and cut the exponent in
    // half (by averaging it with the bias.)
    uint32_t y = ((FLOAT_EXPONENT_BIAS + xe) >> 1) << FLOAT_EXPONENT_SHIFT;
    //printf("initial approximation %.9g\n", u2f(y));

    // We support a large number of iterations but we stop as soon as the
    // number stops changing. The number of correct bits should roughly double
    // each iteration so really this should only take a number of iterations
    // equal to the square root of the length of the significand. We cap the
    // iteration count at the length of the significand to be safe.
    for (size_t i = 0; i < FLOAT_SIGNIFICAND_BITS; ++i) {
        uint32_t next = __float_div(__float_add(y, __float_div(x, y)), TWO);
        //printf("next %.9g\n", u2f(next));
        if (y == next) {
            break;
        }
        y = next;
    }

    //printf("result %.9g\n", u2f(y));
    return y;
}

// Computes log1pf() for values of x in range (-0.5,0.5).
//
// This uses the Mercator series:
//     https://en.wikipedia.org/wiki/Mercator_series
//
// Although the series technically converges in the range (-1,1), it takes an
// astronomical number of iterations for values near -1 or 1. We limit it to
// ensure the iteration count is low; for 0.5 it takes 22 iterations.
static unsigned log1pf_impl(unsigned x) {
    //printf("%s %.9g\n", __func__, u2f(x));

    // Make sure we have a number in the valid range
    assert(FLOAT_EXPONENT(x) != FLOAT_EXPONENT_MASK);
    assert((x & ~FLOAT_SIGN_BIT) < ONE);

    unsigned sum = x;
    unsigned product = x;
    for (size_t i = 2; i < 200; ++i) {
        product = __float_mul(product, x);
        unsigned term = __float_div(product, __float_from_u32(i));
        unsigned sign_bit = (1 - (i & 1)) << FLOAT_SIGN_SHIFT;
        unsigned next = __float_add(sum, term | sign_bit);
        //printf("step %zu term %+.9g sum %.9g\n", i, u2f(term|sign_bit), u2f(next));
        if (sum == next) {
            break;
        }
        sum = next;
    }

    return sum;
}

// Computes logf() for values of x < 0.5 or x >= 1.5.
// This uses Halley's method (or Newton's method) with the exponential
// function:
//     https://en.wikipedia.org/wiki/Natural_logarithm#High_precision
static unsigned logf_impl(unsigned x) {
    //printf("%s %.9g\n", __func__, u2f(x));

    // Make sure we have a positive finite number
    assert(!(x & FLOAT_SIGN_BIT));
    assert(FLOAT_EXPONENT(x) != FLOAT_EXPONENT_MASK);

    // This can sometimes take a lot of iterations, e.g. logf(1.e+30f) takes 29
    // steps. The cap is arbitrary. Since expf() is itself iterative this can
    // take a very long time to run.
    unsigned sum = 0;
    for (size_t i = 2; i < 100; ++i) {
        unsigned e = expf(sum);
        unsigned q = __float_div(__float_sub(x, e), __float_add(x, e));
        unsigned next = __float_add(sum, __float_mul(TWO, q));
        //printf("step %zu q %+.9g sum %.9g\n", i, u2f(q), u2f(next));
        if (sum == next) {
            break;
        }
        sum = next;
    }

    return sum;
}

unsigned logf(unsigned x) {
    //printf("------------\n%s %.9g\n", __func__, u2f(x));

    // Zero
    if (0 == (x & ~FLOAT_SIGN_BIT)) {
        feraiseexcept(FE_DIVBYZERO);
        return FLOAT_INFINITY | FLOAT_SIGN_BIT;
    }

    // Infinity and NaN
    uint32_t xs = x & FLOAT_SIGN_BIT;
    uint32_t xe = FLOAT_EXPONENT(x);
    uint32_t xm = FLOAT_SIGNIFICAND(x);
    if (xe == FLOAT_EXPONENT_MASK) {
        if (xm != 0) {
            // NaN. Signal if it's signaling.
            if (!(xm & FLOAT_QUIET_BIT)) {
                __float_sigfpe();
            }

            // Return the NaN quieted.
            return x | FLOAT_QUIET_BIT;
        }

        if (!xs) {
            // Positive infinity stays the same
            return x;
        }
    }

    // Negative (including negative infinity)
    if (xs) {
        feraiseexcept(FE_INVALID);
        return FLOAT_QUIET_NAN;
    }

    // For x >= 0.5 && x <= 1.5 we use log1pf() because it's more accurate.
    if (x >= HALF && x <= THREE_HALFS) {
        return log1pf_impl(__float_sub(x, ONE));
    }

    return logf_impl(x);
}

unsigned log1pf(unsigned x) {
    //printf("------------\n%s %.9g\n", __func__, u2f(x));

    // Unpack the float
    uint32_t xs = x & FLOAT_SIGN_BIT;
    uint32_t xe = FLOAT_EXPONENT(x);
    uint32_t xm = FLOAT_SIGNIFICAND(x);

    // Handle infinity and NaN
    if (xe == FLOAT_EXPONENT_MASK) {

        // Check for infinity
        if (xm == 0) {
            if (xs) {
                // log1pf(-inf) is NaN
                feraiseexcept(FE_INVALID);
                return FLOAT_QUIET_NAN;
            } else {
                // log1pf(+inf) is +inf
                return x;
            }
        }

        // NaN. Signal if it's signaling.
        if (!(xm & FLOAT_QUIET_BIT)) {
            __float_sigfpe();
        }

        // Return the NaN quieted.
        return x | FLOAT_QUIET_BIT;
    }

    // Special case for zero. Return it with the same sign.
    if (xe == 0 && xm == 0) {
        return x;
    }

    // Special case for -1, result is -infinity.
    if (x == (ONE | FLOAT_SIGN_BIT)) {
        feraiseexcept(FE_DIVBYZERO);
        return FLOAT_INFINITY | FLOAT_SIGN_BIT;
    }

    // Less than -1 is NaN.
    if (xs && (x & ~FLOAT_SIGN_BIT) > ONE) {
        feraiseexcept(FE_INVALID);
        return FLOAT_QUIET_NAN;
    }

    // Greater than 0.5 or less than -0.5 we defer to logf().
    if ((x & ~FLOAT_SIGN_BIT) > HALF) {
        return logf_impl(__float_add(ONE, x));
    }

    return log1pf_impl(x);
}

unsigned expm1f(unsigned x) {
    //printf("------------\nexpm1f %.9g\n", u2f(x));

    // Unpack the float
    uint32_t xs = x & FLOAT_SIGN_BIT;
    uint32_t xe = FLOAT_EXPONENT(x);
    uint32_t xm = FLOAT_SIGNIFICAND(x);

    // Handle infinity and NaN
    if (xe == FLOAT_EXPONENT_MASK) {

        // Check for infinity
        if (xm == 0) {
            if (xs) {
                // expf(-inf) is -1
                return ONE | FLOAT_SIGN_BIT;
            } else {
                // expf(+inf) is +inf
                return x;
            }
        }

        // NaN. Signal if it's signaling.
        if (!(xm & FLOAT_QUIET_BIT)) {
            __float_sigfpe();
        }

        // Return the NaN quieted.
        return x | FLOAT_QUIET_BIT;
    }

    // Special case for zero. We are required to return it with the same sign.
    if (xe == 0 && xm == 0) {
        return x;
    }

    // If the argument is negative, the Taylor series diverges (destroying
    // precision or going to infinity) because the terms alternate between
    // positive and negative. Instead we can compute the exponential of the
    // positive value and use the identity `1/e^x==e^-x` to flip it afterwards.
    if (xs) {
        x &= ~FLOAT_SIGN_BIT;
    }

    // This is a simple Taylor series expansion:
    //     https://en.wikipedia.org/wiki/Exponential_function#Computation
    // It's very slow and a bit inaccurate on very large numbers but it's
    // extremely simple to implement so it'll do for now.

    unsigned sum = x;
    unsigned term = x;

    // Exponentials of very large numbers can take a very long time to
    // converge, e.g. expm1f(50) takes 92 iterations. We just cap it
    // arbitrarily to ensure we don't have an infinite loop.
    for (size_t i = 2; i < 200; ++i) {
        term = __float_div(__float_mul(term, x), __float_from_u32(i));
        unsigned next = __float_add(sum, term);
        //printf("step %zu term %.9g sum %.9g\n", i, u2f(term), u2f(next));
        if (sum == next) {
            break;
        }
        sum = next;
    }

    // If our argument was negative, we need to flip it now. `1/e^x==e^-x`, but
    // we're actually computing `(e^x)-1`, so we have `(e^-x)-1==(1/((e^x-1)+1))-1`.
    if (xs) {
        return __float_sub(__float_div(ONE, __float_add(ONE, sum)), ONE);
    }

    return sum;
}

unsigned expf(unsigned x) {
    return __float_add(ONE, expm1f(x));
}

unsigned exp2f(unsigned x) {
    return expf(__float_mul(x, LOG_TWO));
}
