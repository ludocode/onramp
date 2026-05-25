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

#include <assert.h>   // TODO define NDEBUG when compiling final stages
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __onramp__
    #include <__onramp/__arithmetic.h>
    #include <stdbit.h>
    #include "internal.h"
            //#define printf(...)
#endif
#ifndef __onramp__
    // We support compiling this with an ordinary C compiler, this way we can
    // test against hardware floating point math among other things.
    #include <stdio.h>
    #include <stdlib.h>
    static void __attribute__((unused)) __fatal(const char* message) {
        fputs(message, stderr);
        fputc('\n', stderr);
        exit(1);
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
