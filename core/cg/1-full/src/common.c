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

#include "common.h"

#include "location.h"
#include "libo-error.h"
#include "libo-string.h"

FILE* input_file;
FILE* output_file;

int pass_id;

_Noreturn
void fatal_loc(location_t* location, const char* message) {
    // TODO need to merge this down with libo-error
    current_filename = location->filename->bytes;
    current_line = location->line;
    fatal(message);
}

// This is the same string as current_filename. We keep a strong reference to
// it here.
string_t* current_filename_string;

void set_current_filename_string(string_t* string) {
    string_ref(string);
    string_deref(current_filename_string);
    current_filename_string = string;
    current_filename = string->bytes;
}

void set_current_filename_string_cstr(const char* cstr) {
    string_t* string = string_intern_cstr(cstr);
    set_current_filename_string(string);
    string_deref(string);
}

void current_filename_string_setup(void) {
    current_filename_string = string_intern_cstr("<builtin>");
    current_filename = current_filename_string->bytes;
}

void current_filename_string_teardown(void) {
    string_deref(current_filename_string);
}
