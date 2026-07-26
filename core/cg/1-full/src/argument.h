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

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

struct string_t;
struct temporary_t;
struct variable_t;

typedef enum argument_mode_t {
    argument_mode_read,       // argument is input only
    argument_mode_write,      // argument is output only (register or temporary)
    argument_mode_read_write, // argument is both input and output (register or temporary)
} argument_mode_t;

typedef enum argument_type_t {
    argument_type_sentinel = 0,
    argument_type_temporary,
    argument_type_register, // number is register (0-15)
    argument_type_number,
    argument_type_absolute, // absolute symbol invocation (uses string field)
    argument_type_relative, // relative numbered label invocation (uses number field)
    argument_type_variable,
} argument_type_t;

/**
 * An argument to an instruction.
 */
typedef struct argument_t {
    argument_type_t type;
    union {
        struct string_t* string; // strong reference
        struct temporary_t* temporary;
        struct variable_t* variable;
        // TODO rename number to integer, separate number into reg and integer fields
        uint32_t number;
    };
} argument_t;

argument_t* argument_new_copy(argument_t* argument);

/**
 * Creates a sentinel argument.
 */
argument_t* argument_new_sentinel(void);

/**
 * Creates a register or number argument.
 * TODO deprecated remove this
 */
argument_t* argument_new_number(argument_type_t type, uint32_t number);

argument_t* argument_new_integer(uint32_t integer);

argument_t* argument_new_register(uint32_t reg);

/**
 * Creates an invocation argument.
 *
 * Takes ownership of the given string.
 */
argument_t* argument_new_string(argument_type_t type, struct string_t* string);

/**
 * Creates a temporary argument.
 */
argument_t* argument_new_temporary(struct temporary_t* temporary);

/**
 * Create a variable argument.
 */
argument_t* argument_new_variable(struct variable_t* variable);

void argument_set_register(argument_t* argument, uint32_t reg);

void argument_set_variable(argument_t* argument, struct variable_t* variable);

void argument_set_integer(argument_t* argument, uint32_t value);

void argument_set_temporary(argument_t* argument, struct temporary_t* temporary);

void argument_set_sentinel(argument_t* argument);

static inline struct temporary_t* argument_temporary(argument_t* argument) {
    assert(argument->type == argument_type_temporary);
    return argument->temporary;
}

static inline uint32_t argument_number(argument_t* argument) {
    assert(argument->type == argument_type_number);
    return argument->number;
}

static inline struct string_t* argument_label(argument_t* argument) {
    assert(argument->type == argument_type_relative);
    return argument->string;
}

static inline struct variable_t* argument_variable(argument_t* argument) {
    assert(argument->type == argument_type_variable);
    return argument->variable;
}

static inline int argument_register(argument_t* argument) {
    assert(argument->type == argument_type_register);
    return argument->number;
}

/**
 * Returns true iff this is a temporary or number type.
 */
static inline bool argument_type_is_mix(argument_type_t type) {
    return type == argument_type_temporary || type == argument_type_number;
}

static inline bool argument_is_register(argument_t* argument) {
    return argument->type == argument_type_register;
}

static inline bool argument_is_temporary(argument_t* argument) {
    return argument->type == argument_type_temporary;
}

static inline bool argument_is_absolute(argument_t* argument) {
    return argument->type == argument_type_absolute;
}

static inline bool argument_is_integer(argument_t* argument) {
    return argument->type == argument_type_number;
}

void argument_delete(argument_t* argument);

#endif
