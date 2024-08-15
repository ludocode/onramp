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

#ifndef __ONRAMP_LIBC_STDCKDINT_H_INCLUDED
#define __ONRAMP_LIBC_STDCKDINT_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

#define __STDC_VERSION_STDCKDINT_H__ 202311L

#define ckd_add(out, a, b) (__builtin_add_overflow((a), (b), (out)))
#define ckd_sub(out, a, b) (__builtin_sub_overflow((a), (b), (out)))
#define ckd_mul(out, a, b) (__builtin_mul_overflow((a), (b), (out)))



#if 0

// TODO this is going to be a pain to implement in plain C with generics
// because the types need not be the same width or signedness.
//bool ckd_add(type1 *result, type2 a, type3 b);
//bool ckd_sub(type1 *result, type2 a, type3 b);
//bool ckd_mul(type1 *result, type2 a, type3 b);
// TODO probably delete all this, just make the same builtins as gcc


bool __ckd_add_u8_uu (unsigned char*      out, unsigned long long a, unsigned long long b);
bool __ckd_add_u16_uu(unsigned short*     out, unsigned long long a, unsigned long long b);
bool __ckd_add_u32_uu(unsigned int*       out, unsigned long long a, unsigned long long b);
bool __ckd_add_u64_uu(unsigned long long* out, unsigned long long a, unsigned long long b);

bool __ckd_add_u8_su (unsigned char*      out,          long long a, unsigned long long b);
bool __ckd_add_u16_su(unsigned short*     out,          long long a, unsigned long long b);
bool __ckd_add_u32_su(unsigned int*       out,          long long a, unsigned long long b);
bool __ckd_add_u64_su(unsigned long long* out,          long long a, unsigned long long b);

bool __ckd_add_u8_ss (unsigned char*      out,          long long a,          long long b);
bool __ckd_add_u16_ss(unsigned short*     out,          long long a,          long long b);
bool __ckd_add_u32_ss(unsigned int*       out,          long long a,          long long b);
bool __ckd_add_u64_ss(unsigned long long* out,          long long a,          long long b);

bool __ckd_add_s8_uu (unsigned char*      out, unsigned long long a, unsigned long long b);
bool __ckd_add_s16_uu(unsigned short*     out, unsigned long long a, unsigned long long b);
bool __ckd_add_s32_uu(unsigned int*       out, unsigned long long a, unsigned long long b);
bool __ckd_add_s64_uu(unsigned long long* out, unsigned long long a, unsigned long long b);

bool __ckd_add_s8_su (unsigned char*      out,          long long a, unsigned long long b);
bool __ckd_add_s16_su(unsigned short*     out,          long long a, unsigned long long b);
bool __ckd_add_s32_su(unsigned int*       out,          long long a, unsigned long long b);
bool __ckd_add_s64_su(unsigned long long* out,          long long a, unsigned long long b);

bool __ckd_add_s8_ss (unsigned char*      out,          long long a,          long long b);
bool __ckd_add_s16_ss(unsigned short*     out,          long long a,          long long b);
bool __ckd_add_s32_ss(unsigned int*       out,          long long a,          long long b);
bool __ckd_add_s64_ss(unsigned long long* out,          long long a,          long long b);

#define ckd_add(out, a, b) \
    _Generic()...

#define ckd_sub(out, a, b) \

#endif


#endif
