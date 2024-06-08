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

#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>

#include "instruction.h"

/**
 * A basic block of assembly instructions.
 */
typedef struct block_t {
    int label; // the label to jump to this block, or -1 if there is no label
    instruction_t* instructions;
    size_t instructions_count;
    size_t instructions_capacity;
    bool emitted;
} block_t;

block_t* block_new(int label);

void block_delete(block_t* block);

static inline size_t block_count(block_t* block) {
    return block->instructions_count;
}

static inline instruction_t* block_at(block_t* block, size_t index) {
    assert(index < block->instructions_count);
    return block->instructions + index;
}

/**
 * Appends a new instruction to the end of the block.
 */
void block_add(block_t* block, opcode_t opcode, ...);

/**
 * Emits instructions to subtract a value from the stack pointer, using r9 as
 * temporary storage if needed.
 */
void block_sub_rsp_r9(block_t* block, size_t offset);

/**
 * Emits instructions to add a value from the stack pointer, using r9 as
 * temporary storage if needed.
 */
void block_add_rsp_r9(block_t* block, size_t offset);

#endif
