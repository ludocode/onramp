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

#ifndef RECORD_H_INCLUDED
#define RECORD_H_INCLUDED

#include "common.h"

/**
 * A record is a struct or union type in C.
 *
 * A record is created when a struct or union is first declared. Its fields are
 * initially empty. When the struct or union is later defined, the fields are
 * parsed and assigned to the record.
 *
 * Records are owned by the scope in which they were declared. They are
 * destroyed when the scope is destroyed (which also destroys anything that
 * could reference them, preventing any dangling references.)
 */

/**
 * Creates a new record.
 *
 * The record takes ownership of the given name.
 *
 * If the record is anonymous, the name is an empty (allocated) string.
 */
record_t* record_new(char* name);

void record_delete(record_t* record);

const char* record_name(const record_t* record);

/**
 * Returns the fields of this record.
 *
 * If the record has not been defined yet (i.e. it has only been
 * forward-declared), this returns NULL.
 */
field_t* record_fields(const record_t* record);

/**
 * Sets the fields of the given record.
 *
 * The record takes ownership and frees the fields when done.
 */
void record_set_fields(record_t* record, field_t* fields);

/**
 * Returns the size (as in sizeof) of this record.
 */
size_t record_size(const record_t* record);

/**
 * Finds the field with the given name, raising a fatal error if the record is
 * incomplete or the field does not exist.
 */
const field_t* record_find_field(const record_t* record, const char* name);

#endif
