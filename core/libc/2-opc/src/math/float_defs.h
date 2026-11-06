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

uint32_t float_shru_sticky(uint32_t significand, uint32_t bits);
uint32_t float_round_grs(uint32_t tf);

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

// TODO we should move this to internal.h or something, this will need to be shared
#ifdef __onramp__
    #ifdef __onramp_cci_opc__
        int stdc_leading_zeros_ull(unsigned* v);
        #define stdc_leading_zeros_ull_p stdc_leading_zeros_ull
    #endif
    #ifndef __onramp_cci_opc__
        int stdc_leading_zeros_ull_p(unsigned* v) __asm__("stdc_leading_zeros_ull");
    #endif
#endif
#ifndef __onramp__
    #define stdc_leading_zeros_ull_p(x) stdc_leading_zeros_ull(*(long long*)x)
#endif

/*
 * funcs
 */
unsigned sqrtf(unsigned v);
unsigned cbrtf(unsigned v);
unsigned hypotf(unsigned x, unsigned y);
unsigned powf(unsigned base, unsigned exponent);
unsigned logf(unsigned v);
unsigned log1pf(unsigned v);
unsigned log2f(unsigned v);
unsigned log10f(unsigned v);
unsigned expf(unsigned v);
unsigned expm1f(unsigned v);
unsigned exp2f(unsigned v);
unsigned sinf(unsigned v);
unsigned cosf(unsigned v);
unsigned tanf(unsigned v);
unsigned asinf(unsigned v);
unsigned acosf(unsigned v);
unsigned atanf(unsigned v);
unsigned atan2f(unsigned x, unsigned y);
unsigned sinhf(unsigned v);
unsigned coshf(unsigned v);
unsigned tanhf(unsigned v);
unsigned asinhf(unsigned v);
unsigned acoshf(unsigned v);
unsigned atanhf(unsigned v);
unsigned erff(unsigned v);
unsigned erfcf(unsigned v);
unsigned tgammaf(unsigned v);
unsigned lgammaf(unsigned v);

/*
 * util
 */
unsigned ldexpf(unsigned v, int exp);
unsigned logbf(unsigned v);
unsigned frexpf(unsigned v, int* exp);
unsigned fabsf(unsigned v);
unsigned fmodf(unsigned dividend, unsigned divisor);
unsigned remainderf(unsigned dividend, unsigned divisor);
unsigned remquof(unsigned dividend, unsigned divisor,
        unsigned* partial_quotient);
unsigned fmaf(unsigned factor1, unsigned factor2, unsigned addend);
unsigned fmaxf(unsigned x, unsigned y);
unsigned fminf(unsigned x, unsigned y);
unsigned fdimf(unsigned x, unsigned y);
unsigned nanf(const char* __repr);
unsigned modff(unsigned v, unsigned* integer_part);
int ilogbf(unsigned v);
unsigned nextafterf(unsigned from, unsigned to);
unsigned nexttowardf(unsigned from, unsigned to);
unsigned ceilf(unsigned v);
unsigned floorf(unsigned v);
unsigned truncf(unsigned v);
unsigned roundf(unsigned v);
long lroundf(unsigned v);
void llroundf(unsigned* ll_out, unsigned v);
unsigned nearbyintf(unsigned v);
unsigned rintf(unsigned v);
long lrintf(unsigned v);
void llrintf(unsigned* ll_out, unsigned v);

#endif
