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

#include "location.h"

#include <stdlib.h>

#include "libo-error.h"

location_t* location_new(string_t* filename, int line, int column,
        location_t* /*nullable*/ source)
{
    location_t* location = malloc(sizeof(location_t));
    if (!location) {
        fatal("Out of memory.");
    }
    location->filename = filename ? string_ref(filename) : NULL;
    location->line = line;
    location->column = column;
    location->source = source ? location_ref(source) : NULL;
    return location;
}

location_t* location_new_copy(location_t* other) {
    return location_new(other->filename, other->line, other->column,
            other->source);
}

void location_deref(location_t* location) {
    if (location->filename) {
        string_deref(location->filename);
    }
    if (location->source) {
        location_deref(location->source);
    }
    free(location);
}
