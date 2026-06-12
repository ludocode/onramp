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

#ifndef VARIABLE_H_INCLUDED
#define VARIABLE_H_INCLUDED

#include "libo-table.h"

struct string_t;
struct location_t;

typedef struct variable_t {
    table_entry_t entry;
    struct string_t* name;
    int size;
    int alignment;
    int frame_offset; // assigned offset in the stack frame (usually negative, 0 if not yet assigned)
} variable_t;

/**
 * Finds the given variable, or inserts it into the variable table if it
 * doesn't exist.
 */
variable_t* variable_find_or_insert(const char* name);

void variable_setup(void);
void variable_teardown(void);

/**
 * Clears the variable table.
 */
void variable_clear(void);

#endif
