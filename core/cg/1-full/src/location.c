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

#include "location.h"

#include <assert.h>
#include <stdlib.h>

#include "common.h"
#include "libo-error.h"
#include "libo-util.h"

void location_setup() {
    /*
    string_t* builtin = string_intern_cstr("<builtin>");
    current_location = location_new(builtin, 0, NULL);
    string_deref(builtin);
    */
}

void location_teardown() {
    //location_delete(current_location);
}

void location_init(
        location_t* location,
        string_t* filename,
        int line,
        location_t* /*nullable*/ source)
{
    assert(filename);
    location->filename = string_ref(filename);
    location->line = line;
    location->column = 0;
    location->source = source;
}

void location_destroy(location_t* location) {
    string_deref(location->filename);
}

location_t* location_new(
        string_t* filename,
        int line,
        location_t* /*nullable*/ source)
{
    location_t* location = malloc(sizeof(location_t));
//fprintf(stderr,"location_new %p\n",(void*)location);
    if (!location) {
        fatal("Out of memory.");
    }
    location_init(location, filename, line, source);
    return location;
}

location_t* location_new_copy(const location_t* other) {
    return location_new(
            other->filename,
            other->line,
            other->source);
}

location_t* location_new_current(void) {
    return location_new(current_filename_string, current_line, NULL);
}

void location_delete(location_t* location) {
//fprintf(stderr,"location_delete %p\n",(void*)location);
    location_destroy(location);
    free(location);
}

void location_set_filename(location_t* location, string_t* filename) {
    assert(filename);
    string_ref(filename);
    if (location->filename) {
        string_deref(location->filename);
    }
    location->filename = filename;
}

void location_print(location_t* location) {
    // TODO should be merged with error_prefix
    FILE* file = stdout;
    fputs("ERROR", file);
    fputs(" at ", file);
    fputs(location->filename->bytes, file);
    fputc(':', file);
    fputd(location->line, file);
    if (location->column != 0) {
        fputc(':', file);
        fputd(location->column, file);
    }
    //fputs(": ", file);
}
