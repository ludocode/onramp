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

#ifdef __onramp__
#include "bigint.h"
#endif

#ifndef __onramp__
#include <stddef.h>
#include <stdint.h>
_Noreturn void __fatal(const char* s);
#endif

#include <assert.h>

#ifdef __onramp__
#include "internal.h"
#endif

void __bigint_clear(const uint32_t n, uint32_t* x) {
    for (size_t i = 0u; i < n; ++i) {
        x[i] = 0u;
    }
}

void __bigint_set(const uint32_t n, uint32_t* dest, const uint32_t* src) {
    if (dest == src) {
        return;
    }
    for (size_t i = 0u; i < n; ++i) {
        dest[i] = src[i];
    }
}

void __bigint_set_u32(const uint32_t n, uint32_t* dest, const uint32_t src) {
    assert(n >= 1u);
    dest[0u] = src;
    for (size_t i = 1u; i < n; ++i) {
        dest[i] = 0u;
    }
}

void __bigint_set_u64(const uint32_t n, uint32_t* dest, const uint32_t* src) {
    assert(n >= 1u);
    dest[0u] = src[0u];
    if (n >= 2u) {
        dest[1u] = src[1u];
        for (size_t i = 2u; i < n; ++i) {
            dest[i] = 0u;
        }
    }
}

void __bigint_add(const uint32_t n, uint32_t* dest, const uint32_t* left, const uint32_t* right) {
    uint32_t carry = 0u;
    for (size_t i = 0u; i < n; ++i) {
        uint32_t lw = left[i];
        uint32_t rw = right[i];
        uint32_t dw = lw + carry;
        carry = dw < lw;
        dw += rw;
        carry |= dw < rw;
        dest[i] = dw;
    }
}

void __bigint_sub(const uint32_t n, uint32_t* dest, const uint32_t* left, const uint32_t* right) {
    uint32_t borrow = 0u;
    for (size_t i = 0u; i < n; ++i) {
        uint32_t lw = left[i];
        uint32_t rw = right[i];
        //printf("lw %u rw %u oldborrow %u", lw, rw, borrow);
        uint32_t dw = lw - rw - borrow;
        borrow = (lw < rw) | (lw < (rw + borrow));
        dest[i] = dw;
        //printf(" dw %u newborrow %u\n", dw, borrow);
    }
}

void __bigint_shl(const uint32_t n, uint32_t* dest, const uint32_t* src, uint32_t bits) {
    if (bits == 0u) {
        __bigint_set(n, dest, src);
        return;
    }

    size_t words = bits >> 5u;
    if (words >= n) {
        __bigint_clear(n, dest);
        return;
    }
    bits &= 31u;

    size_t i = n;
    if (bits == 0u) {
        // special case for bits divisible by 32. we don't need to shift the
        // contents of words; we just move them.
        while (i > words) {
            --i;
            dest[i] = src[i - words];
        }
    } else {
        while (i-- > words + 1) {
            dest[i] = (src[i - words] << bits) | src[i - words - 1u] >> (32u - bits);
        }
        dest[i] = src[i - words] << bits;
    }

    // clear any extra words
    while (i-- > 0u) {
        dest[i] = 0u;
    }
}

void __bigint_shru(const uint32_t n, uint32_t* dest, const uint32_t* src, uint32_t bits) {
    if (bits == 0u) {
        __bigint_set(n, dest, src);
        return;
    }

    size_t words = bits >> 5u;
    if (words >= n) {
        __bigint_clear(n, dest);
        return;
    }
    bits &= 31u;

    size_t i = 0u;
    if (bits == 0u) {
        // special case for bits divisible by 32. we don't need to shift the
        // contents of words; we just move them.
        for (; i < n - words; ++i) {
            dest[i] = src[i + words];
        }
    } else {
        for (; i < n - words - 1u; ++i) {
            dest[i] = (src[i + words] >> bits) | src[i + words + 1u] << (32u - bits);
        }
        dest[i] = src[i + words] >> bits;
        ++i;
    }

    // clear any extra words
    for (; i < n; ++i) {
        dest[i] = 0u;
    }
}

void __bigint_mul_u16(const uint32_t n, uint32_t* dest, const uint32_t* left, uint16_t right) {
    assert(n > 0u);
    if (n == 1u) {
        *dest = *left * right;
        return;
    }

    // Long multiplication base 2^16.
    //
    // Starting from the low bits, we multiply every 16-bit short in the
    // multiplicand with the 16-bit multiplier. The results are shifted and
    // added properly, propagating the carry up as we go.

    uint32_t carry = 0;
    for (size_t i = 0u; i < n; ++i) {
        uint32_t left_word = left[i];

        uint32_t low = (left_word & 0xffffu) * right;
        uint32_t high = (left_word >> 16u) * right;

        uint32_t high_raised = high << 16u;
        uint32_t high_lowered = high >> 16u;

        uint32_t dest_word = low + carry;
        carry = (dest_word < low) + high_lowered;
        dest_word += high_raised;
        carry += (dest_word < high_raised);
        dest[i] = dest_word;
    }
}

uint16_t __bigint_div_to_u16(const uint32_t n, const uint32_t* left, const uint32_t* right) {
    (void)n;
    (void)left;
    (void)right;
    __fatal("TODO");
}

void __bigint_print_hex(uint32_t n, const uint32_t* x, char* buffer, size_t size) {
    if (size < 3 + n * 8) {
        __fatal("buffer not large enough to print bigint");
    }
    *buffer++ = '0';
    *buffer++ = 'x';
    while (n-- > 0) {
        uint32_t v = x[n];
        for (size_t i = 0; i < 8; ++i) {
            uint32_t c = (v >> (4 * (7 - i))) & 15;
            *buffer++ = (c < 10) ? '0' + c : 'a' + (c - 10);
        }
    }
    *buffer = 0;
}
