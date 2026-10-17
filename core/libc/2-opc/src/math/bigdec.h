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

#ifndef BIGDEC_H_INCLUDED
#define BIGDEC_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

void __bigdec_print(bigdec_t* bigdec, char* out, size_t outsize);

void __bigdec_trim_leading_zeroes(bigdec_t* bigdec);

void __bigdec_trim_trailing_zeroes(bigdec_t* bigdec);

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
void __bigdec_mul_pow2(bigdec_t* bigdec, int pow2);

/**
 * Get the integer portion of a big decimal.
 *
 * Any remaining fraction is truncated. If the number is too large, it will
 * overflow.
 */
uint32_t __bigdec_uint(bigdec_t* bigdec, bool round);

/**
 * Compare a big decimal to a uint32_t.
 *
 * Returns -1 if the big decimal is smaller, +1 if it's larger, and 0 if they
 * are equal.
 */
int __bigdec_cmp_u32(bigdec_t* bigdec, uint32_t value);

#endif
