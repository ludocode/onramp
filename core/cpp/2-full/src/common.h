/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2025 Fraser Heavy Software
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

#include <stdarg.h>

#include "libo-string.h"
#include "libo-error.h"

// trace() is used to wrap log statements to debug the preprocessor. We can't
// use an object-like macro because we need to be compilable with the previous
// stage preprocessor. Instead we just temporarily uncomment trace() calls
// wherever we want debug output.
#define trace printf

struct token_t;
struct vector_t;

typedef struct location_t {
    string_t* filename;
    int line;
    int column;
    struct token_t* source;
} location_t;

extern location_t location_builtin;
extern location_t location_command_line;

void location_setup(void);
void location_teardown(void);

void location_init(location_t* location, string_t* filename, int line,
        int column, struct token_t* source);
void location_init_copy(location_t* location, location_t* source);

// TODO these are deprecated
void location_init_builtin(location_t* location);
void location_init_command_line(location_t* location);

void location_set_copy(location_t* dest, location_t* src);
void location_set_filename(location_t* location, string_t* filename);

void location_destroy(location_t* location);

/**
 * Prints a fatal error message at the location of the given token.
 */
_Noreturn
void fatal_token(struct token_t* token, const char* format, ...);

/**
 * Prints a fatal error message at the location of the given token.
 */
_Noreturn
void vfatal_token(struct token_t* token, const char* format, va_list args);

/**
 * Outputs the given token, either appending it to the given vector or emitting
 * it to the output file if the vector is null.
 */
void output_token(struct vector_t* /*nullable*/ output, struct token_t* token);

/**
 * Gets the containing path (i.e. "dirname") of the given filename.
 *
 * This removes the last component from the given path. If the given path does
 * not contain a '/' (i.e. it is a relative path with only a filename), this
 * returns ".".
 *
 * The caller assumes ownership of a strong reference to the returned string.
 */
string_t* path_dirname(string_t* filename);

/**
 * Appends the given filename (which may itself contain path components) to the
 * given path (which may or may not end in "/".)
 *
 * If the filename begins with '/', it is returned as is, and the base path is
 * ignored.
 *
 * The caller assumes ownership of the returned string.
 */
char* path_join(string_t* path, string_t* filename);

void destroy_string_vector(struct vector_t* vector);

#endif
