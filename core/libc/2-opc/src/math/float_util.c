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
#include <__onramp/__fatal.h>
#include <stdbit.h>
#endif

ufloat_error_t __ufloat_unpack(ufloat_t* uf, uint32_t f) {
    uint32_t exponent = ((f >> FLOAT_EXPONENT_SHIFT) & FLOAT_EXPONENT_MASK);
    uint32_t mantissa = (f & FLOAT_SIGNIFICAND_MASK);

    if (exponent == FLOAT_EXPONENT_MASK) {
        // infinity or nan
        return mantissa ? ufloat_error_nan : ufloat_error_infinity;
    }

    if (exponent == 0) {
        if (mantissa == 0) {
            return ufloat_error_zero;
        }
        // subnormal
        unsigned shift = (unsigned)stdc_leading_zeros_ui(mantissa);
        //printf("unpack subnormal %#x %#x %#x\n", mantissa, shift, mantissa<<shift);
        mantissa <<= shift;
        exponent += (FLOAT_BITS - FLOAT_SIGNIFICAND_BITS) - shift;
    } else {
        // normal
        //printf("unpack exponent %u\n", exponent);
        mantissa = (mantissa | FLOAT_HIDDEN_BIT) << UFLOAT_MANTISSA_SHIFT;
    }

    uf->mantissa = mantissa;
    uf->exponent = exponent + (UFLOAT_EXPONENT_BIAS - FLOAT_EXPONENT_BIAS);
    //printf("unpacked %#x to e %#x m %#x\n", f, uf->exponent, uf->mantissa);
    return ufloat_ok;
}

uint32_t __ufloat_pack(const ufloat_t* uf, ufloat_error_t error, uint32_t sign_bit) {
    switch (error) {
        case ufloat_ok: break;
        case ufloat_error_zero: return sign_bit;
        case ufloat_error_infinity: return sign_bit | FLOAT_INFINITY;
        case ufloat_error_nan: return sign_bit | FLOAT_QUIET_NAN;
        default: // TODO unreachable
    }

    uint32_t exponent = uf->exponent;
    uint32_t mantissa = uf->mantissa;
    assert(mantissa >> (FLOAT_BITS - 1u)); // number must always be normalized

    // Check for subnormals. They are rounded differently.
    if (exponent <= UFLOAT_EXPONENT_BIAS - FLOAT_EXPONENT_BIAS) {

        // Subnormal. Calculate bits to shift
        uint32_t shift = (UFLOAT_MANTISSA_SHIFT - GRS_BITS)
                + (UFLOAT_EXPONENT_BIAS - FLOAT_EXPONENT_BIAS)
                - exponent + 1u;
        if (shift > (FLOAT_BITS - GRS_BITS)) {
            // TODO raise underflow exception
            return sign_bit; // underflow; return zero
        }
        //printf("%s() subnormal exponent %#x shift bits %u\n", __func__, exponent, shift);

        // Shift sticky
        uint32_t mask = (1u << shift) - 1u;
        uint32_t sticky = !!(mantissa & mask);
        //printf("shifted: 0x%x | %u\n",(mantissa >> shift), 0|sticky);
        //printf("before shift %#x\n", mantissa);
        mantissa = (mantissa >> shift) | sticky;
        //printf("after shift %#x\n", mantissa);

        // Round
        mantissa = float_round_grs(mantissa);
        //printf("result after first round %#x\n", mantissa);
        if (mantissa == 0) {
            // TODO raise underflow exception
            return sign_bit; // zero
        }

        // Rounding may have caused us to become normal. Exponent is 0 if we're
        // still subnormal, 1 if we're not.
        exponent = !!(mantissa & (1u << FLOAT_SIGNIFICAND_BITS));

    } else {

        // Normal. The amount to shift is fixed
        uint32_t shift = UFLOAT_MANTISSA_SHIFT - GRS_BITS;

        // Shift sticky
        uint32_t mask = (1u << shift) - 1u;
        uint32_t sticky = !!(mantissa & mask);
        //printf("shifted: 0x%x | %u\n",(mantissa >> shift), 0|sticky);
        //printf("before shift %#x\n", mantissa);
        mantissa = (mantissa >> shift) | sticky;
        //printf("after shift %#x\n", mantissa);

        // Round
        mantissa = float_round_grs(mantissa);
        //printf("result after first round %#x\n", mantissa);

        // Rounding may have caused us to grow.
        if (mantissa & (1u << (FLOAT_SIGNIFICAND_BITS + 1u))) {
            //printf("  significand %#x too large after rounding. shifting down again\n", mantissa);
            // don't need to check for overflow here, we subtract a fixed
            // amount before checking just below this
            ++exponent;
            mantissa >>= 1u;
        }

        // Lastly, convert exponent
        exponent -= UFLOAT_EXPONENT_BIAS - FLOAT_EXPONENT_BIAS;
        //printf("exponent %u\n", exponent);
        if (exponent >= FLOAT_EXPONENT_MASK) {
            // TODO raise overflow exception
            return sign_bit | FLOAT_INFINITY;
        }
    }

    //printf("returning %#x\n", sign_bit | (exponent << FLOAT_EXPONENT_SHIFT) | (mantissa & FLOAT_SIGNIFICAND_MASK));
    return sign_bit
        | (exponent << FLOAT_EXPONENT_SHIFT)
        | (mantissa & FLOAT_SIGNIFICAND_MASK);
}

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
    return (int)FLOAT_BITS - (int)FLOAT_SIGNIFICAND_BITS
            - (int)FLOAT_EXPONENT_BIAS - stdc_leading_zeros_ui(vm);
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

unsigned fabsf(unsigned v) {
    // TODO handle nan properly
    return v & ~FLOAT_SIGN_BIT;
}

unsigned fmodf(unsigned dividend, unsigned divisor) {
    (void)dividend;
    (void)divisor;
    __fatal("TODO fmodf()");
}

unsigned remainderf(unsigned dividend, unsigned divisor) {
    (void)dividend;
    (void)divisor;
    __fatal("TODO fmodf()");
}

unsigned remquof(unsigned dividend, unsigned divisor,
        unsigned* partial_quotient)
{
    (void)dividend;
    (void)divisor;
    (void)partial_quotient;
    __fatal("TODO fmodf()");
}

unsigned fmaf(unsigned factor1, unsigned factor2, unsigned addend) {
    // TODO should be done in full precision
    return __float_add(__float_mul(factor1, factor2), addend);
}

unsigned fmaxf(unsigned x, unsigned y) {
    // TODO handle nan
    return ((int)x >= (int)y) ? x : y;
}

unsigned fminf(unsigned x, unsigned y) {
    // TODO handle nan
    return ((int)x <= (int)y) ? x : y;
}

unsigned fdimf(unsigned x, unsigned y) {
    (void)x;
    (void)y;
    __fatal("TODO fdimf()");
}

unsigned nanf(const char* __repr) {
    // TODO we should parse repr and stuff it in the low bits
    (void)__repr;
    return FLOAT_QUIET_NAN;
}

unsigned modff(unsigned v, unsigned* integer_part) {
    (void)v;
    (void)integer_part;
    __fatal("TODO modff()");
}

int ilogbf(unsigned v) {
    (void)v;
    __fatal("TODO ilogbf()");
}

unsigned nextafterf(unsigned from, unsigned to) {
    (void)from;
    (void)to;
    __fatal("TODO nextafterf()");
}

unsigned nexttowardf(unsigned from, unsigned to) {
    (void)from;
    (void)to;
    __fatal("TODO nexttowardf()");
}

unsigned ceilf(unsigned v) {
    (void)v;
    __fatal("TODO ceilf()");
}

unsigned floorf(unsigned v) {
    (void)v;
    __fatal("TODO floorf()");
}

unsigned truncf(unsigned v) {
    (void)v;
    __fatal("TODO truncf()");
}

unsigned roundf(unsigned v) {
    (void)v;
    __fatal("TODO roundf()");
}

long lroundf(unsigned v) {
    (void)v;
    __fatal("TODO lroundf()");
}

void llroundf(unsigned* ll_out, unsigned v) {
    (void)ll_out;
    (void)v;
    __fatal("TODO llroundf()");
}

unsigned nearbyintf(unsigned v) {
    (void)v;
    __fatal("TODO nearbyintf()");
}

unsigned rintf(unsigned v) {
    (void)v;
    __fatal("TODO rintf()");
}

long lrintf(unsigned v) {
    (void)v;
    __fatal("TODO lrintf()");
}

void llrintf(unsigned* ll_out, unsigned v) {
    (void)ll_out;
    (void)v;
    __fatal("TODO llrintf()");
}
