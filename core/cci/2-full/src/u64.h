/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Fraser Heavy Software
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

#ifndef LLONG_H_INCLUDED
#define LLONG_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * During bootstrapping we don't have `long long` or `double`. We need to
 * represent them as two `unsigned`. We do our 64-bit math by manually calling
 * out to the `__llong_*()` and `__double_*()` functions in libc/2.
 *
 * When the final stage is re-compiling itself, or when compiling with an
 * native C compiler, we use normal `long long` and `double` math. (This is
 * necessary because, when compiling with a native C compiler, we don't have
 * our libc functions.)
 */
#ifdef __onramp_cci_full__
    #define ONRAMP_U64_NATIVE
#endif
#ifndef __onramp__
    #define ONRAMP_U64_NATIVE
#endif

/**
 * A signed or unsigned 64-bit value used to implement `long long`.
 */
typedef struct u64_t {
    #ifdef ONRAMP_U64_NATIVE
    unsigned long long value;
    #endif
    #ifndef ONRAMP_U64_NATIVE
    unsigned words[2];
    #endif
} u64_t;

// These functions modify the argument in place.
void llong_clear(u64_t* llong);
void llong_set(u64_t* llong, const u64_t* other);
void llong_set_u(u64_t* llong, uint32_t other);
void llong_add(u64_t* llong, const u64_t* other);
void llong_add_u(u64_t* llong, unsigned other);
void llong_sub(u64_t* llong, const u64_t* other);
void llong_mul(u64_t* llong, const u64_t* other);
void llong_mul_u(u64_t* llong, unsigned other);
void llong_divu(u64_t* llong, const u64_t* other);
void llong_divs(u64_t* llong, const u64_t* other);
void llong_modu(u64_t* llong, const u64_t* other);
void llong_mods(u64_t* llong, const u64_t* other);
void llong_shl(u64_t* llong, int bits);
void llong_shru(u64_t* llong, int bits);
void llong_shrs(u64_t* llong, int bits);
void llong_and(u64_t* llong, const u64_t* other);
void llong_or(u64_t* llong, const u64_t* other);
void llong_xor(u64_t* llong, const u64_t* other);
void llong_bit_not(u64_t* llong);
void llong_negate(u64_t* llong);

// These functions do not modify the argument.
uint32_t llong_to_u32(const u64_t* llong);
bool llong_eq(const u64_t* left, const u64_t* right);
bool llong_ltu(const u64_t* left, const u64_t* right);
bool llong_lts(const u64_t* left, const u64_t* right);
bool llong_bool(const u64_t* llong);
void llong_print(const u64_t* llong, FILE* stream);

#endif
