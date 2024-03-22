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

#include "common.h"

void parse_decl_init(void);
void parse_decl_destroy(void);

extern int function_frame_size;
extern bool inside_function;
extern global_t* current_function;

/**
 * A storage class.
 */
typedef int storage_t;

#define STORAGE_DEFAULT 0
#define STORAGE_TYPEDEF 1
#define STORAGE_EXTERN 2
#define STORAGE_STATIC 3

/**
 * Tries to parse a declaration specifier list.
 *
 * If storage is NULL, storage specifiers are not allowed.
 */
bool try_parse_declaration_specifiers(
        type_t** out_type, 
        storage_t* /*nullable*/ out_storage);

/**
 * Tries to parse a declarator using the given base type as the declaration
 * specifier list.
 *
 * Note that we don't include a function's argument list or a variable's
 * initialization list. They are parsed separately.
 *
 * If out_name is NULL, this parses an abstract declarator (a name is not
 * allowed.) If out_name is not NULL and a name is not found, the string
 * pointed to be out_name will be set to NULL.
 */
bool try_parse_declarator(const type_t* base_type, type_t** out_type,
        char** /*nullable*/ out_name);

/**
 * Parses a declarator using the given base type as the declaration specifier
 * list.
 *
 * Note that we don't include a function's argument list or a variable's
 * initialization list. They are parsed separately.
 *
 * If out_name is NULL, this parses an abstract declarator (a name is not
 * allowed.) If out_name is not NULL and a name is not found, the string
 * pointed to be out_name will be set to NULL.
 */
/*
void parse_declarator(const type_t* base_type, type_t** out_type,
        char** out_name);
        */

/**
 * Tries to parse a declaration.
 *
 * If out_name is NULL, this parses an abstract declarator (a name is not
 * allowed.) If out_name is not NULL and a name is not found, the string
 * pointed to be out_name will be set to NULL.
 *
 * If storage is NULL, storage specifiers are not allowed. TODO we can remove
 * this parameter entirely soon, nothing else will use it
 */
bool try_parse_declaration(
        storage_t* /*nullable*/ out_storage,
        type_t** out_type,
        char** /*nullable*/ out_name);

/**
 * Tries to parse a declaration of a local variable plus its initializer,
 * adding the local and compiling the initializer as needed.
 */
bool try_parse_local_declaration(void);

/**
 * Parses a global variable or function, compiling it to the output.
 */
void parse_global(void);

#endif
