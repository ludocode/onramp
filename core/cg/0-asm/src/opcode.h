/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Fraser Heavy Software
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

#ifndef OPCODE_H_INCLUDED
#define OPCODE_H_INCLUDED

#include <stdlib.h>
#include <string.h>

#include "libo-util.h"
#include "libo-error.h"

#include "common.h"

// "virtual" opcodes, i.e. not real opcodes
#define OP_VIRTUAL_MIN   0
#define OP_NOP           0   // emits nothing; used for deleting instructions during optimization
#define OP_NUMBER        1   // a 32-bit decimal or hexadecimal number (in arg0)
#define OP_BYTE          2   // a quoted hex byte (in arg0)
#define OP_STRING        3   // a string literal (in label)
#define OP_DECLARATION   4   // a label declaration
#define OP_INVOCATION    5   // a standalone label invocation (e.g. for a hashtable)
#define OP_VIRTUAL_MAX  19

// arithmetic
#define OP_ARITHMETIC_MIN 20
#define OP_ADD     20
#define OP_SUB     21
#define OP_MUL     22
#define OP_DIVU    23
#define OP_DIVS    24
#define OP_MODU    25
#define OP_MODS    26
#define OP_ZERO    28
#define OP_INC     29
#define OP_DEC     30
#define OP_SXS     31
#define OP_SXB     32
#define OP_TRS     33
#define OP_TRB     34
#define OP_ARITHMETIC_MAX 39

// logic
#define OP_LOGIC_MIN 40
#define OP_AND     40
#define OP_OR      41
#define OP_XOR     42
#define OP_NOT     43
#define OP_SHL     44
#define OP_SHRU    45
#define OP_SHRS    46
#define OP_ROL     47
#define OP_ROR     48
#define OP_MOV     49
#define OP_BOOL    50
#define OP_ISZ     51
#define OP_LOGIC_MAX 59

// memory
#define OP_MEMORY_MIN 60
#define OP_LDW     60
#define OP_LDS     61
#define OP_LDB     62
#define OP_STW     63
#define OP_STS     64
#define OP_STB     65
#define OP_PUSH    66
#define OP_POP     67
#define OP_POPD    68
#define OP_MEMORY_MAX 79

// control
#define OP_CONTROL_MIN 80
#define OP_IMW     80
#define OP_LTU     81
#define OP_LTS     82
#define OP_JZ      83
#define OP_JNZ     84
#define OP_JMP     85
#define OP_CALL    86
#define OP_RET     87
#define OP_ENTER   88
#define OP_LEAVE   89
            // TODO remove
            #define OP_CMPS   90
            #define OP_CMPU   91
#define OP_CONTROL_MAX 99

#define OP_MAX    110

typedef int opcode_t;

// hashtable of opcode names to values
static const char** opcode_names;
static opcode_t* opcode_values;
#define opcodes_size 256 // power of two

// array map of opcode values to names and argcounts
static const char** opcode_value_to_name_map;
static size_t* opcode_value_to_argcount_map;

static void opcode_insert(const char* name, opcode_t value, size_t argcount) {
    // simple linear probing
    size_t i = (fnv1a_cstr(name) & (opcodes_size - 1));
    while (*(opcode_names + i) != NULL) {
        i = ((i + 1) & (opcodes_size - 1));
    }
    *(opcode_names + i) = name;
    *(opcode_values + i) = value;
    *(opcode_value_to_name_map + value) = name;
    *(opcode_value_to_argcount_map + value) = argcount;
}

static opcode_t opcode_name_to_value(const char* name) {
    size_t i = (fnv1a_cstr(name) & (opcodes_size - 1));
    while (*(opcode_names + i) != NULL) {
        if (0 == strcmp(*(opcode_names + i), name)) {
            return *(opcode_values + i);
        }
        i = ((i + 1) & (opcodes_size - 1));
    }
    fatal("Unrecognized opcode.");
}

static const char* opcode_value_to_name(opcode_t value) {
    return *(opcode_value_to_name_map + value);
}

static size_t opcode_argcount(opcode_t opcode) {
    return *(opcode_value_to_argcount_map + opcode);
}

static void opcode_setup(void) {
    opcode_names = calloc(opcodes_size, sizeof(char*));
    opcode_values = calloc(opcodes_size, sizeof(opcode_t));
    opcode_value_to_name_map = calloc(OP_MAX, sizeof(const char*));
    opcode_value_to_argcount_map = calloc(OP_MAX, sizeof(size_t));

    // arithmetic
    opcode_insert("add", OP_ADD, 3);
    opcode_insert("sub", OP_SUB, 3);
    opcode_insert("mul", OP_MUL, 3);
    opcode_insert("divu", OP_DIVU, 3);
    opcode_insert("divs", OP_DIVS, 3);
    opcode_insert("modu", OP_MODU, 3);
    opcode_insert("mods", OP_MODS, 3);
    opcode_insert("zero", OP_ZERO, 1);
    opcode_insert("inc", OP_INC, 1);
    opcode_insert("dec", OP_DEC, 1);
    opcode_insert("sxs", OP_SXS, 2);
    opcode_insert("sxb", OP_SXB, 2);
    opcode_insert("trs", OP_TRS, 2);
    opcode_insert("trb", OP_TRB, 2);

    // logic
    opcode_insert("and", OP_AND, 3);
    opcode_insert("or", OP_OR, 3);
    opcode_insert("xor", OP_XOR, 3);
    opcode_insert("not", OP_NOT, 2);
    opcode_insert("shl", OP_SHL, 3);
    opcode_insert("shru", OP_SHRU, 3);
    opcode_insert("shrs", OP_SHRS, 3);
    opcode_insert("rol", OP_ROL, 3);
    opcode_insert("ror", OP_ROR, 3);
    opcode_insert("mov", OP_MOV, 2);
    opcode_insert("bool", OP_BOOL, 2);
    opcode_insert("isz", OP_ISZ, 2);

    // memory
    opcode_insert("ldw", OP_LDW, 3);
    opcode_insert("lds", OP_LDS, 3);
    opcode_insert("ldb", OP_LDB, 3);
    opcode_insert("stw", OP_STW, 3);
    opcode_insert("sts", OP_STS, 3);
    opcode_insert("stb", OP_STB, 3);
    opcode_insert("push", OP_PUSH, 1);
    opcode_insert("pop", OP_POP, 1);
    opcode_insert("popd", OP_POPD, 0);

    // control
    opcode_insert("imw", OP_IMW, 2);
    opcode_insert("ltu", OP_LTU, 3);
    opcode_insert("lts", OP_LTS, 3);
    opcode_insert("jz", OP_JZ, 2);
    opcode_insert("jnz", OP_JNZ, 2);
    opcode_insert("jmp", OP_JMP, 1);
    opcode_insert("call", OP_CALL, 1);
    opcode_insert("ret", OP_RET, 0);
    opcode_insert("enter", OP_ENTER, 0);
    opcode_insert("leave", OP_LEAVE, 0);
                // TODO remove
                opcode_insert("cmps", OP_CMPS, 3);
                opcode_insert("cmpu", OP_CMPU, 3);
}

static void opcode_teardown(void) {
    free(opcode_values);
    free(opcode_names);
}

#endif
