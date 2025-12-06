/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2025 Fraser Heavy Software
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
 * This is based on strtol.c from libc/1, adding 64-bit strtoll(). We split it
 * into separate functions so we can share as much as possible while still
 * using 32-bit math for strtol() to keep it fast.
 */

// parses the sign, returning true if negative.
static bool parse_sign(const char* restrict* p) {

    // skip leading whitespace
    while (isspace(**p)) {
        ++*p;
    }

    // parse sign
    if (**p == '-') {
        ++*p;
        return true;
    }
    if (**p == '+') {
        ++*p;
    }
    return false;
}

static int parse_base(const char* restrict* p, int base) {

    // base check
    if (base < 0 || base == 1 || base > 36) {
        return -1;
    }

    // skip leading hex 0x/0X
    if (base == 0 || base == 16) {
        if (**p == '0') {
            char x = (*p)[1];
            if (x == 'x' || x == 'X') {
                *p += 2;
                return 16;
            }
        }
    }

    // skip leading binary 0b/0B
    if (base == 0 || base == 2) {
        if (**p == '0') {
            char b = (*p)[1];
            if (b == 'b' || b == 'B') {
                *p += 2;
                return 2;
            }
        }
    }

    // detect leading 0 for octal
    if (base == 0 && **p == '0') {
        ++*p;
        return 8;
    }

    // otherwise assume decimal
    if (base == 0) {
        return 10;
    }

    return base;
}

static int parse_digit(const char* p) {
    int c = *p;
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;
    }
    return 99;
}

long long strtoll(const char* restrict p, char** restrict end, int base) {
    bool negative = parse_sign(&p);
    base = parse_base(&p, base);
    if (base == -1) {
        errno = EINVAL;
        return 0;
    }

    // accumulate digits
    long long value = 0;
    bool overflow = false;
    while (1) {
        int digit = parse_digit(p);
        if (digit >= base) {
            break;
        }

        // TODO we currently parse negative because omC didn't have unsigned
        // when this was written. This should be converted to unsigned math.
        long long new_value = ((value * base) - digit);
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
            errno = ERANGE;
            return LLONG_MIN;
        }
        return value;
    }
    if (overflow) {
        errno = ERANGE;
        return LLONG_MAX;
    }
    if (value == LLONG_MIN) {
        // -LLONG_MIN is not representable.
        errno = ERANGE;
        return LLONG_MAX;
    }
    return -value;
}

// strtol() sets errno but atol() doesn't. We use this helper that can set it
// optionally.
static long strtol_impl(const char* restrict p, char** restrict end, int base, bool set_errno) {
    bool negative = parse_sign(&p);
    base = parse_base(&p, base);
    if (base == -1) {
        // no need to check set_errno, it's only clear on base 10
        errno = EINVAL;
        return 0;
    }

    // accumulate digits
    long value = 0;
    bool overflow = false;
    while (1) {
        int digit = parse_digit(p);
        if (digit >= base) {
            break;
        }

        // We accumulate the number negative so we can do a proper range check.
        // (We don't have unsigned and if we accumulated positive we couldn't
        // parse LONG_MIN.)
        // TODO we currently parse negative because omC didn't have unsigned
        // when this was written. This should be converted to unsigned math.
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
