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

#include "argument.h"

#include <assert.h>
#include <stdlib.h>

#include "libo-string.h"
#include "libo-error.h"
#include "temporary.h"

void argument_clear(argument_t* argument) {
    switch (argument->type) {
        case argument_type_absolute:
        case argument_type_relative:
            string_deref(argument->string);
            break;
        default:
            break;
    }
}

void argument_delete(argument_t* argument) {
    argument_clear(argument);
    free(argument);
}

static argument_t* argument_new_type(argument_type_t type) {
    argument_t* argument = malloc(sizeof(argument_t));
    if (!argument) {
        fatal("Out of memory.");
    }
    argument->type = type;
    return argument;
}

argument_t* argument_new_sentinel(void) {
    return argument_new_type(argument_type_sentinel);
}

argument_t* argument_new_number(argument_type_t type, uint32_t number) {
    assert(type == argument_type_number || type == argument_type_register);
    if (type == argument_type_register) {
        assert(number <= 15);
    }

    argument_t* argument = argument_new_type(type);
    argument->number = number;
    return argument;
}

argument_t* argument_new_integer(uint32_t integer) {
    argument_t* argument = argument_new_type(argument_type_number);
    argument->number = integer;
    return argument;
}

argument_t* argument_new_register(uint32_t reg) {
    argument_t* argument = argument_new_type(argument_type_register);
    argument->number = reg;
    return argument;
}

argument_t* argument_new_string(argument_type_t type, string_t* string) {
    assert(type == argument_type_relative || type == argument_type_absolute);
    argument_t* argument = argument_new_type(type);
    argument->string = string;
    return argument;
}

argument_t* argument_new_temporary(temporary_t* temporary) {
    argument_t* argument = argument_new_type(argument_type_temporary);
    argument->temporary = temporary;
    return argument;
}

argument_t* argument_new_variable(struct variable_t* variable) {
    argument_t* argument = argument_new_type(argument_type_temporary);
    argument->variable = variable;
    return argument;
}

void argument_set_register(argument_t* argument, uint32_t reg) {
    argument_clear(argument);
    argument->type = argument_type_register;
    argument->number = reg;
}

void argument_set_variable(argument_t* argument, struct variable_t* variable) {
    argument_clear(argument);
    argument->type = argument_type_variable;
    argument->variable = variable;
}

void argument_set_integer(argument_t* argument, uint32_t value) {
    argument_clear(argument);
    argument->type = argument_type_number;
    argument->number = value;
}
