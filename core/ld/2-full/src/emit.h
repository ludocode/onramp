/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
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

#ifndef EMIT_H_INCLUDED
#define EMIT_H_INCLUDED

#include "common.h"

void emit_init(void);
void emit_destroy(void);

void emit_byte(char c);
void emit_short(int s);
void emit_int(int s);

/**
 * Emits the current byte count to the debug file.
 */
void emit_byte_count(void);

/**
 * Sets the source location for subsequent bytes.
 *
 * If filename is null, it means the file hasn't changed (the previous filename
 * is kept.)
 *
 * This emits a #line directive in the debug info (or a bare # for a line
 * increment.)
 */
void emit_source_location(const char* /*nullable*/ filename, int line);

/**
 * Sets the symbol name for subsequent bytes.
 *
 * This emits a #symbol directive in the debug info.
 */
void emit_symbol(const char* symbol);

/**
 * Increments the line number by 1 (and emits a byte count for the previous
 * location.)
 */
void emit_increment_line(int line);

/**
 * Emits a char directly to the debug file.
 */
void emit_debug(char c);

#endif
