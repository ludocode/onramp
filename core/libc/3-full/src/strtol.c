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

static unsigned parse_base(const char* restrict* p, unsigned base) {

    // base check
    if (base == 1 || base > 36) {
        return UINT_MAX;
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

static unsigned parse_digit(const char* p) {
    unsigned char c = (unsigned char)*p;
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10u;
    }
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10u;
    }
    return 99u;
}

// Implementations of strtol() and strtoll() follow. They are identical except
// for the types and ranges.

static long strtol_impl(const char* restrict p, char** restrict end, int base_arg,
        bool is_signed, bool set_errno)
{
    bool negative = parse_sign(&p);
    unsigned base = parse_base(&p, base_arg);
    if (base == UINT_MAX) {
        // no need to check set_errno, it's only false on base 10
        errno = EINVAL;
        return 0;
    }

    // accumulate digits
    unsigned long value = 0;
    bool overflow = false;
    for (;;) {
        unsigned digit = parse_digit(p);
        if (digit >= base) {
            break;
        }
        unsigned long new_value = ((value * base) + digit);
        if (new_value < value) {
            overflow = true;
            // On overflow we're supposed to continue parsing the whole number
            // to set the correct end pointer. This is only necessary if we
            // actually have an end pointer to assign.
            if (!end) {
                break;
            }
        }
        value = new_value;
        ++p;
    }
    if (end) {
        *end = (char*)p;
    }

    // check for overflow of unsigned value
    if (overflow) {
        if (set_errno) {
            errno = ERANGE;
        }
        if (is_signed) {
            if (negative) {
                return LONG_MIN;
            } else {
                return LONG_MAX;
            }
        }
        return (long)ULONG_MAX;
    }

    // for unsigned we don't have to do any more checks
    if (!is_signed) {
        return (long)(negative ? (-value) : value);
    }

    // for signed we have to check range of signed value
    if (negative) {
        if (value > ((unsigned long)LONG_MAX + 1)) {
            if (set_errno) {
                errno = ERANGE;
            }
            return LONG_MIN;
        }
        return (long)(-value);
    }
    if (value > ((unsigned long)LONG_MAX)) {
        if (set_errno) {
            errno = ERANGE;
        }
        return LONG_MAX;
    }
    return (long)value;
}

static long long strtoll_impl(const char* restrict p, char** restrict end, int base_arg,
        bool is_signed, bool set_errno)
{
    bool negative = parse_sign(&p);
    unsigned base = parse_base(&p, base_arg);
    if (base == UINT_MAX) {
        // no need to check set_errno, it's only false on base 10
        errno = EINVAL;
        return 0;
    }

    // accumulate digits
    unsigned long long value = 0;
    bool overflow = false;
    for (;;) {
        unsigned digit = parse_digit(p);
        if (digit >= base) {
            break;
        }
        // TODO since base is always small we could optimize this a lot by
        // implementing a __llong_mul_u16() function. We'll probably want such
        // a function for other purposes as well.
        unsigned long long new_value = ((value * base) + digit);
        if (new_value < value) {
            overflow = true;
            // On overflow we're supposed to continue parsing the whole number
            // to set the correct end pointer. This is only necessary if we
            // actually have an end pointer to assign.
            if (!end) {
                break;
            }
        }
        value = new_value;
        ++p;
    }
    if (end) {
        *end = (char*)p;
    }

    // check for overflow of unsigned value
    if (overflow) {
        if (set_errno) {
            errno = ERANGE;
        }
        if (is_signed) {
            if (negative) {
                return LLONG_MIN;
            } else {
                return LLONG_MAX;
            }
        }
        return (long long)ULLONG_MAX;
    }

    // for unsigned we don't have to do any more checks
    if (!is_signed) {
        return (long long)(negative ? (-value) : value);
    }

    // for signed we have to check range of signed value
    if (negative) {
        if (value > ((unsigned long long)LLONG_MAX + 1)) {
            if (set_errno) {
                errno = ERANGE;
            }
            return LLONG_MIN;
        }
        return (long long)(-value);
    }
    if (value > ((unsigned long long)LLONG_MAX)) {
        if (set_errno) {
            errno = ERANGE;
        }
        return LLONG_MAX;
    }
    return (long long)value;
}

// The below wrappers are the public functions. The strto*() functions set
// errno but ato*() don't.

long long strtoll(const char* restrict p, char** restrict end, int base) {
    return strtoll_impl(p, end, base, true, true);
}

unsigned long long strtoull(const char* restrict p, char** restrict end, int base) {
    return (unsigned long long)strtoll_impl(p, end, base, false, true);
}

long long atoll(const char* p) {
    return strtoll_impl(p, NULL, 10, true, false);
}

long strtol(const char* restrict p, char** restrict end, int base) {
    return strtol_impl(p, end, base, true, true);
}

unsigned long strtoul(const char* restrict p, char** restrict end, int base) {
    return strtol_impl(p, end, base, false, true);
}

int atoi(const char* p) {
    return strtol_impl(p, NULL, 10, true, false);
}
