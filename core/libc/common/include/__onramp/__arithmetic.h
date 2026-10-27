/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2026 Fraser Heavy Software
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

#ifndef ONRAMP_STANDALONE_TEST
#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif
#endif

unsigned* __llong_add(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_sub(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_mul(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_divu(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_divs(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_modu(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_mods(unsigned* out, const unsigned* a, const unsigned* b);
void __llong_divmodu(unsigned* __quotient_out, unsigned* /*nullable*/ __remainder_out,
        const unsigned* __dividend, const unsigned* __divisor);
unsigned* __llong_shl(unsigned* out, const unsigned* a, unsigned bits);
unsigned* __llong_shru(unsigned* out, const unsigned* a, unsigned bits);
unsigned* __llong_shrs(unsigned* out, const unsigned* a, unsigned bits);
unsigned* __llong_and(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_or(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_xor(unsigned* out, const unsigned* a, const unsigned* b);
unsigned* __llong_compl(unsigned* out, const unsigned* src);
unsigned* __llong_negate(unsigned* out, const unsigned* src);
_Bool __llong_ltu(const unsigned* a, const unsigned* b);
_Bool __llong_lts(const unsigned* a, const unsigned* b);
_Bool __llong_ne(const unsigned* a, const unsigned* b);

unsigned __float_to_u32(unsigned __f);
int __float_to_i32(unsigned __f);
unsigned* __float_to_u64(unsigned* __out, unsigned __f);
unsigned* __float_to_i64(unsigned* __out, unsigned __f);
unsigned __float_from_u32(unsigned __u);
unsigned __float_from_i32(int __i);
unsigned __float_from_u64(const unsigned* __u);
unsigned __float_from_i64(const unsigned* __i);

unsigned __float_add(unsigned __l, unsigned __r);
unsigned __float_sub(unsigned __l, unsigned __r);
unsigned __float_mul(unsigned __l, unsigned __r);
unsigned __float_div(unsigned __l, unsigned __r);
unsigned __float_negate(unsigned __f);

_Bool __float_ne(unsigned a, unsigned b);
_Bool __float_lt(unsigned a, unsigned b);
_Bool __float_lte(unsigned a, unsigned b);

void __float_sigfpe(void);

unsigned* __double_from_float(unsigned* __d, unsigned __f);
unsigned __double_to_float(const unsigned* __d);

unsigned __double_to_u32(const unsigned* __d);
int __double_to_i32(const unsigned* __d);
unsigned* __double_to_u64(unsigned* __out, const unsigned* __d);
unsigned* __double_to_i64(unsigned* __out, const unsigned* __d);
unsigned* __double_from_u32(unsigned* __d, unsigned __u);
unsigned* __double_from_i32(unsigned* __d, int __i);
unsigned* __double_from_u64(unsigned* __d, const unsigned* __u);
unsigned* __double_from_i64(unsigned* __d, const unsigned* __i);

unsigned* __double_add(unsigned* __d, const unsigned* __l, const unsigned* __r);
unsigned* __double_sub(unsigned* __d, const unsigned* __l, const unsigned* __r);
unsigned* __double_mul(unsigned* __d, const unsigned* __l, const unsigned* __r);
unsigned* __double_div(unsigned* __d, const unsigned* __l, const unsigned* __r);
unsigned* __double_negate(unsigned* __out, const unsigned* __in);

_Bool __double_ne(const unsigned* __l, const unsigned* __r);
_Bool __double_lt(const unsigned* __l, const unsigned* __r);
_Bool __double_lte(const unsigned* __l, const unsigned* __r);

#endif
