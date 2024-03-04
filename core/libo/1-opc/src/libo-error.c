/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Fraser Heavy Software
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

#define _GNU_SOURCE

#include "libo-error.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char error_out_of_memory[] = "Out of memory.";
char* current_filename = NULL;
int current_line = 0;

void set_current_filename(const char* filename) {
    free(current_filename);
    if (filename != NULL) {
        current_filename = strdup(filename);
        if (current_filename == NULL) {
            fatal(error_out_of_memory);
        }
    }
}

static void print_error(const char* prefix, const char* format, va_list args) {
    fputs(prefix, stderr);
    if (current_filename != NULL) {
        fputs(" at ", stderr);
        fputs(current_filename, stderr);
        fputc(':', stderr);
        fprintf(stderr, "%i", current_line);
    }
    fputs(": ", stderr);
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
}

_Noreturn
void fatal(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfatal(format, args);
    // no va_end(); vfatal() doesn't return
}

_Noreturn
void vfatal(const char* format, va_list args) {
    print_error("ERROR", format, args);
    exit(1);
}

void warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vwarning(format, args);
    va_end(args);
}

void vwarning(const char* format, va_list args) {
    print_error("WARNING", format, args);
}
