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

#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include "common.h"

void globals_init(void);
void globals_destroy(void);

const char* global_name(const global_t* global);

const type_t* global_type(const global_t* global);

bool global_is_variable(const global_t* global);

bool global_is_function(const global_t* global);

/**
 * Returns true if the given global function is variadic.
 *
 * The given global must be a function.
 */
bool global_function_is_variadic(const global_t* global);

/**
 * Returns the number of (non-variadic) parameters of the given global function.
 *
 * The given global must be a function.
 */
int global_function_param_count(const global_t* global);

/**
 * Returns the number of (non-variadic) parameters of the given global.
 *
 * The given global must be a function.
 */
type_t* global_function_param_type(const global_t* global, int index);

/**
 * Creates a new global variable.
 *
 * This takes ownership of the given type and name.
 *
 * You must add the global with global_add() after configuring it.
 */
global_t* global_new_variable(type_t* type, char* name);

/**
 * Creates a new global function.
 *
 * This takes ownership of the given type and name. If the global already
 * exists, the given values will be destroyed immediately! Use the returned
 * global's name and other properties after calling this.
 *
 * We can only pass four arguments in omC so the variadic flag is set
 * separately in global_set_variadic().
 *
 * You must add the global with global_add() after configuring it.
 */
global_t* global_new_function(type_t* return_type, char* name,
        int param_count, type_t** param_types);

/**
 * Adds the given global, or deletes the given global and returns the original
 * if a matching global with this name was already declared.
 *
 * If a global with this name but a different type already exists, it's a fatal
 * error.
 */
global_t* global_add(global_t* global);

void global_set_variadic(global_t* global, bool variadic);

global_t* global_find(const char* name);

#endif
