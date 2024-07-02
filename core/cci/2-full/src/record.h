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

#include "libo-table.h"
#include "libo-string.h"

struct type_t;
struct token_t;

/**
 * A member of a record (struct or union).
 */
typedef struct member_t {
    struct member_t* next;
    struct token_t* name; // null if anonymous
    struct type_t* type;
    unsigned offset;
    unsigned bit_size;
} member_t;

/**
 * A record is a struct or union type in C.
 *
 * The record can represent both unions and structs. In a struct, the members
 * have distinct offsets; in a union, the members overlap.
 *
 * A record is created when a struct or union is first declared. Its members
 * are initially empty. When the struct or union is later defined, the members
 * are parsed and assigned to the record. We support structs with no members as
 * an extension so the flag `is_defined` is used to tell whether the record was
 * defined.
 *
 * Direct members are stored in a linked list (whether named or unnamed.) The
 * record owns these members.
 *
 * Named members, as well as the named members of anonymous nested structs and
 * unions, are stored in a hashtable for quick lookups.
 */
typedef struct record_t {
    unsigned refcount;
    bool is_struct;
    bool is_defined;
    struct token_t* tag; // NULL if anonymous
    table_t member_map;
    member_t* member_list;
    size_t size;
} record_t;

/**
 * Creates a new record.
 *
 * The record takes ownership of the given tag.
 *
 * If the record is anonymous, the tag is an empty string.
 */
record_t* record_new(struct token_t* tag, bool is_struct);

static inline record_t* record_ref(record_t* record) {
    ++record->refcount;
    return record;
}

void record_deref(record_t* record);

/**
 * Returns the members of this record.
 *
 * If the record has not been defined yet (i.e. it has only been
 * forward-declared), this returns NULL.
 */
//member_t* record_members(const record_t* record);

/**
 * Sets the members of the given record.
 *
 * The record takes ownership and frees the members when done.
 */
//void record_set_members(record_t* record, member_t* members, bool is_struct);

/**
 * Returns the size (as in sizeof) of this record.
 */
size_t record_size(const record_t* record);

/**
 * Adds a new member to the record.
 */
void record_add(record_t* record, struct token_t* /*nullable*/ token,
        struct type_t* type, unsigned offset);

/**
 * Finds the member and offset with the given name, returning NULL if the member
 * does not exist.
 *
 * NOTE: The returned offset is not necessarily the same as the offset stored
 * in the member because the member may be nested in an anonymous struct or
 * union. The returned offset is the offset from the start of *this* record,
 * whereas member_offset() returns the offset relative to its direct parent.
 *
 * This can only be called on defined records. If the record is incomplete,
 * the program aborts.
 */
struct type_t* record_find(record_t* record, const string_t* name, unsigned* out_offset);

#endif
