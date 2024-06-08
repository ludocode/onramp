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

#ifndef OPTIONS_H_INCLUDED
#define OPTIONS_H_INCLUDED

#include <stdbool.h>

struct token_t;

extern bool optimization;

extern int dump_ast;
#define DUMP_AST_OFF 0
#define DUMP_AST_UNICODE 1
#define DUMP_AST_ASCII 2

typedef enum warning_t {

    // groups
    warning_all,       // -Wall
    warning_extra,     // -Wextra (or -W)
    warning_pedantic,  // -Wpedantic (or -pedantic)

    // incorrect code
    warning_implicit_int,  // -Wimplicit-int

    // extension usage
    warning_statement_expressions,    // -Wstatement-expressions: ({...})
    warning_extra_keywords,           // -Wasm (or -fasm): asm, inline (pre-C99), typeof (pre-C23)
    
    warning_count,
} warning_t;

void warn(warning_t warning, struct token_t* token, const char* message, ...);

void options_init(void);
void options_destroy(void);

#endif
