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
#include <string.h>

#include "field.h"

/*
 * record_t looks like this:
 *
 * typedef struct {
 *     char* name;
 *     field_t* fields;
 *     size_t size;
 * } record_t;
 *
 * A record is created when it is first declared. Structs and unions can be
 * forward-declared; the fields are added and the size is computed later when
 * it is defined.
 */

#define RECORD_OFFSET_NAME 0
#define RECORD_OFFSET_FIELDS 1
#define RECORD_OFFSET_SIZE 2
#define RECORD_FIELD_COUNT 3

record_t* record_new(char* name) {
    record_t* record = malloc(sizeof(void*) * RECORD_FIELD_COUNT);
    if (!record) {
        fatal("Out of memory.");
    }
    *(char**)((void**)record + RECORD_OFFSET_NAME) = name;
    *(field_t**)((void**)record + RECORD_OFFSET_FIELDS) = NULL;
    *(size_t*)((void**)record + RECORD_OFFSET_SIZE) = 0;
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
    free((char*)record_name(record));
    free(record);
}

const char* record_name(const record_t* record) {
    return *(char**)((void**)record + RECORD_OFFSET_NAME);
}

field_t* record_fields(const record_t* record) {
    return*(field_t**)((void**)record + RECORD_OFFSET_FIELDS); 
}

void record_set_fields(record_t* record, field_t* fields) {
    if (record_fields(record) != NULL) {
        fatal_2("Internal error: cannot change fields of record ", record_name(record));
    }
    *(field_t**)((void**)record + RECORD_OFFSET_FIELDS) = fields;

    // compute the size and cache it
    size_t size = 4;
    while (fields) {
        size_t end = field_end(fields);
        if (end > size) {
            size = end;
        }
        fields = field_next(fields);
    }
    size = ((size + 3) & (~3)); // round up to multiple of word size (4)
    *(size_t*)((void**)record + RECORD_OFFSET_SIZE) = size;
}

size_t record_size(const record_t* record) {
    size_t size = *(size_t*)((void**)record + RECORD_OFFSET_SIZE);
    if (size == 0) {
        fatal("Cannot `sizeof` an incomplete struct or union.");
    }
    return size;
}

const field_t* record_find_field(const record_t* record, const char* name) {
    if (record_fields(record) == NULL) {
        fatal("Cannot access members of an incomplete struct or union.");
    }
    field_t* field = record_fields(record);
    while (field) {
        if (0 == strcmp(name, field_name(field))) {
            return field;
        }
        field = field_next(field);
    }
    fatal_2("Struct or union has no such field: ", name);
}
