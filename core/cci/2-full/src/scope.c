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
#include "record.h"
#include "type.h"
#include "symbol.h"
#include "token.h"
#include "generate.h"

scope_t* scope_global;
scope_t* scope_current;

#define NAMESPACE_HASH_MULTIPLIER 37u

/**
 * A type or symbol in the scope's hashtables.
 */
typedef struct scope_element_t {
    table_entry_t entry;
    union {
        struct {
            token_t* name;
            type_t* type;
            namespace_t namespace;
        };
        symbol_t* symbol;
    };
} scope_element_t;

static scope_t* scope_new(scope_t* parent) {
    scope_t* scope = malloc(sizeof(scope_t));
    if (!scope) {
        fatal("Out of memory.");
    }
    scope->refcount = 1;
    scope->parent = parent;
    table_init(&scope->symbols);
    table_init(&scope->types);
    vector_init(&scope->records);
    return scope;
}

void scope_deref(scope_t* scope) {
    assert(scope->refcount != 0);
    if (--scope->refcount != 0) {
        return;
    }

    // free records
    for (size_t i = 0; i < vector_count(&scope->records); ++i) {
        record_delete(vector_at(&scope->records, i));
    }
    vector_destroy(&scope->records);

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

void scope_global_init(void) {
    assert(scope_global == NULL);
    scope_global = (scope_current = scope_new(NULL));
}

void scope_global_destroy(void) {
    assert(scope_global == scope_current);
    assert(scope_global->refcount == 1);
    scope_deref(scope_global);
}

void scope_push(void) {
    scope_current = scope_new(scope_current);
}

void scope_pop(void) {
    scope_t* parent = scope_current->parent;
    scope_deref(scope_current);
    scope_current = parent;
}

scope_t* scope_take(void) {
    scope_t* scope = scope_current;
    assert(scope);
    scope_current = scope->parent;
    scope->parent = NULL;
    return scope;
}

void scope_apply(scope_t* scope) {
    scope->parent = scope_current;
    scope_current = scope_ref(scope);
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

void scope_add_type(scope_t* scope, namespace_t namespace, token_t* name, type_t* type) {
    // TODO make sure the name is not a keyword
    type_t* previous = scope_find_type(scope, namespace, name->value, false);
    if (previous) {
        // duplicate typedefs are allowed as long as the types match.
        // TODO handle duplicate tags
// TODO actually we should completely forbid duplicates here, let the
// parser check for duplicates because different namespaces handle them
// differently (and even different tag types handle them differently)
        if (namespace == NAMESPACE_TYPEDEF && !type_equal(type, previous)) {
            fatal_token(name, "`typedef` redeclared in the same scope with a different type.");
        }
        return;
    }

    scope_element_t* element = malloc(sizeof(scope_element_t));
    //printf("scope adding %s element %p hash %u\n", name->value->bytes, (void*)element, string_hash(name->value) + (unsigned)namespace * NAMESPACE_HASH_MULTIPLIER);
    element->type = type_ref(type);
    element->name = token_ref(name);
    element->namespace = namespace;
    table_put(&scope->types, &element->entry, string_hash(name->value) + (unsigned)namespace * NAMESPACE_HASH_MULTIPLIER);
}

void scope_add_record(scope_t* scope, struct record_t* record) {
    vector_append(&scope->records, record);
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

type_t* scope_find_type(scope_t* scope, namespace_t namespace, const string_t* name, bool recurse) {
    //printf("scope searching for %s hash %u\n", name->bytes, string_hash(name) + (unsigned)namespace * NAMESPACE_HASH_MULTIPLIER);
    do {
        //printf("  searching table\n");
        table_entry_t* entry = table_bucket(&scope->types, string_hash(name) + (unsigned)namespace * NAMESPACE_HASH_MULTIPLIER);
        while (entry) {
            scope_element_t* element = (scope_element_t*)entry;
            if (element->namespace == namespace && string_equal(element->name->value, name)) {
                return element->type;
            }
            entry = table_entry_next(entry);
        }
        scope = scope->parent;
    } while (scope && recurse);
    //printf("  not found\n");
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
                generate_static_variable(symbol, NULL);
            }
        }
    }
}
