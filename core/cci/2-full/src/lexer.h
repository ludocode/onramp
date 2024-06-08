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

#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include <stdbool.h>

#include "libo-string.h"

struct token_t;

/**
 * The current token.
 */
extern struct token_t* lexer_token;

/**
 * Initializes the lexer, opening the given `.i` preprocessed C source file.
 */
void lexer_init(const char* filename);

/**
 * Destroys the lexer.
 */
void lexer_destroy(void);

/**
 * Consumes the current token, replacing lexer_token with the next one from
 * the file.
 *
 * If you want to keep the token, call lexer_take() instead.
 */
void lexer_consume(void);

/**
 * Consumes the current token, returning a strong reference to it.
 *
 * You must dereference the returned token when done.
 */
struct token_t* lexer_take(void);

/**
 * "Un-consumes" the given token, pushing it to the front of the token queue.
 *
 * This will increment the reference count of the given token. You must still
 * dereference it yourself if you are done with it.
 */
void lexer_push(struct token_t* token);

/**
 * Verifies that the current token matches the given alphanumeric or
 * punctuation and consumes it.
 *
 * It the current token does not match, a fatal error is raised with the given
 * error message.
 *
 * This always fails if the current token is not alphanumeric or punctuation.
 */
void lexer_expect(const string_t* value, const char* error_message);

/**
 * Consumes the current token and returns true if it matches the given
 * alphanumeric or punctuation; returns false otherwise.
 *
 * This always returns false if the current token is not alphanumeric or
 * punctuation.
 */
bool lexer_accept(const string_t* value);

/**
 * Returns true if the current token matches the given alphanumeric or
 * punctuation without consuming it.
 *
 * This always returns false if the current token is a string literal, a
 * character literal or the end of the input.
 */
bool lexer_is(const string_t* value);

void lexer_dump_tokens(void);

#endif
