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

#ifndef EMIT_H_INCLUDED
#define EMIT_H_INCLUDED

#include "token.h"

void emit_setup(void);
void emit_teardown(void);

void emit_open(void);

/**
 * Emits linemarkers or newlines as needed to position the next token at the
 * given location.
 */
void emit_location(location_t* location, bool force);

/**
 * Emits a linemarker for the start of a file.
 */
void emit_location_start(string_t* filename);

void emit_pragma_file_push(void);
void emit_pragma_file_pop(void);

void emit_newline_if_needed(void);

/**
 * Emits a token at a particular location.
 *
 * TODO I don't think this is actually called except by emit_token(), this is probably useless
 */
void emit_token_at(token_t* token, location_t* location);

/**
 * Emits a token at its original location.
 */
static inline void emit_token(token_t* token) {
    emit_token_at(token, &token->location);
}

#endif
