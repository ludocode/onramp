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

#ifndef ONRAMP_LIBO_ERROR_H_INCLUDED
#define ONRAMP_LIBO_ERROR_H_INCLUDED

#include <stdarg.h>

extern const char error_out_of_memory[];

extern char* current_filename;

extern int current_line;

void set_current_filename(const char* filename);

/**
 * Prints the prefix for a fatal error message.
 *
 * This can be used for custom formatting of an error message.
 */
void fatal_prefix(void);

/**
 * Prints the given formatted error message along with the current file and
 * line and exits the program.
 */
_Noreturn
void fatal(const char* format, ...);

_Noreturn
void vfatal(const char* format, va_list args);

void warning(const char* format, ...);

void vwarning(const char* format, va_list args);

#endif
