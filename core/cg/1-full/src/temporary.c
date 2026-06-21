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

#include "temporary.h"

#include <stdlib.h>

#include "libo-string.h"
#include "libo-vector.h"

static table_t* temporary_table;
static size_t temporary_anonymous_id;

// Takes ownership of name
static temporary_t* temporary_new(string_t* name) {
    temporary_t* temporary = calloc(1, sizeof(temporary_t));
    temporary->name = name;
    temporary->interval_start = TEMPORARY_INTERVAL_INVALID;
    temporary->interval_end = TEMPORARY_INTERVAL_INVALID;
    return temporary;
}

static void temporary_delete(temporary_t* temporary) {
    string_deref(temporary->name);
    free(temporary);
}

temporary_t* temporary_new_anonymous(void) {

    // Generate a name for the temporary. We give it a number starting at the
    // current temporary count in order to avoid collisions.
    if (temporary_anonymous_id < table_count(temporary_table)) {
        temporary_anonymous_id = table_count(temporary_table);
    }
    for (;;) {
        char cname[16];
        sprintf(cname, "%%%zu\n", temporary_anonymous_id++);
        string_t* name = string_intern_cstr(cname);

        // make sure it doesn't already exist
        for (table_entry_t* entry = table_bucket(temporary_table, string_hash(name));
                entry; entry = table_entry_next(entry))
        {
            temporary_t* temporary = (temporary_t*)entry;
            if (string_equal(temporary->name, name)) {
                string_deref(name);
                continue;
            }
        }

        // create the temporary
        temporary_t* temporary = temporary_new(name);
        table_put(temporary_table, &temporary->entry, string_hash(name));
        return temporary;
    }
}

temporary_t* temporary_find_or_create_impl(const char* cname, bool* found) {
    string_t* name = string_intern_cstr(cname);

    for (table_entry_t* entry = table_bucket(temporary_table, string_hash(name));
            entry; entry = table_entry_next(entry))
    {
        temporary_t* temporary = (temporary_t*)entry;
        if (string_equal(temporary->name, name)) {
            *found = true;
            string_deref(name);
            return temporary;
        }
    }

    *found = false;
    temporary_t* temporary = temporary_new(name);
    table_put(temporary_table, &temporary->entry, string_hash(name));
    return temporary;
}

temporary_t* temporary_find_or_create(const char* cname) {
    bool found;
    return temporary_find_or_create_impl(cname, &found);
}

temporary_t* temporary_create(const char* cname) {
    bool found;
    temporary_t* temporary = temporary_find_or_create_impl(cname, &found);
    if (found) {
        return NULL;
    }
    return temporary;
}

void temporaries_setup(void) {
    temporary_table = table_new();
}

void temporaries_teardown(void) {
    temporaries_clear();
    table_delete(temporary_table);
}

void temporaries_clear(void) {
    for (table_entry_t** bucket = table_first_bucket(temporary_table); bucket;
            bucket = table_next_bucket(temporary_table, bucket))
    {
        for (table_entry_t* entry = *bucket; entry;) {
            table_entry_t* next = table_entry_next(entry);
            temporary_delete((temporary_t*)entry);
            entry = next;
        }
    }
    table_remove_all(temporary_table);
}

void temporaries_list_all(vector_t* temporaries) {
    for (table_entry_t** bucket = table_first_bucket(temporary_table); bucket;
            bucket = table_next_bucket(temporary_table, bucket))
    {
        for (table_entry_t* entry = *bucket; entry; entry = table_entry_next(entry)) {
            vector_append(temporaries, entry);
        }
    }
}

int temporary_compare_live_interval(const void* vleft, const void* vright) {
    const temporary_t* left = *(const temporary_t**)vleft;
    const temporary_t* right = *(const temporary_t**)vright;
    //printf("left %s %zu-%zu\n", left->name->bytes, left->interval_start, left->interval_end);
    //printf("right %s %zu-%zu\n", right->name->bytes, right->interval_start, right->interval_end);

    // Lowest start index comes first.
    if (left->interval_start < right->interval_start) {
        return -1;
    }
    if (left->interval_start > right->interval_start) {
        return 1;
    }

    // Otherwise it doesn't matter.
    return 0;
}
