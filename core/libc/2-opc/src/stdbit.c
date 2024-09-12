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

/*
 * Bit manipulation functions from C23.
 */

#include <stdbit.h>

// Eventually we will implement everything in C23, and the compiler builtins
// (e.g. __builtin_clz) will wrap these. For now we just implement what we need
// for other things (e.g. llong division.)

int stdc_leading_zerosui(unsigned int v) {
    if (v == 0)
        return 32;

    int r = 0;

    // This is an unrolling of:
    //     https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog

    if (v & 0xFFFF0000) {
        v >>= 16;
        r |= 16;
    }
    if (v & 0xFF00) {
        v >>= 8;
        r |= 8;
    }
    if (v & 0xF0) {
        v >>= 4;
        r |= 4;
    }
    if (v & 0xC) {
        v >>= 2;
        r |= 2;
    }
    if (v & 0x2) {
        v >>= 1;
        r |= 1;
    }

    return 31 - r;
}

int stdc_leading_zerosus(unsigned short value) {
    return stdc_leading_zerosui(value) - 16;
}

int stdc_leading_zerosuc(unsigned char value) {
    return stdc_leading_zerosui(value) - 24;
}
