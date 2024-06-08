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

#include "llong.h"

#include "common.h"

#ifndef LLONG_NATIVE
// libc/2 functions
extern void __llong_add(unsigned* out, unsigned* a, unsigned* b);
extern void __llong_sub(unsigned* out, unsigned* a, unsigned* b);
extern void __llong_mul(unsigned* out, unsigned* a, unsigned* b);
extern void __llong_divs(unsigned* out, unsigned* a, unsigned* b);
extern void __llong_divu(unsigned* out, unsigned* a, unsigned* b);
extern bool __llong_ltu(unsigned* a, unsigned* b);
extern bool __llong_lts(unsigned* a, unsigned* b);
extern void __llong_shl(unsigned* out, unsigned* a, int bits);
extern void __llong_shru(unsigned* out, unsigned* a, int bits);
extern void __llong_shrs(unsigned* out, unsigned* a, int bits);
extern void __llong_and(unsigned* out, unsigned* a, unsigned* b);
extern void __llong_or(unsigned* out, unsigned* a, unsigned* b);
extern void __llong_xor(unsigned* out, unsigned* a, unsigned* b);
extern void __llong_not(unsigned* out, unsigned* src);
#endif

void llong_clear(llong_t* llong) {
    #ifdef LLONG_NATIVE
    llong->value = 0;
    #endif

    #ifndef LLONG_NATIVE
    llong->words[0] = 0;
    llong->words[1] = 0;
    #endif
}

void llong_set(llong_t* llong, const llong_t* other) {
    #ifdef LLONG_NATIVE
    llong->value = other->value;
    #endif

    #ifndef LLONG_NATIVE
    llong->words[0] = other->words[0];
    llong->words[1] = other->words[1];
    #endif
}

void llong_add(llong_t* llong, const llong_t* other) {
    #ifdef LLONG_NATIVE
    llong->value += other->value;
    #endif

    #ifndef LLONG_NATIVE
    __llong_add(llong->words, llong->words, other->words);
    #endif
}

void llong_add_u(llong_t* llong, unsigned other) {
    #ifdef LLONG_NATIVE
    llong->value += other;
    #endif

    #ifndef LLONG_NATIVE
    // TODO very inefficient, doesn't matter
    unsigned words[2];
    words[0] = other;
    words[1] = 0;
    __llong_add(llong->words, llong->words, words);
    #endif
}

void llong_sub(llong_t* llong, const llong_t* other) {
    #ifdef LLONG_NATIVE
    llong->value -= other->value;
    #endif

    #ifndef LLONG_NATIVE
    __llong_sub(llong->words, llong->words, other->words);
    #endif
}

void llong_mul(llong_t* llong, const llong_t* other) {
    #ifdef LLONG_NATIVE
    llong->value *= other->value;
    #endif

    #ifndef LLONG_NATIVE
    __llong_mul(llong->words, llong->words, other->words);
    #endif
}

void llong_mul_u(llong_t* llong, unsigned other) {
    #ifdef LLONG_NATIVE
    llong->value *= other;
    #endif

    #ifndef LLONG_NATIVE
    // TODO very inefficient, doesn't matter
    unsigned words[2];
    words[0] = other;
    words[1] = 0;
    __llong_mul(llong->words, llong->words, words);
    #endif
}

void llong_divu(llong_t* llong, const llong_t* other) {
    #ifdef LLONG_NATIVE
    llong->value /= other->value;
    #endif

    #ifndef LLONG_NATIVE
    __llong_divu(llong->words, llong->words, other->words);
    #endif
}

void llong_divs(llong_t* llong, const llong_t* other) {
    #ifdef LLONG_NATIVE
    llong->value = (unsigned long long)(
            (long long)llong->value / (long long)other->value);
    #endif

    #ifndef LLONG_NATIVE
    __llong_divs(llong->words, llong->words, other->words);
    #endif
}

bool llong_ltu(const llong_t* left, const llong_t* right) {
    #ifdef LLONG_NATIVE
    return left->value < right->value;
    #endif

    #ifndef LLONG_NATIVE
    return __llong_ltu(left->words, right->words);
    #endif
}

bool llong_lts(const llong_t* left, const llong_t* right) {
    #ifdef LLONG_NATIVE
    return (long long)left->value < (long long)right->value;
    #endif

    #ifndef LLONG_NATIVE
    return __llong_lts(left->words, right->words);
    #endif
}

void llong_shl(llong_t* llong, int bits) {
    #ifdef LLONG_NATIVE
    llong->value <<= bits;
    #endif

    #ifndef LLONG_NATIVE
    __llong_shl(llong->words, llong->words, bits);
    #endif
}

void llong_shru(llong_t* llong, int bits) {
    #ifdef LLONG_NATIVE
    llong->value >>= bits;
    #endif

    #ifndef LLONG_NATIVE
    __llong_shru(llong->words, llong->words, bits);
    #endif
}

void llong_shrs(llong_t* llong, int bits) {
    #ifdef LLONG_NATIVE
    llong->value = (unsigned long long)((long long)llong->value >> bits);
    #endif

    #ifndef LLONG_NATIVE
    __llong_shrs(llong->words, llong->words, bits);
    #endif
}

void llong_and(llong_t* llong, const llong_t* other) {
    #ifdef LLONG_NATIVE
    llong->value &= other->value;
    #endif

    #ifndef LLONG_NATIVE
    __llong_and(llong->words, llong->words, other->words);
    #endif
}

void llong_or(llong_t* llong, const llong_t* other) {
    #ifdef LLONG_NATIVE
    llong->value |= other->value;
    #endif

    #ifndef LLONG_NATIVE
    __llong_or(llong->words, llong->words, other->words);
    #endif
}

void llong_xor(llong_t* llong, const llong_t* other) {
    #ifdef LLONG_NATIVE
    llong->value ^= other->value;
    #endif

    #ifndef LLONG_NATIVE
    __llong_xor(llong->words, llong->words, other->words);
    #endif
}

void llong_not(llong_t* llong) {
    #ifdef LLONG_NATIVE
    llong->value = ~llong->value;
    #endif

    #ifndef LLONG_NATIVE
    __llong_not(llong->words, llong->words);
    #endif
}

void llong_print(const llong_t* llong, FILE* stream) {
    fatal("TODO llong_print()");
}
