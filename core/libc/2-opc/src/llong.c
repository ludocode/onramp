/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
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
 * Functions for 64-bit integer math.
 *
 * For all functions, the source pointers are allowed to match the destination
 * and each other, but they cannot overlap only partially.
 *
 * Most functions that take an `out` parameter return the same pointer
 * afterwards. This makes codegen easier in cci/2.
 *
 * We do pretty much all of the math using unsigned, even for signed
 * operations, because the overflow behaviour is well-specified and because we
 * only have unsigned operations in Onramp bytecode. (Some signed operations,
 * those that don't match their unsigned counterparts, are simulated in Onramp
 * so they have non-trivial overhead.)
 *
 * TODO: Most of this code was written in omC and originally compiled with
 * cci/0 so it avoids array defererencing, `else`, etc. We actually have cci/1
 * and opC available so this code should be cleaned up to look more like modern
 * C.
 */

#ifdef __onramp__
    #include <__onramp/__arithmetic.h>
#endif
#ifndef __onramp__
    // TODO for now we support compiling this with an ordinary C compiler, this
    // way we can test against a native 64-bit divide among other things
    unsigned* __llong_negate(unsigned* out, const unsigned* src);
    #include <stdbit.h>
    #define stdc_leading_zerosus __builtin_stdc_leading_zeros
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdbit.h>
#include <limits.h>
#include <assert.h>   // TODO define NDEBUG when compiling final stages

#ifdef __onramp__
    extern void __fatal(const char* message);
#endif
#ifndef __onramp__
    // workarounds for testing with another compiler
    #include <stdio.h>
    #include <stdlib.h>
    static void __fatal(const char* message) {
        fputs(message, stderr);
        fputc('\n', stderr);
        _Exit(1);
    }
#endif

unsigned* __llong_add(unsigned* out, const unsigned* a, const unsigned* b) {
    unsigned a0 = *a;
    unsigned a1 = *(a + 1);
    unsigned b0 = *b;
    unsigned b1 = *(b + 1);
    unsigned o0 = (a0 + b0);
    unsigned o1 = (a1 + b1);
    if (o0 < a0) {
        o1 = (o1 + 1);
    }
    *(out) = o0;
    *(out + 1) = o1;
    return out;
}

unsigned* __llong_sub(unsigned* out, const unsigned* a, const unsigned* b) {
    unsigned a0 = *a;
    unsigned a1 = *(a + 1);
    unsigned b0 = *b;
    unsigned b1 = *(b + 1);
    unsigned o0 = (a0 - b0);
    unsigned o1 = (a1 - b1);
    if (o0 > a0) {
        o1 = (o1 - 1);
    }
    *(out) = o0;
    *(out + 1) = o1;
    return out;
}

unsigned* __llong_mul(unsigned* out, const unsigned* a, const unsigned* b) {
    unsigned a0 = *a;
    unsigned a1 = *(a + 1);
    unsigned b0 = *b;
    unsigned b1 = *(b + 1);

    // long multiplication base 2^16

    unsigned ah0;
    unsigned ah1;
    unsigned ah2;
    unsigned ah3;
    ah0 = (a0 & 0xffff);
    ah1 = ((a0 >> 16) & 0xffff);
    ah2 = (a1 & 0xffff);
    ah3 = ((a1 >> 16) & 0xffff);

    unsigned bh0;
    unsigned bh1;
    unsigned bh2;
    unsigned bh3;
    bh0 = (b0 & 0xffff);
    bh1 = ((b0 >> 16) & 0xffff);
    bh2 = (b1 & 0xffff);
    bh3 = ((b1 >> 16) & 0xffff);

    unsigned t00 = (ah0 * bh0);
    unsigned t01 = (ah0 * bh1);
    unsigned t02 = (ah0 * bh2);
    unsigned t03 = (ah0 * bh3);
    unsigned t10 = (ah1 * bh0);
    unsigned t11 = (ah1 * bh1);
    unsigned t12 = (ah1 * bh2);
    unsigned t20 = (ah2 * bh0);
    unsigned t21 = (ah2 * bh1);
    unsigned t30 = (ah3 * bh0);

    unsigned o0 = t00;
    unsigned o1 = 0;

    unsigned temp = (o0 + (t01 << 16));
    if (temp < o0) {
        o1 = (o1 + 1);
    }
    o0 = temp;
    o1 = (o1 + (t01 >> 16));

    temp = (o0 + (t10 << 16));
    if (temp < o0) {
        o1 = (o1 + 1);
    }
    o0 = temp;
    o1 = (o1 + (t10 >> 16));

    o1 = (o1 + t20);
    o1 = (o1 + t11);
    o1 = (o1 + t02);

    o1 = (o1 + (t03 << 16));
    o1 = (o1 + (t12 << 16));
    o1 = (o1 + (t21 << 16));
    o1 = (o1 + (t30 << 16));

    *(out) = o0;
    *(out + 1) = o1;
    return out;
}

// Returns the length in 16-bit digits of the given non-zero 64-bit number.
static unsigned divmodu_length(unsigned short* number) {
    if (number[3] != 0)
        return 4;
    if (number[2] != 0)
        return 3;
    if (number[1] != 0)
        return 2;
    return 1;
}

void __llong_divmodu(unsigned* quotient_out, unsigned* /*nullable*/ remainder_out,
        const unsigned* dividend, const unsigned* divisor)
{
    // special cases
    if (dividend[1] == 0) {
//printf("SPECIAL\n");
        if (divisor[1] != 0) {
            // divisor is larger than dividend
            quotient_out[0] = 0;
            quotient_out[1] = 0;
            if (remainder_out) {
                remainder_out[0] = dividend[0];
                remainder_out[1] = dividend[1];
            }
        } else {
            // both numbers are 32 bits
            quotient_out[0] = dividend[0] / divisor[0];
            quotient_out[1] = 0;
            if (remainder_out) {
                remainder_out[0] = dividend[0] % divisor[0];
                remainder_out[1] = 0;
            }
        }
        return;
    }
    if (divisor[0] == 0 && divisor[1] == 0) {
        // TODO handle this appropriately, for now we just abort
        __fatal("__llong_divmodu(): Division by zero.");
    }

    // Zero out the output now. It is easier than trying to clear the parts we
    // don't fill later.
    quotient_out[0] = 0;
    quotient_out[1] = 0;
    if (remainder_out) {
        remainder_out[0] = 0;
        remainder_out[1] = 0;
    }


    // This is Algorithm D, "Division of nonnegative integers", by Knuth.
    //
    // References:
    //
    // - The Art of Computer Programming Volume 2, 3rd Edition by Donald E. Knuth, section 4.3.1
    //
    // - Hacker's Delight, 2nd Edition by Henry S. Warren, section 9-2: Multiword Divison
    //
    // - Labor of Division (Episode IV): Algorithm D by fish
    //          https://ridiculousfish.com/blog/posts/labor-of-division-episode-iv.html
    //
    // Knuth Algorithm D is an algorithm for performing long division while
    // efficiently estimating the quotient in each step, allowing us to perform
    // the calculation with only a machine divide instruction.
    //
    // The hard part of long division is that if our divisor has n digits, then
    // each step requires division of n+1 digits by n digits. In other words,
    // we're always dividing by the full size of the divisor. With Algorithm D,
    // we can instead do a much smaller division to get a good estimate of the
    // quotient and then quickly correct it to get the right answer at each
    // step.
    //
    // I've tried commenting the hard parts below to make sure I understand it
    // and to maybe even understand it again later.
    // TODO I also left in a bunch of debug code in case I need to fix this
    // later. This needs to be cleaned up.
    //
    // This code does not (yet) use the additional optimizations from fish in
    // the link above.

    // In order to apply this to 64-bit division, we need to choose an
    // appropriate base, i.e. digit size. We want the digit size to be as large
    // as possible while still being able to divide a two digit number by a one
    // digit number in hardware.
    //
    // Since the Onramp virtual machine has a 32-bit division instruction
    // (divu), we can do the calculation with a base of 16 bits. In other words
    // our base is 65536 and our digit size is unsigned short. When we divide a
    // two digit number by a one digit number, we'll be dividing 32 bits by 16
    // bits with our machine instruction.
    const unsigned base = (USHRT_MAX + 1);
    unsigned short* dividend_digits = (unsigned short*)dividend;
    unsigned short* divisor_digits = (unsigned short*)divisor;
    unsigned short* quotient_digits = (unsigned short*)quotient_out;

    // We need at least two digits in our divisor to use Algorithm D. If we
    // have only a one digit divisor, that means each step of long division
    // will have at most a two digit dividend, so we don't need to do any
    // estimation: we can use our machine division instruction directly in each
    // step. This is a straightforward long division.
    unsigned divisor_length = divmodu_length(divisor_digits);
    if (divisor_length == 1) {
//printf("SMALL\n");
        unsigned small_divisor = divisor[0];
        assert(small_divisor < base);
        unsigned step_quotient = 0;
        for (size_t i = divmodu_length(dividend_digits); i-- > 0;) {
            step_quotient = (step_quotient << 16) | dividend_digits[i];
            quotient_digits[i] = step_quotient / small_divisor;
            /*
            printf("step %u: divided %u by %u, result %u, mul %u, remainder %u\n",
                    i,
                    step_quotient, small_divisor, quotient_digits[i],
                    quotient_digits[i] * small_divisor, step_quotient - quotient_digits[i] * small_divisor);
                    */
            step_quotient -= quotient_digits[i] * small_divisor;
            assert(step_quotient < small_divisor);
        }
        if (remainder_out) {
            remainder_out[0] = step_quotient;
        }
        /*
        printf("%u %u %u %u -> %llu\n",quotient_digits[3],quotient_digits[2],quotient_digits[1],quotient_digits[0],*(unsigned long long*)quotient_out);
        */
        return;
    }
//printf("BIG\n");

    // We now do the normalization.
    //
    // Knuth proved that we can easily estimate each step of long division if
    // the top digit of the divisor is at least half the base. In other words,
    // its high bit must be set. (This makes intuitive sense: the larger the
    // divisor, the smaller the quotient, so the more accurate our estimate
    // will be.)
    //
    // We can force this condition to be true by multiplying both the dividend
    // and divisor by a constant. The constant cancels out in the division so
    // the resulting quotient is the same, and we can divide the remainder by
    // the constant at the end.
    //
    // We normalize simply by shifting both numbers up until the high bit of
    // the divisor is set. Note that this adds a digit to our dividend.

    unsigned short normalized_dividend[5];
    unsigned short normalized_divisor[4];
    unsigned shift = stdc_leading_zerosus(divisor_digits[divisor_length - 1]);
//printf("shift %u\n", shift);

    normalized_dividend[0] = dividend_digits[0] << shift;
    normalized_divisor[0] = divisor_digits[0] << shift;
    for (size_t i = 1; i < 4; ++i) {
        normalized_divisor[i] = (unsigned short)((divisor_digits[i] << shift) | (divisor_digits[i - 1] >> (16 - shift)));
        normalized_dividend[i] = (unsigned short)((dividend_digits[i] << shift) | (dividend_digits[i - 1] >> (16 - shift)));
    }
    normalized_dividend[4] = dividend_digits[3] >> (16 - shift);

    // Cache the top two digits of the divisor. We'll be using them a lot.
    unsigned divisor_first = normalized_divisor[divisor_length - 1];
    unsigned divisor_second = normalized_divisor[divisor_length - 2];

    // Now we do our long division.
    // `pos` points to the normalized dividend at the least significant digit of the current step.
    for (size_t pos = 5 - divisor_length; pos-- > 0;) {
//printf("normalized dividend %u %u %u %u %u\n", normalized_dividend[0],normalized_dividend[1],normalized_dividend[2],normalized_dividend[3],normalized_dividend[4]);
//printf("normalized divisor %u %u %u %u\n", normalized_divisor[0],normalized_divisor[1],normalized_divisor[2],normalized_divisor[3]);

        // Estimate: divide the top two digits of the divisor by the top digit
        // of the dividend.
        unsigned dividend_step = (normalized_dividend[pos + divisor_length] << 16) |
                normalized_dividend[pos + divisor_length - 1];
        unsigned estimate = dividend_step / divisor_first;
        unsigned estimate_remainder = dividend_step - estimate * divisor_first;
//printf("POS %zu dividend %u divisor %u ESTIMATE %u\n",pos,dividend_step,divisor_first,estimate);

        // Check if our estimate is too high. This is the magic part. It's
        // explained pretty well in fish's post linked above.
        //
        // Intuitively, the idea is:
        //
        // - We introduced some error in truncating our divisor to the first
        //   digit. We recover most of it by multiplying the estimate by the
        //   second digit of the divisor.
        //
        // - We introduced some error in the remainder of the estimated
        //   division by truncating the dividend to the first two digits. We
        //   recover most of it by adding the third digit back to the
        //   remainder.
        //
        // We can then compare these values. If the amount we truncated from
        // the divisor is greater than the amount we truncated from the
        // dividend, then we estimated too high, so we reduce by 1 and try again.
        //
        // Of course the algorithm is not based on intuition but on rigorous
        // math. Knuth proved that this test will never have a false positive,
        // the loop body happens at most twice, and our estimate won't be off
        // by more than 1 after this. This means we'll only have to do a long
        // multiplication and subtraction once, potentially with a final
        // corrective addition afterwards.
        while (estimate == base || estimate * divisor_second >
                (estimate_remainder << 16) + normalized_dividend[pos + divisor_length - 2])
        {
//printf("OVERESTIMATE  estimate %u second %u estimate*second %u estimate_remainder %u norm_div %u sum %u\n", estimate, divisor_second, estimate*divisor_second, estimate_remainder, normalized_dividend[pos + divisor_length - 1], (estimate_remainder << 16) + normalized_dividend[pos + divisor_length - 1]);
            --estimate;
            estimate_remainder += divisor_first;
            if (estimate_remainder >= USHRT_MAX)
                break;
        }

        // We now multiply the full divisor by our estimate, subtracting the
        // result from the dividend in-place. This leaves the remainder in the
        // dividend.
        int carry = 0;
        for (size_t i = 0; i < divisor_length; ++i) {
            unsigned product = estimate * normalized_divisor[i];
            int remainder = normalized_dividend[pos + i] - (product & 0xffff) - carry;
//printf("digit %zi val %u product %u remainder %u last carry %i  product>>16 %u remainder>>16 %i\n",i,normalized_divisor[i],product,remainder,carry,product>>16,remainder>>16);
            normalized_dividend[pos + i] = (unsigned short)remainder;
            carry = (product >> 16) - (remainder >> 16);
        }
        unsigned last_step = normalized_dividend[pos + divisor_length] - carry;
//printf("last digit val %u carry %i last step %u\n", normalized_dividend[pos + divisor_length], carry, last_step);
        normalized_dividend[pos + divisor_length] = last_step;

        // We now do the final check: if our estimate was still one too high,
        // our remainder will be negative.
        if (last_step < 0x80000000u) {
//printf("GOOD\n");
            // Remainder is positive; the estimate is correct. Store it.
            quotient_digits[pos] = estimate;
        } else {
            // Remainder is negative; the actual quotient is one less than the
            // estimate. We subtracted too much so we need to add one times
            // the divisor back again. It's a simple long addition. This case
            // is apparently pretty rare.
//printf("BAD SUB\n");
            quotient_digits[pos] = estimate - 1;
            carry = 0;
            for (size_t i = 0; i < divisor_length; ++i) {
                unsigned step = normalized_dividend[pos + i] + normalized_divisor[i] + carry;
                normalized_dividend[pos + i] = (unsigned short)step;
                carry = step >> 16;
            }
            normalized_dividend[pos + divisor_length] += carry;
        }
    }

//printf("quotient %u %u %u %u\n", quotient_digits[0],quotient_digits[1],quotient_digits[2],quotient_digits[3]);
//printf("norm remainder %u %u %u %u\n", normalized_dividend[0],normalized_dividend[1],normalized_dividend[2],normalized_dividend[3]);

    // The final remainder has been left in our dividend. It's been normalized
    // by shifting up, so we just shift it down again.
    if (remainder_out) {
        unsigned short* remainder_digits = (unsigned short*)remainder_out;
        for (size_t i = 0; i < divisor_length - 1; ++i)
            remainder_digits[i] = (unsigned short)((normalized_dividend[i] >> shift) | (normalized_dividend[i + 1] << (16 - shift)));
        remainder_digits[divisor_length - 1] = (unsigned short)(normalized_dividend[divisor_length - 1] >> shift);
//printf("denorm remainder %u %u %u %u\n", remainder_digits[0],remainder_digits[1],remainder_digits[2],remainder_digits[3]);
    }
}

unsigned* __llong_divu(unsigned* quotient_out, const unsigned* dividend, const unsigned* divisor) {
    __llong_divmodu(quotient_out, NULL, dividend, divisor);
    return quotient_out;
}

unsigned* __llong_modu(unsigned* remainder_out, const unsigned* dividend, const unsigned* divisor) {
    unsigned quotient[2];
    __llong_divmodu(quotient, remainder_out, dividend, divisor);
    return remainder_out;
}

unsigned* __llong_divs(unsigned* quotient_out, const unsigned* dividend, const unsigned* divisor) {
    bool dividend_negative = dividend[1] >> 31;
    bool divisor_negative = divisor[1] >> 31;
    if (!dividend_negative && !divisor_negative) {
        return __llong_divu(quotient_out, dividend, divisor);
    }

    unsigned dividend_abs[2];
    if (dividend_negative) {
        __llong_negate(dividend_abs, dividend);
    } else {
        dividend_abs[0] = dividend[0];
        dividend_abs[1] = dividend[1];
    }

    unsigned divisor_abs[2];
    if (divisor_negative) {
        __llong_negate(divisor_abs, divisor);
    } else {
        divisor_abs[0] = divisor[0];
        divisor_abs[1] = divisor[1];
    }

    __llong_divu(quotient_out, dividend_abs, divisor_abs);

    bool result_negative = dividend_negative ^ divisor_negative;
    if (result_negative) {
        __llong_negate(quotient_out, quotient_out);
    }

    return quotient_out;
}

unsigned* __llong_mods(unsigned* quotient_out, const unsigned* dividend, const unsigned* divisor) {
    bool dividend_negative = dividend[1] >> 31;
    bool divisor_negative = divisor[1] >> 31;
    if (!dividend_negative && !divisor_negative) {
        return __llong_modu(quotient_out, dividend, divisor);
    }

    unsigned dividend_abs[2];
    if (dividend_negative) {
        __llong_negate(dividend_abs, dividend);
    } else {
        dividend_abs[0] = dividend[0];
        dividend_abs[1] = dividend[1];
    }

    unsigned divisor_abs[2];
    if (divisor_negative) {
        __llong_negate(divisor_abs, divisor);
    } else {
        divisor_abs[0] = divisor[0];
        divisor_abs[1] = divisor[1];
    }

    __llong_modu(quotient_out, dividend_abs, divisor_abs);

    bool result_negative = dividend_negative;
    if (result_negative) {
        __llong_negate(quotient_out, quotient_out);
    }

    return quotient_out;
}

bool __llong_ltu(const unsigned* a, const unsigned* b) {
    unsigned a1 = *(a + 1);
    unsigned b1 = *(b + 1);
    if (a1 < b1) {
        return true;
    }
    if (a1 != b1) {
        return false;
    }
    return *a < *b;
}

bool __llong_lts(const unsigned* a, const unsigned* b) {
    int a1 = (int)*(a + 1);
    int b1 = (int)*(b + 1);
    if (a1 < b1) {
        return true;
    }
    if (a1 != b1) {
        return false;
    }
    return *a < *b;
}

unsigned* __llong_shl(unsigned* out, const unsigned* a, int bits) {
    if (bits >= 32) {
        if (bits == 32) {
            *(out + 1) = *a;
            *out = 0;
            return out;
        }

        *(out + 1) = (*a << (bits - 32));
        *out = 0;
        return out;
    }

    unsigned a0 = *a;
    unsigned a1 = *(a + 1);

    if (bits == 0) {
        *out = a0;
        *(out + 1) = a1;
        return out;
    }

    *out = (a0 << bits);
    *(out + 1) = ((a1 << bits) | (a0 >> (32 - bits)));
    return out;
}

unsigned* __llong_shru(unsigned* out, const unsigned* a, int bits) {
    if (bits >= 32) {
        if (bits == 32) {
            *(out + 1) = 0;
            *out = *(a + 1);
            return out;
        }

        *(out + 1) = 0;
        *out = (*(a + 1) >> (bits - 32));
        return out;
    }

    unsigned a0 = *a;
    unsigned a1 = *(a + 1);

    if (bits == 0) {
        *out = a0;
        *(out + 1) = a1;
        return out;
    }

    *(out + 1) = (a1 >> bits);
    *out = ((a1 << (32 - bits)) | (a0 >> bits));
    return out;
}

unsigned* __llong_shrs(unsigned* out, const unsigned* a, int bits) {
    unsigned a0 = *a;
    unsigned a1 = *(a + 1);

    if (bits < 32) {
        if (bits == 0) {
            *out = a0;
            *(out + 1) = a1;
            return out;
        }

        *(out + 1) = ((signed)a1 >> bits);
        *out = ((a1 << (32 - bits)) | (a0 >> bits));
        return out;
    }

    unsigned sign = -(a1 >> 31);

    if (bits == 32) {
        *(out + 1) = sign;
        *out = a1;
        return out;
    }

    *(out + 1) = sign;
    *out = ((signed)a1 >> (bits - 32));
    return out;
}

unsigned* __llong_bit_and(unsigned* out, const unsigned* a, const unsigned* b) {
    *out = (*a & *b);
    *(out + 1) = (*(a + 1) & *(b + 1));
    return out;
}

unsigned* __llong_bit_or(unsigned* out, const unsigned* a, const unsigned* b) {
    *out = (*a | *b);
    *(out + 1) = (*(a + 1) | *(b + 1));
    return out;
}

unsigned* __llong_bit_xor(unsigned* out, const unsigned* a, const unsigned* b) {
    *out = (*a ^ *b);
    *(out + 1) = (*(a + 1) ^ *(b + 1));
    return out;
}

unsigned* __llong_bit_not(unsigned* out, const unsigned* src) {
    *out = ~*src;
    *(out + 1) = ~*(src + 1);
    return out;
}

unsigned* __llong_negate(unsigned* out, const unsigned* src) {
    out[0] = 0;
    out[1] = 0;
    return __llong_sub(out, out, src);
}
