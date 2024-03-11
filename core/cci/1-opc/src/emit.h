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

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

void emit_init(const char* output_filename);
void emit_destroy(void);

/**
 * Toggles output on or off.
 *
 * When off, nothing is emitted except for line directives; all other emit
 * functions are ignored. This is used to implement sizeof().
 *
 * Output is on by default.
 */
void emit_set_enabled(bool enabled);

bool emit_is_enabled(void);

void emit_char(char c);
void emit_string(const char* c);

/**
 * Emits a term followed by a space.
 *
 * A "term" is one part of an instruction. It can be basically any string.
 * Usually this is used for instruction opcodes, register names, and simple
 * hardcoded arguments like 0 and 1.
 */
void emit_term(const char* term);

void emit_register(int index);
void emit_label(char type, const char* label_name);
void emit_prefixed_label(char type, const char* prefix, const char* label_name);
void emit_computed_label(char type, const char* prefix, int label);
void emit_int(int value);
void emit_string_literal(const char* str);
void emit_character_literal(char c);
void emit_quoted_byte(char byte);
void emit_newline(void);
void emit_zero(void);
void emit_global_divider(void);

/**
 * Emits a directive that indicates that a newline was encountered in the
 * source file.
 */
void emit_line_increment_directive(void);

void emit_line_directive(void);

#endif
