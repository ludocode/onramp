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

/*
 * Functions for 64-bit math.
 *
 * For all functions, the source pointers are allowed to match the destination
 * and each other. We make sure to read all the data we need before writing any
 * out.
 */

#include <stdbool.h>

#ifdef __onramp__
    extern void __fatal(const char* message);
#endif
#ifndef __onramp__
    // workarounds for testing with another compiler
    #include <stdio.h>
    #include <stdlib.h>
    static void __fatal(const char* message) {
        fputs(message, stderr);
        fputc('\n', stderr);
        exit(1);
    }
#endif

void __64_add(unsigned* out, const unsigned* a, const unsigned* b) {
    unsigned a0 = *a;
    unsigned a1 = *(a + 1);
    unsigned b0 = *b;
    unsigned b1 = *(b + 1);
    unsigned o0 = (a0 + b0);
    unsigned o1 = (a1 + b1);
    if (o0 < a0) {
        o1 = (o1 + 1);
    }
    *(out) = o0;
    *(out + 1) = o1;
}

void __64_sub(unsigned* out, const unsigned* a, const unsigned* b) {
    unsigned a0 = *a;
    unsigned a1 = *(a + 1);
    unsigned b0 = *b;
    unsigned b1 = *(b + 1);
    unsigned o0 = (a0 - b0);
    unsigned o1 = (a1 - b1);
    if (o0 > a0) {
        o1 = (o1 - 1);
    }
    *(out) = o0;
    *(out + 1) = o1;
}

void __64_mul(unsigned* out, const unsigned* a, const unsigned* b) {
    unsigned a0 = *a;
    unsigned a1 = *(a + 1);
    unsigned b0 = *b;
    unsigned b1 = *(b + 1);

    // long multiplication base 2^16

    unsigned ah0;
    unsigned ah1;
    unsigned ah2;
    unsigned ah3;
    ah0 = (a0 & 0xffff);
    ah1 = ((a0 >> 16) & 0xffff);
    ah2 = (a1 & 0xffff);
    ah3 = ((a1 >> 16) & 0xffff);

    unsigned bh0;
    unsigned bh1;
    unsigned bh2;
    unsigned bh3;
    bh0 = (b0 & 0xffff);
    bh1 = ((b0 >> 16) & 0xffff);
    bh2 = (b1 & 0xffff);
    bh3 = ((b1 >> 16) & 0xffff);

    unsigned t00 = (ah0 * bh0);
    unsigned t01 = (ah0 * bh1);
    unsigned t02 = (ah0 * bh2);
    unsigned t03 = (ah0 * bh3);
    unsigned t10 = (ah1 * bh0);
    unsigned t11 = (ah1 * bh1);
    unsigned t12 = (ah1 * bh2);
    unsigned t20 = (ah2 * bh0);
    unsigned t21 = (ah2 * bh1);
    unsigned t30 = (ah3 * bh0);

    unsigned o0 = t00;
    unsigned o1 = 0;

    unsigned temp = (o0 + (t01 << 16));
    if (temp < o0) {
        o1 = (o1 + 1);
    }
    o0 = temp;
    o1 = (o1 + (t01 >> 16));

    temp = (o0 + (t10 << 16));
    if (temp < o0) {
        o1 = (o1 + 1);
    }
    o0 = temp;
    o1 = (o1 + (t10 >> 16));

    o1 = (o1 + t20);
    o1 = (o1 + t11);
    o1 = (o1 + t02);

    o1 = (o1 + (t03 << 16));
    o1 = (o1 + (t12 << 16));
    o1 = (o1 + (t21 << 16));
    o1 = (o1 + (t30 << 16));

    *(out) = o0;
    *(out + 1) = o1;
}

void __64_divs(unsigned* out, const unsigned* a, const unsigned* b) {
    __fatal("64-bit divide is not yet implemented.");
}

void __64_divu(unsigned* out, const unsigned* a, const unsigned* b) {
    __fatal("64-bit divide is not yet implemented.");
}

void __64_mods(unsigned* out, const unsigned* a, const unsigned* b) {
    __fatal("64-bit modulus is not yet implemented.");
}

void __64_modu(unsigned* out, const unsigned* a, const unsigned* b) {
    __fatal("64-bit modulus is not yet implemented.");
}

bool __64_ltu(const unsigned* a, const unsigned* b) {
    unsigned a1 = *(a + 1);
    unsigned b1 = *(b + 1);
    if (a1 < b1) {
        return true;
    }
    if (a1 != b1) {
        return false;
    }
    return *a < *b;
}

bool __64_gtu(const unsigned* a, const unsigned* b) {
    unsigned a1 = *(a + 1);
    unsigned b1 = *(b + 1);
    if (a1 > b1) {
        return true;
    }
    if (a1 != b1) {
        return false;
    }
    return *a > *b;
}

void __64_shl(unsigned* out, const unsigned* a, int bits) {
    if (bits >= 32) {
        if (bits == 32) {
            *out = 0;
            *(out + 1) = *a;
            return;
        }

        *out = 0;
        *(out + 1) = (*a << (bits - 32));
        return;
    }

    unsigned a0 = *a;
    unsigned a1 = *(a + 1);

    if (bits == 0) {
        *out = a0;
        *(out + 1) = a1;
        return;
    }

    *out = (a0 << bits);
    *(out + 1) = ((a1 << bits) | (a0 >> (32 - bits)));
}

void __64_shru(unsigned* out, const unsigned* a, int bits) {
    if (bits >= 32) {
        if (bits == 32) {
            *(out + 1) = 0;
            *out = *(a + 1);
            return;
        }

        *(out + 1) = 0;
        *out = (*(a + 1) >> (bits - 32));
        return;
    }

    unsigned a0 = *a;
    unsigned a1 = *(a + 1);

    if (bits == 0) {
        *out = a0;
        *(out + 1) = a1;
        return;
    }

    *(out + 1) = (a1 >> bits);
    *out = ((a1 << (32 - bits)) | (a0 >> bits));
}

void __64_shrs(int* out, int* a, int bits) {
    int a0 = *a;
    int a1 = *(a + 1);

    if (bits < 32) {
        if (bits == 0) {
            *out = a0;
            *(out + 1) = a1;
            return;
        }

        *(out + 1) = (a1 >> bits);
        *out = (((unsigned)a1 << (32 - bits)) | ((unsigned)a0 >> bits));
        return;
    }

    // Rather than trying to figure out the sign, we just shift twice to fill
    // with the sign bit. This is probably way slower than it needs to be.
    // Maybe we could add an assembly instruction to do this.
    int sign = ((a1 >> 16) >> 16);

    if (bits == 32) {
        *(out + 1) = sign;
        *out = a1;
        return;
    }

    *(out + 1) = sign;
    *out = (a1 >> (bits - 32));
}

void __64_and(unsigned* out, const unsigned* a, const unsigned* b) {
    *out = (*a & *b);
    *(out + 1) = (*(a + 1) & *(b + 1));
}

void __64_or(unsigned* out, const unsigned* a, const unsigned* b) {
    *out = (*a | *b);
    *(out + 1) = (*(a + 1) | *(b + 1));
}

void __64_xor(unsigned* out, const unsigned* a, const unsigned* b) {
    *out = (*a ^ *b);
    *(out + 1) = (*(a + 1) ^ *(b + 1));
}

void __64_not(unsigned* out, const unsigned* src) {
    *out = ~*src;
    *(out + 1) = ~*(src + 1);
}
