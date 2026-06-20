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

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stdio.h>

#define RSP 0xC
#define RFP 0xD
#define RPP 0xE
#define RIP 0xF

extern FILE* input_file;
extern FILE* output_file;

struct location_t;
struct string_t;

// A monotonically increasing ID using to mark visited blocks, instructions,
// etc. Reset at the end of every function.
extern int pass_id;

_Noreturn
void fatal_loc(struct location_t* location, const char* message);

extern struct string_t* current_filename_string;

/**
 * Sets current_filename to the given string.
 *
 * A strong reference is held to the string instead of copying it. If this is
 * used, set_current_filename() can't be used because it would free the bytes
 * of the string instead of dereferencing it.
 */
void set_current_filename_string(struct string_t* string);

// TODO this stuff is temporary until we move location_t to libo
void current_filename_string_setup(void);
void current_filename_string_teardown(void);

#endif
