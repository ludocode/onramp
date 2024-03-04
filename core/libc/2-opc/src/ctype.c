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

#include <ctype.h>

#include "internal.h"

/* Onramp supports only the "C" locale. */

int isalnum(int x) {
    return isalpha(x) || isdigit(x);
}

int isalpha(int x) {
    return isupper(x) || islower(x);
}

int isascii(int x) {
    return x >= 0 && x <= 127;
}

int isblank(int x) {
    return x == ' ' || x == '\t';
}

int iscntrl(int x) {
    return (x >= 0 && x <= 31) || x == 127;
}

int isdigit(int x) {
    return x >= '0' && x <= '9';
}

int isgraph(int x) {
    return x >= 33 && x <= 126;
}

int islower(int x) {
    return x >= 'a' && x <= 'z';
}

int isprint(int x) {
    return x >= 32 && x <= 126;
}

int ispunct(int x) {
    return !isspace(x) && !isalnum(x);
}

int isspace(int x) {
    return x == ' ' || x == '\f' || x == '\n' ||
            x == '\r' || x == '\t' || x == '\v';
}

int isupper(int x) {
    return x >= 'A' && x <= 'Z';
}

int isxdigit(int x) {
    return (x >= '0' && x <= '9') ||
            (x >= 'A' && x <= 'F') ||
            (x >= 'a' && x <= 'f');
}

int tolower(int x) {
    if (x >= 'A' && x <= 'Z')
        return x + 32;
    return x;
}

int toupper(int x) {
    if (x >= 'a' && x <= 'z')
        return x - 32;
    return x;
}
