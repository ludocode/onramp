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

#ifndef SCOPE_H_INCLUDED
#define SCOPE_H_INCLUDED

#include "libo-table.h"
#include "libo-vector.h"
#include "libo-string.h"

struct symbol_t;
struct type_t;
struct string_t;
struct token_t;

typedef enum namespace_t {
    NAMESPACE_TYPEDEF = 1,
    NAMESPACE_TAG,
} namespace_t;

typedef struct scope_t {
    struct scope_t* parent;
    table_t symbols;
    table_t types; // typedefs, structs, unions, enums
    vector_t anonymous_records; // TODO get rid of this, refcount records instead
} scope_t;

void scope_global_init(void);
void scope_global_destroy(void);

void scope_delete(scope_t* scope);

extern scope_t* scope_global;
extern scope_t* scope_current;
void scope_push(void);
void scope_pop(void);

// When a function declaration is parsed, it creates a scope for its arguments,
// and we then "take" the scope to remove it. If the function is then defined,
// we put the scope back. It's important to get this right. For example if a
// function definition defines an enum in one argument and a function pointer 
scope_t* scope_take(void);
void scope_apply(scope_t* scope);

void scope_emit_tentative_definitions(void);

/**
 * Adds the given symbol to the given scope, taking ownership of this strong
 * reference to the symbol.
 *
 * This does not check for duplicates! Duplicate checking must be done
 * separately (to properly handle e.g. extern, tentative definitions, etc.)
 */
void scope_add_symbol(scope_t* scope, struct symbol_t* symbol);

void scope_add_type(scope_t* scope, namespace_t namespace, struct token_t* name, struct type_t* type);

/**
 * Removes the given symbol from the given scope, relinquishing ownership of
 * it.
 *
 * The symbol must exist in this scope.
 */
void scope_remove_symbol(scope_t* scope, struct symbol_t* symbol);

/**
 * Finds a symbol of the given name in this scope, or any parent scope if
 * recurse is true.
 *
 * Returns NULL if no such symbol exists.
 */
struct symbol_t* scope_find_symbol(scope_t* scope, const string_t* name, bool recurse);

/**
 * Finds a type of the given name in the given namespace in this scope, or any
 * parent scope if recurse is true.
 *
 * Returns NULL if no such typedef exists.
 */
struct type_t* scope_find_type(scope_t* scope, namespace_t namespace, const string_t* name, bool recurse);

#endif
