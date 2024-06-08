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
string_t* STR_ALIGNAS;         // alignas
string_t* STR_ALIGNAS_X;       // _Alignas
string_t* STR_ALIGNOF;         // alignof
string_t* STR_ALIGNOF_X;       // _Alignof
string_t* STR_ASM;             // asm
string_t* STR_ASM_X;           // __asm__
string_t* STR_ATOMIC;          // _Atomic
string_t* STR_AUTO;            // auto
string_t* STR_BITINT;          // _Bitint
string_t* STR_BOOL;            // _Bool
string_t* STR_BREAK;           // break
string_t* STR_CASE;            // case
string_t* STR_CHAR;            // char
string_t* STR_CONST;           // const
string_t* STR_CONSTEXPR;       // constexpr
string_t* STR_CONTINUE;        // continue
string_t* STR_DEFAULT;         // default
string_t* STR_DO;              // do
string_t* STR_DOUBLE;          // double
string_t* STR_ELSE;            // else
string_t* STR_ENUM;            // enum
string_t* STR_EXTERN;          // extern
string_t* STR_FLOAT;           // float
string_t* STR_FOR;             // for
string_t* STR_GENERIC;         // _Generic
string_t* STR_GOTO;            // goto
string_t* STR_IF;              // if
string_t* STR_INLINE;          // inline
string_t* STR_INT;             // int
string_t* STR_LONG;            // long
string_t* STR_NORETURN;        // noreturn
string_t* STR_NORETURN_X;      // _Noreturn
string_t* STR_NORETURN_XX;     // __noreturn__
string_t* STR_NULLPTR;         // nullptr
string_t* STR_REGISTER;        // register
string_t* STR_RESTRICT;        // restrict
string_t* STR_RETURN;          // return
string_t* STR_SHORT;           // short
string_t* STR_SIGNED;          // signed
string_t* STR_SIZEOF;          // sizeof
string_t* STR_STATIC;          // static
string_t* STR_STATIC_ASSERT;   // static_assert
string_t* STR_STATIC_ASSERT_X; // _Static_assert
string_t* STR_STRUCT;          // struct
string_t* STR_SWITCH;          // switch
string_t* STR_THREAD_LOCAL;    // _Thread_local
string_t* STR_TYPEDEF;         // typedef
string_t* STR_TYPEOF;          // typeof
string_t* STR_TYPEOF_UNQUAL;   // typeof_unqual
string_t* STR_TYPEOF_X;        // __typeof__
string_t* STR_UNION;           // union
string_t* STR_UNSIGNED;        // unsigned
string_t* STR_VOID;            // void
string_t* STR_VOLATILE;        // volatile
string_t* STR_WHILE;           // while

// punctuation
string_t* STR_BRACE_CLOSE;             // }
string_t* STR_BRACE_OPEN;              // {
string_t* STR_PAREN_CLOSE;             // )
string_t* STR_PAREN_OPEN;              // (
string_t* STR_SQUARE_CLOSE;            // ]
string_t* STR_SQUARE_OPEN;             // [
string_t* STR_PLUS;                    // +
string_t* STR_MINUS;                   // -
string_t* STR_ASTERISK;                // *
string_t* STR_SLASH;                   // /
string_t* STR_PERCENT;                 // %
string_t* STR_AMPERSAND;               // &
string_t* STR_PIPE;                    // |
string_t* STR_CARET;                   // ^
string_t* STR_BANG;                    // !
string_t* STR_LESS;                    // <
string_t* STR_GREATER;                 // >
string_t* STR_ASSIGN;                  // =
string_t* STR_DOT;                     // .
string_t* STR_QUESTION;                // ?
string_t* STR_COLON;                   // :
string_t* STR_COMMA;                   // ,
string_t* STR_SEMICOLON;               // ;
string_t* STR_PLUS_EQUAL;              // +=
string_t* STR_MINUS_EQUAL;             // -=
string_t* STR_ASTERISK_EQUAL;          // *=
string_t* STR_SLASH_EQUAL;             // /=
string_t* STR_PERCENT_EQUAL;           // %=
string_t* STR_AMPERSAND_EQUAL;         // &=
string_t* STR_PIPE_EQUAL;              // |=
string_t* STR_LESS_EQUAL;              // <=
string_t* STR_GREATER_EQUAL;           // >=
string_t* STR_EQUAL_EQUAL;             // ==
string_t* STR_PLUS_PLUS;               // ++
string_t* STR_MINUS_MINUS;             // --
string_t* STR_AMPERSAND_AMPERSAND;     // &&
string_t* STR_PIPE_PIPE;               // ||
string_t* STR_LESS_LESS;               // <<
string_t* STR_GREATER_GREATER;         // >>
string_t* STR_LESS_LESS_EQUAL;         // <<=
string_t* STR_GREATER_GREATER_EQUAL;   // >>=
string_t* STR_ELLIPSIS;                // ...

void strings_init(void) {

    // keywords
    STR_ALIGNAS                     = string_intern_cstr("alignas");
    STR_ALIGNAS_X                   = string_intern_cstr("_Alignas");
    STR_ALIGNOF                     = string_intern_cstr("alignof");
    STR_ALIGNOF_X                   = string_intern_cstr("_Alignof");
    STR_ASM                         = string_intern_cstr("asm");
    STR_ASM_X                       = string_intern_cstr("__asm__");
    STR_ATOMIC                      = string_intern_cstr("_Atomic");
    STR_AUTO                        = string_intern_cstr("auto");
    STR_BITINT                      = string_intern_cstr("_Bitint");
    STR_BOOL                        = string_intern_cstr("_Bool");
    STR_BREAK                       = string_intern_cstr("break");
    STR_CASE                        = string_intern_cstr("case");
    STR_CHAR                        = string_intern_cstr("char");
    STR_CONST                       = string_intern_cstr("const");
    STR_CONSTEXPR                   = string_intern_cstr("constexpr");
    STR_CONTINUE                    = string_intern_cstr("continue");
    STR_DEFAULT                     = string_intern_cstr("default");
    STR_DO                          = string_intern_cstr("do");
    STR_DOUBLE                      = string_intern_cstr("double");
    STR_ELSE                        = string_intern_cstr("else");
    STR_ENUM                        = string_intern_cstr("enum");
    STR_EXTERN                      = string_intern_cstr("extern");
    STR_FLOAT                       = string_intern_cstr("float");
    STR_FOR                         = string_intern_cstr("for");
    STR_GENERIC                     = string_intern_cstr("_Generic");
    STR_GOTO                        = string_intern_cstr("goto");
    STR_IF                          = string_intern_cstr("if");
    STR_INLINE                      = string_intern_cstr("inline");
    STR_INT                         = string_intern_cstr("int");
    STR_LONG                        = string_intern_cstr("long");
    STR_NORETURN                    = string_intern_cstr("noreturn");
    STR_NORETURN_X                  = string_intern_cstr("_Noreturn");
    STR_NORETURN_XX                 = string_intern_cstr("__noreturn__");
    STR_NULLPTR                     = string_intern_cstr("nullptr");
    STR_REGISTER                    = string_intern_cstr("register");
    STR_RESTRICT                    = string_intern_cstr("restrict");
    STR_RETURN                      = string_intern_cstr("return");
    STR_SHORT                       = string_intern_cstr("short");
    STR_SIGNED                      = string_intern_cstr("signed");
    STR_SIZEOF                      = string_intern_cstr("sizeof");
    STR_STATIC                      = string_intern_cstr("static");
    STR_STATIC_ASSERT               = string_intern_cstr("static_assert");
    STR_STATIC_ASSERT_X             = string_intern_cstr("_Static_assert");
    STR_STRUCT                      = string_intern_cstr("struct");
    STR_SWITCH                      = string_intern_cstr("switch");
    STR_THREAD_LOCAL                = string_intern_cstr("_Thread_local");
    STR_TYPEDEF                     = string_intern_cstr("typedef");
    STR_TYPEOF                      = string_intern_cstr("typeof");
    STR_TYPEOF_UNQUAL               = string_intern_cstr("typeof_unqual");
    STR_TYPEOF_X                    = string_intern_cstr("__typeof__");
    STR_UNION                       = string_intern_cstr("union");
    STR_UNSIGNED                    = string_intern_cstr("unsigned");
    STR_VOID                        = string_intern_cstr("void");
    STR_VOLATILE                    = string_intern_cstr("volatile");
    STR_WHILE                       = string_intern_cstr("while");

    // punctuation
    STR_BRACE_CLOSE                 = string_intern_cstr("}");
    STR_BRACE_OPEN                  = string_intern_cstr("{");
    STR_PAREN_CLOSE                 = string_intern_cstr(")");
    STR_PAREN_OPEN                  = string_intern_cstr("(");
    STR_SQUARE_CLOSE                = string_intern_cstr("]");
    STR_SQUARE_OPEN                 = string_intern_cstr("[");
    STR_PLUS                        = string_intern_cstr("+");
    STR_MINUS                       = string_intern_cstr("-");
    STR_ASTERISK                    = string_intern_cstr("*");
    STR_SLASH                       = string_intern_cstr("/");
    STR_PERCENT                     = string_intern_cstr("%");
    STR_AMPERSAND                   = string_intern_cstr("&");
    STR_PIPE                        = string_intern_cstr("|");
    STR_CARET                       = string_intern_cstr("^");
    STR_BANG                        = string_intern_cstr("!");
    STR_LESS                        = string_intern_cstr("<");
    STR_GREATER                     = string_intern_cstr(">");
    STR_ASSIGN                      = string_intern_cstr("=");
    STR_DOT                         = string_intern_cstr(".");
    STR_QUESTION                    = string_intern_cstr("?");
    STR_COLON                       = string_intern_cstr(":");
    STR_COMMA                       = string_intern_cstr(",");
    STR_SEMICOLON                   = string_intern_cstr(";");
    STR_PLUS_EQUAL                  = string_intern_cstr("+=");
    STR_MINUS_EQUAL                 = string_intern_cstr("-=");
    STR_ASTERISK_EQUAL              = string_intern_cstr("*=");
    STR_SLASH_EQUAL                 = string_intern_cstr("/=");
    STR_PERCENT_EQUAL               = string_intern_cstr("%=");
    STR_AMPERSAND_EQUAL             = string_intern_cstr("&=");
    STR_PIPE_EQUAL                  = string_intern_cstr("|=");
    STR_LESS_EQUAL                  = string_intern_cstr("<=");
    STR_GREATER_EQUAL               = string_intern_cstr(">=");
    STR_EQUAL_EQUAL                 = string_intern_cstr("==");
    STR_PLUS_PLUS                   = string_intern_cstr("++");
    STR_MINUS_MINUS                 = string_intern_cstr("--");
    STR_AMPERSAND_AMPERSAND         = string_intern_cstr("&&");
    STR_PIPE_PIPE                   = string_intern_cstr("||");
    STR_LESS_LESS                   = string_intern_cstr("<<");
    STR_GREATER_GREATER             = string_intern_cstr(">>");
    STR_LESS_LESS_EQUAL             = string_intern_cstr("<<=");
    STR_GREATER_GREATER_EQUAL       = string_intern_cstr(">>=");
    STR_ELLIPSIS                    = string_intern_cstr("...");
}

void strings_destroy(void) {

    // keywords
    string_deref(STR_ALIGNAS);
    string_deref(STR_ALIGNAS_X);
    string_deref(STR_ALIGNOF);
    string_deref(STR_ALIGNOF_X);
    string_deref(STR_ASM);
    string_deref(STR_ASM_X);
    string_deref(STR_ATOMIC);
    string_deref(STR_AUTO);
    string_deref(STR_BITINT);
    string_deref(STR_BOOL);
    string_deref(STR_BREAK);
    string_deref(STR_CASE);
    string_deref(STR_CHAR);
    string_deref(STR_CONST);
    string_deref(STR_CONSTEXPR);
    string_deref(STR_CONTINUE);
    string_deref(STR_DEFAULT);
    string_deref(STR_DO);
    string_deref(STR_DOUBLE);
    string_deref(STR_ELSE);
    string_deref(STR_ENUM);
    string_deref(STR_EXTERN);
    string_deref(STR_FLOAT);
    string_deref(STR_FOR);
    string_deref(STR_GENERIC);
    string_deref(STR_GOTO);
    string_deref(STR_IF);
    string_deref(STR_INLINE);
    string_deref(STR_INT);
    string_deref(STR_LONG);
    string_deref(STR_NORETURN);
    string_deref(STR_NORETURN_X);
    string_deref(STR_NORETURN_XX);
    string_deref(STR_NULLPTR);
    string_deref(STR_REGISTER);
    string_deref(STR_RESTRICT);
    string_deref(STR_RETURN);
    string_deref(STR_SHORT);
    string_deref(STR_SIGNED);
    string_deref(STR_SIZEOF);
    string_deref(STR_STATIC);
    string_deref(STR_STATIC_ASSERT);
    string_deref(STR_STATIC_ASSERT_X);
    string_deref(STR_STRUCT);
    string_deref(STR_SWITCH);
    string_deref(STR_THREAD_LOCAL);
    string_deref(STR_TYPEDEF);
    string_deref(STR_TYPEOF);
    string_deref(STR_TYPEOF_UNQUAL);
    string_deref(STR_TYPEOF_X);
    string_deref(STR_UNION);
    string_deref(STR_UNSIGNED);
    string_deref(STR_VOID);
    string_deref(STR_VOLATILE);
    string_deref(STR_WHILE);

    // punctuation
    string_deref(STR_BRACE_CLOSE);
    string_deref(STR_BRACE_OPEN);
    string_deref(STR_PAREN_CLOSE);
    string_deref(STR_PAREN_OPEN);
    string_deref(STR_SQUARE_CLOSE);
    string_deref(STR_SQUARE_OPEN);
    string_deref(STR_PLUS);
    string_deref(STR_MINUS);
    string_deref(STR_ASTERISK);
    string_deref(STR_SLASH);
    string_deref(STR_PERCENT);
    string_deref(STR_AMPERSAND);
    string_deref(STR_PIPE);
    string_deref(STR_CARET);
    string_deref(STR_BANG);
    string_deref(STR_LESS);
    string_deref(STR_GREATER);
    string_deref(STR_ASSIGN);
    string_deref(STR_DOT);
    string_deref(STR_QUESTION);
    string_deref(STR_COLON);
    string_deref(STR_COMMA);
    string_deref(STR_SEMICOLON);
    string_deref(STR_PLUS_EQUAL);
    string_deref(STR_MINUS_EQUAL);
    string_deref(STR_ASTERISK_EQUAL);
    string_deref(STR_SLASH_EQUAL);
    string_deref(STR_PERCENT_EQUAL);
    string_deref(STR_AMPERSAND_EQUAL);
    string_deref(STR_PIPE_EQUAL);
    string_deref(STR_LESS_EQUAL);
    string_deref(STR_GREATER_EQUAL);
    string_deref(STR_EQUAL_EQUAL);
    string_deref(STR_PLUS_PLUS);
    string_deref(STR_MINUS_MINUS);
    string_deref(STR_AMPERSAND_AMPERSAND);
    string_deref(STR_PIPE_PIPE);
    string_deref(STR_LESS_LESS);
    string_deref(STR_GREATER_GREATER);
    string_deref(STR_LESS_LESS_EQUAL);
    string_deref(STR_GREATER_GREATER_EQUAL);
    string_deref(STR_ELLIPSIS);
}
