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

/**
 * During bootstrapping we don't have `long long`. We need to represent it as
 * two `unsigned`. We do our 64-bit math by manually calling out to the
 * __llong_*() functions in libc/2.
 *
 * When the final stage is re-compiling itself (or when compiling with an
 * ordinary C compiler) we use normal `long long` math.
 */
#ifdef __onramp_cci_full__
    #define LLONG_NATIVE
#endif
#ifndef __onramp__
    #define LLONG_NATIVE
#endif

/**
 * A signed or unsigned 64-bit value used to implement `long long`.
 */
typedef struct llong_t {
    #ifdef LLONG_NATIVE
    unsigned long long value;
    #endif
    #ifndef LLONG_NATIVE
    unsigned words[2];
    #endif
} llong_t;

void llong_clear(llong_t* llong);
void llong_add(llong_t* llong, const llong_t* other);
void llong_add_i(llong_t* llong, int other);
void llong_sub(llong_t* llong, const llong_t* other);
void llong_mul(llong_t* llong, const llong_t* other);
void llong_divu(llong_t* llong, const llong_t* other);
void llong_divs(llong_t* llong, const llong_t* other);
bool llong_ltu(const llong_t* left, const llong_t* right);
bool llong_lts(const llong_t* left, const llong_t* right);
void llong_shl(llong_t* llong, int bits);
void llong_shru(llong_t* llong, int bits);
void llong_shrs(llong_t* llong, int bits);
void llong_and(llong_t* llong, const llong_t* other);
void llong_or(llong_t* llong, const llong_t* other);
void llong_xor(llong_t* llong, const llong_t* other);
void llong_not(llong_t* llong);
void llong_print(const llong_t* llong, FILE* stream);

#endif
