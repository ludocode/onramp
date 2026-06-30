/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2026 Fraser Heavy Software
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

#ifndef FUNCTION_H_INCLUDED
#define FUNCTION_H_INCLUDED

#include "libo-vector.h"
#include "libo-string.h"
#include "libo-table.h"

struct block_t;
struct type_t;
struct token_t;
struct symbol_t;
struct node_t;
struct record_t;
struct string_t;

/**
 * A function.
 */
typedef struct function_t {
    struct type_t* type;
    struct token_t* name;
    string_t* asm_name;
    struct node_t* root;
    vector_t blocks;
    table_t labels; // map of label strings to nodes
    #ifndef CCI2_IR
    int variadic_offset; // offset above rfp where variadic args start
    #endif
    #ifdef CCI2_IR
    int variadic_temporary; // temporary containing address of first variadic param
    #endif
    int name_label; // label for __func__ string
    struct symbol_t* symbol;
    vector_t records;
    #ifdef CCI2_IR
    vector_t* strings;
    #endif
} function_t;

function_t* function_new(struct type_t* type, struct token_t* name,
        string_t* asm_name, struct node_t* root);

void function_delete(function_t* function);

void function_add_block(function_t* function, struct block_t* block);

void function_add_label(function_t* function, struct node_t* label);

struct node_t* function_find_label(function_t* function, struct string_t* name);

void function_add_record(function_t* function, struct record_t* record);

/**
 * Takes ownership of the given string.
 *
 * This is used for strings we don't want to hold onto manually (e.g.
 * symbol names in argument_t.)
 */
#ifdef CCI2_IR
void function_take_string(function_t* function, struct string_t* string);
#endif

#endif
