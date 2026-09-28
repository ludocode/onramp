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

#ifndef BIGINT_H_INCLUDED
#define BIGINT_H_INCLUDED

/**
 * Arbitrary precision integer arithmetic.
 *
 * All arbitrary-precision integers are little-endian arrays of `n` 32-bit
 * words. The parameter `n` is followed by a pointer to the array of words.
 * Output parameters appear first, so the parameter `n` is the first parameter
 * to almost all functions. `n` must be non-zero.
 *
 * For those functions that take multiple bigints, unless marked restrict, the
 * pointers may match (but may not partially overlap.)
 */

#include <stddef.h>
#include <stdint.h>

/*
 * For now I'm only implementing the operations required for floating point
 * printing as described in "How to Print Floating-Point Numbers Accurately" by
 * Steele and White (1990).
 *
 * The operations are:
 *
 * - assignment
 * - shift up
 * - shift down (unsigned)
 * - addition
 * - subtraction
 * - multiplication by a small integer
 * - division (unsigned) where the result is a small integer
 *
 * (For the purpose of float printing in decimal, multiplication is always by
 * 10, and division always results in a number less than 10. This bigint
 * implementation allows "small" to be a uint16_t.)
 *
 * This could eventually be expanded to replace much of llong.c and to
 * implement 128-bit arithmetic (int128_t.)
 */

void __bigint_clear(const uint32_t n, uint32_t* x);

void __bigint_set(const uint32_t n, uint32_t* dest, const uint32_t* src);

void __bigint_set_u32(const uint32_t n, uint32_t* dest, const uint32_t src);

void __bigint_set_u64(const uint32_t n, uint32_t* dest, const uint32_t* src);

void __bigint_add(const uint32_t n, uint32_t* dest, const uint32_t* left, const uint32_t* right);

void __bigint_sub(const uint32_t n, uint32_t* dest, const uint32_t* left, const uint32_t* right);

/**
 * Shift left (up).
 *
 * There is no restriction on the number of bits. If the number of bits given
 * is not less than the number of bits in the bigint, the result is zero.
 */
void __bigint_shl(const uint32_t n, uint32_t* dest, const uint32_t* src, uint32_t bits);

/**
 * Shift right (down) unsigned.
 *
 * There is no restriction on the number of bits. If the number of bits given
 * is not less than the number of bits in the bigint, the result is zero.
 */
void __bigint_shru(const uint32_t n, uint32_t* dest, const uint32_t* src, uint32_t bits);

/**
 * Multiplies a big integer by a 16-bit number.
 */
void __bigint_mul_u16(const uint32_t n, uint32_t* dest, const uint32_t* left, uint16_t right);

/**
 * Performs an unsigned division on two numbers where the quotient is known to
 * fit in a 16-bit integer, returning it.
 */
uint16_t __bigint_div_to_u16(const uint32_t n, const uint32_t* dividend, const uint32_t* divisor);

/**
 * Prints a big integer in hexadecimal with a "0x" prefix.
 *
 * The buffer must be at least 3+8*n bytes.
 */
void __bigint_print_hex(uint32_t n, const uint32_t* x, char* buffer, size_t size);

#endif
