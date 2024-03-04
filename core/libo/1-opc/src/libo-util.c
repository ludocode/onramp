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

#include "libo-util.h"

#include <stdio.h>
#include <stdlib.h>

uint32_t fnv1a_cstr(const char* s) {
    uint32_t hash = 2166136261u;
    while (*s != 0) {
        hash = ((hash ^ (*s++ & 0xFFu)) * 16777619u);
    }
    return hash;
}

uint32_t fnv1a_bytes(const char* p, size_t count) {
    const char* end = p + count;
    uint32_t hash = 2166136261u;
    while (p != end) {
        hash = ((hash ^ (*p++ & 0xFFu)) * 16777619u);
    }
    return hash;
}

// TODO our opc libc will have sprintf(), move this there to become part of
// string formatting and just call sprintf() here

char* itoa_d(int value, char* buffer) {

    // special case for zero
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = 0;
        return buffer;
    }

    // if the number is negative, convert to positive
    // (this works even for INT_MIN because we do the rest as unsigned)
    int was_negative = value < 0;
    unsigned int uvalue;
    if (was_negative) {
        uvalue = (unsigned int)(-value);
    } else {
        uvalue = (unsigned int)value;
    }

    // format (backwards)
    unsigned int i;
    for (i = 0; uvalue != 0; i++) {
        unsigned int new_uvalue = uvalue / 10;
        unsigned int digit = uvalue - (new_uvalue * 10);
        buffer[i] = '0' + digit;
        uvalue = new_uvalue;
    }
    if (was_negative) {
        buffer[i++] = '-';
    }
    buffer[i] = 0;

    // reverse characters
    unsigned int half = i >> 1;
    for (unsigned int j = 0; j < half; ++j) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
    return buffer;
}

void fputd(int number, FILE* file) {
    char buffer[12];
    itoa_d(number, buffer);
    fputs(buffer, file);
}

void putd(int number) {
    fputd(number, stdout);
}
