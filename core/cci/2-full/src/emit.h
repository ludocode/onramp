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

#ifndef EMIT_H_INCLUDED
#define EMIT_H_INCLUDED

#include <stdint.h>

#include "libo-string.h"

struct function_t;
struct token_t;

void emit_init(const char* output_filename);
void emit_destroy(void);

void emit_global_divider(void);
void emit_newline(void);
void emit_char(char c);
void emit_cstr(const char* cstr);
void emit_string(const string_t* string);
void emit_number(int number);
void emit_hex_number(int number);
void emit_quoted_byte(char byte);

void emit_string_literal(const char* str);

void emit_arg_mix(int8_t byte);
void emit_arg_number(int number);
void emit_arg_invocation_prefix(char sigil, const char* prefix, int number);
void emit_arg_invocation(char sigil, const char* prefix);

void emit_function(struct function_t* function);

/**
 * Emits `#line` directives as needed to set the source location to that of the
 * given token.
 */
void emit_source_location(struct token_t* token);

#endif
