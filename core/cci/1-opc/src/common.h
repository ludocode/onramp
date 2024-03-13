/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
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

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stdbool.h>

#include "libo-error.h"
#include "libo-util.h"

#define JUMP_LABEL_PREFIX     "_Lx"
#define STRING_LABEL_PREFIX   "_Sx"
#define FUNCTION_LABEL_PREFIX "_F_"
#define USER_LABEL_PREFIX     "_U_"



// We don't have structs so we can't forward-declare them. We have to put
// these typedefs here instead.
#ifdef __onramp_cci_omc__
    typedef void field_t;
    typedef void global_t;
    typedef void record_t;
    typedef void type_t;
#endif
#ifndef __onramp_cci_omc__
    // When compiling with a better compiler than cci/0, we use empty structs
    // to ensure that these pointers are not implicitly convertible with each
    // other or anything else.
    typedef struct {void* unused;} field_t;
    typedef struct {void* unused;} global_t;
    typedef struct {void* unused;} record_t;
    typedef struct {void* unused;} type_t;
#endif



// Functions for signaling a fatal error. We don't have variadic functions so
// we use this workaround instead. The messages are all concatenated.

_Noreturn
void fatal_2(const char* message_1, const char* message_2);

_Noreturn
void fatal_3(const char* message_1, const char* message_2,
        const char* message_3);

_Noreturn
void fatal_4(const char* message_1, const char* message_2,
        const char* message_3, const char* message_4);

// TODO we should put a real assert in libc/0
#ifdef __GNUC__
    #undef NDEBUG
    #include <assert.h>
#endif
#ifndef __GNUC__
    void assert(bool x);
#endif



void* memdup(const void* other, size_t size);
char* strdup_checked(const char* str);



#endif
