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

#ifndef LOCALS_H_INCLUDED
#define LOCALS_H_INCLUDED

#include "type.h"

#include <stdbool.h>

/**
 * The total number of variables in all currently accessible scopes.
 */
extern int locals_count;

void locals_init(void);
void locals_destroy(void);

/**
 * Adds a new variable with the given name and type.
 *
 * This takes ownership of the given name. It will be freed when locals_pop()
 * is called.
 */
void locals_add(char* name, type_t* type);

/**
 * Destroys all variables beyond the given variable count.
 */
void locals_pop(int previous_locals_count);

/**
 * Finds a variable, returning its type and offset.
 */
bool locals_find(const char* name, const type_t** type, int* offset);

/**
 * Returns the total size of all local variables currently defined, i.e. the
 * necessary stack frame size to store them.
 */
int locals_frame_size(void);

/**
 * Prints all variables.
 */
void dump_variables(void);

#endif
