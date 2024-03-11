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

field_t* field_new(const char* name, type_t* type, size_t offset, field_t* next) {
    field_t* field = malloc(sizeof(void*) * 4);
    if (!field) {
        fatal("Out of memory.");
    }

    *field = next;

    if (!(*(field + 1) = strdup(name))) {
        fatal("Out of memory.");
    }

    // TODO should we duplicate the type or not? should we just make it reference counted?
    /*
    if (!(*(field + 2) = type_dup(type))) {
        fatal("Out of memory.");
    }
    */
    *(field + 2) = type;

    *(field + 3) = offset;

    return field;
}

void field_delete(field_t* field) {
    free(*(field + 1)); // name
    type_delete(*(field + 2)); // type
    free(field);
}

field_t* field_next(field_t* field) {
    return *field;
}

const char* field_name(field_t* field) {
    return *(field + 1);
}

type_t* field_type(field_t* field) {
    return *(field + 2);
}

size_t field_offset(field_t* field) {
    return (size_t)*(field + 3);
}

/*
void field_set_next(field_t* field, field_t* next) {
    *field = next;
}
*/

size_t field_size(field_t* field) {
    return type_size(field_type(field));
}

size_t field_end(field_t* field) {
    return field_offset(field) + field_size(field);
}
