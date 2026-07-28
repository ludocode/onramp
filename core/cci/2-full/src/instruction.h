/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2026 Fraser Heavy Software
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

#ifndef INSTRUCTION_H_INCLUDED
#define INSTRUCTION_H_INCLUDED

#include <assert.h>
#include <stdint.h>

#include "common.h"
#include "token.h"

struct token_t;

typedef enum opcode_t {

    // "virtual" opcodes, i.e. not real opcodes
    NOP = 0,   // emits nothing; used for deleting instructions during optimization
    VALUE,     // a value without an opcode, for example in the jump table of a switch

    // arithmetic
    ADD,
    SUB,
    MUL,
    DIVU,
    DIVS,
    MODU,
    MODS,
    ZERO,
    INC,
    DEC,
    SXS,
    SXB,
    TRS,
    TRB,

    // logic
    AND,
    OR,
    XOR,
    NOT,
    SHL,
    SHRU,
    SHRS,
    ROL,
    ROR,
    MOV,
    BOOL,
    ISZ,

    // memory
    SYM,
    VAR,
    ALLOC,
    FREE,
    LDW,
    LDS,
    LDB,
    STW,
    STS,
    STB,
    PUSH,
    POP,
    POPD,

    // control
    LTU,
    LTS,
    BR,
    JMP,
    CALL,
    RET,
    ENTER,
    LEAVE,
    SYS,

} opcode_t;

const char* opcode_to_string(opcode_t opcode);

typedef enum argument_type_t {
    argument_type_sentinel = 1,
    argument_type_temporary,
    argument_type_number,
    argument_type_absolute, // absolute label invocation (uses string field)
    argument_type_relative, // relative numbered label invocation (uses number field)
    argument_type_varargs, // the varargs keyword for variadic calls
} argument_type_t;

typedef struct argument_t {
    argument_type_t type;
    union {
        // String references in arguments are unowned in order to minimize
        // per-instruction memory management. The names of strings are held in
        // a pool in the function.
        string_t* string;
        uint32_t number;
    };
} argument_t;

/**
 * A generated instruction in the intermediate representation.
 *
 * An instruction consists of an opcode and a list of arguments.
 *
 * The instruction contains a strong reference to the nearest token from which
 * the instruction was generated (typically the token of the tree node, for
 * example the `+` for an ADD instruction.) This is used to generate debug
 * info.
 */
typedef struct instruction_t {
    struct token_t* /*nullable*/ token;
    opcode_t opcode;

    argument_t* arguments;
    size_t argument_count;
} instruction_t;

void instruction_init(instruction_t* instruction, token_t* token, opcode_t opcode, size_t arg_count);

void instruction_destroy(instruction_t* instruction);

static inline argument_t* instruction_argument(instruction_t* instruction, size_t arg) {
    assert(arg < instruction->argument_count);
    return instruction->arguments + arg;
}

void instruction_set_arg_number(instruction_t* instruction, size_t arg, uint32_t number);

void instruction_set_arg_sentinel(instruction_t* instruction, size_t arg);

void instruction_set_arg_temporary(instruction_t* instruction, size_t arg, int temporary);

//! Does not keep a strong reference to the string
void instruction_set_arg_absolute(instruction_t* instruction, size_t arg, string_t* label);

void instruction_set_arg_absolute_cstr(instruction_t* instruction, size_t arg, const char* label);

void instruction_set_arg_relative(instruction_t* instruction, size_t arg, uint32_t label);

void instruction_set_arg_varargs(instruction_t* instruction, size_t arg);

/**
 * Set the arguments for the given instruction to the given two temporaries.
 */
void instruction_set_args_tt(instruction_t* instruction, int a, int b);

/**
 * Set the arguments for the given instruction to the given three temporaries.
 */
void instruction_set_args_ttt(instruction_t* instruction, int a, int b, int c);

/**
 * Set the arguments for the given instruction to the given two temporaries
 * followed by the given immediate value.
 */
void instruction_set_args_ttn(instruction_t* instruction,
        int temp_a, int temp_b, uint32_t number);

void instruction_emit(instruction_t* instruction);

#endif
