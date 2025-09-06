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

/*
 * This file contains a register array which stores the current state of all
 * registers.
 *
 * This is used during parsing and during optimization passes to store things
 * like which registers are in use, whether registers store a constant value,
 * whether registers have been written to and not read, etc.
 */

#ifndef REGISTER_H_INCLUDED
#define REGISTER_H_INCLUDED

#include "common.h"

#define REG_RSP 0x8C
#define REG_RFP 0x8D
#define REG_RPP 0x8E
#define REG_RIP 0x8F

#ifdef __GNUC__
// Make sure we're not using the register_t in <sys/types.h>
#pragma GCC poison register_t
#endif

typedef void reg_t;
typedef void instruction_t;

#define REGISTER_CONTENT_UNKNOWN 0
#define REGISTER_CONTENT_CONSTANT 1
#define REGISTER_CONTENT_REGISTER 2

#define REGISTER_CONTENT_TYPE 0 // One of the REGISTER_CONTENT_* constants
#define REGISTER_VALUE 1 // The constant or other register this register contains
#define REGISTER_USED 2 // Whether the register is used (for dead store elimination)
#define REGISTER_INSTRUCTION 3 // The last instruction that wrote to this register
#define REGISTER_SIZE 4

static reg_t* registers;

/**
 * Returns one of the REGISTER_CONTENT_* constants indicating what we know
 * about the contents of this register.
 */
static int register_content_type(reg_t* reg) {
    return *(int*)((size_t*)reg + REGISTER_CONTENT_TYPE);
}

/**
 * Returns the constant or other register this register contains.
 */
static int register_value(reg_t* reg) {
    return *(int*)((size_t*)reg + REGISTER_VALUE);
}

/**
 * Returns true if this register is possibly used.
 *
 * If a register is possibly used, a preceding store to it must be preserved by
 * dead store elimination. If a register is not used, stores to the register
 * can be eliminated.
 *
 * When walking backwards through a block, this returns true if a write to this
 * register is expected (because it is possibly read), and false if no write is
 * expected (because it was written to later in the block without a read in
 * between, or because its value is discarded by a call or ret.)
 */
static bool register_used(reg_t* reg) {
    return *(bool*)((size_t*)reg + REGISTER_USED);
}

static void register_set_used(reg_t* reg, bool used) {
    *(bool*)((size_t*)reg + REGISTER_USED) = used;
}

/**
 * Returns a pointer to the last instruction that modified this register, or
 * NULL if nothing has modified it yet.
 */
static instruction_t* register_instruction(reg_t* reg) {
    return *(instruction_t**)((size_t*)reg + REGISTER_INSTRUCTION);
}

static void register_clear(reg_t* reg) {
    *(int*)((size_t*)reg + REGISTER_CONTENT_TYPE) = REGISTER_CONTENT_UNKNOWN;
}

/**
 * Records the fact that this register was assigned a constant value by the
 * given instruction.
 */
static void register_set_constant(reg_t* reg, int constant_value, instruction_t* instruction) {
    *(int*)((size_t*)reg + REGISTER_CONTENT_TYPE) = REGISTER_CONTENT_CONSTANT;
    *(int*)((size_t*)reg + REGISTER_VALUE) = constant_value;
    *(instruction_t**)((size_t*)reg + REGISTER_INSTRUCTION) = instruction;
}

/**
 * Records the fact that this register was assigned the value of another
 * register by the given instruction.
 */
static void register_set_register(reg_t* reg, int src, instruction_t* instruction) {
    assert(is_register(src));
    *(int*)((size_t*)reg + REGISTER_CONTENT_TYPE) = REGISTER_CONTENT_REGISTER;
    *(int*)((size_t*)reg + REGISTER_VALUE) = src;
    *(instruction_t**)((size_t*)reg + REGISTER_INSTRUCTION) = instruction;
}

static void register_set_unknown(reg_t* reg, instruction_t* instruction) {
    *(int*)((size_t*)reg + REGISTER_CONTENT_TYPE) = REGISTER_CONTENT_UNKNOWN;
    *(instruction_t**)((size_t*)reg + REGISTER_INSTRUCTION) = instruction;
}

static void registers_clear(void) {
    memset(registers, 0, 16 * (REGISTER_SIZE * sizeof(size_t)));
}

/**
 * Returns the register with the given bytecode value (0x80-0x8F).
 */
static reg_t* register_get(int name) {
    assert(is_register(name));
    return (reg_t*)((size_t*)registers + ((name - 0x80) * REGISTER_SIZE));
}

/**
 * Returns the register with the given index.
 */
static reg_t* register_at(int index) {
    return (reg_t*)((size_t*)registers + (index * REGISTER_SIZE));
}

/**
 * Sets all registers up to and including the given index as used, and sets all
 * other numbered registers as unused.
 */
static void registers_set_used_up_to(size_t index) {
    size_t i = 0;
    while (i <= index) {
        register_set_used(register_at(i), true);
        i = (i + 1);
    }
    while (i <= 9) {
        register_set_used(register_at(i), false);
        i = (i + 1);
    }
}

static void register_setup(void) {
    registers = calloc(16, REGISTER_SIZE * sizeof(size_t));
}

static void register_teardown(void) {
    free(registers);
}

#endif
