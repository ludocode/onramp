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

#include "libo-table.h"

struct string_t;
struct location_t;
struct vector_t;

#define TEMPORARY_INTERVAL_INVALID ((size_t)-1)

typedef struct temporary_t {
    table_entry_t entry;
    struct string_t* name;

    int frame_offset; // assigned offset in the stack frame (usually negative)

    // Live interval
    int live; // Used by the live range analysis algorithm
    size_t interval_start;
    size_t interval_end;
} temporary_t;

/**
 * Finds the given temporary, or inserts it into the temporary table if it
 * doesn't exist.
 */
temporary_t* temporary_find_or_insert(const char* name);

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

#endif
