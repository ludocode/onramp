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

#ifndef INSTRUCTION_H_INCLUDED
#define INSTRUCTION_H_INCLUDED

#include <stdint.h>
#include <stdarg.h>

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
    CMPU,
    CMPS,
    JZ,
    JNZ,
    JL,
    JG,
    JLE,
    JGE,
    JMP,
    CALL,
    RET,
    ENTER,
    LEAVE,
    SYS,

} opcode_t;

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

// Some instructions support multiple arg types. In that case they take this
// enum value first which describes the remaining arguments.
typedef enum instruction_argtypes_t {
    ARGTYPE_NUMBER,      // An immediate number
    ARGTYPE_NAME,        // A named symbol
    ARGTYPE_GENERATED,   // A generated name (prefix + number)
} instruction_argtypes_t;

/**
 * An assembly instruction.
 *
 * An instruction consists of an opcode and up to three arguments of various
 * types. The fields in use depend on the opcode.
 */
typedef struct instruction_t {
    opcode_t opcode;

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
} instruction_t;

/**
 * Configures an instruction.
 */
void instruction_set(instruction_t* instruction, opcode_t opcode, ...);

void instruction_vset(instruction_t* instruction, opcode_t opcode, va_list args);

void instruction_emit(instruction_t* instruction);

#endif
