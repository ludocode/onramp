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

static table_t* temporary_table;

// Takes ownership of name
static temporary_t* temporary_new(string_t* name) {
    temporary_t* temporary = malloc(sizeof(temporary_t));
    temporary->name = name;
    return temporary;
}

static void temporary_delete(temporary_t* temporary) {
    string_deref(temporary->name);
    free(temporary);
}

temporary_t* temporary_find_or_insert(const char* cname) {
    string_t* name = string_intern_cstr(cname);

    for (table_entry_t* entry = table_bucket(temporary_table, string_hash(name));
            entry; entry = table_entry_next(entry))
    {
        temporary_t* temporary = (temporary_t*)entry;
        if (string_equal(temporary->name, name)) {
            string_deref(name);
            return temporary;
        }
    }

    temporary_t* temporary = temporary_new(name);
    table_put(temporary_table, &temporary->entry, string_hash(name));
    return temporary;
}

void temporary_setup(void) {
    temporary_table = table_new();
}

void temporary_teardown(void) {
    temporary_clear();
    table_delete(temporary_table);
}

void temporary_clear(void) {
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
