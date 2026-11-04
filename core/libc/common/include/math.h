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

#ifndef __ONRAMP_LIBC_MATH_H_INCLUDED
#define __ONRAMP_LIBC_MATH_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

#include <__onramp/__math_float.h>

#define HUGE_VALF INFINITY
#define HUGE_VAL ((double)INFINITY)
#define HUGE_VALL ((long double)INFINITY)

#define FP_INFINITE 1
#define FP_NAN 2
#define FP_NORMAL 3
#define FP_SUBNORMAL 4
#define FP_ZERO 5 // not zero



/*
 * Classification
 */

#ifndef __onramp_cpp_omc__

/*
 * These classification macros should use _Generic or should be builtins. For
 * now we only support floats.
 *
 * They are required to evaluate to ints where non-zero means true. On Onramp
 * they always return 0 or 1.
 */

// TODO these are C99

#ifndef __ONRAMP_LIBC_FLOATING_IMPL
int __float_fpclassify(float __x);
int __float_signbit(float __x);
int __float_signbit(float __x);
int __float_issignaling(float __x);
#endif

#define fpclassify(x) __float_fpclassify(x)
#define signbit(x) __float_signbit(x)

#define isinf(x) ((int)(__float_fpclassify(x) == FP_INFINITE))
#define isnan(x) ((int)(__float_fpclassify(x) == FP_NAN))
#define isnormal(x) ((int)(__float_fpclassify(x) == FP_NORMAL)) // does not include zero

#define isfinite(x) ((int) __extension__ ({ \
            int __c = float_fpclassify(x); __c != FP_INFINITE && __c != FP_NAN }))

// TODO these are C23

#define issignaling(x) __float_issignaling(x)
#define iszero(x) ((int)(__float_fpclassify(x) == FP_ZERO))
#define issubnormal(x) ((int)(__float_fpclassify(x) == FP_SUBNORMAL))

#endif



// TODO C99 quiet macros isless(), isgreater(), etc.

#ifndef __ONRAMP_LIBC_FLOATING_IMPL

/*
 * Basic functions
 */

float fabsf(float __v);
double fabs(double __v);
long double fabsl(long double __v);

float fmodf(float __dividend, float __divisor);
double fmod(double __dividend, double __divisor);
long double fmodl(long double __dividend, long double __divisor);

float remainderf(float __dividend, float __divisor);
double remainder(double __dividend, double __divisor);
long double remainderl(long double __dividend, long double __divisor);

float remquof(float __dividend, float __divisor,
        float* __partial_quotient);
double remquo(double __dividend, double __divisor,
        double* __partial_quotient);
long double remquol(long double __dividend, long double __divisor,
        long double* __partial_quotient);

float fmaf(float __factor1, float __factor2, float __addend);
double fma(double __factor1, double __factor2, double __addend);
long double fmal(long double __factor1, long double __factor2,
        long double __addend);

float fmaxf(float __x, float __y);
double fmax(double __x, double __y);
long double fmaxl(long double __x, long double __y);

float fminf(float __x, float __y);
double fmin(double __x, double __y);
long double fminl(long double __x, long double __y);

float fdimf(float __x, float __y);
double fdim(double __x, double __y);
long double fdiml(long double __x, long double __y);

float nanf(const char* __repr);
double nan(const char* __repr);
long double nanl(const char* __repr);


/*
 * Power functions
 */

float sqrtf(float __v);
double sqrt(double __v);
long double sqrtl(long double __v);

float cbrtf(float __v);
double cbrt(double __v);
long double cbrtl(long double __v);

float hypotf(float __x, float __y);
double hypot(double __x, double __y);
long double hypotl(long double __x, long double __y);

float powf(float __base, float __exponent);
double pow(double __base, double __exponent);
long double powl(long double __base, long double __exponent);

/*
 * Exponential functions
 */

float logf(float __v);
double log(double __v);
long double logl(long double __v);

float log1pf(float __v);
double log1p(double __v);
long double log1pl(long double __v);

float log2f(float __v);
double log2(double __v);
long double log2l(long double __v);

float log10f(float __v);
double log10(double __v);
long double log10l(long double __v);

float expf(float __v);
double exp(double __v);
long double expl(long double __v);

float exp1mf(float __v);
double exp1m(double __v);
long double exp1ml(long double __v);

float exp2f(float __v);
double exp2(double __v);
long double exp2l(long double __v);

/*
 * Utility functions
 */

float frexpf(float __v, int __exp);
double frexp(double __v, int __exp);
long double frexpl(long double __v, int __exp);

float ldexpf(float __v, int __exp);
double ldexp(double __v, int __exp);
long double ldexpl(long double __v, int __exp);

float modff(float __v, float* __integer_part);
double modf(double __v, double* __integer_part);
long double modfl(long double __v, long double* __integer_part);

float logbf(float __v);
double logb(double __v);
long double logbl(long double __v);

int ilogbf(float __v);
int ilogb(double __v);
int ilogbl(long double __v);

float nextafterf(float __from, float __to);
double nextafter(double __from, double __to);
long double nextafterl(long double __from, long double __to);

float nexttowardf(float __from, long double __to);
double nexttoward(double __from, long double __to);
long double nexttowardl(long double __from, long double __to);

float copysignf(float __magnitude, float __sign);
double copysign(double __magnitude, double __sign);
long double copysign(long double __magnitude, long double __sign);

#ifndef __onramp_cci_omc__
#ifndef __onramp_cci_opc__
float scalbnf(float __v, int __exp) __asm__("ldexpf");
double scalbn(double __v, int __exp) __asm__("ldexp");
long double scalbnl(long double __v, int __exp) __asm__("ldexpl");

float scalblnf(float __v, long __exp) __asm__("ldexpf");
double scalbln(double __v, long __exp) __asm__("ldexp");
long double scalblnl(long double __v, long __exp) __asm__("ldexpl");
#endif
#endif

/*
 * Trigonometric functions
 */

float sinf(float __v);
double sin(double __v);
long double sinl(long double __v);

float cosf(float __v);
double cos(double __v);
long double cosl(long double __v);

float tanf(float __v);
double tan(double __v);
long double tanl(long double __v);

float asinf(float __v);
double asin(double __v);
long double asinl(long double __v);

float acosf(float __v);
double acos(double __v);
long double acosl(long double __v);

float atanf(float __v);
double atan(double __v);
long double atanl(long double __v);

float atan2f(float __x, float __y);
double atan2(double __x, double __y);
long double atan2l(long double __x, long double __y);

/*
 * Hyperbolic functions
 */

float sinhf(float __v);
double sinh(double __v);
long double sinhl(long double __v);

float coshf(float __v);
double cosh(double __v);
long double coshl(long double __v);

float tanhf(float __v);
double tanh(double __v);
long double tanhl(long double __v);

float asinhf(float __v);
double asinh(double __v);
long double asinhl(long double __v);

float acoshf(float __v);
double acosh(double __v);
long double acoshl(long double __v);

float atanhf(float __v);
double atanh(double __v);
long double atanhl(long double __v);

/*
 * Rounding functions
 */

float ceilf(float __v);
double ceil(double __v);
long double ceill(long double __v);

float floorf(float __v);
double floor(double __v);
long double floorl(long double __v);

float truncf(float __v);
double trunc(double __v);
long double truncl(long double __v);

float roundf(float __v);
double round(double __v);
long double roundl(long double __v);

long lroundf(float __v);
long lround(double __v);
long lroundl(long double __v);

long long llroundf(float __v);
long long llround(double __v);
long long llroundl(long double __v);

float nearbyintf(float __v);
double nearbyint(double __v);
long double nearbyintl(long double __v);

float rintf(float __v);
double rint(double __v);
long double rintl(long double __v);

long lrintf(float __v);
long lrint(double __v);
long lrintl(long double __v);

long long llrintf(float __v);
long long llrint(double __v);
long long llrintl(long double __v);

/*
 * Error and gamma
 */

float erff(float __v);
double erf(double __v);
long double erfl(long double __v);

float erfcf(float __v);
double erfc(double __v);
long double erfcl(long double __v);

float tgammaf(float __v);
double tgamma(double __v);
long double tgammal(long double __v);

float lgammaf(float __v);
double lgamma(double __v);
long double lgammal(long double __v);

#endif

#endif
