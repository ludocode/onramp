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

/*
 * Big decimal math.
 *
 * This is currently used for printing and parsing floating point numbers (for
 * converting between binary and decimal.)
 */

#include "bigdec.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef ONRAMP_STANDALONE_TEST
#include "internal.h"
#endif

void __bigdec_print(bigdec_t* bigdec, char* out, size_t outsize) {
    unsigned char* buffer = bigdec->buffer;
    size_t length = bigdec->length;
    int exponent = bigdec->exponent;

    // make sure we have enough space
    size_t needed = bigdec->length;
    if (length == 0) {
        needed = 1;
    } else if (exponent > 0) {
        needed += (size_t)exponent;
    } else if ((size_t)-exponent >= length) {
        needed += (size_t)-exponent - length + 2;
    } else if (exponent < 0) {
        needed += 1;
    }
    if (needed >= outsize) {
        __fatal("buffer too small");
    }

//printf("    print length %i exponent %i\n", (int)length, exponent);
    if (length == 0) {
        *out++ = '0';
    } else if (exponent >= 0) {
        for (size_t i = 0; i != length; ++i) {
//printf("    print buffer %i is %i\n", (int)i, buffer[i]);
            *out++ = '0' + buffer[i];
        }
        for (size_t i = 0; i != (size_t)exponent; ++i) {
            *out++ = '0';
        }
    } else if (length > (size_t)-exponent) {
        size_t c = length + exponent;
        size_t i = 0;
//printf("    print c %i length %i exponent %i\n", (int)c, (int)length, (int)exponent);
        for (; i != c; ++i) {
            *out++ = '0' + buffer[i];
        }
        *out++ = '.';
        for (; i != length; ++i) {
            *out++ = '0' + buffer[i];
        }
    } else {
        *out++ = '0';
        *out++ = '.';
        size_t z = -exponent - length;
        for (size_t i = 0; i < z; ++i) {
            *out++ = '0';
        }
        for (size_t i = 0; i != length; ++i) {
            *out++ = '0' + buffer[i];
        }
    }

    *out = 0;
}

void __bigdec_trim_leading_zeroes(bigdec_t* bigdec) {
    size_t length = bigdec->length;
    if (length == 0) {
        return;
    }

    unsigned char* buffer = bigdec->buffer;
    if (buffer[0] != 0) {
        return;
    }

    size_t shift = 0;
    while (buffer[shift] == 0 && shift < length) {
        ++shift;
    }

    if (shift == length) {
        bigdec->length = 0;
        bigdec->exponent = 0;
        return;
    }

    length -= shift;
    for (size_t i = 0; i < length; ++i) {
        buffer[i] = buffer[i + shift];
    }
    bigdec->length = length;
}

// TODO merge into above
void __bigdec_trim_trailing_zeroes(bigdec_t* bigdec) {
    size_t length = bigdec->length;
    if (length == 0) {
        return;
    }

    unsigned char* buffer = bigdec->buffer;
    while (length > 0 && buffer[length - 1] == 0) {
        --length;
        ++bigdec->exponent;
    }
    bigdec->length = length;
}

// TODO need to break up the below into separate functions. mul2, div2

void __bigdec_mul_pow2(bigdec_t* bigdec, int pow2) {
    if (pow2 == 0) {
        return;
    }

    size_t length = bigdec->length;
    if (length == 0) {
        return;
    }
    assert(bigdec->buffer[0] != 0); // no leading zeroes

    unsigned char* buffer = bigdec->buffer;
    size_t capacity = bigdec->capacity;
    int exponent = bigdec->exponent;

    if (pow2 > 0) {

        // Multiply.
        while (pow2-- != 0) {

            // First we check if we will need an extra digit. This happens if the
            // first digit is 5 or greater.
            size_t extra_digit = buffer[0] >= 5;
            if (length + extra_digit > capacity) {
                __fatal("Internal error: insufficient capacity for bigdec pow2 multiply");
            }

            // Next we multiply each digit by two starting at the bottom.
            uint32_t carry = 0;
            for (size_t k = length; k-- > 0;) {
                uint32_t result = (buffer[k] << 1) + carry;
                if (result >= 10) {
                    result -= 10;
                    carry = 1;
                } else {
                    carry = 0;
                }
                buffer[k + extra_digit] = (unsigned char)result;
            }
            assert(extra_digit == carry);
            if (extra_digit) {
                buffer[0] = 1;
            }
            length += extra_digit;

            // Trim trailing zeroes.
            while (buffer[length - 1] == 0) {
                --length;
                ++exponent;
            }
        }

    } else {

        // Divide.

        // Start by trimming trailing zeroes. (A divide can never add a
        // trailing zero.)
        // TODO no, should be done separately
        while (buffer[length - 1] == 0) {
            --length;
            ++exponent;
        }

        while (pow2++ != 0) {
            uint32_t carry = 0;

            // If the first digit is 1, we're going to eliminate it.
            size_t eliminate_digit = 0;
            if (buffer[0] == 1) {
                //printf("eliminate digit\n");
                eliminate_digit = 1;
                --length;
                carry = 10;
            }

            // Divide through each digit, carrying a 5 wherever we get an odd number
            for (size_t k = 0; k < length; ++k) {
                uint32_t sum = buffer[k + eliminate_digit] + carry;
                //printf("digit %u carry %u sum %u\n",buffer[k + eliminate_digit], carry,sum);
                carry = (sum & 1) * 10;
                buffer[k] = (unsigned char)(sum >> 1);
            }

            // If the last digit was odd, add the 5
            //printf("final carry %u\n",carry);
            if (carry != 0) {
                buffer[length++] = 5;
                --exponent;
            }
        }
    }

    bigdec->length = length;
    bigdec->exponent = exponent;
}

uint32_t __bigdec_uint(bigdec_t* bigdec, bool round) {
    size_t length = bigdec->length;
    if (length == 0) {
        return 0;
    }

    unsigned char* buffer = bigdec->buffer;
    int integer_digits = bigdec->exponent + length;
    if (integer_digits < 0) {
        // If we have less than zero integer digits, the number is 0.0X, so we
        // round or truncate down to zero. If we have exactly zero integer
        // digits, we may need to round correctly.
        return 0;
    }

    // Convert the integer portion of the bigdecimal to an integer
    uint32_t decint = 0;
    size_t real_digits = ((size_t)integer_digits < length) ? (size_t)integer_digits : length;
    size_t i = 0;
    for (; i < real_digits; ++i) {
        decint = (decint * 10) + buffer[i];
    }
    for (; i < (size_t)integer_digits; ++i) {
        decint *= 10;
    }

    if (round) {
        if (length > (size_t)integer_digits) {
            // We have a fraction.
            uint32_t fraction_digit = buffer[integer_digits];
            if (fraction_digit > 5) {
                ++decint;
            } else if (fraction_digit == 5) {
                if (length > ((size_t)integer_digits + 1)) {
                    ++decint; // the fraction digit is 5, but we have additional digits; round up
                } else if (integer_digits != 0 && (buffer[integer_digits - 1] & 1)) {
                    ++decint; // the fraction is exactly 5 and the previous digit is odd; round to even
                }
            }
        }
    }

    return decint;
}

int __bigdec_cmp_u32(bigdec_t* bigdec, uint32_t value) {
    size_t length = bigdec->length;
    if (length == 0) {
        return (value == 0) ? 0 : -1;
    }

    // No leading or trailing zeroes
    unsigned char* buffer = bigdec->buffer;
    assert(buffer[0] != 0);
    assert(buffer[length - 1] != 0);

    // Check if the decimal is outside the range of a uint32_t. In this case we
    // don't need to compare at all.
    int exponent = bigdec->exponent;
    int integer_digits = exponent + length;
    if (integer_digits < 0) {
        return -1;
    }
    if (integer_digits > 10) {
        return 1;
    }

    // Compare integer portion of the value
    uint32_t uint = __bigdec_uint(bigdec, false);
    if (uint < value) {
        return -1;
    }
    if (uint > value) {
        return 1;
    }

    // Integer portion is equal. Decimal is larger if it has a fraction.
    return (length > (size_t)integer_digits) ? 1 : 0;
}
