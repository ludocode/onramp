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



/*
 * Opcodes
 */

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
#define OP_LDW     60    // don't reorder these, see opcode_adds() below()
#define OP_LDS     61    // ^^^
#define OP_LDB     62    // ^^^
#define OP_STW     63    // ^^^
#define OP_STS     64    // ^^^
#define OP_STB     65    // ^^^
#define OP_PUSH    66
#define OP_POP     67
#define OP_POPD    68 // TODO remove
#define OP_MEMORY_MAX 79

// control
#define OP_CONTROL_MIN 80
#define OP_IMW     80
#define OP_LTU     81
#define OP_LTS     82
#define OP_JZ      83    // don't re-order these, see optimize_dead_store()
#define OP_JNZ     84    // ^^^
#define OP_JMP     85    // ^^^
#define OP_CALL    86
#define OP_RET     87
#define OP_ENTER   88
#define OP_LEAVE   89
            // TODO remove
            #define OP_CMPS   90
            #define OP_CMPU   91
#define OP_CONTROL_MAX 99

#define OP_MAX    110



/*
 * Opcode styles.
 *
 * Each opcode has a "style", which defines what it does with its arguments
 * (i.e. which ones are input or output parameters, which ones are reg/mix vs.
 * constants, etc.) See opcode_style() below.
 *
 * (The only really important thing here is whether the first register is input,
 * output or both. It might be simpler to change it to that and get rid of the
 * extra cases; see the TODOs below.)
 */

// No arguments.
#define STYLE_NONE 0

// reg, mix*. First argument is a register output; all additional arguments are
// mix-type inputs.
// (e.g. mul, sxb, shru, ldw, mov, ltu... Almost all opcodes are this style.)
#define STYLE_REG_MIX 1

// mix*. All arguments are mix-type inputs except for a possible label.
// (e.g. stw, stb, jz, jmp, push)
#define STYLE_MIX 2

// reg, imm. First argument is a register output; second argument is a constant
// immediate value or a label.
// (e.g. imw)
// TODO this could probably be collapsed into STYLE_REG_MIX. The constant
// should be stored in a separate field like the label, not in arg1, that way
// arg0/arg1/arg2 would always contain mix-type bytes, where only arg0 is
// sometimes restricted to being a register.
#define STYLE_REG_CON 3

// reg. Takes a single argument which is an output register.
// (e.g. pop, zero)
// TODO this could probably be collapsed into STYLE_REG_MIX
#define STYLE_REG 4

// reg. Takes a single argument which is both an input and output register.
// (e.g. inc, dec)
#define STYLE_IN_PLACE 5



typedef size_t opcode_t;
typedef size_t style_t;

// hashtable of opcode names to values
static const char** opcode_names;
static opcode_t* opcode_values;
#define opcodes_size 256 // power of two

// array map of opcode values to names, argcounts, styles, sizes
// TODO wrap this in a struct once we put struct support in cci/0
static const char** opcode_value_to_name_map;
static size_t* opcode_value_to_argcount_map;
static style_t* opcode_value_to_style_map;
static size_t* opcode_value_to_size_map;

static int opcode_insert(const char* name, opcode_t value) {
    // simple linear probing
    size_t i = (fnv1a_cstr(name) & (opcodes_size - 1));
    while (*(opcode_names + i) != NULL) {
        i = ((i + 1) & (opcodes_size - 1));
    }
    *(opcode_names + i) = name;
    *(opcode_values + i) = value;
    *(opcode_value_to_name_map + value) = name;
    return value;
}

// this has to be separated from opcode_insert because cci/0 only supports four arguments
static void opcode_set_metadata(int value, size_t argcount, style_t style, size_t size) {
    *(opcode_value_to_argcount_map + value) = argcount;
    *(opcode_value_to_style_map + value) = style;
    *(opcode_value_to_size_map + value) = size;
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

    // allocate the hashtable
    opcode_names = calloc(opcodes_size, sizeof(char*));
    opcode_values = calloc(opcodes_size, sizeof(opcode_t));

    // allocate the opcode data tables
    // we zero these because we don't insert the virtual opcodes. all virtual
    // opcodes have style NONE by default.
    opcode_value_to_name_map = calloc(OP_MAX, sizeof(const char*));
    opcode_value_to_argcount_map = calloc(OP_MAX, sizeof(size_t));
    opcode_value_to_style_map = calloc(OP_MAX, sizeof(size_t));
    opcode_value_to_size_map = calloc(OP_MAX, sizeof(size_t));

    // arithmetic
    opcode_set_metadata(opcode_insert("add", OP_ADD), 3, STYLE_REG_MIX, 1);
    opcode_set_metadata(opcode_insert("sub", OP_SUB), 3, STYLE_REG_MIX, 1);
    opcode_set_metadata(opcode_insert("mul", OP_MUL), 3, STYLE_REG_MIX, 1);
    opcode_set_metadata(opcode_insert("divu", OP_DIVU), 3, STYLE_REG_MIX, 1);
    opcode_set_metadata(opcode_insert("divs", OP_DIVS), 3, STYLE_REG_MIX, 15);
    opcode_set_metadata(opcode_insert("modu", OP_MODU), 3, STYLE_REG_MIX, 15);
    opcode_set_metadata(opcode_insert("mods", OP_MODS), 3, STYLE_REG_MIX, 15);
    opcode_set_metadata(opcode_insert("zero", OP_ZERO), 1, STYLE_REG, 1); // TODO maybe remove, replace with imw
    opcode_set_metadata(opcode_insert("inc", OP_INC), 1, STYLE_IN_PLACE, 1);
    opcode_set_metadata(opcode_insert("dec", OP_DEC), 1, STYLE_IN_PLACE, 1);
    opcode_set_metadata(opcode_insert("sxs", OP_SXS), 2, STYLE_REG_MIX, 6);
    opcode_set_metadata(opcode_insert("sxb", OP_SXB), 2, STYLE_REG_MIX, 6);
    opcode_set_metadata(opcode_insert("trs", OP_TRS), 2, STYLE_REG_MIX, 3);
    opcode_set_metadata(opcode_insert("trb", OP_TRB), 2, STYLE_REG_MIX, 3);

    // logic
    opcode_set_metadata(opcode_insert("and", OP_AND), 3, STYLE_REG_MIX, 1);
    opcode_set_metadata(opcode_insert("or", OP_OR), 3, STYLE_REG_MIX, 1);
    opcode_set_metadata(opcode_insert("xor", OP_XOR), 3, STYLE_REG_MIX, 3);
    opcode_set_metadata(opcode_insert("not", OP_NOT), 2, STYLE_REG_MIX, 1);
    opcode_set_metadata(opcode_insert("shl", OP_SHL), 3, STYLE_REG_MIX, 1);
    opcode_set_metadata(opcode_insert("shru", OP_SHRU), 3, STYLE_REG_MIX, 1);
    opcode_set_metadata(opcode_insert("shrs", OP_SHRS), 3, STYLE_REG_MIX, 10);
    opcode_set_metadata(opcode_insert("rol", OP_ROL), 3, STYLE_REG_MIX, 2);
    opcode_set_metadata(opcode_insert("ror", OP_ROR), 3, STYLE_REG_MIX, 2);
    opcode_set_metadata(opcode_insert("mov", OP_MOV), 2, STYLE_REG_MIX, 1);
    opcode_set_metadata(opcode_insert("bool", OP_BOOL), 2, STYLE_REG_MIX, 1);
    opcode_set_metadata(opcode_insert("isz", OP_ISZ), 2, STYLE_REG_MIX, 1);

    // memory
    opcode_set_metadata(opcode_insert("ldw", OP_LDW), 3, STYLE_REG_MIX, 1);
    opcode_set_metadata(opcode_insert("lds", OP_LDS), 3, STYLE_REG_MIX, 5);
    opcode_set_metadata(opcode_insert("ldb", OP_LDB), 3, STYLE_REG_MIX, 1);
    opcode_set_metadata(opcode_insert("stw", OP_STW), 3, STYLE_MIX, 1);
    opcode_set_metadata(opcode_insert("sts", OP_STS), 3, STYLE_MIX, 4);
    opcode_set_metadata(opcode_insert("stb", OP_STB), 3, STYLE_MIX, 1);
    opcode_set_metadata(opcode_insert("push", OP_PUSH), 1, STYLE_MIX, 2);
    opcode_set_metadata(opcode_insert("pop", OP_POP), 1, STYLE_REG, 2);
    opcode_set_metadata(opcode_insert("popd", OP_POPD), 0, STYLE_NONE, 1);

    // control
    opcode_set_metadata(opcode_insert("imw", OP_IMW), 2, STYLE_REG_CON, 2);
    opcode_set_metadata(opcode_insert("ltu", OP_LTU), 3, STYLE_REG_MIX, 1);
    opcode_set_metadata(opcode_insert("lts", OP_LTS), 3, STYLE_REG_MIX, 4);
    opcode_set_metadata(opcode_insert("jz", OP_JZ), 2, STYLE_MIX, 1);
    opcode_set_metadata(opcode_insert("jnz", OP_JNZ), 2, STYLE_MIX, 2);
    opcode_set_metadata(opcode_insert("jmp", OP_JMP), 1, STYLE_MIX, 1);
    opcode_set_metadata(opcode_insert("call", OP_CALL), 1, STYLE_MIX, 7);
    opcode_set_metadata(opcode_insert("ret", OP_RET), 0, STYLE_NONE, 1);
    opcode_set_metadata(opcode_insert("enter", OP_ENTER), 0, STYLE_NONE, 3);
    opcode_set_metadata(opcode_insert("leave", OP_LEAVE), 0, STYLE_NONE, 3);
                // TODO remove
                opcode_set_metadata(opcode_insert("cmps", OP_CMPS), 3, STYLE_REG_MIX, 15);
                opcode_set_metadata(opcode_insert("cmpu", OP_CMPU), 3, STYLE_REG_MIX, 9);
}

static void opcode_teardown(void) {
    free(opcode_values);
    free(opcode_names);
}

/**
 * Returns the "style" of the opcode, which defines what it does with its
 * arguments (i.e. which ones are input or output parameters, which ones are
 * reg/mix vs. constants, etc.)
 *
 * See the STYLE constants above.
 */
static style_t opcode_style(opcode_t opcode) {
    return *(opcode_value_to_style_map + opcode);
}

/**
 * Returns the approximate number of primitive instructions the assembler will
 * emit for this opcode.
 */
static size_t opcode_size(opcode_t opcode) {
    return *(opcode_value_to_size_map + opcode);
}

/**
 * Returns true if the given opcode adds two input registers.
 */
static bool opcode_adds(opcode_t opcode) {
    if (opcode == OP_ADD) {
        return true;
    }
    if (opcode >= OP_LDW) {
        if (opcode <= OP_STB) {
            return true;
        }
    }
    return false;
}

#endif
