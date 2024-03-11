/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
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

#include "libo-vector.h"

#include <stdlib.h>
#include <string.h>

#include "libo-error.h"

void vector_init_blank(vector_t* vector) {
    vector->elements = NULL;
    vector->count = 0;
    vector->capacity = 0;
}

// Ensures there is space for one more value.
static void vector_grow_if_necessary(vector_t* vector) {
    if (vector->count < vector->capacity) {
        return;
    }

    // calculate new capacity
    size_t new_capacity;
    if (vector->capacity == 0) {
        new_capacity = 4;
    }
    if (vector->capacity != 0) {
        new_capacity *= 2;
        if (new_capacity <= vector->capacity) {
            fatal("Out of address space.");
        }
    }

    // realloc
    vector->elements = realloc(vector->elements, new_capacity * sizeof(void*));
    if (vector->elements == NULL) {
        fatal("Out of memory.");
    }
    vector->capacity = new_capacity;
}

void vector_append(vector_t* vector, void* element) {
    vector_grow_if_necessary(vector);
    vector->elements[vector->count] = element;
    ++vector->count;
}

void vector_insert(vector_t* vector, size_t index, void* element) {
    assert(index <= vector->count);
    vector_grow_if_necessary(vector);
    memmove(vector->elements + index + 1,
            vector->elements + index,
            vector->count - index);
    vector->elements[index] = element;
    ++vector->count;
}

void* vector_remove(vector_t* vector, size_t index) {
    assert(index < vector->count);
    void* element = vector->elements[index];
    --vector->count;
    memmove(vector->elements + index,
            vector->elements + index + 1,
            vector->count - index);
    return element;
}

void* vector_remove_last(vector_t* vector) {
    void* element = vector->elements[vector->count - 1];
    vector_remove(vector, vector->count - 1);
    return element;
}
