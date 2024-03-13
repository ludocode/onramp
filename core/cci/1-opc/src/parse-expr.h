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

#ifndef PARSE_EXPR_H_INCLUDED
#define PARSE_EXPR_H_INCLUDED

#include <stdbool.h>

#include "type.h"

void parse_expr_init(void);
void parse_expr_destroy(void);

/**
 * Tries to parse a constant expression.
 */
bool try_parse_constant_expression(type_t** type, int* value);

type_t* parse_expression(void);

// TODO temporarily public for old initializer code
type_t* parse_unary_expression(void);

/**
 * When tentatively parsing a label, if it turns out to not be a label, we've
 * already consumed the identifier token. We stash it here so that the
 * expression parser can find it.
 *
 * This is sort of an ugly workaround to not having real lookahead in our
 * lexer.
 */
void parse_expr_stash_identifier(char* identifier);

#endif
