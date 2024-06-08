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

record_t* record_new(string_t* name) {
    record_t* record = malloc(sizeof(record_t));
    record->refcount = 1;
    record->name = string_ref(name);
    record->fields = NULL;
    return record;
}

void record_deref(record_t* record) {
    if (--record->refcount != 0) {
        return;
    }

    string_deref(record->name);

    size_t count = 1 << record->fields_bits;
    for (size_t i = 0; i < count; ++i) {
        /*
        field_t* field = record->fields[i];
        if (field->type) {
            type_deref(field->type);
            string_deref(field->name);
        }
        */
    }
    free(record->fields);
}

size_t record_size(const record_t* record) {
    fatal("TODO record_size();");
    return 0;
}
