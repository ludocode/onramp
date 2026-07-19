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

#include "symbol.h"

#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "libo-otable.h"
#include "libo-vector.h"
#include "location.h"
#include "variable.h"
#include "temporary.h"

typedef struct substitution_t {
    variable_t* variable;
    temporary_t* temporary;
} substitution_t;

symbol_t* symbol_new(location_t* location, bool is_static) {
    symbol_t* symbol = calloc(1, sizeof(symbol_t));
    symbol->location = location;
    symbol->is_static = is_static;
    symbol->parameters = vector_new();
    symbol->blocks = vector_new();
    symbol->frame_size = 0;
    symbol->constructor_priority = PRIORITY_INVALID;
    symbol->destructor_priority = PRIORITY_INVALID;
    symbol->substitutions = otable_new();
    return symbol;
}

void symbol_delete(symbol_t* symbol) {
    for (void** entry = otable_begin(symbol->substitutions); entry;
            entry = otable_next(symbol->substitutions, entry))
    {
        free(*entry);
    }
    otable_delete(symbol->substitutions);

    vector_delete(symbol->blocks);
    vector_delete(symbol->parameters);
    location_delete(symbol->location);
    free(symbol->name);
    free(symbol);
}

void symbol_add_substitution(symbol_t* symbol, struct temporary_t* temporary,
        struct variable_t* variable)
{
    substitution_t* substitution = malloc(sizeof(substitution_t));
    substitution->temporary = temporary;
    substitution->variable = variable;
    otable_put(symbol->substitutions, substitution, temporary_hash(temporary));
}

variable_t* /*nullable*/ symbol_find_substitution(symbol_t* symbol, struct temporary_t* temporary) {
    uint32_t hash = temporary_hash(temporary);
    for (void** entry = otable_find(symbol->substitutions, hash); entry;
            entry = otable_collision(symbol->substitutions, entry, hash))
    {
        substitution_t* substitution = *entry;
        if (substitution->temporary == temporary) {
            return substitution->variable;
        }
    }
    return NULL;
}
