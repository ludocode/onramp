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

#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "common.h"

extern FILE* input_file;

//! The most recently parsed identifier or label name
extern char* identifier;

//! The current character (the last read, not yet consumed), or -1 at the end
//! of the file
extern char current_char;

extern int label_flags;
extern label_type_t label_type;

//! Consumes the current char and reads a new one into current_char.
void read_char(void);

bool try_parse_invocation_absolute(void);
bool try_parse_invocation_relative(void);
bool try_parse_invocation_short(void); // high, low or relative
bool try_parse_identifier(void);
bool try_parse_number(int32_t* value);
bool try_parse_quoted_byte(uint8_t* out);
bool try_parse_character_or_quoted_byte(uint8_t* out);
bool try_parse_and_emit_short(void);
bool try_parse_and_emit_string(size_t* out_length);
bool try_parse_register(uint8_t* out);

void parse_invocation_absolute(void);
void parse_invocation_relative(void);
uint8_t parse_quoted_byte(void);
uint8_t parse_numbered_register(void);
uint8_t parse_register(void);
uint8_t parse_register_non_scratch(void);
uint8_t parse_register_numbered(void);
uint8_t parse_mix(void);
uint8_t parse_mix_non_scratch(void); // excludes scratch registers required for implementing compound instructions
uint8_t parse_syscall_number(void);

//! Performs one step of parsing. Returns false at end-of-file.
bool parse(void);

#endif
