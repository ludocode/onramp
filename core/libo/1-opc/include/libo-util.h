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

#ifndef ONRAMP_LIBO_UTIL_H_INCLUDED
#define ONRAMP_LIBO_UTIL_H_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifndef __onramp_cpp_omc__
    #define containerof(pointer, structure, member) \
            ((structure*)((char*)(pointer) - offsetof(structure, member)))
#endif

/**
 * Calculates the Knuth multiplicative hash of the given 32-bit unsigned value
 * with the result reduced to the given number of bits.
 *
 * This gives us a reasonable distribution even for sequential hashes.
 */
// TODO the unsigned cast here is temporary, cci/1 is incorrectly converting
// the left side to signed
#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define knuth_hash_32(value, bits) (((value) * 2654435761u) >> (unsigned)(32 - (bits)))
    #endif
#endif
#ifndef knuth_hash_32
    static inline uint32_t knuth_hash_32(uint32_t value, int bits) {
        return (value * 2654435761u) >> (unsigned)(32 - bits);
    }
#endif

/**
 * Calculates the FNV-1a hash of the given null-terminated string.
 */
uint32_t fnv1a_cstr(const char* s);

/**
 * Calculates the FNV-1a hash of the given bytes.
 */
uint32_t fnv1a_bytes(const char* p, size_t count);

/**
 * Converts a number to decimal.
 */
char* itoa_d(int value, char* buffer);

/**
 * Writes a number in decimal to the given file.
 */
void fputd(int number, FILE* file);

/**
 * Writes a number in decimal to standard output.
 */
void putd(int number);

#endif
