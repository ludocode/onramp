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

#include <stdlib.h>

#include "internal.h"

#include <ctype.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>



/*
 * Number Parsing
 *
 * strtol() sets errno but atol() doesn't. We use this helper that can set it
 * optionally.
 */

long strtol_impl(const char* restrict p, char** restrict end, int base, bool set_errno) {

    // base check
    if (((base < 0) | (base == 1)) | (base > 36)) {
        __fatal("Invalid base for strtol()");
    }

    // skip leading whitespace
    while (isspace(*p)) {
        p = (p + 1);
    }

    // parse sign
    bool negative = false;
    if (*p == '-') {
        negative = true;
        p = (p + 1);
    }
    if (!negative) {
        if (*p == '+') {
            p = (p + 1);
        }
    }

    // skip leading hex 0x/0X
    if ((base == 0) | (base == 16)) {
        if (*p == '0') {
            char x = *(p + 1);
            if ((x == 'x') | (x == 'X')) {
                base = 16;
                p = (p + 2);
            }
        }
    }

    // skip leading binary 0b/0B
    if ((base == 0) | (base == 2)) {
        if (*p == '0') {
            char b = *(p + 1);
            if ((b == 'b') | (b == 'B')) {
                base = 2;
                p = (p + 2);
            }
        }
    }

    // skip a leading zero, switching to octal if we're detecting
    if (*p == '0') {
        if (base == 0) {
            base = 8;
        }
        p = (p + 1);
    }

    // otherwise assume decimal
    if (base == 0) {
        base = 10;
    }

    // accumulate digits
    long value = 0;
    bool overflow = false;
    while (1) {
        long digit = 99;
        if ((*p >= '0') & (*p <= '9')) {
            digit = (*p - '0');
        }
        if ((*p >= 'a') & (*p <= 'z')) {
            digit = ((*p - 'a') + 10);
        }
        if ((*p >= 'A') & (*p <= 'Z')) {
            digit = ((*p - 'A') + 10);
        }
        if (digit >= base) {
            break;
        }
        // We accumulate the number negative so we can do a proper range check.
        // (We don't have unsigned and if we accumulated positive we couldn't
        // parse LONG_MIN.)
        long new_value = ((value * base) - digit);
        if (new_value > value) {
            // on overflow we're supposed to continue parsing the whole number
            overflow = true;
        }
        value = new_value;
        p = (p + 1);
    }
    if (end) {
        *end = (char*)p;
    }

    // range and sign checks
    if (negative) {
        if (overflow) {
            if (set_errno) {
                errno = ERANGE;
            }
            return LONG_MIN;
        }
        return value;
    }
    if (overflow) {
        if (set_errno) {
            errno = ERANGE;
        }
        return LONG_MAX;
    }
    if (value == LONG_MIN) {
        // -LONG_MIN is not representable.
        if (set_errno) {
            errno = ERANGE;
        }
        return LONG_MAX;
    }
    return -value;
}

long strtol(const char* restrict p, char** restrict end, int base) {
    return strtol_impl(p, end, base, true);
}

int atoi(const char* p) {
    return strtol_impl(p, NULL, 10, false);
}

long atol(const char* p) {
    return strtol_impl(p, NULL, 10, false);
}
