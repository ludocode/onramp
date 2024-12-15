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

#ifndef STREAM_H_INCLUDED
#define STREAM_H_INCLUDED

#include "libo-vector.h"
#include "libo-string.h"

struct lexer_t;
struct macro_t;
struct token_t;
struct vector_t;

/**
 * A helper to semantically parse and buffer a stream of tokens.
 *
 * The stream has three sources of tokens, served in this order:
 *
 * - An internal stack onto which tokens can be pushed;
 * - An external buffer of tokens, for example the expansion string of a macro;
 * - The current lexer.
 *
 * All of these are optional. Once all sources are exhausted, the stream
 * returns tokens of type end.
 *
 * A stream around a lexer is used for almost all purposes, such as parsing the
 * input files normally, parsing directives, etc. A stream around a token
 * buffer is used when evaluating an `#if` expression after it has been
 * macro-expanded. The stream stack is used to store tokens during macro
 * expansion.
 */
typedef struct stream_t {
    bool use_lexer;
    vector_t stack;
    void** buffer_start;
    void** buffer_end;
} stream_t;

/**
 * Initializes a stream.
 *
 * If use_lexer is true, the stream will serve tokens from the current lexer
 * (when the stack and buffer are empty.)
 *
 * If a buffer is given, tokens will be served from it (when the stack is
 * empty.) The stream does not take ownership of the buffer or its contents;
 * the buffer must not be modified while the stream exists.
 */
void stream_init(stream_t* stream, bool use_lexer, vector_t* /*nullable*/ buffer);

void stream_set_buffer(stream_t* stream, vector_t* vector);

void stream_destroy(stream_t* stream);

/**
 * Reserve space for the given number of tokens to be pushed into the buffer.
 */
void stream_reserve(stream_t* stream, size_t count);

/**
 * Peeks the next token.
 *
 * The token remains in the stream and the stream retains ownership of it.
 */
struct token_t* stream_peek(stream_t* stream);

void stream_consume(stream_t* stream);

/**
 * Push a token into the buffer.
 *
 * The stream takes ownership of the given token.
 */
void stream_push(stream_t* stream, struct token_t* token);

/**
 * Pops the next token, returning it.
 *
 * The caller takes ownership of the returned token and is must decrement its
 * reference count when done.
 */
struct token_t* stream_take(stream_t* stream);

void stream_delete(stream_t* stream);

void stream_dump_tokens(stream_t* stream);
void stream_print_stack(stream_t* stream);

/**
 * Skips all horizontal whitespace tokens.
 */
void stream_skip_horizontal_space(stream_t* stream);

/**
 * Skips all space and newline tokens.
 */
void stream_skip_whitespace(stream_t* stream);

void stream_expect(stream_t* stream, string_t* string, const char* error_message);
bool stream_accept(stream_t* stream, string_t* string);

bool stream_accept_newline(stream_t* stream);

/**
 * Returns true if the current token is alphanumeric or punctuation and matches
 * the given string.
 */
bool stream_is(stream_t* stream, string_t* string);

#endif
