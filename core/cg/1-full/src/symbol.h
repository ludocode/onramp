/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025-2026 Fraser Heavy Software
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

#include <stdbool.h>
#include <stddef.h>

struct block_t;
struct location_t;
struct otable_t;
struct temporary_t;
struct variable_t;
struct vector_t;

#define PRIORITY_INVALID ((size_t)-1)

/**
 * All data for the current symbol being compiled.
 *
 * This is mostly gathered together as a convenience so we don't have stuff
 * scattered everywhere.
 *
 * TODO actually currently we are using globals, this is pretty useless
 */
typedef struct symbol_t {
    char* name; // TODO string_t
    struct location_t* location;
    bool is_static;
    bool is_data;

    struct vector_t* parameters; // list of temporaries, contains NULLs for sentinels
    struct temporary_t* varargs; // varargs temporary or null
    struct vector_t* blocks;
    size_t frame_size;
    size_t constructor_priority;
    size_t destructor_priority;

    struct otable_t* substitutions; // keyed by temporary
} symbol_t;

/**
 * Creates a symbol.
 *
 * Takes ownership of the given location.
 */
symbol_t* symbol_new(struct location_t* location, bool is_static);

void symbol_delete(symbol_t* symbol);

/**
 * Adds a variable as a substitution for the given temporary.
 *
 * This is used for defined parameters and variables (`param`, `varargs` and
 * `var` in the preamble.)
 *
 * The temporary will be substituted with the variable wherever it appears.
 */
void symbol_add_substitution(symbol_t* symbol, struct temporary_t* temporary,
        struct variable_t* variable);

/**
 * Finds the variable to substitute for the given temporary, or NULL if this
 * temporary does not represent a preamble variable.
 */
struct variable_t* /*nullable*/ symbol_find_substitution(
        symbol_t* symbol, struct temporary_t* temporary);

#endif
