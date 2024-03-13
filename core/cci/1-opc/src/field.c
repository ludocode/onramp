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

#include "field.h"

#include <stdlib.h>

#include "type.h"

/*
 * field_t looks like this:
 *
 * typedef struct {
 *     field_t* next;
 *     char* name;
 *     type_t* type;
 *     size_t offset;
 * } field_t;
 *
 */

#define FIELD_OFFSET_NEXT 0
#define FIELD_OFFSET_NAME 1
#define FIELD_OFFSET_TYPE 2
#define FIELD_OFFSET_OFFSET 3
#define FIELD_FIELD_COUNT 4

field_t* field_new(char* name, type_t* type, size_t offset, field_t* next) {
    field_t* field = malloc(sizeof(void*) * FIELD_FIELD_COUNT);
    if (!field) {
        fatal("Out of memory.");
    }
    *(char**)((void**)field + FIELD_OFFSET_NAME) = name;
    *(type_t**)((void**)field + FIELD_OFFSET_TYPE) = type;
    *(size_t*)((void**)field + FIELD_OFFSET_OFFSET) = offset;
    *(field_t**)((void**)field + FIELD_OFFSET_NEXT) = next;
    return field;
}

void field_delete(field_t* field) {
    free((char*)field_name(field));
    type_delete((type_t*)field_type(field));
    free(field);
}

field_t* field_next(const field_t* field) {
    return *(field_t**)((void**)field + FIELD_OFFSET_NEXT);
}

const char* field_name(const field_t* field) {
    return *(char**)((void**)field + FIELD_OFFSET_NAME);
}

type_t* field_type(const field_t* field) {
    return *(type_t**)((void**)field + FIELD_OFFSET_TYPE);
}

size_t field_offset(const field_t* field) {
    return *(size_t*)((void**)field + FIELD_OFFSET_OFFSET);
}

size_t field_size(const field_t* field) {
    return type_size(field_type(field));
}

size_t field_end(const field_t* field) {
    return field_offset(field) + field_size(field);
}
