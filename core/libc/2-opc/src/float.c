/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Fraser Heavy Software
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
 * Functions for floating point math.
 *
 * This is intended to eventually be a mostly correct software implementation
 * of IEEE 754.
 *
 * (We're not aiming for perfection here; this is just for bootstrapping after
 * all, and a perfect implementation requires arbitrary-precision arithmetic
 * among other things. This just needs to be good enough to bootstrap a
 * compiler, or perhaps bootstrap a dedicated software floating point library.)
 *
 * We use unsigned integer math everywhere and avoid signed integers because
 * the Onramp VM only has unsigned math instructions. (Signed integer math
 * operations are themselves emulated on Onramp.)
 */

#include <assert.h>   // TODO define NDEBUG when compiling final stages
#include <signal.h>
#include <stdint.h>

#ifdef __onramp__
    #include <__onramp/__arithmetic.h>
    #include <stdbit.h>
    #include "internal.h"
#endif
#ifndef __onramp__
    // We support compiling this with an ordinary C compiler, this way we can
    // test against hardware floating point math among other things.
    #include <stdio.h>
    #include <stdlib.h>
    static void __attribute__((unused)) __fatal(const char* message) {
        fputs(message, stderr);
        fputc('\n', stderr);
        _Exit(1);
    }
    #define stdc_leading_zerosui __builtin_clz
#endif



#define FLOAT_BITS 32u

#define FLOAT_EXPONENT_BITS 8u
#define FLOAT_EXPONENT_SHIFT FLOAT_SIGNIFICAND_BITS
#define FLOAT_EXPONENT_MASK (((uint32_t)1u << FLOAT_EXPONENT_BITS) - 1u)
#define FLOAT_EXPONENT_BIAS (((uint32_t)1u << (FLOAT_EXPONENT_BITS - 1u)) - 1u)

#define FLOAT_SIGNIFICAND_BITS 23u
#define FLOAT_SIGNIFICAND_MASK (((uint32_t)1u << FLOAT_SIGNIFICAND_BITS) - 1u)
#define FLOAT_SIGNIFICAND_IMPLICIT_BIT ((uint32_t)1u << FLOAT_SIGNIFICAND_BITS)

#define FLOAT_SIGN_SHIFT (FLOAT_SIGNIFICAND_BITS + FLOAT_EXPONENT_BITS)

#define FLOAT_SIGN_BIT ((uint32_t)1u << FLOAT_SIGN_SHIFT)
#define FLOAT_HIDDEN_BIT ((uint32_t)1u << FLOAT_SIGNIFICAND_BITS)
#define FLOAT_QUIET_BIT ((uint32_t)1u << (FLOAT_SIGNIFICAND_BITS - 1u))

#define FLOAT_INFINITY (FLOAT_EXPONENT_MASK << FLOAT_EXPONENT_SHIFT)

// Our canonical NaN has only the quiet bit set.
#define FLOAT_QUIET_NAN (FLOAT_INFINITY | FLOAT_QUIET_BIT)

// The number of leading zeroes in the significand of a normal float (with
// hidden bit set.)
// TODO not used
//#define FLOAT_NORMAL_LEADING_ZEROES (FLOAT_BITS - FLOAT_SIGNIFICAND_BITS - 1u)

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

/**
 * Shifts the significand of a float right by the given number of bits,
 * preserving any shifted off bits in the sticky bit.
 */
static uint32_t __float_shru_sticky(uint32_t significand, uint32_t bits) {
    //printf("__float_shru_sticky 0x%x by %u\n", significand, bits);
    if (bits == 0) {
        //printf("no shift\n");
        return significand;
    }

    if (bits >= FLOAT_BITS) {
        // We're shifting the entire number out. Collapse the whole number into
        // the sticky bit.
        //printf("collapse: %u\n",!!significand);
        return !!significand;
    }

    uint32_t mask = (1u << bits) - 1u;
    uint32_t sticky = !!(significand & mask);
    //printf("shifted: %u | %u\n",(significand >> bits), 0|sticky);
    return (significand >> bits) | sticky;
}

/**
 * Add the given unpacked unsigned floats, returning the result.
 *
 * Neither argument may be NaN.
 */
static uint32_t __float_add_impl(uint32_t ae, uint32_t be, uint32_t af, uint32_t bf) {
    //printf("----------------------------------------\n");
    //printf("add\n");
    //printf("ae 0x%x af 0x%x\n", ae, af);
    //printf("be 0x%x bf 0x%x\n", be, bf);

    // Handle infinity
    if (ae == FLOAT_EXPONENT_MASK || be == FLOAT_EXPONENT_MASK) {
        // At least one argument is infinity and the other is not NaN. Since
        // both arguments have the same sign (otherwise we'd be subtracting),
        // the result is infinity.
        return FLOAT_INFINITY;
    }

    // if not subnormal, add the hidden bit.
    // if subnormal, normalize the exponent to 1. (the exponent of subnormals
    // is the same as the lowest normal exponent, just without the hidden bit.)
    if (ae == 0u) {
        ae = 1u;
    } else {
        af |= FLOAT_HIDDEN_BIT;
    }
    if (be == 0u) {
        be = 1u;
    } else {
        bf |= FLOAT_HIDDEN_BIT;
    }

    // the target exponent for the calculation is the larger of the exponents
    uint32_t te = (ae > be ? ae : be);
    //printf("te 0x%x\n", te);

    // shift significands up to add guard, round and sticky bits (grs)
    af <<= 3u;
    bf <<= 3u;
    //printf("shifted for grs\n");
    //printf("af 0x%x\n", af);
    //printf("bf 0x%x\n", bf);

    // shift each significand down to match the target exponent, maintaining
    // the sticky bit
    af = __float_shru_sticky(af, te - ae);
    bf = __float_shru_sticky(bf, te - be);
    //printf("af 0x%x\n", af);
    //printf("bf 0x%x\n", bf);

    // add the significands
    uint32_t ts = af + bf;
    //printf("ts 0x%x\n", ts);

    // if our significand has grown larger than fits in this exponent, we need
    // to shift off another bit
    // Our max bits during calculation is significand bits plus the hidden
    // bit plus the three grs bits.
    //printf("max significand with grs: 0x%x\n", (1u << (FLOAT_SIGNIFICAND_BITS + 1u + 3u)));
    if (ts >= (1u << (FLOAT_SIGNIFICAND_BITS + 1u + 3u))) {
        //printf("significand has grown, shifting down\n");
        ++te;
        if (te == FLOAT_EXPONENT_MASK) {
            //printf("overflow\n");
            ts = 0u;
            goto end;
        }
        ts = ((ts >> 1u) | (ts & 1u)); // shru sticky inlined
        //printf("ts now 0x%x\n", ts);
        //printf("te now 0x%x\n", te);
    }

    // shift off our grs bits, rounding correctly
    // TODO support other rounding modes
    uint32_t grs = ts & 7u;
    ts >>= 3u;
    //printf("grs 0x%x %u%u%u\n", grs, grs>>2, !!(grs&2), grs&1);
    if (grs == 0b100u) {
        // round to even
        if (ts & 1u) {
            //printf("round up to even\n");
            ++ts;
        } else {
            //printf("round down to even\n");
        }
    } else if (grs >= 0b100u) {
        //printf("round up\n");
        ++ts;
    } else {
        //printf("round down\n");
    }
    //printf("ts after rounding 0x%x\n", ts);
    //printf("ts after removing grs bits 0x%x\n", ts);

    // Rounding may have caused it to grow again.
    //printf("max significand without grs: 0x%x\n", (1u << (FLOAT_SIGNIFICAND_BITS + 1u)));
    if (ts >= (1u << (FLOAT_SIGNIFICAND_BITS + 1u))) {
        //printf("significand too large after rounding. shifting down again\n");
        /*
        if (ts & 3u) {
            //printf("rounding up to even\n");
            ts += 2u;
        }
        */
        ++te;
        if (te == FLOAT_EXPONENT_MASK) {
            //printf("overflow\n");
            ts = 0u;
            goto end;
        }
        ts >>= 1u;
    }

    // check for overflow
    if (te == FLOAT_EXPONENT_MASK) {
        ts = 0u; // infinity
        goto end;
    }

    // check for subnormal
    if (!(ts & FLOAT_HIDDEN_BIT)) {
        assert(te == 1u);
        te = 0u;
        goto end;
    }

end:
    //printf("packing te 0x%x ts 0x%x hidden bit 0x%x\n", te, ts, ts & FLOAT_SIGNIFICAND_MASK);

    // A few sanity checks
    if (te == FLOAT_EXPONENT_MASK) {
        assert(ts == 0u); // infinity, not NAN
    } else {
        // Make sure our hidden bit is correct (it should be zero iff we're subnormal)
        assert(!(ts & FLOAT_HIDDEN_BIT) == (te == 0u));
        // Make sure we have no bits set above the hidden bit
        assert(((ts & ~FLOAT_SIGNIFICAND_MASK) & ~FLOAT_HIDDEN_BIT) == 0u);
    }

    // Pack the float back up again, removing the hidden bit
    //printf("result 0x%x\n", (te << FLOAT_EXPONENT_SHIFT) | (ts & FLOAT_SIGNIFICAND_MASK));
    return (te << FLOAT_EXPONENT_SHIFT) | (ts & FLOAT_SIGNIFICAND_MASK);
}

/**
 * Subtracts the given unpacked unsigned floats, returning the result.
 *
 * The smaller float with exponent `be` and significand (fraction) `bf` is
 * subtracted from the larger float with exponent `ae` and significand `af`.
 *
 * Neither argument may be NaN, and `a` must be larger in magnitude than `b`.
 */
static uint32_t __float_sub_impl(uint32_t ae, uint32_t be, uint32_t af, uint32_t bf) {

    // Handle infinity
    if (ae == FLOAT_EXPONENT_MASK || be == FLOAT_EXPONENT_MASK) {

        // Neither argument is NaN. The first argument must be infinity because
        // the caller made sure it's the larger number.
        assert(ae == FLOAT_EXPONENT_MASK);
        assert(af == 0);

        // If the second argument is also infinity, return NaN.
        if (be == FLOAT_EXPONENT_MASK) {
            assert(be == 0);
            return FLOAT_QUIET_NAN;
        }

        // The first argument is infinity and the other is finite. The result
        // is infinity.
        return FLOAT_INFINITY;
    }

    __fatal("TODO __float_sub_impl() not implemented yet");
    return FLOAT_QUIET_NAN;
}

uint32_t __float_add(uint32_t a, uint32_t b) {

    // Unpack arguments
    uint32_t as = a & FLOAT_SIGN_BIT;
    uint32_t bs = b & FLOAT_SIGN_BIT;
    uint32_t ae = FLOAT_EXPONENT(a);
    uint32_t be = FLOAT_EXPONENT(b);
    uint32_t af = FLOAT_SIGNIFICAND(a);
    uint32_t bf = FLOAT_SIGNIFICAND(b);

    // Handle NaNs
    if (ae == FLOAT_EXPONENT_MASK || be == FLOAT_EXPONENT_MASK) {

        // Signal if either NaN is signaling.
        // TODO should we signal twice if both are signaling? Assuming not but
        // this should be tested.
        if ((ae == FLOAT_EXPONENT_MASK && !(af & FLOAT_QUIET_BIT)) ||
            (be == FLOAT_EXPONENT_MASK && !(bf & FLOAT_QUIET_BIT)))
        {
            raise(SIGFPE);
        }

        // If either argument is NaN, we return the NaN quieted. (If both are
        // NaN, we return the first argument quieted.)
        if (ae == FLOAT_EXPONENT_MASK && af != 0u) {
            return a | FLOAT_QUIET_BIT;
        }
        if (be == FLOAT_EXPONENT_MASK && bf != 0u) {
            return b | FLOAT_QUIET_BIT;
        }
    }

    // If the signs match, it's a normal addition, preserving sign.
    if (as == bs) {
        return as | __float_add_impl(ae, be, af, bf);
    }

    // The signs differ so we need to subtract. We subtract the smaller
    // magnitude from the larger and tack the larger's sign bit on afterwards.
    uint32_t am = a & ~FLOAT_SIGN_BIT;
    uint32_t bm = b & ~FLOAT_SIGN_BIT;
    if (am > bm) {
        return as | __float_sub_impl(ae, be, af, bf);
    } else {
        return bs | __float_sub_impl(be, ae, bf, af);
    }
}

uint32_t __float_sub(uint32_t a, uint32_t b) {

    // We could just flip the sign bit of the subtrahend and forward to
    // addition, except that we would not be returning the original subtrahend
    // quieted if it's NaN. We need to check for NaN first.

    // Unpack arguments
    uint32_t as = a & FLOAT_SIGN_BIT;
    uint32_t bs = b & FLOAT_SIGN_BIT;
    uint32_t ae = FLOAT_EXPONENT(a);
    uint32_t be = FLOAT_EXPONENT(b);
    uint32_t af = FLOAT_SIGNIFICAND(a);
    uint32_t bf = FLOAT_SIGNIFICAND(b);

    // Handle NaNs
    if (ae == FLOAT_EXPONENT_MASK || be == FLOAT_EXPONENT_MASK) {

        // Signal if either NaN is signaling.
        // TODO should we signal twice if both are signaling? Assuming not but
        // this should be tested.
        if ((ae == FLOAT_EXPONENT_MASK && !(af & FLOAT_QUIET_BIT)) ||
            (be == FLOAT_EXPONENT_MASK && !(bf & FLOAT_QUIET_BIT)))
        {
            raise(SIGFPE);
        }

        // If either argument is NaN, we return the NaN quieted. (If both are
        // NaN, we return the first argument quieted.)
        if (ae == FLOAT_EXPONENT_MASK && af != 0u) {
            return a | FLOAT_QUIET_BIT;
        }
        if (be == FLOAT_EXPONENT_MASK && bf != 0u) {
            return b | FLOAT_QUIET_BIT;
        }
    }

    // A subtraction is the same as an addition with the sign of the subtrahend
    // flipped. If the signs differ, it's going to be an addition with the sign
    // of the minuend.
    if (as != bs) {
        return as | __float_add_impl(ae, be, af, bf);
    }

    // The signs are the same so we need to subtract. We subtract the smaller
    // magnitude from the larger and tack the larger's sign bit on afterwards.
    uint32_t am = a & ~FLOAT_SIGN_BIT;
    uint32_t bm = b & ~FLOAT_SIGN_BIT;
    if (am > bm) {
        return as | __float_sub_impl(ae, be, af, bf);
    } else {
        return bs | __float_sub_impl(be, ae, bf, af);
    }
}
