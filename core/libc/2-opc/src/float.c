/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025-2026 Fraser Heavy Software
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

#define __ONRAMP_LIBC_FLOAT_IMPL

#include <assert.h>   // TODO define NDEBUG when compiling final stages
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

#ifdef __onramp__
    #include <__onramp/__arithmetic.h>
    #include <stdbit.h>
    #include "internal.h"
            //#define printf(...)
#endif
#ifndef __onramp__
    // We support compiling this with an ordinary C compiler, this way we can
    // test against hardware floating point math among other things.
    // TODO we need to move all of this to libo
    #include <stdio.h>
    #include <stdlib.h>
    static void __attribute__((unused)) __fatal(const char* message) {
        fputs(message, stderr);
        fputc('\n', stderr);
        exit(1);
    }
    #define stdc_leading_zerosui __builtin_clz
    #define copysignf __onramp_copysignf
    #define __llong_negate(out, x) ({unsigned* __out=(out); (*(uint64_t*)__out = -*(uint64_t*)(x)); __out;})
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

#define FLOAT_SIGN_BIT ((uint32_t)1u << FLOAT_SIGN_SHIFT)  // 1 is negative, 0 is positive
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
static uint32_t float_shru_sticky(uint32_t significand, uint32_t bits) {
    //printf("float_shru_sticky() 0x%x by %u\n", significand, bits);
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
    //printf("shifted: 0x%x | %u\n",(significand >> bits), 0|sticky);
    return (significand >> bits) | sticky;
}

/**
 * Rounds a significand that has additional guard-round-sticky bits. The three
 * extra bits are shifted off.
 *
 * Note that this may cause the significand to grow to an extra bit. In this
 * case it needs to be downshifted and the exponent incremented again.
 *
 * @param tf target significand (fraction)
 */
static uint32_t float_round_grs(uint32_t tf) {
    uint32_t grs = tf & 7u;
    //printf("float_round_grs() rounding 0x%x", tf);
    tf >>= 3u;
    //printf("  downshifted 0x%x\n", tf);
    //printf("  grs 0x%x %u%u%u\n", grs, grs>>2, !!(grs&2), grs&1);

    // TODO support other rounding modes
    if (grs == 0b100u) {
        // round to even
        if (tf & 1u) {
            //printf("  round up to even\n");
            ++tf;
        } else {
            //printf("  round down to even\n");
        }
    } else if (grs >= 0b100u) {
        //printf("  round up\n");
        ++tf;
    } else {
        //printf("  round down\n");
    }
    //printf("  tf after rounding 0x%x\n", tf);
    //printf("  tf after removing grs bits 0x%x\n", tf);
    return tf;
}

/**
 * Add the given unpacked unsigned floats, returning the packed result.
 *
 * Neither argument may be NaN.
 */
static uint32_t float_add_impl(uint32_t ae, uint32_t be, uint32_t af, uint32_t bf) {
    //printf("add impl\n");
    //printf("ae 0x%x af 0x%x\n", ae, af);
    //printf("be 0x%x bf 0x%x\n", be, bf);

    // Handle infinity
    if (ae == FLOAT_EXPONENT_MASK || be == FLOAT_EXPONENT_MASK) {
        // At least one argument is infinity and the other is not NaN. Since
        // both arguments have the same sign (otherwise we'd be subtracting),
        // the result is infinity.
        return FLOAT_INFINITY;
    }

    // If not subnormal, add the hidden bit.
    // If subnormal, normalize the exponent to 1. (The exponent of subnormals
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

    // The target exponent for the calculation is the larger of the exponents
    uint32_t te = (ae > be ? ae : be);
    //printf("te 0x%x\n", te);

    // Shift significands up to add guard, round and sticky bits (grs)
    af <<= 3u;
    bf <<= 3u;
    //printf("shifted for grs\n");
    //printf("af 0x%x\n", af);
    //printf("bf 0x%x\n", bf);

    // Shift each significand down to match the target exponent, maintaining
    // the sticky bit
    af = float_shru_sticky(af, te - ae);
    bf = float_shru_sticky(bf, te - be);
    //printf("af 0x%x\n", af);
    //printf("bf 0x%x\n", bf);

    // Add the significands
    uint32_t tf = af + bf;
    //printf("tf 0x%x\n", tf);

    // if our significand has grown larger than fits in this exponent, we need
    // to shift off another bit
    // Our max bits during calculation is significand bits plus the hidden
    // bit plus the three grs bits.
    //printf("max significand with grs: 0x%x\n", (1u << (FLOAT_SIGNIFICAND_BITS + 1u + 3u)));
    if (tf >= (1u << (FLOAT_SIGNIFICAND_BITS + 1u + 3u))) {
        //printf("significand has grown, shifting down\n");
        ++te;
        if (te == FLOAT_EXPONENT_MASK) {
            //printf("overflow\n");
            tf = 0u;
            goto end;
        }
        tf = ((tf >> 1u) | (tf & 1u)); // shru sticky inlined
        //printf("tf now 0x%x\n", tf);
        //printf("te now 0x%x\n", te);
    }

    // shift off our grs bits, rounding correctly
    tf = float_round_grs(tf);

    // Rounding may have caused it to grow again.
    //printf("  max significand without grs: 0x%x\n", (1u << (FLOAT_SIGNIFICAND_BITS + 1u)));
    if (tf >= (1u << (FLOAT_SIGNIFICAND_BITS + 1u))) {
        //printf("  significand too large after rounding. shifting down again\n");
        /*
        if (tf & 3u) {
            //printf("rounding up to even\n");
            tf += 2u;
        }
        */
        ++te;
        if (te == FLOAT_EXPONENT_MASK) {
            //printf("  overflow\n");
            tf = 0u;
            goto end;
        }
        tf >>= 1u;
    }

    // check for overflow
    if (te == FLOAT_EXPONENT_MASK) {
        tf = 0u; // infinity
        goto end;
    }

    // check for subnormal
    if (!(tf & FLOAT_HIDDEN_BIT)) {
        assert(te == 1u);
        te = 0u;
        goto end;
    }

end:
    //printf("packing te 0x%x tf 0x%x hidden bit 0x%x\n", te, tf, tf & FLOAT_SIGNIFICAND_MASK);

    // A few sanity checks
    if (te == FLOAT_EXPONENT_MASK) {
        assert(tf == 0u); // infinity, not NAN
    } else {
        // Make sure our hidden bit is correct (it should be zero iff we're subnormal)
        assert(!(tf & FLOAT_HIDDEN_BIT) == (te == 0u));
        // Make sure we have no bits set above the hidden bit
        assert(((tf & ~FLOAT_SIGNIFICAND_MASK) & ~FLOAT_HIDDEN_BIT) == 0u);
    }

    // Pack the float back up again, removing the hidden bit
    //printf("result 0x%x\n", (te << FLOAT_EXPONENT_SHIFT) | (tf & FLOAT_SIGNIFICAND_MASK));
    return (te << FLOAT_EXPONENT_SHIFT) | (tf & FLOAT_SIGNIFICAND_MASK);
}

/**
 * Subtracts the given unpacked unsigned floats, returning the packed result.
 *
 * The smaller float with exponent `be` and significand (fraction) `bf` is
 * subtracted from the larger float with exponent `ae` and significand `af`.
 *
 * Neither argument may be NaN, and `a` must be larger in magnitude than `b`.
 */
static uint32_t float_sub_impl(uint32_t ae, uint32_t be, uint32_t af, uint32_t bf) {
    //printf("sub impl\n");
    //printf("ae 0x%x af 0x%x\n", ae, af);
    //printf("be 0x%x bf 0x%x\n", be, bf);


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

    // A little optimization here: if the exponents differ by more than the
    // number of bits in our significand plus our grs bits, the subtrahend will
    // be shifted off to zero so the subtraction will have no effect.
    if (ae - be > FLOAT_SIGNIFICAND_BITS + 1 + 3) {
        //printf("exponent difference is %u which is more bits than significand; skipping subtraction\n", ae - be);
        //printf("returning minuend 0x%x\n", (ae << FLOAT_EXPONENT_SHIFT) | af);
        return (ae << FLOAT_EXPONENT_SHIFT) | af;
    }

    // TODO a lot of this is copy and pasted from float_add_impl. We could
    // consider sharing the code, perhaps with a macro or an always inline
    // function.

    // If not subnormal, add the hidden bit.
    // If subnormal, normalize the exponent to 1. (The exponent of subnormals
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

    // The target exponent for the calculation is that of the minuend
    uint32_t te = ae;
    //printf("te 0x%x\n", te);

    // Shift significands up to add guard, round and sticky bits (grs)
    af <<= 3u;
    bf <<= 3u;
    //printf("shifted for grs\n");
    //printf("af 0x%x\n", af);
    //printf("bf 0x%x\n", bf);

    // Shift the subtrahend down to match the target exponent, maintaining the
    // sticky bit
    bf = float_shru_sticky(bf, ae - be);
    //printf("af 0x%x\n", af);
    //printf("bf 0x%x\n", bf);

    // Subtract the significands
    uint32_t tf = af - bf;
    //printf("tf 0x%x\n", tf);

    // We can simplify the below logic by checking for zero now. This can only
    // happen if the exponents and significands are identical.
    if (tf == 0) {
        return 0;
    }

    // Find the position of the highest bit in the resulting significand.
    //printf("af has %i leading zeroes\n", stdc_leading_zerosui(af));
    //printf("tf has %i leading zeroes\n", stdc_leading_zerosui(tf));
    //printf("tf bits %i\n", 32 - stdc_leading_zerosui(tf));
    //printf("af bits %i\n", 32 - stdc_leading_zerosui(af));
    int bits = 32 - stdc_leading_zerosui(tf);
    //fflush(stdout);abort();

    // Shift the result up and exponent down as much as possible. We want our
    // significand bits plus a hidden bit plus grs bits.
    uint32_t normal_bits = FLOAT_SIGNIFICAND_BITS + 1 + 3;
    uint32_t shift = normal_bits - bits;
    // The minimum exponent is 1.
    if (te - 1 < normal_bits - bits) {
        // We don't have enough exponent to shift up. The number will be
        // subnormal.
        //printf("not enough exponent; shifting by te-1==%u to make subnormal\n",te-1);
        tf <<= te - 1;
        te = 1;
    } else {
        //printf("shifting by %u to make normal\n", shift);
        te -= shift;
        tf <<= shift;
    }

    // Round off the grs bits
    // TODO we could optimize this by only rounding when needed. We only need
    // to round if we've shifted up by less than 3 bits. (If we shifted up by 3
    // bits or more then grs is zero.)
    tf = float_round_grs(tf);

    // Rounding may have caused it to grow again.
    // TODO move this to rounding function
    //printf("  max significand without grs: 0x%x\n", (1u << (FLOAT_SIGNIFICAND_BITS + 1u)));
    if (tf >= (1u << (FLOAT_SIGNIFICAND_BITS + 1u))) {
        //printf("  significand too large after rounding. shifting down again\n");
        /*
        if (tf & 3u) {
            //printf("rounding up to even\n");
            tf += 2u;
        }
        */
        ++te;
        if (te == FLOAT_EXPONENT_MASK) {
            assert(false); // TODO probably not possible, it wasn't infinity before and we're subtracting
            //printf("  overflow\n");
            tf = 0u;
            goto end;
        }
        tf >>= 1u;
    }

    // check for overflow
    if (te == FLOAT_EXPONENT_MASK) {
        assert(false); // TODO probably not possible, it wasn't infinity before and we're subtracting
        tf = 0u; // infinity
        goto end;
    }

    // check for subnormal
    if (!(tf & FLOAT_HIDDEN_BIT)) {
        assert(te == 1u);
        te = 0u;
        goto end;
    }

end:
    //printf("packing te 0x%x tf 0x%x hidden bit 0x%x\n", te, tf, tf & FLOAT_SIGNIFICAND_MASK);

    // A few sanity checks
    if (te == FLOAT_EXPONENT_MASK) {
        assert(tf == 0u); // infinity, not NAN
    } else {
        // Make sure our hidden bit is correct (it should be zero iff we're subnormal)
        assert(!(tf & FLOAT_HIDDEN_BIT) == (te == 0u));
        // Make sure we have no bits set above the hidden bit
        assert(((tf & ~FLOAT_SIGNIFICAND_MASK) & ~FLOAT_HIDDEN_BIT) == 0u);
    }

    // Pack the float back up again, removing the hidden bit
    //printf("result 0x%x\n", (te << FLOAT_EXPONENT_SHIFT) | (tf & FLOAT_SIGNIFICAND_MASK));
    return (te << FLOAT_EXPONENT_SHIFT) | (tf & FLOAT_SIGNIFICAND_MASK);
}

/**
 * Adds the two given floats.
 *
 * A call to this function is emitted by the compiler for operator+ on floats.
 */
uint32_t __float_add(uint32_t a, uint32_t b) {
    //printf("----------------------------------------\n");
    //printf("add\n");
    //printf("0x%08x\n", a);
    //printf("0x%08x\n", b);

    // Unpack arguments
    uint32_t as = a & FLOAT_SIGN_BIT;
    uint32_t bs = b & FLOAT_SIGN_BIT;
    uint32_t ae = FLOAT_EXPONENT(a);
    uint32_t be = FLOAT_EXPONENT(b);
    uint32_t af = FLOAT_SIGNIFICAND(a);
    uint32_t bf = FLOAT_SIGNIFICAND(b);

    // Handle NaNs
    if (ae == FLOAT_EXPONENT_MASK || be == FLOAT_EXPONENT_MASK) {

        // Signal (once) if either NaN is signaling.
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
        return as | float_add_impl(ae, be, af, bf);
    }

    // The signs differ so we need to subtract. We subtract the smaller
    // magnitude from the larger and tack the larger's sign bit on afterwards.
    uint32_t am = a & ~FLOAT_SIGN_BIT;
    uint32_t bm = b & ~FLOAT_SIGN_BIT;
    uint32_t result;
    if (am > bm) {
        result = as | float_sub_impl(ae, be, af, bf);
    } else {
        result = bs | float_sub_impl(be, ae, bf, af);
    }
    //printf("__float_add() got sub result 0x%x\n", result);

    // (x+-x) is always +0. We need a special case for it.
    if ((result & ~FLOAT_SIGN_BIT) == 0) {
        //printf("  result is zero. returning +0\n");
        return 0;
    }

    return result;
}

/**
 * Subtracts the two given floats.
 *
 * A call to this function is emitted by the compiler for operator- on floats.
 */
uint32_t __float_sub(uint32_t a, uint32_t b) {
    //printf("----------------------------------------\n");
    //printf("sub\n");
    //printf("0x%08x\n", a);
    //printf("0x%08x\n", b);

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

        // Signal (once) if either NaN is signaling.
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

    // x-x (as x+-x) is +0 regardless of sign of input under all rounding
    // except roundToNegative. We special case it here.
    if (a == b) {
        return 0;
    }

    // A subtraction is the same as an addition with the sign of the subtrahend
    // flipped. If the signs differ, it's going to be an addition with the sign
    // of the minuend.
    if (as != bs) {
        return as | float_add_impl(ae, be, af, bf);
    }

    // The signs are the same so we need to subtract. We subtract the smaller
    // magnitude from the larger and tack the larger's sign bit on afterwards.
    uint32_t am = a & ~FLOAT_SIGN_BIT;
    uint32_t bm = b & ~FLOAT_SIGN_BIT;
    if (am >= bm) {
        return as | float_sub_impl(ae, be, af, bf);
    } else {
        // The sign of the subtrahend is flipped.
        return (bs ^ FLOAT_SIGN_BIT) | float_sub_impl(be, ae, bf, af);
    }
}

/**
 * Returns true if the given floats are equal.
 *
 * Positive zero equals negative zero, and NaN does not equal anything (not
 * even itself.) Otherwise, floats are equal if they have the same bit
 * representation.
 *
 * A call to this function is emitted by the compiler for operator== and
 * operator!= (with the result inverted) on floats.
 */
_Bool __float_eq(unsigned a, unsigned b) {

    // Handle NaNs
    uint32_t ae = FLOAT_EXPONENT(a);
    uint32_t be = FLOAT_EXPONENT(b);
    if (ae == FLOAT_EXPONENT_MASK || be == FLOAT_EXPONENT_MASK) {
        uint32_t af = FLOAT_SIGNIFICAND(a);
        uint32_t bf = FLOAT_SIGNIFICAND(b);

        // Signal (once) if either NaN is signaling.
        if ((ae == FLOAT_EXPONENT_MASK && !(af & FLOAT_QUIET_BIT)) ||
            (be == FLOAT_EXPONENT_MASK && !(bf & FLOAT_QUIET_BIT)))
        {
            raise(SIGFPE);
        }

        // If either argument is NaN, we return false.
        if (ae == FLOAT_EXPONENT_MASK && af != 0u) {
            return false;
        }
        if (be == FLOAT_EXPONENT_MASK && bf != 0u) {
            return false;
        }
    }

    // Negative zero equals positive zero. We can use bitwise or to check if
    // any bits are set.
    if (((a | b) & ~FLOAT_SIGN_BIT) == 0) {
        return true;
    }

    // Otherwise the numbers are equal if and only if the bit representation
    // exactly matches.
    return a == b;
}

/**
 * Returns true if a is less than b.
 *
 * Positive zero equals negative zero, and NaN does not equal anything (not
 * even itself.)
 *
 * A call to this function is emitted by the compiler for operator< and
 * operator>= (with arguments swapped) on floats.
 */
_Bool __float_lt(unsigned a, unsigned b) {

    // Handle NaNs
    uint32_t ae = FLOAT_EXPONENT(a);
    uint32_t be = FLOAT_EXPONENT(b);
    if (ae == FLOAT_EXPONENT_MASK || be == FLOAT_EXPONENT_MASK) {
        uint32_t af = FLOAT_SIGNIFICAND(a);
        uint32_t bf = FLOAT_SIGNIFICAND(b);

        // Signal (once) if either NaN is signaling.
        if ((ae == FLOAT_EXPONENT_MASK && !(af & FLOAT_QUIET_BIT)) ||
            (be == FLOAT_EXPONENT_MASK && !(bf & FLOAT_QUIET_BIT)))
        {
            raise(SIGFPE);
        }

        // If either argument is NaN, we return false.
        if (ae == FLOAT_EXPONENT_MASK && af != 0u) {
            return false;
        }
        if (be == FLOAT_EXPONENT_MASK && bf != 0u) {
            return false;
        }
    }

    // Check whether the signs match.
    uint32_t as = a & FLOAT_SIGN_BIT;
    uint32_t bs = b & FLOAT_SIGN_BIT;
    if (as != bs) {
        // Signs differ.

        // Negative zero equals positive zero. We can use bitwise or to check if
        // any bits are set.
        if ((a | b) == FLOAT_SIGN_BIT) {
            return false;
        }

        // Otherwise the negative number comes before the positive one.
        return as;
    }

    // Otherwise we can compare the numbers bitwise. If the numbers are
    // negative we have to reverse the arguments. (We can't just flip the
    // result because we still have to return false on equal numbers.)
    return as ? (b < a) : (a < b);
}

/**
 * Returns true if a is less than or equal to b.
 *
 * Positive zero equals negative zero, and NaN does not equal anything (not
 * even itself.)
 *
 * A call to this function is emitted by the compiler for operator<= and
 * operator> (with arguments swapped) on floats.
 */
_Bool __float_lte(unsigned a, unsigned b) {

    // Handle NaNs
    uint32_t ae = FLOAT_EXPONENT(a);
    uint32_t be = FLOAT_EXPONENT(b);
    if (ae == FLOAT_EXPONENT_MASK || be == FLOAT_EXPONENT_MASK) {
        uint32_t af = FLOAT_SIGNIFICAND(a);
        uint32_t bf = FLOAT_SIGNIFICAND(b);

        // Signal (once) if either NaN is signaling.
        if ((ae == FLOAT_EXPONENT_MASK && !(af & FLOAT_QUIET_BIT)) ||
            (be == FLOAT_EXPONENT_MASK && !(bf & FLOAT_QUIET_BIT)))
        {
            raise(SIGFPE);
        }

        // If either argument is NaN, we return false.
        if (ae == FLOAT_EXPONENT_MASK && af != 0u) {
            return false;
        }
        if (be == FLOAT_EXPONENT_MASK && bf != 0u) {
            return false;
        }
    }

    // Check whether the signs match.
    uint32_t as = a & FLOAT_SIGN_BIT;
    uint32_t bs = b & FLOAT_SIGN_BIT;
    if (as != bs) {
        // Signs differ.

        // Negative zero equals positive zero. We can use bitwise or to check if
        // any bits are set.
        if ((a | b) == FLOAT_SIGN_BIT) {
            return true;
        }

        // Otherwise the negative number comes before the positive one.
        return as;
    }

    // Otherwise we can compare the numbers bitwise. If the numbers are
    // negative we have to flip the result.
    return as ? (b <= a) : (a <= b);
}

int __float_fpclassify(unsigned x) {
    uint32_t xe = FLOAT_EXPONENT(x);
    uint32_t xf = FLOAT_SIGNIFICAND(x);
    if (xe == 0) {
        return xf == 0 ? FP_ZERO : FP_SUBNORMAL;
    }
    if (xe == FLOAT_EXPONENT_MASK) {
        return xf == 0 ? FP_INFINITE : FP_NAN;
    }
    return FP_NORMAL;
}

int __float_signbit(unsigned x) {
    return x >> (FLOAT_BITS - 1);
}

unsigned copysignf(unsigned magnitude, unsigned sign) {
    return (magnitude & ~FLOAT_SIGN_BIT) | (sign & FLOAT_SIGN_BIT);
}

int __float_issignaling(unsigned x) {
    uint32_t xe = FLOAT_EXPONENT(x);
    uint32_t xf = FLOAT_SIGNIFICAND(x);
    return xe == FLOAT_EXPONENT_MASK && xf != 0 && !(xf & FLOAT_QUIET_BIT);
}

/**
 * An arbitrary precision decimal.
 *
 * The digits are stored as 8-bit binary coded decimal in the given buffer.
 * Each byte is a decimal digit (in range 0-9, not offset by '0'.) The buffer
 * represents an integer; there is no fractional part.
 *
 * The exponent is the power of ten multiplier. A positive exponent is a
 * positive power of ten; a negative exponent is a negative power of ten. In
 * other words, a positive exponent indicates a number of extra zeroes, and a
 * negative exponent indicates how many digits are after the decimal point.
 * TODO we don't check for overflow on the exponent yet so it has to be reasonable.
 *
 * The capacity is fixed; it must be pre-allocated correctly. If there is
 * insufficient capacity for an operation, a fatal error occurs.
 *
 * Leading and trailing zeroes should be avoided. A value of 0 is normally
 * represented by length 0.
 */
typedef struct bigdec_t {
    size_t capacity;
    size_t length;
    unsigned char* buffer;
    int exponent;
} bigdec_t;

void __bigdec_print(bigdec_t* bigdec, char* out, size_t outsize) {
    unsigned char* buffer = bigdec->buffer;
    size_t length = bigdec->length;
    int exponent = bigdec->exponent;

    // make sure we have enough space
    size_t needed = bigdec->length;
    if (length == 0) {
        needed = 1;
    } else if (exponent > 0) {
        needed += (size_t)exponent;
    } else if ((size_t)-exponent >= length) {
        needed += (size_t)-exponent - length + 2;
    } else if (exponent < 0) {
        needed += 1;
    }
    if (needed >= outsize) {
        __fatal("buffer too small");
    }

//printf("    print length %i exponent %i\n", (int)length, exponent);
    if (length == 0) {
        *out++ = '0';
    } else if (exponent >= 0) {
        for (size_t i = 0; i != length; ++i) {
//printf("    print buffer %i is %i\n", (int)i, buffer[i]);
            *out++ = '0' + buffer[i];
        }
        for (size_t i = 0; i != (size_t)exponent; ++i) {
            *out++ = '0';
        }
    } else if (length > (size_t)-exponent) {
        size_t c = length + exponent;
        size_t i = 0;
//printf("    print c %i length %i exponent %i\n", (int)c, (int)length, (int)exponent);
        for (; i != c; ++i) {
            *out++ = '0' + buffer[i];
        }
        *out++ = '.';
        for (; i != length; ++i) {
            *out++ = '0' + buffer[i];
        }
    } else {
        *out++ = '0';
        *out++ = '.';
        size_t z = -exponent - length;
        for (size_t i = 0; i < z; ++i) {
            *out++ = '0';
        }
        for (size_t i = 0; i != length; ++i) {
            *out++ = '0' + buffer[i];
        }
    }

    *out = 0;
}

static void __bigdec_trim_leading_zeroes(bigdec_t* bigdec) {
    size_t length = bigdec->length;
    if (length == 0) {
        return;
    }

    unsigned char* buffer = bigdec->buffer;
    if (buffer[0] != 0) {
        return;
    }

    size_t shift = 0;
    while (buffer[shift] == 0 && shift < length) {
        ++shift;
    }

    if (shift == length) {
        bigdec->length = 0;
        bigdec->exponent = 0;
        return;
    }

    length -= shift;
    for (size_t i = 0; i < length; ++i) {
        buffer[i] = buffer[i + shift];
    }
    bigdec->length = length;
}

// TODO merge into above
static void __bigdec_trim_trailing_zeroes(bigdec_t* bigdec) {
    size_t length = bigdec->length;
    if (length == 0) {
        return;
    }

    unsigned char* buffer = bigdec->buffer;
    while (length > 0 && buffer[length - 1] == 0) {
        --length;
        ++bigdec->exponent;
    }
    bigdec->length = length;
}

// TODO need to break up the below into separate functions. mul2, div2

/**
 * Multiply an arbitrary precision decimal by the given power of two,
 * maintaining full precision.
 *
 * The result is computed by iterating a multiplication or division by 2 the
 * given number of times.
 *
 * There must be no leading zeroes. Any trailing zeroes will be trimmed.
 */
static void __bigdec_mul_pow2(bigdec_t* bigdec, int pow2) {
    if (pow2 == 0) {
        return;
    }

    size_t length = bigdec->length;
    if (length == 0) {
        return;
    }
    assert(bigdec->buffer[0] != 0); // no leading zeroes

    unsigned char* buffer = bigdec->buffer;
    size_t capacity = bigdec->capacity;
    int exponent = bigdec->exponent;

    if (pow2 > 0) {

        // Multiply.
        while (pow2-- != 0) {

            // First we check if we will need an extra digit. This happens if the
            // first digit is 5 or greater.
            size_t extra_digit = buffer[0] >= 5;
            if (length + extra_digit > capacity) {
                __fatal("Internal error: insufficient capacity for bigdec pow2 multiply");
            }

            // Next we multiply each digit by two starting at the bottom.
            uint32_t carry = 0;
            for (size_t k = length; k-- > 0;) {
                uint32_t result = (buffer[k] << 1) + carry;
                if (result >= 10) {
                    result -= 10;
                    carry = 1;
                } else {
                    carry = 0;
                }
                buffer[k + extra_digit] = (unsigned char)result;
            }
            assert(extra_digit == carry);
            if (extra_digit) {
                buffer[0] = 1;
            }
            length += extra_digit;

            // Trim trailing zeroes.
            while (buffer[length - 1] == 0) {
                --length;
                ++exponent;
            }
        }

    } else {

        // Divide.

        // Start by trimming trailing zeroes. (A divide can never add a
        // trailing zero.)
        // TODO no, should be done separately
        while (buffer[length - 1] == 0) {
            --length;
            ++exponent;
        }

        while (pow2++ != 0) {
            uint32_t carry = 0;

            // If the first digit is 1, we're going to eliminate it.
            size_t eliminate_digit = 0;
            if (buffer[0] == 1) {
                //printf("eliminate digit\n");
                eliminate_digit = 1;
                --length;
                carry = 10;
            }

            // Divide through each digit, carrying a 5 wherever we get an odd number
            for (size_t k = 0; k < length; ++k) {
                uint32_t sum = buffer[k + eliminate_digit] + carry;
                //printf("digit %u carry %u sum %u\n",buffer[k + eliminate_digit], carry,sum);
                carry = (sum & 1) * 10;
                buffer[k] = (unsigned char)(sum >> 1);
            }

            // If the last digit was odd, add the 5
            //printf("final carry %u\n",carry);
            if (carry != 0) {
                buffer[length++] = 5;
                --exponent;
            }
        }
    }

    bigdec->length = length;
    bigdec->exponent = exponent;
}

/**
 * Gets the integer portion of a big decimal.
 *
 * Any remaining fraction is truncated. If the number is too large, it will
 * overflow.
 */
uint32_t __bigdec_uint(bigdec_t* bigdec, bool round) {
    size_t length = bigdec->length;
    if (length == 0) {
        return 0;
    }

    unsigned char* buffer = bigdec->buffer;
    int integer_digits = bigdec->exponent + length;
    if (integer_digits < 0) {
        // If we have less than zero integer digits, the number is 0.0X, so we
        // round or truncate down to zero. If we have exactly zero integer
        // digits, we may need to round correctly.
        return 0;
    }

    // Convert the integer portion of the bigdecimal to an integer
    uint32_t decint = 0;
    size_t real_digits = ((size_t)integer_digits < length) ? (size_t)integer_digits : length;
    size_t i = 0;
    for (; i < real_digits; ++i) {
        decint = (decint * 10) + buffer[i];
    }
    for (; i < (size_t)integer_digits; ++i) {
        decint *= 10;
    }

    if (round) {
        if (length > (size_t)integer_digits) {
            // We have a fraction.
            uint32_t fraction_digit = buffer[integer_digits];
            if (fraction_digit > 5) {
                ++decint;
            } else if (fraction_digit == 5) {
                if (length > ((size_t)integer_digits + 1)) {
                    ++decint; // the fraction digit is 5, but we have additional digits; round up
                } else if (integer_digits != 0 && (buffer[integer_digits - 1] & 1)) {
                    ++decint; // the fraction is exactly 5 and the previous digit is odd; round to even
                }
            }
        }
    }

    return decint;
}

/**
 * Compares a big decimal to a uint32_t.
 *
 * Returns -1 if the big decimal is smaller, +1 if it's larger, and 0 if they
 * are equal.
 */
int __bigdec_cmp_u32(bigdec_t* bigdec, uint32_t value) {
    size_t length = bigdec->length;
    if (length == 0) {
        return (value == 0) ? 0 : -1;
    }

    // No leading or trailing zeroes
    unsigned char* buffer = bigdec->buffer;
    assert(buffer[0] != 0);
    assert(buffer[length - 1] != 0);

    // Check if the decimal is outside the range of a uint32_t. In this case we
    // don't need to compare at all.
    int exponent = bigdec->exponent;
    int integer_digits = exponent + length;
    if (integer_digits < 0) {
        return -1;
    }
    if (integer_digits > 10) {
        return 1;
    }

    // Compare integer portion of the value
    uint32_t uint = __bigdec_uint(bigdec, false);
    if (uint < value) {
        return -1;
    }
    if (uint > value) {
        return 1;
    }

    // Integer portion is equal. Decimal is larger if it has a fraction.
    return (length > (size_t)integer_digits) ? 1 : 0;
}

/**
 * Parse a string to a float.
 *
 * We return uint32_t instead of float so this can be compiled with our earlier
 * bootstrapping stages.
 *
 * The implementation is based on iterated multiplication/division by 2 of
 * arbitrary precision decimals. It's very slow but it's also very simple.
 * Printing is described by the following article; parsing is the reverse:
 *
 *     https://research.swtch.com/ftoa
 */
uint32_t strtof(const char* restrict str, char** /*nullable*/ restrict out_end) {

    // Skip leading whitespace
    const unsigned char* restrict p = (const unsigned char*)str;
    while (isspace(*p)) {
        ++p;
    }

    // Check for nan
    // TODO need to use strncasecmp_l() in the C locale
    if (0 == strncasecmp((char*)p, "nan", 3)) {
        // TODO glibc supports a following (...) containing a decimal, 0 octal,
        // or 0x hexadecimal number representing the mantissa of the nan. if
        // necessary we could easily implement it.
        *out_end = (char*)p + 3;
        return FLOAT_QUIET_NAN;
    }

    // Parse the sign
    uint32_t sign = 0;
    if (*p == '-') {
        sign = 1;
        ++p;
    } else if (*p == '+') {
        ++p;
    }

    if (*p == '0' && (p[1] == 'x' || p[1] == 'X')) {
        __fatal("TODO hex float parsing");
    }

    // Check for infinity
    // TODO need to use strncasecmp_l() in the C locale
    if (0 == strncasecmp((char*)p, "inf", 3)) {
        p += 3;
        if (0 == strncasecmp((char*)p, "inity", 5)) {
            p += 5;
        }
        *out_end = (char*)p;
        return FLOAT_INFINITY | (sign << FLOAT_SIGN_SHIFT);
    }

    // Count the decimal digits
    const unsigned char* digits = p;
    size_t digit_count = 0;
    int dec_exponent = 0;
    while (isdigit(*p)) {
        ++p;
        ++digit_count;
    }
    if (*p == '.') {
        dec_exponent = 0;
        ++p;
        while (isdigit(*p)) {
            ++p;
            ++digit_count;
            --dec_exponent;
        }
    }

    // Make sure we have at least one digit
    if (digit_count == 0) {
        if (out_end) {
            *out_end = (char*)str;
        }
        return 0;
    }

    // Allocate a big decimal sufficiently large
    bigdec_t bigdec;
    bigdec.capacity = 150 + digit_count; // probably overkill, TODO shorten it later, TODO use alloca if it's small enough
    bigdec.length = digit_count;
    bigdec.buffer = malloc(bigdec.capacity);
    if (bigdec.buffer == 0) {
        errno = ENOMEM; // not a legal error code
        *out_end = (char*)str;
        return 0;
    }

    // Fill in the digits
    for (size_t i = 0; digits != p; ++digits) {
        if (*digits != '.') {
            bigdec.buffer[i++] = *digits - '0';
        }
    }

    // Get the exponent if any.
    // (If the exponent is incomplete, we ignore it, and out_end will be set to
    // the start of the bad exponent.)
    const unsigned char* end = p;
    if (*p == 'e' || *p == 'E') {
        ++p;
        if (isdigit(*p) || *p == '+' || *p == '-') {
            const unsigned char* exp_end = p;
            long decl_exponent = strtol((const char*)p, (char**)&exp_end, 10);
            if (p != exp_end) {

                // Limit the range
                // (We'll limit again once we convert decimal to binary; this
                // is just to prevent an overflow in our big decimal
                // calculation.)
                if (decl_exponent > 40 || decl_exponent < -60) {
                    free(bigdec.buffer);
                    errno = ERANGE;
                    // TODO this is wrong, if exponent is too small we should return FLT_MIN and set ERANGE
                    return FLOAT_INFINITY | (sign << FLOAT_SIGN_SHIFT);
                }

                // We have a valid exponent.
                dec_exponent += decl_exponent;
                end = exp_end;
            }
        }
    }
    bigdec.exponent = dec_exponent;
    *out_end = (char*)end;

    __bigdec_trim_leading_zeroes(&bigdec);
    __bigdec_trim_trailing_zeroes(&bigdec);
    //{char buf[256]; __bigdec_print(&bigdec, buf, sizeof(buf)); printf("loaded %s\n", buf);}
    if (bigdec.length == 0) {
        free(bigdec.buffer);
        return sign << FLOAT_SIGN_SHIFT;
    }

    // Our mantissa must be exactly a 24-bit number (with a leading 1.) We need
    // to multiply or divide by 2 until the integer part of the big decimal
    // fits in exactly this many bits.
    int pow2 = FLOAT_SIGNIFICAND_BITS + FLOAT_EXPONENT_BIAS;
    if (-1 == __bigdec_cmp_u32(&bigdec, 1 << FLOAT_SIGNIFICAND_BITS)) {
        // Our number is too small. We have to multiply by 2.
        do {
            --pow2;
            __bigdec_mul_pow2(&bigdec, 1);
        } while (pow2 > 1 && -1 == __bigdec_cmp_u32(&bigdec, 1 << FLOAT_SIGNIFICAND_BITS));
    } else {
        // Our number may be too large. We have to divide by two.
        while (-1 != __bigdec_cmp_u32(&bigdec, 1 << (FLOAT_SIGNIFICAND_BITS + 1))) {
            ++pow2;
            __bigdec_mul_pow2(&bigdec, -1);
        }
    }
    //{char buf[256]; __bigdec_print(&bigdec, buf, sizeof(buf)); printf("result %s\n", buf);}

    if (pow2 > (1 << FLOAT_EXPONENT_BITS)) {
        free(bigdec.buffer);
        errno = ERANGE;
        return FLOAT_INFINITY | (sign << FLOAT_SIGN_SHIFT);
    }

    uint32_t mantissa = __bigdec_uint(&bigdec, true);

    // if the number is subnormal, we have to raise ERANGE
    if (!(mantissa & FLOAT_HIDDEN_BIT)) {
        pow2 = 0;
        errno = ERANGE;
    }

    free(bigdec.buffer);
    return (sign << FLOAT_SIGN_SHIFT)
            | ((uint32_t)pow2 << FLOAT_EXPONENT_SHIFT)
            | (mantissa & FLOAT_SIGNIFICAND_MASK);
}



/*
 * Conversions from integers
 */

// To do the conversion, we need to shift the integer to exactly 26 bits: the
// significand bits, the hidden bit, a round bit and a sticky bit.
#define FLOAT_FROM_INT_BITS (FLOAT_SIGNIFICAND_BITS + 1u + 2u)

// A helper to create a float from a 26 bit significand. The significand must
// have the hidden bit plus a round bit and a sticky bit.
static unsigned __float_from_u26_impl(unsigned x, unsigned exponent) {

    // Round. If the round bit is set, we round up if the sticky bit is set,
    // otherwise we round to even.
    //printf("__float_from_u26_impl() value x %u %#x\n",x,x);
    if ((x & 2u) && ((x & 1u) || (x & 4u))) {
        //printf("+4\n");
        x += 4u;
    }

    // This may have caused us to grow by one bit, in which case we need to down
    // shift again.
    if (x >= (1u << FLOAT_FROM_INT_BITS)) {
        //printf("round again\n");
        x >>= 1;
        ++exponent;
    }

    // remove the rs bits
    x >>= 2u;
    //printf("downshifted %u %#x\n",x,x);

    // trim hidden bit
    x &= FLOAT_SIGNIFICAND_MASK;

    // assemble float
    //printf("exponent %u mantissa %u\n",exponent,x);
    return (exponent << FLOAT_EXPONENT_SHIFT) | x;
}

unsigned __float_from_u32(unsigned x) {
    if (x == 0u) {
        return 0u;
    }

    // Find the first set bit
    unsigned bits = 32u - stdc_leading_zerosui(x);
    unsigned exponent = FLOAT_EXPONENT_BIAS + bits - 1u;

    // Shift up (normal) or down (sticky!) until 26 bits are set
    // TODO we should probably optimize this to not have to call shru_sticky().
    // we should create a mask for the bits to be shifted off; we only need to
    // shift once.
    //printf("x %#x bits %u FLOAT_FROM_INT_BITS %u exponent %u\n", x, bits, FLOAT_FROM_INT_BITS, exponent);
    if (bits > FLOAT_FROM_INT_BITS) {
        x = float_shru_sticky(x, bits - FLOAT_FROM_INT_BITS);
    } else {
        x <<= FLOAT_FROM_INT_BITS - bits;
    }
    //printf("shifted %#x\n", x);

    return __float_from_u26_impl(x, exponent);
}

unsigned __float_from_u64(const unsigned* x) {
    unsigned x0 = x[0];
    unsigned x1 = x[1];

    // Find the first set bit
    unsigned bits;
    if (x1 == 0u) {
        if (x0 == 0u) {
            return 0u;
        }
        bits = 32u - stdc_leading_zerosui(x0);
    } else {
        bits = 64u - stdc_leading_zerosui(x1);
    }
    //printf("x %lu bits %u\n",*(uint64_t*)x,bits);
    unsigned exponent = FLOAT_EXPONENT_BIAS + bits - 1u;

    // Shift bits around until exactly 26 bits are set
    unsigned m;
    if (bits <= FLOAT_FROM_INT_BITS) {
        // not enough bits, shift low word up
        m = x0 << (FLOAT_FROM_INT_BITS - bits);
    } else if (bits <= 32u) {
        // all bits in the low word, shift it down
        unsigned shift = bits - FLOAT_FROM_INT_BITS;
        m = x0 >> shift;
        m |= !!(x0 & ((1u << shift) - 1u)); // sticky bit
    } else if (bits == 32u + FLOAT_FROM_INT_BITS) {
        // exactly 26 bits in the high word
        m = x1 | !!x0; // sticky bit
    } else if (bits > 32u + FLOAT_FROM_INT_BITS) {
        // more than 26 bits in the high word, shift it down
        unsigned shift = bits - 32u - FLOAT_FROM_INT_BITS;
        m = x1 >> shift;
        m |= !!x0 | !!(x1 & ((1u << shift) - 1u)); // sticky bit
    } else {
        // less than 26 bits in the high word. we need bits from both words.
        unsigned shift = 32 + FLOAT_FROM_INT_BITS - bits;
        m = (x1 << shift) | (x0 >> (32 - shift));
        //printf("!!!!!!!!!!! high %#x low %#x shift %u m %#x sticky %u stickymask %u\n",x1,x0,shift,m,(x0 & ((1u << shift) - 1u)),((1u << (32 - shift)) - 1u));
        m |= !!(x0 & ((1u << (32 - shift)) - 1u)); // sticky bit
    }

    return __float_from_u26_impl(m, exponent);
}

unsigned __float_from_i32(int x) {

    // Handle INT_MIN specially
    if (x == INT_MIN) {
        return FLOAT_SIGN_BIT | __float_from_u32((unsigned)INT_MAX + 1);
    }

    // Non-negative numbers
    if (x >= 0) {
        return __float_from_u32((unsigned)x);
    }

    // For negative numbers, flip the sign and attach the sign bit
    return FLOAT_SIGN_BIT | __float_from_u32((unsigned)-x);

}

unsigned __float_from_i64(const unsigned* x) {

    // Handle INT64_MIN specially
    if (x[0] == 0 && x[1] == 0x8000000) {
        return FLOAT_SIGN_BIT | __float_from_u64(x);
    }

    // Non-negative numbers
    if (!(x[1] >> 31)) {
        return __float_from_u64(x);
    }

    // For negative numbers, flip the sign and attach the sign bit
    unsigned ux[2];
    return FLOAT_SIGN_BIT | __float_from_u64(__llong_negate(ux, x));

}
