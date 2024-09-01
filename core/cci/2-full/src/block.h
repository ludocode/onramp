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
#include "libo-string.h"

struct token_t;

/**
 * A basic block of assembly instructions.
 */
typedef struct block_t {
    int label; // the label to jump to this block, or -1 if there is no label
    string_t* user_label; // non-null if this is a user-defined label
    // TODO should also store the token for the user label so we can emit file/line info for it
    instruction_t* instructions;
    size_t instructions_count;
    size_t instructions_capacity;
    bool emitted;
} block_t;

block_t* block_new(int label);

block_t* block_new_user_label(string_t* label);

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
void block_append(block_t* block, struct token_t* token, opcode_t opcode, ...);

/**
 * Appends instructions to subtract a value from the stack pointer, keeping the
 * stack aligned.
 */
void block_sub_rsp(block_t* block, struct token_t* /*nullable*/ token, size_t offset);

/**
 * Appends instructions to add a value to the stack pointer, keeping the stack
 * aligned.
 */
void block_add_rsp(block_t* block, struct token_t* /*nullable*/ token, size_t offset);

/**
 * Emits instructions to perform a basic opcode with an immediate value to the
 * given register. This will use a temporary register if necessary (if the
 * value doesn't fit in a mix-type byte.)
 */
void block_append_op_imm(block_t* block, struct token_t* token, opcode_t opcode,
        int reg_out, int reg_in, int value);

#endif
