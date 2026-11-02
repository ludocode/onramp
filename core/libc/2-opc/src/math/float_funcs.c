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
#include <stdbool.h>

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
unsigned logf(unsigned x);
unsigned powf(unsigned x, unsigned y);

/*
 * TODO the accuracy of all these functions is poor. expf() and powf() in
 * particular lose several of the lower bits to rounding errors, and they
 * overflow or underflow on huge or subnormal values even if there is
 * sufficient range to represent the result. Most other functions here wrap one
 * of these two so it affects nearly everything.
 *
 * I have some ideas for how to fix this. We need to unpack floats into a
 * separate exponent and 32-bit mantissa (including the leading non-hidden
 * bit), and then all the algorithms should be implemented on the unpacked
 * representation, that way they can call eachother arbitrarily. We only round
 * back to 23 bits at the end of a public function when we convert back to
 * float. This is basically how 8087 hardware floating point works (except it
 * has a 64-bit mantissa which we would also do in our implementation of
 * doubles.) I should have done it this way from the start. Most of the
 * higher-level functions would probably be a lot faster too.
 *
 * The main complication with that is that we still want to be compilable with
 * cci/0 eventually which means limited function arguments and no structs on
 * the stack. If we pass exponent and mantissa separately we'll run out of
 * function arguments and if we pack them in a struct it will be a mess to
 * allocate temporaries (and will be extremely slow during bootstrapping.) I
 * haven't completely figured out what to do there yet.
 */



/*
 * Power functions
 */

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

unsigned cbrtf(unsigned f) {

    // Wrap powf() but preserve the sign
    unsigned xs = f & FLOAT_SIGN_BIT;
    return xs | powf(f & ~FLOAT_SIGN_BIT, ONE_THIRD);
}

unsigned hypotf(unsigned x, unsigned y) {
    return sqrtf(__float_add(__float_mul(x, x), __float_mul(y, y)));
}

unsigned powf(unsigned b, unsigned e) {

    //printf("----------------------------------------\n");
    //printf("powf(%.9g %#x, %.9g %#x)\n", u2f(b), b, u2f(e), e);
    //printf("%s:%i except %i\n", __FILE__,__LINE__,fetestexcept(FE_ALL_EXCEPT));

    // Unpack
    uint32_t bs = b & FLOAT_SIGN_BIT;
    uint32_t be = FLOAT_EXPONENT(b);
    uint32_t bm = FLOAT_SIGNIFICAND(b);
    uint32_t es = e & FLOAT_SIGN_BIT;
    uint32_t ee = FLOAT_EXPONENT(e);
    uint32_t em = FLOAT_SIGNIFICAND(e);

    // There are a huge number of special cases for pow(); see here:
    //     https://en.cppreference.com/c/numeric/math/pow

    // Check if the exponent is an integer and whether it is even or odd.
    bool e_is_int = false;
    bool e_is_odd = false;
    // TODO this check is not very robust. This would only matter for huge
    // exponents that overflow the range of an int32_t so hopefully it doesn't
    // really matter.
    // only used in cases of even integers to eliminate the sign, so we
    // shouldn't need to care if it doesn't work out of the bounds of an int.
    fenv_t env;
    feholdexcept(&env);
    feclearexcept(FE_ALL_EXCEPT);
    int ei = __float_to_i32(e);
    if (fetestexcept(FE_ALL_EXCEPT) == 0 && __float_from_i32(ei) == e) {
        e_is_int = true;
        e_is_odd = ei & 1;
    }
    fesetenv(&env);

    // Zero exponent
    if ((e & ~FLOAT_SIGN_BIT) == 0) {
        // Returns 1 for any base even when NaN. I'm not sure
        // if we should signal or not; assuming not.
        return ONE;
    }

    // +1 base
    if (b == ONE) {
        // Returns 1 for any exponent even when NaN. I'm not sure
        // if we should signal or not; assuming not.
        return ONE;
    }

    // Check base for NAN
    if (be == FLOAT_EXPONENT_MASK && bm) {
        // Signal if it's signaling
        if (!(bm & FLOAT_QUIET_BIT)) {
            __float_sigfpe();
        }
        // Return the same NAN quieted.
        return b | FLOAT_QUIET_BIT;
    }

    // Check exponent for NAN
    if (ee == FLOAT_EXPONENT_MASK && em) {
        // Signal if it's signaling
        if (!(em & FLOAT_QUIET_BIT)) {
            __float_sigfpe();
        }
        // Return the same NAN quieted.
        return e | FLOAT_QUIET_BIT;
    }

    // TODO probably would have been simpler to check for infinite exponents here

    // Zero base
    if (be == 0 && bm == 0) {
        if (ee == FLOAT_EXPONENT_MASK) {
            if (em == 0) {
                if (es) {
                    // +/-0^-inf == +inf. Apparently as of C23 this is no longer
                    // allowed to raise FE_DIVBYZERO.
                    return FLOAT_INFINITY;
                }
                // +/-0^inf == 0
                return 0;
            }

            // +/-0^NaN is NaN. Signal if it's signaling
            // TODO already checked for NAN above
            if (!(em & FLOAT_QUIET_BIT)) {
                __float_sigfpe();
            }
            // Return the same NaN quieted
            return em | FLOAT_QUIET_NAN;
        }

        if (es) {
            feraiseexcept(FE_DIVBYZERO);
            if (bs && e_is_int && e_is_odd) {
                return FLOAT_SIGN_BIT | FLOAT_INFINITY;
            }
            return FLOAT_INFINITY;
        }

        if (e_is_int && e_is_odd) {
            // +/-0 raised to positive odd integer preserves base sign
            return b;
        }
        // +/-0 raised to positive even integer or positive non-integer gives
        // positive zero
        return 0;
    }

    // -1 base
    if (b == (ONE | FLOAT_SIGN_BIT)) {
        if (ee == FLOAT_EXPONENT_MASK && em == 0) {
            // -1^+/-inf is 1
            return ONE;
        }
        if (e_is_int) {
            if (e_is_odd) {
                return ONE | FLOAT_SIGN_BIT;
            }
            return ONE;
        }
        // negative base raised to finite non-integer is domain error
        feraiseexcept(FE_INVALID);
        return FLOAT_QUIET_NAN;
    }

    // Infinite exponent
    if (ee == FLOAT_EXPONENT_MASK) {
        return (((b & ~FLOAT_SIGN_BIT) < ONE) ^ !!es) ? 0 : FLOAT_INFINITY;
    }

    // Infinite base
    if (be == FLOAT_EXPONENT_MASK) {
        // odd integer exponent preserves sign; positive otherwise
        uint32_t rs = (e_is_int && e_is_odd) ? bs : 0;
        // negative exponent goes to zero; infinity otherwise
        return rs | (es ? 0 : FLOAT_INFINITY);
    }

    // Negative base and non-integer exponent
    if (bs && !e_is_int) {
        // glibc returns -NAN here. I'm not sure why.
        feraiseexcept(FE_INVALID);
        return FLOAT_QUIET_NAN;
    }

    // Finally we can compute the result
    //printf("%s:%i except %i\n", __FILE__,__LINE__,fetestexcept(FE_ALL_EXCEPT));
    unsigned r = expf(__float_mul(e, logf(b & ~FLOAT_SIGN_BIT)));

    // Negative base and odd integer exponent makes a negative result. All
    // other combinations are positive.
    if (bs && e_is_odd) {
        assert(e_is_int);
        r |= FLOAT_SIGN_BIT;
    }

    return r;
}



/*
 * Exponential functions
 */

unsigned log2f(unsigned f) {
    return __float_div(logf(f), LOG_TWO);
}

unsigned log10f(unsigned f) {
    return __float_div(logf(f), LOG_TEN);
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
