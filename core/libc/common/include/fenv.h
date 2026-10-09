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

/**
 * Floating point environment.
 *
 * This is mostly untested and probably has lots of bugs.
 *
 * TODO access to this is supposed to require `#pragma STDC FENV_ACCESS ON` and
 * C99 or later. We should also allow it under -fgnu-extensions.
 *
 *     https://en.cppreference.com/c/numeric/fenv
 */

#ifndef __ONRAMP_LIBC_FENV_H_INCLUDED
#define __ONRAMP_LIBC_FENV_H_INCLUDED

#define FE_DIVBYZERO    (1u << 0u)
#define FE_INEXACT      (1u << 1u)
#define FE_INVALID      (1u << 2u)
#define FE_OVERFLOW     (1u << 3u)
#define FE_UNDERFLOW    (1u << 4u)
#define FE_ALL_EXCEPT  ((1u << 5u) - 1u)

#define __FE_ROUND_SHIFT 29u
#define __FE_ROUND_MASK (~((1u << __FE_ROUND_SHIFT) - 1u))
#define __FE_ROUND_MAX 3u

#define FE_TONEAREST   (0u << __FE_ROUND_SHIFT)
#define FE_DOWNWARD    (1u << __FE_ROUND_SHIFT)
#define FE_UPWARD      (2u << __FE_ROUND_SHIFT)
#define FE_TOWARDZERO  (3u << __FE_ROUND_SHIFT)

#define FE_DFL_ENV 0u

extern unsigned __float_env;

typedef unsigned fexcept_t;
typedef unsigned fenv_t;

// TODO some of these are a bit big to be static inline, just doing it this way
// for now for simplicity. might want to move them if/when we add the option to
// raise(SIGFPE) on exceptions (e.g. glibc feenableexcept(), msvc __control87().)

static inline int feclearexcept(int exceptions) {
    __float_env &= ~((unsigned)exceptions & FE_ALL_EXCEPT);
    return 0;
}

static inline int fetestexcept(int exceptions) {
    return __float_env & (unsigned)exceptions & FE_ALL_EXCEPT;
}

static inline int feraiseexcept(int exceptions) {
    __float_env |= (unsigned)exceptions & FE_ALL_EXCEPT;
    return 0;
}

static inline int fegetexceptflag(fexcept_t* flags, int exceptions) {
    *flags =  __float_env & (unsigned)exceptions & FE_ALL_EXCEPT;
    return 0;
}

static inline int fesetexceptflag(const fexcept_t* flags, int exceptions) {
    __float_env &= ~((unsigned)exceptions & FE_ALL_EXCEPT);
    __float_env |= *flags & (unsigned)exceptions & FE_ALL_EXCEPT;
    return 0;
}

static inline int fegetround(void) {
    return __float_env >> __FE_ROUND_SHIFT;
}

static inline int fesetround(int round) {
    if ((unsigned)round > __FE_ROUND_MAX) {
        // invalid rounding mode
        return -1;
    }

    // TODO changing rounding mode is not supported yet
    //__float_env = __float_env & ~__FE_ROUND_MASK | round << __FE_ROUND_SHIFT;
    return -1;
}

static inline int fegetenv(fenv_t* env) {
    *env = __float_env;
    return 0;
}

static inline int fesetenv(const fenv_t* env) {
    if ((*env >> __FE_ROUND_SHIFT) > __FE_ROUND_MAX) {
        // invalid rounding mode
        return -1;
    }
    if (*env & ~(FE_ALL_EXCEPT | __FE_ROUND_MASK)) {
        // extraneous bits
        return -1;
    }
    __float_env = *env;
    return 0;
}

static inline int feholdexcept(fenv_t* env) {
    *env = __float_env;
    __float_env &= ~FE_ALL_EXCEPT;
    // TODO if we add the option to raise(SIGFPE), need to disable it here
    return 0;
}

static inline int feupdateenv(const fenv_t* env) {
    __float_env = (__float_env & FE_ALL_EXCEPT) | *env;
    return 0;
}

#endif
