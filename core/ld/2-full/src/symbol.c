/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
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

#include "common.h"
#include "emit.h"



/*
 * Symbol
 */

symbol_t* symbol_new(string_t* name) {
    //printf("new symbol %s\n",name->bytes);
    symbol_t* symbol = calloc(1, sizeof(symbol_t));
    symbol->name = name;
    return symbol;
}

void symbol_delete(symbol_t* symbol) {
    //printf("delete symbol %s\n",symbol->name->bytes);
    string_deref(symbol->name);
    free(symbol->use);
    free(symbol);
}

void symbol_add_use(symbol_t* symbol, symbol_t* other) {

    // grow if needed
    if (symbol->use_count == symbol->use_capacity) {
        size_t new_capacity = symbol->use_capacity * 2;
        if (new_capacity < 2)
            new_capacity = 2;
        if (new_capacity <= symbol->use_capacity)
            fatal("Out of memory.");
        symbol->use = realloc(symbol->use, new_capacity);
        if (symbol->use == NULL)
            fatal("Out of memory.");
        symbol->use_capacity = new_capacity;
    }

    symbol->use[symbol->use_count++] = other;
}

static void symbol_walk(symbol_t* symbol) {
    if (symbol->is_used) {
        return;
    }
    symbol->is_used = true;
    for (size_t i = 0; i < symbol->use_count; ++i) {
        symbol_walk(symbol->use[i]);
    }
}



/*
 * Symbol lists
 */

typedef struct symbol_node_t {
    symbol_t* symbol;
    struct symbol_node_t* next;
} symbol_node_t;

static symbol_node_t* constructors; // constructors in declaration order
static symbol_node_t* constructors_end; // (constructors are appended to the end)
static symbol_node_t* destructors; // destructors in reverse declaration order
static int constructors_count;
static int destructors_count;

static void symbol_node_walk(symbol_node_t* node) {
    while (node) {
        symbol_walk(node->symbol);
        node = node->next;
    }
}

static void symbol_node_destroy(symbol_node_t* node) {
    while (node) {
        symbol_node_t* next = node->next;
        free(node);
        node = next;
    }
}



/*
 * Symbol hashtable
 */

// The hashtable that contains all symbols, global and static.
static symbol_t** symbols;
#define SYMBOLS_SIZE 1024
#define SYMBOLS_MASK 1023

// The linked list of all symbols in the order they are encountered.
static symbol_t* all_symbols;
static symbol_t* all_symbols_end;

void symbols_init(void) {
    symbols = calloc(SYMBOLS_SIZE, sizeof(symbol_t*));
}

void symbols_destroy(void) {
    symbol_t* symbol = all_symbols;
    while (symbol) {
        symbol_t* next = symbol->next_all;
        symbol_delete(symbol);
        symbol = next;
    }

    free(symbols);

    symbol_node_destroy(constructors);
    symbol_node_destroy(destructors);
}

symbol_t* symbols_find(const char* bytes, size_t length, int file_index) {
    uint32_t hash = fnv1a_bytes(bytes, length);
    //printf("%s %i %x\n", bytes, (int)(hash & SYMBOLS_MASK), (int)hash);
    symbol_t* global = NULL;

    symbol_t* symbol = symbols[hash & SYMBOLS_MASK];
    while (symbol) {
        if (string_equal_bytes(symbol->name, bytes, length)) {
            // Prefer a matching static symbol to a global symbol.
            if (symbol->file_index == file_index)
                return symbol;
            if (symbol->file_index == -1) {
                assert(!global);
                global = symbol;
            }
        }
        symbol = symbol->next_bucket;
    }
    // If no static symbol was found, return the global if any.
    return global;
}

symbol_t* symbols_define(const char* bytes, size_t length, int file_index) {
    string_t* name = string_intern_bytes(bytes, length);
    uint32_t hash = fnv1a_bytes(bytes, length);

    // walk through symbols looking for a match
    symbol_t* symbol = symbols[hash & SYMBOLS_MASK];
    while (symbol) {
        if (string_equal(symbol->name, name) && symbol->file_index == file_index) {
            fatal("Duplicate %s symbol: %s", file_index == -1 ? "global" : "static",
                    symbol->name->bytes);
        }
        symbol = symbol->next_bucket;
    }

    // create the symbol
    symbol = symbol_new(name);
    symbol->file_index = file_index;
    if (!optimize) {
        symbol->is_used = true;
    }
    return symbol;
}

void symbols_insert(symbol_t* symbol) {
    assert(symbol->next_all == NULL);
    assert(symbol->next_bucket == NULL);

    // Insert the symbol into the global symbol list
    if (all_symbols == NULL) {
        // This is the first symbol. Make sure its name is __start.
        if (!string_equal_cstr(symbol->name, "__start")) {
            print_warning("The first symbol is not named `__start`!");
        }
        all_symbols = symbol;
    } else {
        all_symbols_end->next_all = symbol;
    }
    all_symbols_end = symbol;

    // Insert the symbol into the hashtable
    size_t index = string_hash(symbol->name) & SYMBOLS_MASK;
    //printf("%s %i %x\n", symbol->name->bytes, (int)index, symbol->name->hash);
    symbol->next_bucket = symbols[index];
    symbols[index] = symbol;

    // If it's a constructor, append it to the constructor list
    if (symbol->constructor) {
        symbol_node_t* node = malloc(sizeof(symbol_node_t));
        node->symbol = symbol;
        node->next = NULL;
        if (constructors_end) {
            constructors_end->next = node;
        } else {
            constructors = node;
        }
        constructors_end = node;
        ++constructors_count;
    }

    // If it's a destructor, insert it at the front of the destructor list
    if (symbol->destructor) {
        symbol_node_t* node = malloc(sizeof(symbol_node_t));
        node->symbol = symbol;
        node->next = destructors;
        destructors = node;
        ++destructors_count;
    }
}

void symbols_walk_use(void) {

    // The first symbol is the entry point.
    if (all_symbols != NULL) {
        symbol_walk(all_symbols);
    }

    // Constructors and destructors are always kept.
    symbol_node_walk(constructors);
    symbol_node_walk(destructors);
}

void symbols_assign_addresses(void) {
    size_t address = 0;
    symbol_t* symbol = all_symbols;
    while (symbol) {
        if (symbol->is_used) {
            symbol->address = address;
            address += symbol->size;
            address = (address + 3) & (~3); // align to a word boundary
        }
        symbol = symbol->next_all;
    }
}

static void symbols_create_generated_list(const char* name, size_t count) {
    symbol_t* symbol = symbols_define(name, strlen(name), -1);
    symbol->size = 4 * (count + 1);
    symbols_insert(symbol);
}

void symbols_create_generated(void) {
    symbols_create_generated_list("__constructors", constructors_count);
    symbols_create_generated_list("__destructors", destructors_count);
}

static void symbols_emit_generated_list(const char* name, symbol_node_t* list) {
    emit_source_location("<builtin>", 0);
    emit_symbol(name);
    for (; list; list = list->next) {
        emit_int(list->symbol->address);
    }
    emit_int(0);
}

void symbols_emit_generated(void) {

    // align address
    for (int i = current_address; i & 3; ++i) {
        emit_byte(0);
    }

    // TODO lists must be sorted by priority!

    symbols_emit_generated_list("__constructors", constructors);
    symbols_emit_generated_list("__destructors", destructors);
}
