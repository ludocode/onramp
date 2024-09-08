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
 * This is an internal Onramp header. It declares the libc functions for float,
 * double, and long long arithmetic. These are used by cci/2 so it can perform
 * 64-bit and floating point math without support from cci/1.
 */

#ifndef __ONRAMP_LIBC_ONRAMP_ARITHMETIC_H_INCLUDED
#define __ONRAMP_LIBC_ONRAMP_ARITHMETIC_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

unsigned* __llong_add(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_sub(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_mul(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_divu(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_divs(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_modu(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_mods(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_shl(unsigned* out, const unsigned* a, int bits);
unsigned* __llong_shru(unsigned* out, const unsigned* a, int bits);
unsigned* __llong_shrs(unsigned* out, const unsigned* a, int bits);
unsigned* __llong_and(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_or(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_xor(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_bit_not(unsigned* out, const unsigned* src);
_Bool __llong_ltu(const unsigned* a, const unsigned* b);
_Bool __llong_lts(const unsigned* a, const unsigned* b);
_Bool __llong_neq(const unsigned* a, const unsigned* b);

unsigned __float_add(unsigned a, unsigned b);
unsigned __float_sub(unsigned a, unsigned b);
unsigned __float_mul(unsigned a, unsigned b);
unsigned __float_divu(unsigned a, unsigned b);
unsigned __float_divs(unsigned a, unsigned b);
unsigned __float_modu(unsigned a, unsigned b);
unsigned __float_mods(unsigned a, unsigned b);
_Bool __float_ltu(unsigned a, unsigned b);
_Bool __float_lts(unsigned a, unsigned b);

unsigned* __double_add(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __double_sub(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __double_mul(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __double_divu(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __double_divs(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __double_modu(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __double_mods(unsigned* out, const unsigned* a, const unsigned* b);
_Bool __double_ltu(const unsigned* a, const unsigned* b);
_Bool __double_lts(const unsigned* a, const unsigned* b);

#endif
