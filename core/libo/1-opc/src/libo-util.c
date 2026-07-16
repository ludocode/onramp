/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2026 Fraser Heavy Software
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

#include "libo-error.h"

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

// TODO share this, make not static
static char int_to_hexu(unsigned value) {
    if (value <= 9)
        return '0' + value;
    if (value <= 15)
        return 'A' + value - 10;
    fatal("Internal error: invalid hex value");
}



// TODO our opc libc will have sprintf(), move these there to become part of
// string formatting and just call sprintf() here
//
// TODO these should be in libc/1 so we can use them earlier than sprintf(), we
// need them in cci/2 which will eventually be compiled with cci/0

static inline char* reverse(char* buffer, size_t length) {
    size_t half = length >> 1;
    for (size_t i = 0; i < half; ++i) {
        char temp = buffer[i];
        buffer[i] = buffer[length - i - 1];
        buffer[length - i - 1] = temp;
    }
    return buffer;
}

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
    return reverse(buffer, i);
}

char* itoa_hexu(uint32_t value, char* buffer) {

    // format (backwards)
    size_t i = 0;
    do {
        buffer[i++] = int_to_hexu(value & 0xF);
        value >>= 4;
    } while (value != 0);
    buffer[i] = 0;

    // reverse characters
    reverse(buffer, i);
    return buffer + i;
}



void fputd(int number, FILE* file) {
    char buffer[12];
    itoa_d(number, buffer);
    fputs(buffer, file);
}

void putd(int number) {
    fputd(number, stdout);
}

#ifndef __onramp__
// TODO for lack of a better place to put this. We need this to build libo
// natively. On Onramp it's in the libc.
#include <string.h>
void* __memdup(const void* src, size_t count) {
    void* dest = malloc(count);
    if (!dest) {
        // malloc() set errno
        return NULL;
    }
    memcpy(dest, src, count);
    return dest;
}
#endif
