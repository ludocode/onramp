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

#ifndef MACRO_H_INCLUDED
#define MACRO_H_INCLUDED

#include "libo-vector.h"
#include "libo-table.h"
#include "libo-string.h"
#include "token.h"

struct stream_t;

/**
 * A macro.
 *
 * Although macros are stored in a macro table, they are also
 * reference-counted. The reason is because it's possible to `#undef` a macro
 * inside its own argument list (see test `arg-directive-undef.c`.) We store a
 * strong reference to a macro while expanding it in case the macro is
 * undefined before we're done.
 */
typedef struct macro_t {
    table_entry_t entry;
    unsigned refcount;
    token_t* name;
    vector_t* params; // contains string_t*. null if object-like macro. does not include variadic param.
    bool is_variadic;

    // A macro can expand to either an expansion list (for macros defined with
    // #define) or a builtin function (e.g. __COUNTER__, has_include(), etc.)
    // Only one of these will be used; if the function is non-null, the
    // expansion list is ignored and must be empty.
    vector_t expansion; // a list of strong references to tokens
    void (*function)(struct stream_t* input, vector_t* /*nullable*/ output);
} macro_t;

void macro_setup(void);
void macro_teardown(void);

void macro_define_builtins(void);

/**
 * TODO hide this
 *
 * TODO this does not return a strong reference right now, the macro is held by
 * the table. It shouldn't be called "new".
 */
macro_t* macro_new(token_t* name);

/**
 * Defines a new macro, parsing its arguments and expansion from the given
 * lexer.
 *
 * This is called on a `#define` directive as well as from the `-D` option on
 * the command-line.
 */
void macro_define(struct stream_t* stream);

static inline macro_t* macro_ref(macro_t* macro) {
    //printf("ref'ing macro %s\n",macro->name->value->bytes);
    ++macro->refcount;
    return macro;
}

void macro_deref(macro_t* macro);

/**
 * Undefines a macro with the given name if any.
 */
void macro_undef(string_t* name);

/**
 * Appends a token to the macro's expansion list.
 */
void macro_append(macro_t* macro, token_t* token);

static inline size_t macro_hash(macro_t* macro) {
    return string_hash(macro->name->value);
}

/**
 * Performs macro expansion on the given token and all tokens it expands to
 * recursively, outputting the fully expanded tokens.
 *
 * Tokens can be outputted to the output file or, in the case of an `#if` or
 * `#include`, to an output buffer for furthur processing (e.g. for expression
 * evaluation.)
 *
 * If the given token is a macro with arguments that contain preprocessor
 * directives, the directives will be parsed before the macro is expanded.
 */
void macro_expand(struct stream_t* stream,
        vector_t* /*nullable*/ output, token_t* token);

macro_t* macro_find(string_t* name);

/**
 * Finds the index of the given parameter, or returns -1 if it is not a
 * parameter.
 */
int macro_param(macro_t* macro, token_t* token);

#endif
