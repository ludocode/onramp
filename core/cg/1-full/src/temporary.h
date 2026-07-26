/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Fraser Heavy Software
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

#ifndef TEMPORARY_H_INCLUDED
#define TEMPORARY_H_INCLUDED

#include <stdbool.h>

#include "libo-string.h"
#include "libo-table.h"

struct block_t;
struct instruction_t;
struct location_t;
struct location_t;
struct otable_t;
struct variable_t;
struct vector_t;

#define TEMPORARY_REGISTER_INVALID (-1)
#define TEMPORARY_INTERVAL_INVALID ((size_t)-1)

typedef struct temporary_t {
    table_entry_t entry;
    struct string_t* name;

    // After register allocation, each used temporary is assigned a register
    // and/or a variable. A temporary may be assigned both if it lives in a
    // register but needs stack space to be preserved across a call.
    int reg;
    struct variable_t* variable;

    // Live interval
    size_t interval_start;
    size_t interval_end;

    // Instruction in which the temporary was assigned (or last assigned during forward pass.)
    struct instruction_t* instruction;

    // Block containing the above instruction (only during forward pass.)
    struct block_t* block;

    // True if the temporary is only assigned once.
    bool is_ssa;

    // True if the temporary is used as an input. This is used to optimize dead
    // stores.
    bool is_used;
} temporary_t;

/**
 * Creates a new temporary.
 */
temporary_t* temporary_new_anonymous(void);

/**
 * Finds the given temporary, or inserts it into the temporary table if it
 * doesn't exist.
 */
temporary_t* temporary_find_or_create(const char* name);

/**
 * Creates a temporary with the given name, ensuring that it doesn't already
 * exist.
 *
 * Returns NULL if it already exists.
 */
temporary_t* temporary_create(const char* cname);

void temporaries_setup(void);
void temporaries_teardown(void);

/**
 * Clears the temporary table.
 */
void temporaries_clear(void);

/**
 * Add all temporaries to the given vector.
 */
void temporaries_list_all(struct vector_t* temporaries);

/**
 * Compares the live intervals of the two temporaries.
 *
 * This is used for sorting temporaries in linear scan register allocation.
 */
int temporary_compare_live_interval(const void* vleft, const void* vright);

static inline size_t temporary_interval_length(temporary_t* temporary) {
    return temporary->interval_end - temporary->interval_start;
}

static inline uint32_t temporary_hash(temporary_t* temporary) {
    return string_hash(temporary->name);
}

void temporaries_print_table(struct otable_t* table);

#endif
