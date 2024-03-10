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

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libo-error.h"

#include "lexer.h"

/*
 * Fatal error functions
 */

void fatal_2(const char* message_1, const char* message_2) {
    fatal_4(message_1, message_2, "", "");
}

void fatal_3(const char* message_1, const char* message_2,
        const char* message_3)
{
    fatal_4(message_1, message_2, message_3, "");
}

void fatal_4(const char* message_1, const char* message_2,
        const char* message_3, const char* message_4)
{
    fatal_prefix();
    fputs(message_1, stderr);
    fputs(message_2, stderr);
    fputs(message_3, stderr);
    fputs(message_4, stderr);
    fputc('\n', stderr);
    _Exit(1);
}

#ifndef __GNUC__
void assert(bool x) {
    if (!x) {
        fatal("Assertion failed");
    }
}
#endif

void* memdup(const void* other, size_t size) {
    void* p = malloc(size);
    if (p) {
        memcpy(p, other, size);
    }
    return p;
}
