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

#include <stddef.h>
#include <stdbool.h>

struct string_t;

typedef struct variable_t {
    size_t id;
    size_t size; // 0 for a synthetic variable, e.g. $_Vargs
    size_t alignment;

    // offset in the stack frame.
    // - usually negative (local variables and spilled temporaries)
    // - rarely positive (stack-passed arguments)
    // - 0 if not yet assigned
    int offset;

    // A unique name for debug purposes. This doesn't affect codegen.
    struct string_t* name;
} variable_t;

/**
 * Creates a new variable and adds it to the variable table.
 *
 * The variable is assigned an id automatically.
 */
variable_t* variable_create(struct string_t* name, size_t size, size_t alignment);

/**
 * Gets the variable with the given id.
 */
variable_t* variable_get(size_t id);

void variables_setup(void);
void variables_teardown(void);

/**
 * Clears the variable table.
 */
void variables_clear(void);

#endif
