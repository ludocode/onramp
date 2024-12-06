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

// TODO external compilers don't properly support char8_t yet
typedef unsigned char char8_t;

#include "libo-table.h"
#include "libo-string.h"
#include "common.h"

struct hideset_t;

typedef enum token_type_t {
    token_type_directive = 'd',      // `#` at the start of a line
    token_type_alphanumeric = 'a',
    token_type_number = 'n',
    token_type_character = 'c',
    token_type_string = 's',
    token_type_angle_include = 'v',  // angle-bracketed include path
    token_type_punctuation = 'p',
    token_type_space = 'w',   // horizontal whitespace
    token_type_newline = 'l', // end-of-line token
    token_type_end = 'e',
    token_type_invalid = 'i',
} token_type_t;

const char* token_type_to_string(token_type_t type);

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
 */
typedef struct token_t {
    unsigned refcount;

    token_type_t type;
    string_t* value;
    location_t location;
    token_prefix_t prefix;

    // macro expansion
    struct hideset_t* hideset;
} token_t;

/**
 * A sentinel end-of-stream token.
 */
extern token_t* token_end;

void token_setup(void);
void token_teardown(void);

token_t* token_new(token_type_t type, string_t* value, location_t* location);

token_t* token_new_bytes(token_type_t type, const char8_t* bytes, size_t count, location_t* location);

/**
 * Clones the given token for macro expansion.
 *
 * The new token's location and hideset are replaced with those given.
 */
token_t* token_new_expansion(token_t* token, location_t* location, struct hideset_t* hideset);


/**
 * Returns true if the given token is of type punctuation and its value matches
 * the given string.
 */
bool token_is_punctuation(token_t* token, string_t* punctuation);

/**
 * Returns true if the given token is of type alphanumeric and its value matches
 * the given string.
 */
bool token_is_keyword(token_t* token, string_t* keyword);

static inline token_t* token_ref(token_t* token) {
    ++token->refcount;
    return token;
}

void token_deref(token_t* token);

// debugging
void token_print(token_t* token);

#endif
