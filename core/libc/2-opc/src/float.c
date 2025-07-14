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
    static void __fatal(const char* message) {
        fputs(message, stderr);
        fputc('\n', stderr);
        _Exit(1);
    }
    #define stdc_leading_zerosui __builtin_clz
#endif



#define FLOAT_BITS 32u

#define FLOAT_EXPONENT_BITS 8u
#define FLOAT_EXPONENT_SHIFT FLOAT_SIGNIFICAND_BITS
#define FLOAT_EXPONENT_MASK ((1u << FLOAT_EXPONENT_BITS) - 1u)
#define FLOAT_EXPONENT_BIAS ((1u << (FLOAT_EXPONENT_BITS - 1u)) - 1u)

#define FLOAT_SIGNIFICAND_BITS 23u
#define FLOAT_SIGNIFICAND_MASK ((1u << FLOAT_SIGNIFICAND_BITS) - 1u)
#define FLOAT_SIGNIFICAND_IMPLICIT_BIT (1u << FLOAT_SIGNIFICAND_BITS)

#define FLOAT_SIGN_SHIFT (FLOAT_SIGNIFICAND_BITS + FLOAT_EXPONENT_BITS)

#define FLOAT_SIGN_BIT (1u << FLOAT_SIGN_SHIFT)
#define FLOAT_HIDDEN_BIT (1u << FLOAT_SIGNIFICAND_BITS)
#define FLOAT_SIGNALING_BIT (1u << (FLOAT_SIGNIFICAND_BITS - 1u))

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

uint32_t __float_add(uint32_t a, uint32_t b) {
    //printf("----------------------------------------\n");
    //printf("a 0x%x\n", a);
    //printf("b 0x%x\n", b);

    // TODO use a different algorithm for subtraction. split this into
    // __float_add_impl() and __float_sub_impl().

    // get our exponents
    uint32_t ae = FLOAT_EXPONENT(a);
    uint32_t be = FLOAT_EXPONENT(b);
    //printf("ae %u\n", ae);
    //printf("be %u\n", be);

    // handle infinities and nans
    if (ae == FLOAT_EXPONENT_MASK || be == FLOAT_EXPONENT_MASK) {
        // TODO figure out what to do. if both are infinities of opposite sign,
        // what do? otherwise if one is infinity and other is not nan, return it?
        // if either is signaling, we signal and return signaling? return quiet
        // only if both are quiet?
        __fatal("TODO INFINITY OR NAN");
    }

    // unpack significand
    uint32_t as = FLOAT_SIGNIFICAND(a);
    uint32_t bs = FLOAT_SIGNIFICAND(b);
    //if (ae != 0u) as |= FLOAT_HIDDEN_BIT;
    //if (be != 0u) bs |= FLOAT_HIDDEN_BIT;
    //printf("as 0x%x\n", as);
    //printf("bs 0x%x\n", bs);

    // if not subnormal, add the hidden bit.
    // if subnormal, normalize the exponent to 1. (the exponent of subnormals
    // is the same as the lowest normal exponent, just without the hidden bit.)
    if (ae == 0u) {
        ae = 1u;
    } else {
        as |= FLOAT_HIDDEN_BIT;
    }
    if (be == 0u) {
        be = 1u;
    } else {
        bs |= FLOAT_HIDDEN_BIT;
    }

    // the target exponent for the calculation is the larger of the exponents
    uint32_t te = (ae > be ? ae : be);
    //printf("te 0x%x\n", te);

    // shift significands up to add guard, round and sticky bits (grs)
    as <<= 3u;
    bs <<= 3u;
    //printf("shifted for grs\n");
    //printf("as 0x%x\n", as);
    //printf("bs 0x%x\n", bs);

    // shift each significand down to match the target exponent, maintaining
    // the sticky bit
    as = __float_shru_sticky(as, te - ae);
    bs = __float_shru_sticky(bs, te - be);
    //printf("as 0x%x\n", as);
    //printf("bs 0x%x\n", bs);

    // add the significands
    uint32_t ts = as + bs;
    //printf("ts 0x%x\n", ts);

    // if our significand has grown larger than fits in this exponent, we need
    // to shift off another bit
    // Our max bits during calculation is significand bits plus the hidden
    // bit plus the three grs bits.
    //printf("max significand with grs: 0x%x\n", (1u << (FLOAT_SIGNIFICAND_BITS + 1u + 3u)));
    if (ts >= (1u << (FLOAT_SIGNIFICAND_BITS + 1u + 3u))) {
        //printf("significand has grown, shifting down\n");
        te += 1u;
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
    //printf("grs 0x%x %u%u%u\n", grs, grs>>2, !!(grs&2), grs&1);
    if (grs == 0b100u) {
        // round to even
        if (ts & 8u) {
            //printf("round up to even\n");
            ts += 8u;
        } else {
            //printf("round down to even\n");
        }
    } else if (grs >= 0b100u) {
        //printf("round up\n");
        ts += 8u;
    } else {
        //printf("round down\n");
    }
    //printf("ts after rounding 0x%x\n", ts);
    ts >>= 3u;
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
        te += 1u;
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
    // TODO sign bit, for now it's zero

    //printf("result 0x%x\n", (te << FLOAT_EXPONENT_SHIFT) | (ts & FLOAT_SIGNIFICAND_MASK));
    return (te << FLOAT_EXPONENT_SHIFT) | (ts & FLOAT_SIGNIFICAND_MASK);
}

uint32_t __float_sub(uint32_t a, uint32_t b) {
    // TODO this is definitely incorrect, need to split above into
    // __float_add_impl() and __float_sub_impl()
    b ^= FLOAT_SIGN_BIT;
    return __float_add(a, b);
}
