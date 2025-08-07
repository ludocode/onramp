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

typedef void reg_t;

#define REGISTER_ORIGINALLY_USED 0
#define REGISTER_CONTAINS_CONSTANT 1
#define REGISTER_CONSTANT_VALUE 2
#define REGISTER_CONTAINS_VARIABLE 3
#define REGISTER_VARIABLE_OFFSET 4
#define REGISTER_WAS_WRITTEN 5
#define REGISTER_SIZE 6

/**
 * True if the register was originally used in this function.
 *
 * Registers that are not used in the original function can be used for
 * optimizations.
 */
static bool register_originally_used(reg_t* reg) {
    return *(bool*)((size_t*)reg + REGISTER_ORIGINALLY_USED);
}

/**
 * True if the register contains a compile-time constant value.
 *
 * This is used for constant propagation.
 */
static bool register_contains_constant(reg_t* reg) {
    return *(bool*)((size_t*)reg + REGISTER_CONTAINS_CONSTANT);
}

/**
 * The constant value of a register (if it contains a constant.)
 */
static int register_constant_value(reg_t* reg) {
    return *(int*)((size_t*)reg + REGISTER_CONSTANT_VALUE);
}

/**
 * True if the register contains the value of a variable (i.e. it was last
 * assigned by a load from a particular stack offset.)
 *
 * This is used for load elimination.
 */
static bool register_contains_variable(reg_t* reg) {
    return *(bool*)((size_t*)reg + REGISTER_CONTAINS_VARIABLE);
}

/**
 * The frame offset of the variable contained in a register (if it contains a
 * variable.)
 */
static int register_variable_offset(reg_t* reg) {
    return *(int*)((size_t*)reg + REGISTER_VARIABLE_OFFSET);
}

/**
 * When walking backwards through a block, this is true if the register was
 * last written to, and false if the register was last read.
 *
 * This is used for dead store elimination. (If a register is written to twice
 * without a read in between, the first write can be eliminated.)
 */
static bool register_was_written(reg_t* reg) {
    return *(bool*)((size_t*)reg + REGISTER_WAS_WRITTEN);
}

static void register_set_originally_used(reg_t* reg, bool originally_used) {
    *(bool*)((size_t*)reg + REGISTER_ORIGINALLY_USED) = originally_used;
}

static void register_set_contains_constant(reg_t* reg, bool contains_constant) {
    *(bool*)((size_t*)reg + REGISTER_CONTAINS_CONSTANT) = contains_constant;
}

static void register_set_constant_value(reg_t* reg, bool constant_value) {
    *(bool*)((size_t*)reg + REGISTER_CONSTANT_VALUE) = constant_value;
}

static void register_set_contains_variable(reg_t* reg, bool contains_variable) {
    *(bool*)((size_t*)reg + REGISTER_CONTAINS_VARIABLE) = contains_variable;
}

static void register_set_variable_offset(reg_t* reg, bool variable_offset) {
    *(bool*)((size_t*)reg + REGISTER_VARIABLE_OFFSET) = variable_offset;
}

static void register_set_was_written(reg_t* reg, bool was_written) {
    *(bool*)((size_t*)reg + REGISTER_WAS_WRITTEN) = was_written;
}

/*
static reg_t* register_new(void) {
    reg_t* reg = calloc(REGISTER_SIZE, sizeof(size_t));
}

static void register_delete(reg_t* reg) {
    free(reg);
}
*/

static reg_t* registers;

/**
 * Returns the register with the given index.
 */
static reg_t* register_at(int i) {
    return (reg_t*)((size_t*)registers + (i * REGISTER_SIZE));
}

static void register_setup(void) {
    registers = calloc(16, REGISTER_SIZE * sizeof(size_t));
}

static void register_teardown(void) {
    free(registers);
}

#endif
