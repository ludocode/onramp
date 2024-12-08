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

#include "strings.h"

// keywords
string_t* STR_DEFINE;
string_t* STR_ELIF;
string_t* STR_ELIFDEF;
string_t* STR_ELIFNDEF;
string_t* STR_ELSE;
string_t* STR_EMBED;
string_t* STR_ENDIF;
string_t* STR_ERROR;
string_t* STR_IF;
string_t* STR_IFDEF;
string_t* STR_IFNDEF;
string_t* STR_INCLUDE;
string_t* STR_INCLUDE_NEXT;
string_t* STR_LINE;
string_t* STR_PRAGMA;
string_t* STR_UNDEF;
string_t* STR_WARNING;

// punctuation
string_t* STR_PAREN_CLOSE;             // )
string_t* STR_PAREN_OPEN;              // (
string_t* STR_DOT;                     // .
string_t* STR_COMMA;                   // ,
string_t* STR_ELLIPSIS;                // ...
string_t* STR_HASH;                    // #
string_t* STR_HASH_HASH;               // ##

// miscellaneous
string_t* STR_SPACE;          // " "
string_t* STR_BUILT_IN;       // <built-in>
string_t* STR_COMMAND_LINE;   // <command line>

void strings_setup(void) {

    // keywords
    STR_DEFINE = string_intern_cstr("define");
    STR_ELIF = string_intern_cstr("elif");
    STR_ELIFDEF = string_intern_cstr("elifdef");
    STR_ELIFNDEF = string_intern_cstr("elifndef");
    STR_ELSE = string_intern_cstr("else");
    STR_EMBED = string_intern_cstr("embed");
    STR_ENDIF = string_intern_cstr("endif");
    STR_ERROR = string_intern_cstr("error");
    STR_IF = string_intern_cstr("if");
    STR_IFDEF = string_intern_cstr("ifdef");
    STR_IFNDEF = string_intern_cstr("ifndef");
    STR_INCLUDE = string_intern_cstr("include");
    STR_INCLUDE_NEXT = string_intern_cstr("include_next");
    STR_LINE = string_intern_cstr("line");
    STR_PRAGMA = string_intern_cstr("pragma");
    STR_UNDEF = string_intern_cstr("undef");
    STR_WARNING = string_intern_cstr("warning");

    // punctuation
    STR_PAREN_CLOSE                 = string_intern_cstr(")");
    STR_PAREN_OPEN                  = string_intern_cstr("(");
    STR_DOT                         = string_intern_cstr(".");
    STR_COMMA                       = string_intern_cstr(",");
    STR_ELLIPSIS                    = string_intern_cstr("...");
    STR_HASH                        = string_intern_cstr("#");
    STR_HASH_HASH                   = string_intern_cstr("##");

    // miscellaneous
    STR_SPACE = string_intern_cstr(" ");
    STR_BUILT_IN = string_intern_cstr("<built-in>");
    STR_COMMAND_LINE = string_intern_cstr("<command line>");
}

void strings_teardown(void) {

    // keywords
    string_deref(STR_DEFINE);
    string_deref(STR_ELIF);
    string_deref(STR_ELIFDEF);
    string_deref(STR_ELIFNDEF);
    string_deref(STR_ELSE);
    string_deref(STR_EMBED);
    string_deref(STR_ENDIF);
    string_deref(STR_ERROR);
    string_deref(STR_IF);
    string_deref(STR_IFDEF);
    string_deref(STR_IFNDEF);
    string_deref(STR_INCLUDE);
    string_deref(STR_INCLUDE_NEXT);
    string_deref(STR_LINE);
    string_deref(STR_PRAGMA);
    string_deref(STR_UNDEF);
    string_deref(STR_WARNING);

    // punctuation
    string_deref(STR_PAREN_CLOSE);
    string_deref(STR_PAREN_OPEN);
    string_deref(STR_DOT);
    string_deref(STR_COMMA);
    string_deref(STR_ELLIPSIS);
    string_deref(STR_HASH);
    string_deref(STR_HASH_HASH);

    // miscellaneous
    string_deref(STR_SPACE);
    string_deref(STR_BUILT_IN);
    string_deref(STR_COMMAND_LINE);
}
