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

#ifndef STRINGS_H_INCLUDED
#define STRINGS_H_INCLUDED

#include "libo-string.h"

// keywords
extern string_t* STR_DEFINE;
extern string_t* STR_ELIF;
extern string_t* STR_ELIFDEF;
extern string_t* STR_ELIFNDEF;
extern string_t* STR_ELSE;
extern string_t* STR_EMBED;
extern string_t* STR_ENDIF;
extern string_t* STR_ERROR;
extern string_t* STR_IF;
extern string_t* STR_IFDEF;
extern string_t* STR_IFNDEF;
extern string_t* STR_INCLUDE;
extern string_t* STR_INCLUDE_NEXT;
extern string_t* STR_LINE;
extern string_t* STR_PRAGMA;
extern string_t* STR_UNDEF;
extern string_t* STR_WARNING;

// punctuation
extern string_t* STR_PAREN_CLOSE;             // )
extern string_t* STR_PAREN_OPEN;              // (
extern string_t* STR_DOT;                     // .
extern string_t* STR_COMMA;                   // ,
extern string_t* STR_ELLIPSIS;                // ...

// miscellaneous
extern string_t* STR_BUILT_IN;       // <built-in>
extern string_t* STR_COMMAND_LINE;   // <command line>

void strings_setup(void);
void strings_teardown(void);

#endif
