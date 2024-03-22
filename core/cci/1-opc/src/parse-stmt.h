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

#ifndef PARSE_STMT_H_INCLUDED
#define PARSE_STMT_H_INCLUDED

#include <stdbool.h>

#include "common.h"

extern global_t* current_function;

void parse_stmt_init(void);
void parse_stmt_destroy(void);

int parse_generate_label(void);

/**
 * Stores a string literal to be emitted as a global after the current function
 * is done, returning its id.
 *
 * This consumes the current token, which must be a string literal.
 */
int store_string_literal(void);

/**
 * Parses a block (also referred to as a "compound statement" in the C spec.)
 */
void parse_block(void);

/**
 * Compiles all string literals that were collected during the parsing of the
 * current function.
 */
void output_string_literals(void);

#endif
