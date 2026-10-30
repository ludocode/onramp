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

#include <assert.h>
#include <fenv.h>
#include <limits.h>

#ifndef ONRAMP_STANDALONE_TEST
#include <__onramp/__arithmetic.h>
#include <stdbit.h>
#endif

/**
 * Raise SIGFPE, called when performing arithmetic with a signaling NaN.
 */
void __float_sigfpe(void) {
    // TODO this is disabled.
    //raise(SIGFPE);
}

/**
 * Shifts the significand of a float right by the given number of bits,
 * preserving any shifted off bits in the sticky bit.
 */
uint32_t float_shru_sticky(uint32_t significand, uint32_t bits) {
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
uint32_t float_round_grs(uint32_t tf) {
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

unsigned ldexpf(unsigned x, int exp) {
    //printf("----------------------------------------\n");
    //printf("ldexpf\n");
    //printf("%.9g %#x exp %i\n", u2f(x), x, exp);

    if (exp == 0) {
        return x;
    }

    // Unpack the float
    uint32_t xs = x & FLOAT_SIGN_BIT;
    uint32_t xe = FLOAT_EXPONENT(x);
    uint32_t xm = FLOAT_SIGNIFICAND(x);
    //printf("float %#x xs %u xe %u xm %u exp %i\n",x,xs,xe,xm,exp);

    if (xe == FLOAT_EXPONENT_MASK) {
        // Infinity or NaN
        //printf("inf or nan\n");
        return x;
    }

    // If not subnormal, add the hidden bit.
    if (xe == 0u) {
        if (xm == 0) {
            // zero
            //printf("zero\n");
            return x;
        }
        xe = 1u;
    } else {
        xm |= FLOAT_HIDDEN_BIT;
    }

    // Adjust exponent
    if (exp > 0) {
        unsigned uexp = (unsigned)exp;

        // Shift up subnormal
        if (!(xm & FLOAT_HIDDEN_BIT)) {
            unsigned bits = 32u - stdc_leading_zeros_ui(xm);
            //printf("subnormal bits %u\n",bits);
            if (uexp > (FLOAT_SIGNIFICAND_BITS - bits)) {
                // Number becomes normal
                unsigned shift = FLOAT_SIGNIFICAND_BITS - bits + 1;
                //printf("number becomes normal, shift %u\n",shift);
                xm <<= shift;
                xe = 1 + uexp - shift;
            } else {
                // Number remains subnormal
                xm <<= uexp;
                xe = 0;
                //printf("number remains subnormal, xm %u\n", xm);
            }
        } else {
            // For normal numbers, just adjust exponent
            xe += uexp;
            if (xe >= FLOAT_EXPONENT_MASK) {
                feraiseexcept(FE_OVERFLOW);
                return xs | FLOAT_INFINITY;
            }
        }
    } else {
        unsigned uexp = (unsigned)-exp;
        if (xe > uexp) {
            //printf("%i uexp %u xe %u\n",__LINE__,uexp,xe);
            xe -= uexp;
        } else {
            //printf("subnormal xm %u\n",xm);
            // Subnormal.
            feraiseexcept(FE_UNDERFLOW);

            // Add guard, round and sticky bits
            // (TODO guard is not needed but we want to use float_round_grs().
            // We need a different helper for only rs().)
            xm <<= 3;
            //printf("raised xm %u\n",xm);

            // Shift down sticky
            //printf("exp %u xe %u exp-(xe-1) %u\n",exp,xe,(unsigned)exp - (xe - 1));
            xm = float_shru_sticky(xm, uexp - (xe - 1));
            //printf("shifted xm %u\n",xm);
            xe = 0;

            // Round
            xm = float_round_grs(xm);
            //printf("rounded xm %u\n",xm);
            if (xm >= FLOAT_HIDDEN_BIT) {
                // Rounding caused it to be normal again.
                //printf("rounded back to normal\n");
                xe = 1;
            }
        }
    }

    if (xe >= FLOAT_EXPONENT_MASK) {
        // infinity
        xe = FLOAT_EXPONENT_MASK;
        xm = 0;
    }

    // Assemble the float
    return xs
            | (xe << FLOAT_EXPONENT_SHIFT)
            | (xm & FLOAT_SIGNIFICAND_MASK);
}

static int logbf_impl(unsigned v) {
    uint32_t ve = FLOAT_EXPONENT(v);
    uint32_t vm = FLOAT_SIGNIFICAND(v);

    assert(ve != FLOAT_EXPONENT_MASK);

    // Normal numbers, just return the unbiased exponent
    if (ve > 0) {
        return (int)ve - (int)FLOAT_EXPONENT_BIAS;
    }

    // Subnormal. Count the zeroes.
    return (32 - FLOAT_SIGNIFICAND_BITS -(int)FLOAT_EXPONENT_BIAS) - stdc_leading_zeros_ui(vm);
}

unsigned logbf(unsigned v) {
    //printf("----------------------------------------\n");
    //printf("logbf\n");
    //printf("%.9g %#x\n", u2f(v), v);
    uint32_t ve = FLOAT_EXPONENT(v);
    uint32_t vm = FLOAT_SIGNIFICAND(v);

    // Zero is -inf
    if (ve == 0 && vm == 0) {
        feraiseexcept(FE_DIVBYZERO);
        return FLOAT_SIGN_BIT | FLOAT_INFINITY;
    }

    if (ve == FLOAT_EXPONENT_MASK) {
        if (vm == 0) {
            // +/-inf is +inf
            return FLOAT_INFINITY;
        }

        // NaN. Signal if it's signaling.
        if (!(vm & FLOAT_QUIET_BIT)) {
            __float_sigfpe();
        }

        // Return the NaN quieted.
        return v | FLOAT_QUIET_BIT;
    }

    return __float_from_i32(logbf_impl(v));
}

unsigned frexpf(unsigned v, int* e) {
    //printf("----------------------------------------\n");
    //printf("frexpf\n");
    //printf("%.9g %#x\n", u2f(v), v);

    // Unpack the float
    //uint32_t vs = v & FLOAT_SIGN_BIT;
    uint32_t ve = FLOAT_EXPONENT(v);
    uint32_t vm = FLOAT_SIGNIFICAND(v);

    // Zero is unchanged and the exponent is zero
    if (ve == 0 && vm == 0) {
        *e = 0;
        return v;
    }

    if (ve == FLOAT_EXPONENT_MASK) {
        if (vm == 0) {
            // +/-inf is unchanged.
            // The exponent is unspecified. We return 0 to match glibc.
            *e = 0;
            return v;
        }

        // NaN. Signal if it's signaling.
        if (!(vm & FLOAT_QUIET_BIT)) {
            __float_sigfpe();
        }

        // Return the NaN quieted.
        // The exponent is unspecified. We return 0 to match glibc.
        *e = 0;
        return v | FLOAT_QUIET_BIT;
    }

    if (0 == (v & ~FLOAT_SIGN_BIT)) {
        *e = 0;
    } else {
        *e = 1 + logbf_impl(v);
    }
    return ldexpf(v, -*e);
}
