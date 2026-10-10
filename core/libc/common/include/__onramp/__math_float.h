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

#ifndef __ONRAMP_LIBC_ONRAMP_MATH_FLOAT_H_INCLUDED
#define __ONRAMP_LIBC_ONRAMP_MATH_FLOAT_H_INCLUDED

/*
 * Some float stuff seems to have moved from <math.h> to <float.h> in C23. They
 * are still in <math.h> though, just deprecated, so we need to include them in
 * both.
 */

// INFINITY must be of type float.
#define INFINITY __builtin_bit_cast(float, 0b0'11111111'00000000000000000000000)

// NAN must be quiet and of type float. In Onramp the high bit of the mantissa
// is the quiet bit.
#define NAN __builtin_bit_cast(float, 0b0'11111111'10000000000000000000000)

#endif
