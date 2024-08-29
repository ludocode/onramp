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
#include "u64.h"
#include "common.h"

struct type_t;
struct token_t;

typedef enum symbol_kind_t {
    symbol_kind_variable,
    symbol_kind_function,
    symbol_kind_constant,
    symbol_kind_builtin,
} symbol_kind_t;

typedef enum symbol_linkage_t {
    symbol_linkage_none = 0,
    symbol_linkage_internal,
    symbol_linkage_external,
} symbol_linkage_t;

/**
 * A symbol is a variable, a function, a constant, or a builtin.
 */
typedef struct symbol_t {
    int refcount;

    symbol_kind_t kind;
    struct type_t* type; // null for builtins; parser fills in the type
    struct token_t* token;
    string_t* name;     // name in C; always non-null, empty string if anonymous
    string_t* asm_name; // name in assembly; null if this is not a global

    // Offset in the stack frame if this is a local variable. This can be
    // positive (e.g. function argument) or negative (e.g. normal variable.)
    // This is ignored for variables with linkage.
    int offset;

    symbol_linkage_t linkage;

    bool is_weak : 1;
    bool is_defined : 1;
    bool is_tentative : 1;

    bool is_constructor : 1;
    bool is_destructor : 1;
    int constructor_priority;
    int destructor_priority;

    union {
        uint32_t u32; // float or int/short/char
        u64_t u64;    // double or long long
        builtin_t builtin;
    };
} symbol_t;

symbol_t* symbol_new(symbol_kind_t kind, struct type_t* type,
        struct token_t* name, string_t* /*nullable*/ asm_name);

static inline bool symbol_is_global(symbol_t* symbol) {
    return symbol->linkage != symbol_linkage_none;
}

static inline symbol_t* symbol_ref(symbol_t* symbol) {
    ++symbol->refcount;
    return symbol;
}

void symbol_deref(symbol_t* symbol);

void symbol_create_builtins(void);

#endif
