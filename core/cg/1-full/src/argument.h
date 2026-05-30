/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025-2026 Fraser Heavy Software
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

#ifndef ARGUMENT_H_INCLUDED
#define ARGUMENT_H_INCLUDED

#include "libo-string.h"

typedef enum argument_type_t {
    argument_type_sentinel = 0,
    argument_type_temporary,
    argument_type_register, // number is register (0-15)
    argument_type_number,
    argument_type_absolute, // absolute symbol invocation (uses string field)
    argument_type_relative, // relative numbered label invocation (uses number field)
} argument_type_t;

/**
 * An argument to an instruction.
 */
typedef struct argument_t {
    argument_type_t type;
    union {
        string_t* string; // strong reference
        uint32_t number;
    };
} argument_t;

/**
 * Creates a sentinel argument.
 */
argument_t* argument_new_sentinel(void);

/**
 * Creates a register or number argument.
 */
argument_t* argument_new_number(argument_type_t type, uint32_t number);

/**
 * Creates an invocation or temporary argument.
 *
 * Takes ownership of the given string.
 */
argument_t* argument_new_string(argument_type_t type, string_t* string);

void argument_delete(argument_t* argument);

#endif
