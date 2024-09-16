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

#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED

#include "libo-string.h"

typedef enum token_type_t {
    token_type_alphanumeric = 'a',
    token_type_number = 'n',
    token_type_character = 'c',
    token_type_string = 's',
    token_type_punctuation = 'p',
    token_type_end = 'e',
} token_type_t;

/**
 * The prefix for a string or character literal.
 */
typedef enum token_prefix_t {
    token_prefix_none = 0,
    token_prefix_L = 'L',
    token_prefix_u = 'u',
    token_prefix_U = 'U',
    token_prefix_u8 = '8',
} token_prefix_t;

/**
 * A token.
 *
 * Tokens are immutable once created.
 */
typedef struct token_t {
    unsigned refcount;

    token_type_t type;
    string_t* value;
    token_prefix_t prefix;

    // source information
    string_t* filename;
    int line;
    struct token_t* source;
} token_t;

/**
 * Creates a token.
 *
 * The token takes ownership of the given string reference for convenience.
 */
token_t* token_new(
        token_type_t type,
        string_t* value,
        token_prefix_t prefix,
        string_t* filename,
        int line,
        struct token_t* source);

/**
 * Creates a token for a builtin.
 */
token_t* token_new_builtin(const char* name);

/**
 * Creates a token at the source location of another token.
 */
token_t* token_new_at(string_t* name, token_t* source);

static inline token_t* token_ref(token_t* token) {
    ++token->refcount;
    return token;
}

void token_deref(token_t* token);

void token_print(token_t* token);

#endif
