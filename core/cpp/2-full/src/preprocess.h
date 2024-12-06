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

#ifndef PREPROCESS_H_INCLUDED
#define PREPROCESS_H_INCLUDED

struct lexer_t;
struct string_t;
struct vector_t;
struct preprocessor_t;
struct token_t;

#include <stdio.h>

#include "stream.h"

void preprocess_setup(void);
void preprocess_teardown(void);

void preprocess_add_include_path(const char* path);

void preprocess_add_force_include(const char* filename);

/**
 * Prepare to include a new file.
 *
 * This may unload an open file to make room.
 */
void preprocess_prepare_include(void);

/**
 * Performs an `#include` search for the given file and includes it.
 */
void preprocess_include_search(stream_t* stream, struct token_t* token);

void preprocess_output(struct preprocessor_t* preprocessor, struct token_t* token);

void preprocess(const char* root_filename);

#endif
