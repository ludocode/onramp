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

#ifndef SYMBOL_H_INCLUDED
#define SYMBOL_H_INCLUDED

#include <limits.h>

#include "libo-string.h"

struct type_t;
struct token_t;

typedef enum symbol_kind_t {
    symbol_kind_variable,
    symbol_kind_function,
    symbol_kind_constant,
} symbol_kind_t;

#define SYMBOL_OFFSET_GLOBAL INT_MAX

/**
 * A symbol is a variable, a function or a constant.
 */
typedef struct symbol_t {
    int refcount;

    symbol_kind_t kind;
    struct type_t* type;
    struct token_t* token;
    string_t* name;     // name in C; always non-null, empty string if anonymous
    string_t* asm_name; // name in assembly; null if this is not a global

    // Offset in the stack frame if this is a local variable. This can be
    // positive or negative. (Positive values are used for function arguments
    // that are passed by the caller on the stack.) For local variables this is
    // 0 until assigned at code generation time. For global variables this is
    // SYMBOL_OFFSET_GLOBAL.
    int offset;

    bool is_tentative : 1;
    bool is_extern : 1;
    bool is_static : 1;

    union {
        // TODO llong, float
        int i;
    } constant;
} symbol_t;

symbol_t* symbol_new(symbol_kind_t kind, struct type_t* type,
        struct token_t* name, string_t* /*nullable*/ asm_name);

static inline bool symbol_is_global(symbol_t* symbol) {
    return symbol->offset == SYMBOL_OFFSET_GLOBAL;
}

static inline symbol_t* symbol_ref(symbol_t* symbol) {
    ++symbol->refcount;
    return symbol;
}

void symbol_deref(symbol_t* symbol);

#endif
