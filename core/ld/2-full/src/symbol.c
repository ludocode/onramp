/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2026 Fraser Heavy Software
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

static void symbol_walk(symbol_t* symbol);

symbol_t* symbol_new(string_t* name) {
    //printf("new symbol %s\n",name->bytes);
    symbol_t* symbol = calloc(1, sizeof(symbol_t));
    symbol->name = name;
    vector_init(&symbol->uses);
    return symbol;
}

void symbol_delete(symbol_t* symbol) {
    //printf("delete symbol %s\n",symbol->name->bytes);
    vector_destroy(&symbol->uses);
    string_deref(symbol->name);
    free(symbol);
}

void symbol_add_use(symbol_t* symbol, symbol_t* other) {
    vector_append(&symbol->uses, other);
}

// Walk a vector of symbols.
static void symbol_walk_vector(vector_t* vector) {
    for (size_t i = vector_count(vector); i-- > 0;) {
        symbol_walk(vector_at(vector, i));
    }
}

static void symbol_walk(symbol_t* symbol) {
    if (symbol->is_used) {
        return;
    }
    symbol->is_used = true;
    symbol_walk_vector(&symbol->uses);
}



/*
 * Symbol lists
 */

static vector_t constructors;
static vector_t destructors;



/*
 * Symbol hashtable
 */

// The hashtable that contains all symbols, global and static.
table_t symbols;

// The linked list of all symbols in the order they are encountered.
static symbol_t* all_symbols;
static symbol_t* all_symbols_end;

void symbols_init(void) {
    table_init(&symbols);
    vector_init(&constructors);
    vector_init(&destructors);

    // growing the table is expensive so reserve a pretty big size right away.
    table_reserve_bits(&symbols, 14); // 16k buckets, 64 kB
}

void symbols_destroy(void) {
    vector_destroy(&destructors);
    vector_destroy(&constructors);
    table_destroy(&symbols);

    symbol_t* symbol = all_symbols;
    while (symbol) {
        symbol_t* next = symbol->next;
        symbol_delete(symbol);
        symbol = next;
    }
}

symbol_t* symbols_find(const char* bytes, size_t length, int file_index) {
    uint32_t hash = fnv1a_bytes(bytes, length);
    symbol_t* global = NULL;

    table_entry_t* entry = table_bucket(&symbols, hash);
    for (; entry; entry = table_entry_next(entry)) {
        symbol_t* symbol = (symbol_t*)entry;
        if (string_equal_bytes(symbol->name, bytes, length)) {
            // Prefer a matching static symbol to a global symbol.
            if (symbol->file_index == file_index)
                return symbol;
            if (symbol->file_index == -1) {
                assert(!global);
                global = symbol;
            }
        }
    }
    // If no static symbol was found, return the global if any.
    return global;
}

symbol_t* symbols_define(const char* bytes, size_t length, int file_index) {

    // Check for duplicates
    symbol_t* symbol = symbols_find(bytes, length, file_index);
    if (symbol && symbol->file_index == file_index) {
        fatal("Duplicate %s symbol: %s",
                file_index == -1 ? "global" : "static",
                symbol->name->bytes);
    }

    // Create the symbol
    symbol = symbol_new(string_intern_bytes(bytes, length));
    symbol->file_index = file_index;
    if (!optimize) {
        symbol->is_used = true;
    }
    return symbol;
}

void symbols_insert(symbol_t* symbol) {
    assert(symbol->next == NULL);

    // Insert the symbol into the global symbol list
    if (all_symbols == NULL) {
        // This is the first symbol. Make sure its name is __start.
        if (!string_equal_cstr(symbol->name, "__start")) {
            print_warning("The first symbol is not named `__start`!");
        }
        all_symbols = symbol;
    } else {
        all_symbols_end->next = symbol;
    }
    all_symbols_end = symbol;

    // Insert the symbol into the hashtable
    table_put(&symbols, &symbol->entry, string_hash(symbol->name));

    // Append it to the relevant lists
    if (symbol->constructor) {
        vector_append(&constructors, symbol);
    }
    if (symbol->destructor) {
        vector_append(&destructors, symbol);
    }
}

void symbols_walk_use(void) {

    // The first symbol is the entry point.
    if (all_symbols != NULL) {
        symbol_walk(all_symbols);
    }

    // Constructors and destructors are always kept.
    symbol_walk_vector(&constructors);
    symbol_walk_vector(&destructors);
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
        symbol = symbol->next;
    }
}

static void symbols_create_generated_list(const char* name, size_t count) {
    symbol_t* symbol = symbols_define(name, strlen(name), -1);
    symbol->size = 4 * (count + 1);
    symbols_insert(symbol);
}

void symbols_create_generated(void) {
    symbols_create_generated_list("__constructors", vector_count(&constructors));
    symbols_create_generated_list("__destructors", vector_count(&destructors));
}

static void symbols_emit_generated_list(const char* name, vector_t* vector, bool reverse) {
    emit_source_location("<builtin>", 0);
    emit_symbol(name);

    if (reverse) {
        for (size_t i = vector_count(vector); i-- > 0;) {
            emit_int(((symbol_t*)vector_at(vector, i))->address);
        }
    } else {
        size_t count = vector_count(vector);
        for (size_t i = 0; i < count; ++i) {
            emit_int(((symbol_t*)vector_at(vector, i))->address);
        }
    }
    emit_int(0);
}

void symbols_emit_generated(void) {

    // align address
    for (int i = current_address; i & 3; ++i) {
        emit_byte(0);
    }

    // TODO lists must be sorted by priority!

    symbols_emit_generated_list("__constructors", &constructors, false);
    symbols_emit_generated_list("__destructors", &destructors, true);
}
