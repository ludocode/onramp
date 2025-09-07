/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Fraser Heavy Software
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

#ifndef LOCATION_H_INCLUDED
#define LOCATION_H_INCLUDED

#include "libo-string.h"

/**
 * A source location.
 *
 * This will eventually be moved to libo/1, but cpp/2 has to be converted to
 * match this first.
 */
typedef struct location_t {
    unsigned refcount;
    string_t* /*nullable*/ filename;
    unsigned line;
    unsigned column;
    struct location_t* /*nullable*/ source;
} location_t;

location_t* location_new(string_t* filename, int line, int column,
        location_t* /*nullable*/ source);

location_t* location_new_copy(location_t* other);

static inline location_t* location_ref(location_t* location) {
    ++location->refcount;
    return location;
}

void location_deref(location_t* location);

#endif
