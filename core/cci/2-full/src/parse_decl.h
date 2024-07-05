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

#ifndef PARSE_DECL_H_INCLUDED
#define PARSE_DECL_H_INCLUDED

#include <stdbool.h>

struct node_t;
struct type_t;
struct specifiers_t;
struct function_t;

typedef enum storage_t {
    storage_default,
    storage_typedef,
    storage_extern,
    storage_static,
} storage_t;

extern struct function_t* current_function;

void parse_decl_init(void);

void parse_decl_destroy(void);

void parse_global(void);

/**
 * Parses a declaration, which may consist of several comma-separated
 * declarators and initializers.
 */
bool try_parse_declaration(struct node_t* /*nullable*/ parent);

/**
 * Parses a type, i.e. an abstract declarator. These are used in cast
 * expressions, sizeof, typeof, etc.
 */
struct type_t* try_parse_type(void);

#endif
