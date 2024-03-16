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

#include <stdlib.h>

/*
 * rand()
 *
 * An affine congruential PRNG (typically called linear in the literature,
 * though this is a misnomer.)
 */

static unsigned rand_seed = 1;

int rand(void) {
    return rand_r(&rand_seed);
}

void srand(unsigned seed) {
    rand_seed = seed;
}

int rand_r(unsigned* seed) {
    // The first number in the m=2^32 category in Table 4 of the errata to
    // "Tables of Linear Congruential Generators of Different Sizes and Good
    // Lattice Structure" by Steele & Vigna
    //     https://arxiv.org/abs/2001.05304
    //     https://www.iro.umontreal.ca/~lecuyer/myftp/papers/latrules99Errata.pdf
    const unsigned a = 2891336453u;
    const unsigned c = 1u;
    *seed = a * *seed + c; // mod 2^32 is implied
    return *seed & RAND_MAX;
}
