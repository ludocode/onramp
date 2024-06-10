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

#include "scope.h"

#include <stdlib.h>

#include "libo-error.h"
#include "libo-vector.h"
#include "type.h"
#include "symbol.h"
#include "token.h"
#include "generate.h"

scope_t* scope_global;
scope_t* scope_current;

#define TAG_HASH_MULTIPLIER 37u

/**
 * A type or symbol in the scope's hashtables.
 */
typedef struct scope_element_t {
    table_entry_t entry;
    union {
        struct {
            token_t* name;
            type_t* type;
            tag_t tag;
        };
        symbol_t* symbol;
    };
} scope_element_t;

static scope_t* scope_new(scope_t* parent) {
    scope_t* scope = malloc(sizeof(scope_t));
    if (!scope) {
        fatal("Out of memory.");
    }
    scope->parent = parent;
    table_init(&scope->symbols);
    table_init(&scope->types);
    vector_init(&scope->anonymous_records);
    return scope;
}

static void scope_delete(scope_t* scope) {

    // free anonymous records
    for (size_t i = 0; i < vector_count(&scope->anonymous_records); ++i) {
        type_deref(vector_at(&scope->anonymous_records, i));
    }
    vector_destroy(&scope->anonymous_records);

    // free types
    for (table_entry_t** bucket = table_first_bucket(&scope->types);
            bucket; bucket = table_next_bucket(&scope->types, bucket))
    {
        for (table_entry_t* entry = *bucket; entry;) {
            table_entry_t* next = table_entry_next(entry);
            type_deref(((scope_element_t*)entry)->type);
            token_deref(((scope_element_t*)entry)->name);
            free(entry);
            entry = next;
        }
    }
    table_destroy(&scope->types);

    // free symbols
    for (table_entry_t** bucket = table_first_bucket(&scope->symbols);
            bucket; bucket = table_next_bucket(&scope->symbols, bucket))
    {
        for (table_entry_t* entry = *bucket; entry;) {
            table_entry_t* next = table_entry_next(entry);
            symbol_deref(((scope_element_t*)entry)->symbol);
            free(entry);
            entry = next;
        }
    }
    table_destroy(&scope->symbols);

    free(scope);
}

static void scope_create_va_list(void) {

    // We store __builtin_va_list as a typedef of int*. This means it's
    // implicitly convertible with int*, can be copied by value, etc. That's
    // fine; we don't need to diagnose improper use.
    type_t* list = type_new_base(BASE_SIGNED_INT);
    type_t* listp = type_new_pointer(list, false, false, false);
    string_t* filename = string_intern_cstr("<builtin>");
    token_t* token = token_new(
            token_type_alphanumeric,
            string_intern_cstr("__builtin_va_list"),
            token_prefix_none,
            filename,
            0,
            NULL);
    scope_add_type(scope_global, TAG_TYPEDEF, token, listp);

    token_deref(token);
    string_deref(filename);
    type_deref(listp);
    type_deref(list);
}

void scope_global_init(void) {
    assert(scope_global == NULL);
    scope_global = (scope_current = scope_new(NULL));
    scope_create_va_list();
}

void scope_global_destroy(void) {
    assert(scope_global == scope_current);
    scope_delete(scope_global);
}

void scope_push(void) {
    scope_current = scope_new(scope_current);
}

void scope_pop(void) {
    scope_t* scope = scope_current->parent;
    scope_delete(scope_current);
    scope_current = scope;
}

void scope_remove_symbol(scope_t* scope, symbol_t* symbol) {
    table_entry_t* entry = table_bucket(&scope->symbols, string_hash(symbol->name));
    while (entry) {
        if (symbol == ((scope_element_t*)entry)->symbol) {
            table_remove(&scope->symbols, entry);
            free(entry);
            return;
        }
        entry = table_entry_next(entry);
    }
    fatal("Internal error: Cannot remove non-existant symbol");
}

void scope_add_symbol(scope_t* scope, symbol_t* symbol) {
    scope_element_t* element = malloc(sizeof(scope_element_t));
    element->symbol = symbol_ref(symbol);
    table_put(&scope->symbols, &element->entry, string_hash(symbol->name));
}

void scope_add_type(scope_t* scope, tag_t tag, token_t* name, type_t* type) {
    // TODO make sure the name is not a keyword
    type_t* previous = scope_find_type(scope, tag, name->value, false);
    if (previous) {
        // duplicate typedefs are allowed as long as the types match.
        // TODO handle duplicates of other tags
        if (tag == TAG_TYPEDEF && !type_equal(type, previous)) {
            fatal_token(name, "`typedef` redeclared in the same scope with a different type.");
        }
        return;
    }

    scope_element_t* element = malloc(sizeof(scope_element_t));
    element->type = type_ref(type);
    element->name = token_ref(name);
    table_put(&scope->types, &element->entry, string_hash(name->value) + (unsigned)tag * TAG_HASH_MULTIPLIER);
}

symbol_t* scope_find_symbol(scope_t* scope, const string_t* name, bool recurse) {
    do {
        table_entry_t* entry = table_bucket(&scope->symbols, string_hash(name));
        while (entry) {
            symbol_t* symbol = ((scope_element_t*)entry)->symbol;
            if (string_equal(symbol->name, name)) {
                return symbol;
            }
            entry = table_entry_next(entry);
        }
        scope = scope->parent;
    } while (scope && recurse);
    return NULL;
}

type_t* scope_find_type(scope_t* scope, tag_t tag, const string_t* name, bool recurse) {
    do {
        table_entry_t* entry = table_bucket(&scope->types, string_hash(name) + (unsigned)tag * TAG_HASH_MULTIPLIER);
        while (entry) {
            scope_element_t* element = (scope_element_t*)entry;
            if (element->tag == tag && string_equal(element->name->value, name)) {
                return element->type;
            }
            entry = table_entry_next(entry);
        }
        scope = scope->parent;
    } while (scope && recurse);
    return NULL;
}

void scope_emit_tentative_definitions(void) {
    scope_t* scope = scope_global;
    for (table_entry_t** bucket = table_first_bucket(&scope->symbols);
            bucket; bucket = table_next_bucket(&scope->symbols, bucket))
    {
        for (table_entry_t* entry = *bucket; entry; entry = table_entry_next(entry)) {
            symbol_t* symbol = ((scope_element_t*)entry)->symbol;
            //printf("scope_emit_tentative_definitions() symbol %s is_tentative: %i\n",symbol->name->bytes, symbol->is_tentative);
            if (symbol->is_tentative) {
                generate_global_variable(symbol, NULL);
            }
        }
    }
}
