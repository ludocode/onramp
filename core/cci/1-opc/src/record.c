#ifdef DISABLED
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

/*
 * record_t looks like this:
 *
 * typedef struct {
 *     char* name;
 *     field_t* fields;
 * } record_t;
 *
 * TODO we need an additional field to say whether it's a struct or union (or
 * do we? would that be something for the type table instead?)
 */

record_t* record_new(const char* name) {
    record_t* record = malloc(sizeof(void*) * 2);
    if (!record) {
        fatal("Out of memory.");
    }

    *record = strdup(name);
    if (!*record) {
        fatal("Out of memory.");
    }

    *(record + 1) = NULL; // fields
    return record;
}

static void record_destroy_fields(record_t* record) {
    field_t* field = record_fields(record);
    while (field) {
        field_t* next = field_next(field);
        field_delete(field);
        field = next;
    }
}

void record_delete(record_t* record) {
    record_destroy_fields(record);
    free(*record); // name
    free(record);
}

const char* record_name(record_t* record) {
    return *record;
}

field_t* record_fields(record_t* record) {
    return *(record + 1);
}

void record_set_fields(record_t* record, field_t* fields) {
    record_destroy_fields(record);
    *(record + 1) = fields;
}

size_t record_size(record_t* record) {
    size_t size = 4;
    field_t* field = record_fields(record);
    while (field) {
        size_t end = field_end(field);
        if (end > size) {
            size = end;
        }
        field = field_next(field);
    }
    field = ((field + 3) & (~3)); // round up to multiple of word size (4)
    return field;
}
#endif
