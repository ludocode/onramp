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

#include "libo-util.h"

#ifndef __onramp__
// On Onramp this is defined in libo-data.os because our opC compiler doesn't
// support global initializers.
// TODO use this in cci/2
const char error_out_of_memory[] = "Out of memory.";
#endif

char* current_filename;
int current_line;

void set_current_filename(const char* filename) {
    free(current_filename);
    if (filename != NULL) {
        current_filename = strdup(filename);
        if (current_filename == NULL) {
            fatal(error_out_of_memory);
        }
    }
}

static void error_prefix(const char* type) {
    fflush(stdout);
    fputs(type, stderr);
    if (current_filename != NULL) {
        fputs(" at ", stderr);
        fputs(current_filename, stderr);
        fputc(':', stderr);
        fputd(current_line, stderr);
    }
    fputs(": ", stderr);
}

void fatal_prefix(void) {
    error_prefix("ERROR");
}

static void print_error(const char* type, const char* format, va_list args) {
    error_prefix(type);
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

    // Under ASAN, give us a stack trace (need ASAN_OPTIONS="handle_abort=1")
    #ifdef __SANITIZE_ADDRESS__
    fflush(stdout);
    fflush(stderr);
    //abort();
    #endif

    _Exit(1);
}

void print_warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprint_warning(format, args);
    va_end(args);
}

void vprint_warning(const char* format, va_list args) {
    print_error("WARNING", format, args);
}
