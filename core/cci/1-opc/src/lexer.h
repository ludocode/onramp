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

#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include <stdbool.h>

typedef int lexer_type_t;

// TODO convert to all caps
#define lexer_type_alphanumeric 'a'
#define lexer_type_number 'n'
#define lexer_type_character 'c'
#define lexer_type_string 's'
#define lexer_type_punctuation 'p'
#define lexer_type_end 'e'

const char* lexer_type_to_string(lexer_type_t type);

/**
 * The current token being processed.
 *
 * In the case of an alphanumeric token, a number, or punctuation
 * (operators, braces, etc.), this is the literal token text as parsed.
 *
 * In the case of a string or character literal, this is the content of the
 * string with escape sequences parsed.
 *
 * When the end of the input has been reached, this is an empty string.
 */
extern char* lexer_token;

/**
 * The type of the current token.
 */
extern lexer_type_t lexer_type;

/**
 * Initializes the lexer, opening the given `.i` preprocessed C source file.
 */
void lexer_init(const char* filename);

/**
 * Destroys the lexer.
 */
void lexer_destroy(void);

/**
 * Consumes the current token, replacing current_token with the next one from
 * the file.
 *
 * If you want to keep the token, call lexer_take() instead.
 */
void lexer_consume(void);

/**
 * Consumes the current token, returning ownership of it.
 *
 * You must free the returned string.
 */
char* lexer_take(void);

/**
 * Verifies that the current token matches that given and consumes it.
 *
 * It the current token does not match, a fatal error is raised.
 *
 * This always fails if the current token is a string, a character literal or
 * the end of the input.
 */
void lexer_expect(const char* token, const char* error_message);

/**
 * Consumes the current token and returns true if it matches that given;
 * returns false otherwise.
 *
 * This always returns false if the current token is a string literal, a
 * character literal or the end of the input.
 */
bool lexer_accept(const char* token);

/**
 * Returns true if the current token matches that given without consuming it.
 *
 * This always returns false if the current token is a string literal, a
 * character literal or the end of the input.
 */
bool lexer_is(const char* token);

#endif
