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
    if (member->name)
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

record_t* record_new(token_t* tag, bool is_struct) {
    record_t* record = calloc(1, sizeof(record_t));
    record->refcount = 1;
    record->tag = tag ? token_ref(tag) : NULL;
    record->is_struct = is_struct;
    table_init(&record->member_map);
    return record;
}

void record_deref(record_t* record) {
    if (--record->refcount != 0)
        return;

    if (record->tag)
        token_deref(record->tag);

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

static void record_add_to_table(record_t* record, member_t* member, unsigned offset) {
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
    element->offset = offset;
    table_put(&record->member_map, &element->entry, string_hash(member->name->value));
}

/*
 * Adds all members of the given anonymous record member to this record.
 *
 * This is used to add all the members of an anonymous record member to the
 * parent. Since we're adding the contents of its map to our own, this also
 * includes the members of its own anonymous member records recursively.
 */
static void record_add_anonymous_to_table(record_t* record, member_t* member, unsigned offset) {
    assert(type_matches_base(member->type, BASE_RECORD));
    record_t* child = member->type->record;
    for (table_entry_t** bucket = table_first_bucket(&child->member_map);
            bucket; bucket = table_next_bucket(&child->member_map, bucket))
    {
        for (table_entry_t* entry = *bucket; entry; entry = table_entry_next(entry)) {
            record_element_t* element = (record_element_t*)entry;
            // We add the offset within the child record to the offset of the
            // anonymous member in this record to get the full offset.
            record_add_to_table(record, element->member, offset + element->offset);
        }
    }
}

void record_add(record_t* record, token_t* /*nullable*/ token, struct type_t* type) {

    // determine offset of member
    int offset;
    if (record->is_struct && record->member_list) {
        member_t* last = record->member_list;
        offset = last->offset + type_size(last->type);
    } else {
        offset = 0;
    }

    // update alignment
    size_t alignment = type_alignment(type);
    if (record->alignment < alignment)
        record->alignment = alignment;

    // align member
    offset = (offset + alignment - 1) & ~(alignment - 1);
    //printf("assigned offset %u to member %s\n", offset, token ? token->value->bytes : "<anonymous>");

    // create member
    member_t* member = member_new(token, type, offset);
    member->next = record->member_list;
    record->member_list = member;

    // add to table
    if (token) {
        record_add_to_table(record, member, offset);
    } else {
        record_add_anonymous_to_table(record, member, member->offset);
    }

    // update size
    size_t extent = type_size(type);
    if (extent < alignment)
        extent = alignment;
    unsigned end = offset + extent;
    if (end > record->size)
        record->size = end;
}

type_t* record_find(record_t* record, const string_t* name, unsigned* out_offset) {
    if (!record->is_defined)
        fatal("Internal error: Cannot call record_find() on incomplete record.");

    for (table_entry_t* entry = table_bucket(&record->member_map, string_hash(name));
            entry; entry = table_entry_next(entry))
    {
        record_element_t* element = (record_element_t*)entry;
        member_t* member = element->member;
        if (string_equal(name, member->name->value)) {
            *out_offset = element->offset;
            return member->type;
        }
    }
    return NULL;
}
