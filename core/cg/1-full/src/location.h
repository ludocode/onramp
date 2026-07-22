/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025-2026 Fraser Heavy Software
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
 * The location contains a reference to its source. This is used to display the
 * chain of include files leading to a particular error.
 *
 * Note that memory management for source locations is not handled by the
 * location itself. It is up to the owner of the location to also maintain
 * ownership of the source chain (by reference counting tokens for example.)
 *
 * TODO: This should be moved to libo/1, but cpp/2 and cci/2 have to be
 * converted to match first.
 */
typedef struct location_t {
    string_t* filename;
    unsigned line;
    unsigned column;
    struct location_t* /*nullable*/ source;
} location_t;

void location_setup(void);
void location_teardown(void);

/**
 * Initializes a source location.
 * 
 * This doesn't take a column number to keep the argument count at 4 (the limit
 * of cci/0.) It is initialized to 0 and can be set separately if used.
 */
void location_init(
        location_t* location,
        string_t* /*nullable*/ filename,
        int line,
        location_t* /*nullable*/ source);

void location_destroy(location_t* location);

location_t* location_new(
        string_t* /*nullable*/ filename,
        int line,
        location_t* /*nullable*/ source);

/**
 * Creates a location based on current_filename and current_line.
 */
location_t* location_new_current(void);

/**
 * Clones a location.
 */
location_t* location_new_copy(const location_t* other);

void location_delete(location_t* location);

void location_set_filename(location_t* location, string_t* /*nullable*/ filename);

void location_print(location_t* location);

#endif
