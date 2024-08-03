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

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stdbool.h>
#include <stdarg.h>

#include "libo-string.h"

#define JUMP_LABEL_PREFIX         "_Lx"
#define STRING_LABEL_PREFIX       "_Sx"
#define INITIALIZER_LABEL_PREFIX  "_Ix"
#define USER_LABEL_PREFIX         "_U_"

#define ASM_INDENT "  "

struct token_t;

/**
 * Prints a fatal error message at the location of the given token.
 */
_Noreturn
void fatal_token(struct token_t* token, const char* format, ...);

/**
 * Prints a fatal error message at the location of the given token.
 */
_Noreturn
void vfatal_token(struct token_t* token, const char* format, va_list args);

_Noreturn
void fatal(const char* format, ...);

// TODO these may be in libc in C2x, should move them there
bool is_pow2(int n);
// TODO fls() used in generate_ops

#endif
