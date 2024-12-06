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

#include <stdint.h>
#include <stdio.h>

#include "token.h"
#include "libo-string.h"
#include "libo-reader.h"

/**
 * State tracking whether we should attempt to parse an angle-bracketed
 * `#include` path.
 */
typedef enum lexer_include_state_t {
    lexer_include_state_none = 0,
    lexer_include_state_directive,  // last token was `#` at start of line
    lexer_include_state_include,    // last token was `include` and previous state was `directive`
} lexer_include_state_t;

/**
 * A lexer parses tokens from a file.
 */
typedef struct lexer_t {

    // The UTF-8 file reader.
    reader_t reader;

    // We have a lookahead of two newline-normalized chars to more easily parse
    // comments. TODO no not anymore. i don't remember what previous_char is to be honest
    char32_t previous_char;
    char32_t next_char;

    location_t char_location; // location of next_char
    location_t token_location; // location of the start of the current token
    bool start_of_line; // true if we've seen a logical newline since parsing a non-whitespace token

    // The most recently parsed token.
    token_t* token;

    // A buffer for accumulating token bytes
    char8_t* buffer;
    size_t buffer_capacity;
    size_t buffer_count;

    // Other relevant state.
    struct lexer_t* /*nullable*/ parent;  // The file that included this one
    location_t parent_location;  // The location in the parent where we were included
    lexer_include_state_t include_state;

} lexer_t;

/**
 * The current lexer (and, by the parent member, the stack of lexers.)
 *
 * TODO is this right? or should it be a stack of streams instead of a stack of lexers?
 */
extern lexer_t* lexer_current;

void lexer_setup(void);
void lexer_teardown(void);

/*
 * Creates a new lexer that will tokenize the given file.
 *
 * If the file has already been opened, it can be passed in. Otherwise the file
 * will be opened.
 */
lexer_t* lexer_new_file(string_t* filename, FILE* /*nullable*/ file, token_t* /*nullable*/ source);

/**
 * Creates a new lexer that tokenizes the given byte array.
 */
lexer_t* lexer_new_bytes(string_t* filename, char* bytes, size_t length);

void lexer_delete(lexer_t* lexer);

/**
 * Peeks the next token.
 *
 * The token remains in the lexer and the lexer retains ownership of it.
 */
token_t* lexer_peek(lexer_t* lexer);

/**
 * Pops the next token, returning it.
 *
 * The caller takes ownership of the returned token and is responsible for
 * deleting it.
 */
token_t* lexer_take(lexer_t* lexer);

/**
 * Pushes a token back onto the lexer.
 */
void lexer_push(lexer_t* lexer, token_t*);

void lexer_consume(lexer_t* lexer);

/**
 * Unloads the file opened by this lexer.
 */
static inline void lexer_unload(lexer_t* lexer) {
    reader_unload(&lexer->reader);
}

#endif
