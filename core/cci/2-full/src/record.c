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

#include "type.h"

member_t* member_new(string_t* name, type_t* type) {
    member_t* member = calloc(1, sizeof(member_t));
    member->name = name;
    member->type = type;
    return member;
}

void member_delete(member_t* member) {
    string_deref(member->name);
    type_deref(member->type);
    free(member);
}

/**
 * A member in the record's member map hashtable.
 */
/*typedef struct record_element_t {
    table_entry_t entry;
    member_t* member;
} record_element_t;*/

record_t* record_new(string_t* name) {
    record_t* record = calloc(1, sizeof(record_t));
    record->refcount = 1;
    record->name = string_ref(name);
    table_init(&record->member_map);
    return record;
}

void record_deref(record_t* record) {
    if (--record->refcount != 0)
        return;

    string_deref(record->name);

    // clear table
    /*
    for (table_entry_t** bucket = table_first_bucket(&record->member_map);
            bucket; bucket = table_next_bucket(&record->member_map, bucket))
    {
        for (table_entry_t* entry = *bucket; entry;) {
            table_entry_t* next = table_entry_next(entry);
            free(entry);
            entry = next;
        }
    }
    */
    table_destroy(&record->member_map);

    // delete members
    for (member_t* member = record->member_list; member;) {
        member_t* next = member->next;
        member_delete(member);
        member = next;
    }
}

size_t record_size(const record_t* record) {
    fatal("TODO record_size();");
    return 0;
}

void record_add(record_t* record, string_t* name, struct type_t* type, unsigned offset) {

    // add member
    member_t* member = member_new(name, type);
    member->next = record->member_list;
    record->member_list = member;
    if (!string_is_empty(name))
        table_put(&record->member_map, &member->map_entry, string_hash(name));

    // update size
    unsigned end = offset + type_size(type);
    if (end > record->size) {
        record->size = end;
    }
}
