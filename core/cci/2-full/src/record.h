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

struct type_t;

/**
 * A field (member) of a record (struct or union).
 */
typedef struct field_t {
    unsigned offset;
    unsigned bit_size;
    string_t* name;
    struct type_t* type;
} field_t;

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
 *
 * Fields are stored in a hashtable. The fields of anonymous nested structs
 * and unions are flattened into it.
 */
typedef struct record_t {
    unsigned refcount;
    string_t* name;
    field_t* fields;
    size_t fields_bits;
} record_t;

/**
 * Creates a new record.
 *
 * The record takes ownership of the given name.
 *
 * If the record is anonymous, the name is an empty (allocated) string.
 */
record_t* record_new(string_t* name);

static inline record_t* record_ref(record_t* record) {
    ++record->refcount;
    return record;
}

void record_deref(record_t* record);

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
void record_set_fields(record_t* record, field_t* fields, bool is_struct);

/**
 * Returns the size (as in sizeof) of this record.
 */
size_t record_size(const record_t* record);

/**
 * Finds the field and offset with the given name, returning NULL if the field
 * does not exist.
 *
 * NOTE: The returned offset is not necessarily the same as the offset stored
 * in the field because the field may be nested in an anonymous struct or
 * union. The returned offset is the offset from the start of *this* record,
 * whereas field_offset() returns the offset relative to its direct parent.
 *
 * If the record is incomplete, a fatal error is raised.
 */
const field_t* record_find_field(const record_t* record, const char* name,
        size_t* out_offset);

#endif
