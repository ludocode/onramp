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

#include "arithmetic.h"

#ifndef ONRAMP_U64_NATIVE
#include <__onramp/__arithmetic.h>
#endif

#include "common.h"

void llong_clear(u64_t* llong) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value = 0;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    llong->words[0] = 0;
    llong->words[1] = 0;
    #endif
}

void llong_set(u64_t* llong, const u64_t* other) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value = other->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    llong->words[0] = other->words[0];
    llong->words[1] = other->words[1];
    #endif
}

void llong_set_u(u64_t* llong, uint32_t other) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value = other;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    llong->words[0] = other;
    llong->words[1] = 0;
    #endif
}

void llong_add(u64_t* llong, const u64_t* other) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value += other->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    __llong_add(llong->words, llong->words, other->words);
    #endif
}

void llong_add_u(u64_t* llong, unsigned other) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value += other;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    // TODO very inefficient, doesn't matter
    unsigned words[2];
    words[0] = other;
    words[1] = 0;
    __llong_add(llong->words, llong->words, words);
    #endif
}

void llong_sub(u64_t* llong, const u64_t* other) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value -= other->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    __llong_sub(llong->words, llong->words, other->words);
    #endif
}

void llong_mul(u64_t* llong, const u64_t* other) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value *= other->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    __llong_mul(llong->words, llong->words, other->words);
    #endif
}

void llong_mul_u(u64_t* llong, unsigned other) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value *= other;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    // TODO very inefficient, doesn't matter
    unsigned words[2];
    words[0] = other;
    words[1] = 0;
    __llong_mul(llong->words, llong->words, words);
    #endif
}

void llong_divu(u64_t* llong, const u64_t* other) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value /= other->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    __llong_divu(llong->words, llong->words, other->words);
    #endif
}

void llong_divs(u64_t* llong, const u64_t* other) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value = (unsigned long long)(
            (long long)llong->value / (long long)other->value);
    #endif

    #ifndef ONRAMP_U64_NATIVE
    __llong_divs(llong->words, llong->words, other->words);
    #endif
}

void llong_modu(u64_t* llong, const u64_t* other) {
    fatal("TODO llong_modu() not yet implemented");
}

void llong_mods(u64_t* llong, const u64_t* other) {
    fatal("TODO llong_mods() not yet implemented");
}

bool llong_eq(const u64_t* left, const u64_t* right) {
    #ifdef ONRAMP_U64_NATIVE
    return left->value == right->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    return left->words[0] == right->words[0] &&
            left->words[1] == right->words[1];
    #endif
}

bool llong_ltu(const u64_t* left, const u64_t* right) {
    #ifdef ONRAMP_U64_NATIVE
    return left->value < right->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    return __llong_ltu(left->words, right->words);
    #endif
}

bool llong_lts(const u64_t* left, const u64_t* right) {
    #ifdef ONRAMP_U64_NATIVE
    return (long long)left->value < (long long)right->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    return __llong_lts(left->words, right->words);
    #endif
}

bool llong_gtu(const u64_t* left, const u64_t* right) {
    #ifdef ONRAMP_U64_NATIVE
    return left->value > right->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    return __llong_ltu(right->words, left->words);
    #endif
}

bool llong_gts(const u64_t* left, const u64_t* right) {
    #ifdef ONRAMP_U64_NATIVE
    return (long long)left->value > (long long)right->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    return __llong_lts(right->words, left->words);
    #endif
}

bool llong_leu(const u64_t* left, const u64_t* right) {
    #ifdef ONRAMP_U64_NATIVE
    return left->value <= right->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    return !__llong_ltu(right->words, left->words);
    #endif
}

bool llong_les(const u64_t* left, const u64_t* right) {
    #ifdef ONRAMP_U64_NATIVE
    return (long long)left->value <= (long long)right->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    return !__llong_lts(right->words, left->words);
    #endif
}

bool llong_geu(const u64_t* left, const u64_t* right) {
    #ifdef ONRAMP_U64_NATIVE
    return left->value >= right->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    return !__llong_ltu(left->words, right->words);
    #endif
}

bool llong_ges(const u64_t* left, const u64_t* right) {
    #ifdef ONRAMP_U64_NATIVE
    return (long long)left->value >= (long long)right->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    return !__llong_lts(left->words, right->words);
    #endif
}

void llong_shl(u64_t* llong, int bits) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value <<= bits;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    __llong_shl(llong->words, llong->words, bits);
    #endif
}

void llong_shru(u64_t* llong, int bits) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value >>= bits;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    __llong_shru(llong->words, llong->words, bits);
    #endif
}

void llong_shrs(u64_t* llong, int bits) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value = (unsigned long long)((long long)llong->value >> bits);
    #endif

    #ifndef ONRAMP_U64_NATIVE
    __llong_shrs(llong->words, llong->words, bits);
    #endif
}

void llong_and(u64_t* llong, const u64_t* other) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value &= other->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    __llong_and(llong->words, llong->words, other->words);
    #endif
}

void llong_or(u64_t* llong, const u64_t* other) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value |= other->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    __llong_or(llong->words, llong->words, other->words);
    #endif
}

void llong_xor(u64_t* llong, const u64_t* other) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value ^= other->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    __llong_xor(llong->words, llong->words, other->words);
    #endif
}

void llong_bit_not(u64_t* llong) {
    #ifdef ONRAMP_U64_NATIVE
    llong->value = ~llong->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    __llong_bit_not(llong->words, llong->words);
    #endif
}

bool llong_bool(const u64_t* llong) {
    #ifdef ONRAMP_U64_NATIVE
    return !!llong->value;
    #endif

    #ifndef ONRAMP_U64_NATIVE
    return !!(llong->words[0] | llong->words[1]);
    #endif
}

void llong_negate(u64_t* llong) {
    // TODO surely there's a faster way to do this but I don't feel like
    // testing right now
    u64_t temp;
    llong_set_u(&temp, 0);
    llong_sub(&temp, llong);
    llong_set(llong, &temp);
}

void llong_print(const u64_t* llong, FILE* stream) {
    fatal("TODO llong_print()");
}
