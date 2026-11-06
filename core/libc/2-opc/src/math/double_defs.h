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

#ifndef DOUBLE_DEFS_H_INCLUDED
#define DOUBLE_DEFS_H_INCLUDED

/*
 * ops
 */
unsigned* __double_from_float(unsigned* d, unsigned f);
unsigned __double_to_float(const unsigned* d);
unsigned __double_to_u32(const unsigned* d);
int __double_to_i32(const unsigned* d);
unsigned* __double_to_u64(unsigned* u, const unsigned* d);
unsigned* __double_to_i64(unsigned* i, const unsigned* d);
unsigned* __double_from_u32(unsigned* d, unsigned u);
unsigned* __double_from_i32(unsigned* d, int i);
unsigned* __double_from_u64(unsigned* d, const unsigned* u);
unsigned* __double_from_i64(unsigned* d, const unsigned* i);
unsigned* __double_add(unsigned* d, const unsigned* l, const unsigned* r);
unsigned* __double_sub(unsigned* d, const unsigned* l, const unsigned* r);
unsigned* __double_mul(unsigned* d, const unsigned* l, const unsigned* r);
unsigned* __double_div(unsigned* d, const unsigned* l, const unsigned* r);
unsigned* __double_negate(unsigned* out, const unsigned* in);
_Bool __double_ne(const unsigned* l, const unsigned* r);
_Bool __double_lt(const unsigned* l, const unsigned* r);
_Bool __double_lte(const unsigned* l, const unsigned* r);
void strtod(unsigned* out, const char* restrict str, char** /*nullable*/ restrict out_end);

/*
 * funcs
 */
void sqrt(unsigned* out, unsigned* v);
void cbrt(unsigned* out, unsigned* v);
void hypot(unsigned* out, unsigned* x, unsigned* y);
void pow(unsigned* out, unsigned* x, unsigned* y);
void log(unsigned* out, unsigned* v);
void log1p(unsigned* out, unsigned* v);
void log2(unsigned* out, unsigned* v);
void log10(unsigned* out, unsigned* v);
void exp(unsigned* out, unsigned* v);
void expm1(unsigned* out, unsigned* v);
void exp2(unsigned* out, unsigned* v);
void sin(unsigned* out, unsigned* v);
void cos(unsigned* out, unsigned* v);
void tan(unsigned* out, unsigned* v);
void asin(unsigned* out, unsigned* v);
void acos(unsigned* out, unsigned* v);
void atan(unsigned* out, unsigned* v);
void atan2(unsigned* out, unsigned* x, unsigned* y);
void sinh(unsigned* out, unsigned* v);
void cosh(unsigned* out, unsigned* v);
void tanh(unsigned* out, unsigned* v);
void asinh(unsigned* out, unsigned* v);
void acosh(unsigned* out, unsigned* v);
void atanh(unsigned* out, unsigned* v);
void erf(unsigned* out, unsigned* v);
void erfc(unsigned* out, unsigned* v);
void tgamma(unsigned* out, unsigned* v);
void lgamma(unsigned* out, unsigned* v);

/*
 * util
 */
void ldexp(unsigned* out, unsigned* v, int exp);
void logb(unsigned* out, unsigned* v);
void frexp(unsigned* out, unsigned* v, int* exp);
void fabs(unsigned* out, unsigned* v);
void fmod(unsigned* out, unsigned* dividend, unsigned* divisor);
void remainder(unsigned* out, unsigned* dividend, unsigned* divisor);
void remquo(unsigned* out, unsigned* dividend, unsigned* divisor,
        unsigned* partial_quotient);
void fma(unsigned* out, unsigned* factor1, unsigned* factor2, unsigned* addend);
void fmax(unsigned* out, unsigned* x, unsigned* y);
void fmin(unsigned* out, unsigned* x, unsigned* y);
void fdim(unsigned* out, unsigned* x, unsigned* y);
void nan(unsigned* out, const char* __repr);
void modf(unsigned* out, unsigned* v, unsigned* integer_part);
int ilogb(unsigned* v);
void nextafter(unsigned* out, unsigned* from, unsigned* to);
void nexttoward(unsigned* out, unsigned* from, unsigned* to);
void ceil(unsigned* out, unsigned* v);
void floor(unsigned* out, unsigned* v);
void trunc(unsigned* out, unsigned* v);
void round(unsigned* out, unsigned* v);
long lround(unsigned* v);
void llround(unsigned* ll_out, unsigned* v);
void nearbyint(unsigned* out, unsigned* v);
void rint(unsigned* out, unsigned* v);
long lrint(unsigned* v);
void llrint(unsigned* ll_out, unsigned* v);

#endif
