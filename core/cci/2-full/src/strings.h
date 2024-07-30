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
extern string_t* STR_ALIGNAS;         // alignas
extern string_t* STR_ALIGNAS_X;       // _Alignas
extern string_t* STR_ALIGNOF;         // alignof
extern string_t* STR_ALIGNOF_X;       // _Alignof
extern string_t* STR_ASM;             // asm
extern string_t* STR_ASM_X;           // __asm__
extern string_t* STR_ATOMIC;          // _Atomic
extern string_t* STR_AUTO;            // auto
extern string_t* STR_BITINT;          // _Bitint
extern string_t* STR_BOOL_X;          // _Bool
extern string_t* STR_BREAK;           // break
extern string_t* STR_CASE;            // case
extern string_t* STR_CHAR;            // char
extern string_t* STR_CONST;           // const
extern string_t* STR_CONSTEXPR;       // constexpr
extern string_t* STR_CONTINUE;        // continue
extern string_t* STR_DEFAULT;         // default
extern string_t* STR_DO;              // do
extern string_t* STR_DOUBLE;          // double
extern string_t* STR_ELSE;            // else
extern string_t* STR_ENUM;            // enum
extern string_t* STR_EXTERN;          // extern
extern string_t* STR_FLOAT;           // float
extern string_t* STR_FOR;             // for
extern string_t* STR_GENERIC;         // _Generic
extern string_t* STR_GOTO;            // goto
extern string_t* STR_IF;              // if
extern string_t* STR_INLINE;          // inline
extern string_t* STR_INT;             // int
extern string_t* STR_LONG;            // long
extern string_t* STR_NORETURN;        // noreturn
extern string_t* STR_NORETURN_X;      // _Noreturn
extern string_t* STR_NORETURN_XX;     // __noreturn__
extern string_t* STR_NULLPTR;         // nullptr
extern string_t* STR_REGISTER;        // register
extern string_t* STR_RESTRICT;        // restrict
extern string_t* STR_RETURN;          // return
extern string_t* STR_SHORT;           // short
extern string_t* STR_SIGNED;          // signed
extern string_t* STR_SIZEOF;          // sizeof
extern string_t* STR_STATIC;          // static
extern string_t* STR_STATIC_ASSERT;   // static_assert
extern string_t* STR_STATIC_ASSERT_X; // _Static_assert
extern string_t* STR_STRUCT;          // struct
extern string_t* STR_SWITCH;          // switch
extern string_t* STR_THREAD_LOCAL;    // _Thread_local
extern string_t* STR_TYPEDEF;         // typedef
extern string_t* STR_TYPEOF;          // typeof
extern string_t* STR_TYPEOF_UNQUAL;   // typeof_unqual
extern string_t* STR_TYPEOF_X;        // __typeof__
extern string_t* STR_UNION;           // union
extern string_t* STR_UNSIGNED;        // unsigned
extern string_t* STR_VOID;            // void
extern string_t* STR_VOLATILE;        // volatile
extern string_t* STR_WHILE;           // while

// punctuation
extern string_t* STR_BRACE_CLOSE;             // }
extern string_t* STR_BRACE_OPEN;              // {
extern string_t* STR_PAREN_CLOSE;             // )
extern string_t* STR_PAREN_OPEN;              // (
extern string_t* STR_SQUARE_CLOSE;            // ]
extern string_t* STR_SQUARE_OPEN;             // [
extern string_t* STR_PLUS;                    // +
extern string_t* STR_MINUS;                   // -
extern string_t* STR_ASTERISK;                // *
extern string_t* STR_SLASH;                   // /
extern string_t* STR_PERCENT;                 // %
extern string_t* STR_AMPERSAND;               // &
extern string_t* STR_PIPE;                    // |
extern string_t* STR_CARET;                   // ^
extern string_t* STR_BANG;                    // !
extern string_t* STR_LESS;                    // <
extern string_t* STR_GREATER;                 // >
extern string_t* STR_ASSIGN;                  // =
extern string_t* STR_DOT;                     // .
extern string_t* STR_QUESTION;                // ?
extern string_t* STR_COLON;                   // :
extern string_t* STR_COMMA;                   // ,
extern string_t* STR_SEMICOLON;               // ;
extern string_t* STR_ARROW;                   // ->
extern string_t* STR_PLUS_EQUAL;              // +=
extern string_t* STR_MINUS_EQUAL;             // -=
extern string_t* STR_ASTERISK_EQUAL;          // *=
extern string_t* STR_SLASH_EQUAL;             // /=
extern string_t* STR_PERCENT_EQUAL;           // %=
extern string_t* STR_AMPERSAND_EQUAL;         // &=
extern string_t* STR_PIPE_EQUAL;              // |=
extern string_t* STR_LESS_EQUAL;              // <=
extern string_t* STR_GREATER_EQUAL;           // >=
extern string_t* STR_EQUAL_EQUAL;             // ==
extern string_t* STR_PLUS_PLUS;               // ++
extern string_t* STR_MINUS_MINUS;             // --
extern string_t* STR_AMPERSAND_AMPERSAND;     // &&
extern string_t* STR_PIPE_PIPE;               // ||
extern string_t* STR_LESS_LESS;               // <<
extern string_t* STR_GREATER_GREATER;         // >>
extern string_t* STR_LESS_LESS_EQUAL;         // <<=
extern string_t* STR_GREATER_GREATER_EQUAL;   // >>=
extern string_t* STR_ELLIPSIS;                // ...

void strings_init(void);
void strings_destroy(void);

#endif
