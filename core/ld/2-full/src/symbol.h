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

#ifndef SYMBOL_H_INCLUDED
#define SYMBOL_H_INCLUDED

#include "common.h"
#include "libo-vector.h"
#include "libo-table.h"

struct label_t;



/*
 * Symbol
 */

typedef struct symbol_t {
    table_entry_t entry; // Entry in the symbol hashtable
    struct symbol_t* next; // The next symbol in the list of all symbols

    string_t* name;
    size_t address; // The address assigned to this symbol in the output
    size_t size; // The size of the symbol in bytes
    bool is_used; // Whether this symbol is use (transitively from a root)
    bool is_static; // True if static, false if global
    int file_index; // Index of the object file in which this symbol is defined
    vector_t uses; // Symbols this symbol references
    table_t* labels; // All labels defined in this symbol, or NULL if none

    // Flags
    bool constructor : 1;
    bool destructor : 1;
    bool weak : 1;
    int constructor_priority;
    int destructor_priority;
} symbol_t;

/**
 * Creates a new symbol with the given name.
 *
 * The symbol takes ownership of the given string. (You must own a reference
 * before calling this and you must not deref it afterwards.)
 */
symbol_t* symbol_new(string_t* name);

/**
 * Deletes the given symbol.
 */
void symbol_delete(symbol_t* symbol);

/**
 * Adds a reference to a symbol that is use by this symbol.
 */
void symbol_add_use(symbol_t* symbol, symbol_t* other);

/**
 * Defines a label with the given name within the symbol.
 *
 * The label's offset is not assigned here; it is done in the parser. This also
 * doesn't check for duplicates; the parser does.
 */
struct label_t* symbol_define_label(symbol_t* symbol, const char* bytes, size_t length);

/**
 * Gets the label with the given name.
 */
struct label_t* symbol_find_label(symbol_t* symbol, const char* bytes, size_t length);



/*
 * Symbol hashtable
 */

void symbols_init(void);

void symbols_destroy(void);

/**
 * Defines a new symbol with the given bytes, length and file, returning it.
 *
 * This does not insert the symbol into the various symbol tables. This must be
 * done after configuring the symbol.
 */
symbol_t* symbols_define(const char* bytes, size_t length, int file_index, bool is_static);

/**
 * Finds a static symbol in the given file with the given name, or a global
 * symbol with the given name, or null if the symbol isn't found.
 */
symbol_t* symbols_find(const char* bytes, size_t length, int file_index);

void symbols_insert(symbol_t* symbol);

void symbols_walk_use(void);

void symbols_assign_addresses(void);

/**
 * Creates generated symbols. Called after all symbols are parsed but before
 * symbol addresses are assigned.
 */
void symbols_create_generated(void);

/**
 * Emits generated symbols. Called after all user-defined symbols are emitted.
 */
void symbols_emit_generated(void);

#endif
