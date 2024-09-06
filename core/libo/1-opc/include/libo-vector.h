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
#include <stdbool.h>

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

// TODO implement function inlining in cci/2. It would be nice to get rid of
// all of these ugly macros.

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

#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define vector_is_empty(vector) ((vector)->count == 0)
    #endif
#endif
#ifndef vector_is_empty
    static inline bool vector_is_empty(vector_t* vector) {
        return vector->count == 0;
    }
#endif

#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define vector_address(vector, index) ((vector)->elements + (index))
    #endif
#endif
#ifndef vector_address
    /**
     * Gets the address of the element at the given index.
     */
    static inline void** vector_address(vector_t* vector, size_t index) {
        assert(index < vector->count);
        return vector->elements + index;
    }
#endif

#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define vector_at(vector, index) ((vector)->elements[(index)])
    #endif
#endif
#ifndef vector_at
    /**
     * Gets the element at the given index.
     */
    static inline void* vector_at(vector_t* vector, size_t index) {
        assert(index < vector->count);
        return vector->elements[index];
    }
#endif

#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define vector_set(vector, index) ((vector)->elements[(index)] = (value))
    #endif
#endif
#ifndef vector_set
    /*
     * Replaces the element at the given index.
     */
    static inline void vector_set(vector_t* vector, size_t index, void* value) {
        assert(index < vector->count);
        vector->elements[index] = value;
    }
#endif

#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define vector_at(vector, index) ((vector)->elements[(index)])
    #endif
#endif
#ifndef vector_at
    /*
     * Returns the first element. The vector must not be empty.
     */
    static inline void* vector_first(vector_t* vector) {
        assert(vector->count != 0);
        return vector->elements[0];
    }
#endif

/**
 * Changes the number of elements to that given.
 *
 * If this is longer than the current length, the added elements are null. If
 * this is shorter that the current length, the removed elements are discarded
 * (not freed or deleted.)
 */
void vector_resize(vector_t* vector, size_t count);

void vector_ensure_size(vector_t* vector, size_t count);

/*
 * Returns the last element. The vector must not be empty.
 */
void* vector_last(vector_t* vector);

void vector_append(vector_t* vector, void* element);

/*
 * Inserts the given element before the element at the given index.
 */
void vector_insert(vector_t* vector, size_t index, void* element);

void* vector_remove(vector_t* vector, size_t index);

void* vector_remove_last(vector_t* vector);

#endif
