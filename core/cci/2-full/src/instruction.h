/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2025 Fraser Heavy Software
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

#ifndef CCI2_IR
// The IR generation won't use variadic functions. This is just here
// temporarily to support the old generator.
#include <stdarg.h>
#endif

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
    //IMS, // TODO probably can remove this, we never generate it
    IMW,
    LTU,
    LTS,
    JZ,
    JNZ,
    JMP,
    CALL,
    RET,
    ENTER,
    LEAVE,
    SYS,

} opcode_t;

#ifndef CCI2_IR
#define R0 0x80
#define R1 0x81
#define R2 0x82
#define R3 0x83
#define R4 0x84
#define R5 0x85
#define R6 0x86
#define R7 0x87
#define R8 0x88
#define R9 0x89
#define RA 0x8A
#define RB 0x8B
#define RSP 0x8C
#define RFP 0x8D
#define RPP 0x8E
#define RIP 0x8F
#endif

#ifdef CCI2_IR
typedef enum argument_type_t {
    argument_type_sentinel,
    argument_type_temporary,
    argument_type_number,
    argument_type_absolute, // absolute label invocation (uses label field)
    argument_type_relative, // relative numbered label invocation (uses number field)
} argument_type_t;

typedef struct argument_t {
    argument_type_t type;
    union {
        // String references in arguments are unowned in order to minimize
        // per-instruction memory management. The names of strings and
        // temporaries are held in a pool in the function.
        string_t* string;
        uint32_t number;
    };
} argument_t;
#endif

#ifndef CCI2_IR
// Some instructions support multiple arg types. In that case they take this
// enum value first which describes the remaining arguments.
typedef enum instruction_argtypes_t {
    ARGTYPE_REGISTER,    // A register
    ARGTYPE_NUMBER,      // An immediate number
    ARGTYPE_NAME,        // A named symbol
    ARGTYPE_GENERATED,   // A generated name (prefix + number)
} instruction_argtypes_t;
#endif

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

    #ifdef CCI2_IR
    argument_t* arguments;
    size_t argument_count;
    #endif

    #ifndef CCI2_IR
    instruction_argtypes_t argtypes;
    int8_t arg1;
    int8_t arg2;
    int8_t arg3;
    char invocation_type;

    union {
        const char* invocation_label;
        struct {
            const char* invocation_prefix;
            int invocation_number;
        };
        int number;
    };
    #endif
} instruction_t;

#ifdef CCI2_IR
void instruction_init(instruction_t* instruction, token_t* token, opcode_t opcode, size_t arg_count);
#endif
#ifndef CCI2_IR
void instruction_init(instruction_t* instruction);
#endif

void instruction_destroy(instruction_t* instruction);

#ifdef CCI2_IR
static inline argument_t* instruction_argument(instruction_t* instruction, size_t arg) {
    assert(arg < instruction->argument_count);
    return instruction->arguments + arg;
}

void instruction_set_arg_number(instruction_t* instruction, size_t arg, uint32_t number);

void instruction_set_arg_sentinel(instruction_t* instruction, size_t arg);

void instruction_set_arg_temporary(instruction_t* instruction, size_t arg, string_t* temporary);

void instruction_set_arg_absolute(instruction_t* instruction, size_t arg, string_t* label);

void instruction_set_arg_relative(instruction_t* instruction, size_t arg, uint32_t label);
#endif

#ifndef CCI2_IR
/**
 * Configures an instruction.
 */
void instruction_set(instruction_t* instruction, struct token_t* /*nullable*/ token,
        opcode_t opcode, ...);

void instruction_vset(instruction_t* instruction, struct token_t* /*nullable*/ token,
        opcode_t opcode, va_list args);
#endif

void instruction_emit(instruction_t* instruction);

#endif
