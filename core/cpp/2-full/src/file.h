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

#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include "libo-string.h"

struct token_t;

/*
 * A conditional branch (`#if`, `#ifdef` or `#ifndef`.)
 *
 * We use a global conditional stack, not a per-file stack. Each conditional
 * points to the lexer that contained it so we can make sure conditionals don't
 * cross file boundaries.
 */
typedef struct conditional_t {
    struct token_t* token;
    bool taken;
} conditional_t;

/**
 * State tracking whether this file has a standard include guard.
 */
typedef enum guard_state_t {
    guard_state_start,  // no tokens or directives have yet been seen
    guard_state_ifndef, // `#ifndef` was seen in state `start`; macro was recorded
    guard_state_define, // `#define` was seen with matching macro in state `ifndef`
    guard_state_endif,  // matching `#endif` was seen in state `define`
    guard_state_fail,   // file does not have an acceptable include guard
} guard_state_t;

/**
 * A file_t is a container for all of the state of a file.
 */
typedef struct file_t {
    struct lexer_t* lexer;

    // The stack of conditionals in this file (i.e. `#if`, `#ifdef`)
    struct conditional_t* conditionals;
    size_t conditionals_capacity;
    size_t conditionals_count;

    // Include guard
    guard_state_t guard_state;
    string_t* /*nullable*/ guard_name;
} file_t;

// The current file being parsed.
// This can change at any time; we accept `#include` even inside macro arguments.
extern file_t* file_current;

file_t* file_new(string_t* filename, FILE* cfile, struct token_t* source);

void file_delete(file_t* file);

void file_conditional_push(file_t* file, struct token_t* token);
void file_conditional_pop(file_t* file, struct token_t* token);

#endif
