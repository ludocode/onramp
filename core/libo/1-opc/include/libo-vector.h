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

#ifndef ONRAMP_LIBO_VECTOR_H_INCLUDED
#define ONRAMP_LIBO_VECTOR_H_INCLUDED

#include <assert.h>
#include <stddef.h>

/**
 * A vector is a growable array of void pointers.
 */
typedef struct vector_t {
    void** elements;
    size_t count;
    size_t capacity;
} vector_t;

void vector_init(vector_t* vector);

void vector_destroy(vector_t* vector);

#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define vector_count(vector) ((vector)->count)
    #endif
#endif
#ifndef vector_count
    static inline size_t vector_count(vector_t* vector) {
        return vector->count;
    }
#endif

static inline void* vector_at(vector_t* vector, size_t index) {
    assert(index < vector->count);
    return vector->elements[index];
}

void vector_append(vector_t* vector, void* element);

void vector_insert(vector_t* vector, size_t index, void* element);

void* vector_remove(vector_t* vector, size_t index);

void* vector_remove_last(vector_t* vector);

#endif
