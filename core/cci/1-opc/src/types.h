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

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include "common.h"

/*
 * The types table contains the definitions of all structs, unions and
 * typedefs.
 *
 * These types are only allowed to be declared at global scope in opC so we
 * store them all in one big table here.
 */

void types_init(void);
void types_destroy(void);

/**
 * Adds the given typedef if it does not already exist.
 * 
 * The types table takes ownership of the given type. If the typedef already
 * exists, the given type is deleted and the existing type is returned. Do not
 * continue to use a type after passing it to this function! Use the returned
 * type instead.
 */
type_t* types_add_typedef(char* name, type_t* type);

/**
 * Finds a typedef with the given name, or NULL if none exists.
 */
const type_t* types_find_typedef(const char* name);

/**
 * Adds the given struct or union.
 */
void types_add_record(record_t* record);

/**
 * Finds a struct or union with the given tag.
 */
record_t* types_find_record(const char* name);

/**
 * Adds an anonymous record.
 *
 * All records live for the lifetime of the program. If they are not stored in
 * the types table, they are stored in a list here so they can be cleaned up on
 * exit (to ensure there are no memory leaks.)
 */
void types_add_anonymous_record(record_t* record);

#endif
