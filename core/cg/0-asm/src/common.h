/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Fraser Heavy Software
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

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "libo-error.h"
#include "libo-util.h"

static FILE* input_file;
static FILE* output_file;

static int current_char;

static void read_char(void) {
    if (current_char == '\n') {
        current_line = (current_line + 1);
    }
    current_char = fgetc(input_file);
    if (current_char == EOF) {
        // TODO we need ferror() in libc/0 to properly detect errors.
        #ifndef __onramp_libc_oo__
        if (ferror(input_file)) {
            fatal("An error occurred reading the input file.");
        }
        #endif
    }
}

// TODO put is_identifier_char in libo/0
static bool is_identifier_char(char c, bool first_char) {
    if (isdigit(c)) {
        return !first_char;
    }
    if (isalpha(c)) {
        return true;
    }
    if (c == '_') {
        return true;
    }
    if (c == '$') { // GNU extension, probably don't need this for bootstrapping
        return true;
    }
    return false;
}

static char int_to_hex(int value) { // TODO unsigned
    if (value <= 9) {
        return '0' + value;
    }
    if (value <= 15) {
        return 'A' + (value - 10);
    }
    fatal("Internal error: invalid hex value");
}

// Returns true if the given mix-type byte is a register, false otherwise.
static bool is_register(int reg) {
    return ((reg & 0xFFFFFFF0) == 0x80);
}

// Converts the given non-register mix-type argument to an integer.
static int mix_to_int(int x) {
    assert((x & 0xF0) != 0x80); // cannot be a register
    return (int)(char)x; // sxb, sign-extend byte
}

// TODO hack, __assert() is not defined in libc/0 which cg/0 is linked against.
// Probably we should just define this straight in libc/0 so we can use asserts
// everywhere.
#ifdef __onramp_libc_oo__
void __assert(int expression) {
    if (!expression) {
        fputs("Assertion failed.", stderr);
        fflush(stderr);
        abort();
    }
}
#endif

#endif
