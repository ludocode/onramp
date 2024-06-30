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

#include "record.h"

#include <stdlib.h>
#include <assert.h>

#include "type.h"
#include "token.h"

static member_t* member_new(token_t* /*nullable*/ name, type_t* type, int offset) {
    member_t* member = calloc(1, sizeof(member_t));
    member->name = name ? token_ref(name) : NULL;
    member->type = type_ref(type);
    member->offset = offset;
    return member;
}

static void member_delete(member_t* member) {
    token_deref(member->name);
    type_deref(member->type);
    free(member);
}

/**
 * A member in the record's member map hashtable.
 *
 * We don't own or reference count these members. Direct members are owned by
 * the member list. For indirect members (those of anonymous records), we have
 * their anonymous parent as a real member in the member list and it holds a
 * strong reference to their record.
 */
typedef struct record_element_t {
    table_entry_t entry;
    member_t* member;
    unsigned offset;
} record_element_t;

record_t* record_new(string_t* name, bool is_struct) {
    record_t* record = calloc(1, sizeof(record_t));
    record->refcount = 1;
    record->name = string_ref(name);
    record->is_struct = is_struct;
    table_init(&record->member_map);
    return record;
}

void record_deref(record_t* record) {
    if (--record->refcount != 0)
        return;

    string_deref(record->name);

    // clear table
    for (table_entry_t** bucket = table_first_bucket(&record->member_map);
            bucket; bucket = table_next_bucket(&record->member_map, bucket))
    {
        for (table_entry_t* entry = *bucket; entry;) {
            table_entry_t* next = table_entry_next(entry);
            free(entry);
            entry = next;
        }
    }
    table_destroy(&record->member_map);

    // delete members
    for (member_t* member = record->member_list; member;) {
        member_t* next = member->next;
        member_delete(member);
        member = next;
    }

    free(record);
}

size_t record_size(const record_t* record) {
    return record->size;
}

static void record_add_to_table(record_t* record, member_t* member) {
    assert(!string_is_empty(member->name->value));

    // check for duplicates
    unsigned unused_offset;
    type_t* duplicate = record_find(record, member->name->value, &unused_offset);
    if (duplicate) {
        fatal_token(member->name, "struct/field member defined with the same name as a previous member.");
    }

    // TODO check for duplicates!
    record_element_t* element = malloc(sizeof(record_element_t));
    element->member = member;
    table_put(&record->member_map, &element->entry, string_hash(member->name->value));
}

static void record_add_anonymous_to_table(record_t* record, member_t* member, unsigned offset) {
    //TODO
}

void record_add(record_t* record, token_t* /*nullable*/ token, struct type_t* type, unsigned offset) {

    // create member
    member_t* member = member_new(token, type, offset);
    member->next = record->member_list;
    record->member_list = member;

    // add to table
    if (token) {
        record_add_to_table(record, member);
    } else {
        record_add_anonymous_to_table(record, member, member->offset);
    }

    // update size
    unsigned end = offset + type_size(type);
    if (end > record->size) {
        record->size = end;
    }
}

type_t* record_find(record_t* record, const string_t* name, unsigned* out_offset) {
    if (!record->is_defined)
        fatal("Internal error: Cannot call record_find() on incomplete record.");

    member_t* member = (member_t*)table_bucket(&record->member_map, string_hash(name));
    while (member) {
        if (string_equal(name, member->name->value)) {
            *out_offset = member->offset;
            return member->type;
        }
        member = member->next;
    }
    return NULL;
}
