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

#include "variable.h"

#include <stdlib.h>

#include "libo-string.h"

static table_t* variable_table;

// Takes ownership of name
static variable_t* variable_new(string_t* name) {
    variable_t* variable = malloc(sizeof(variable_t));
    variable->name = name;
    return variable;
}

static void variable_delete(variable_t* variable) {
    string_deref(variable->name);
    free(variable);
}

variable_t* variable_find_or_insert(const char* cname) {
    string_t* name = string_intern_cstr(cname);

    for (table_entry_t* entry = table_bucket(variable_table, string_hash(name));
            entry; entry = table_entry_next(entry))
    {
        variable_t* variable = (variable_t*)entry;
        if (string_equal(variable->name, name)) {
            string_deref(name);
            return variable;
        }
    }

    variable_t* variable = variable_new(name);
    table_put(variable_table, &variable->entry, string_hash(name));
    return variable;
}

void variable_setup(void) {
    variable_table = table_new();
}

void variable_teardown(void) {
    variable_clear();
    table_delete(variable_table);
}

void variable_clear(void) {
    for (table_entry_t** bucket = table_first_bucket(variable_table); bucket;
            bucket = table_next_bucket(variable_table, bucket))
    {
        for (table_entry_t* entry = *bucket; entry;) {
            table_entry_t* next = table_entry_next(entry);
            variable_delete((variable_t*)entry);
            entry = next;
        }
    }
    table_remove_all(variable_table);
}
