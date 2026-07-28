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

#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>

#include "instruction.h"
#include "libo-string.h"

struct token_t;
struct type_t;

/**
 * A basic block of assembly instructions.
 */
typedef struct block_t {
    int label; // the label to jump to this block, or -1 if user-defined
    string_t* user_label; // non-null if this is a user-defined label
    // TODO should also store the token for the user label so we can emit file/line info for it
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

/*
 * Appends a new instruction to a block with the given number of arguments,
 * returning it.
 *
 * The arguments must be set separately.
 */
instruction_t* block_append(block_t* block, struct token_t* token, opcode_t opcode, size_t arg_count);

/**
 * Appends a jump to the given label.
 */
instruction_t* block_append_jmp(block_t* block, struct token_t* token,
        uint32_t label);

/**
 * Appends a branch to the given true and false labels.
 *
 * The predicate must be set separately (cci/0 functions are limited to four
 * arguments.)
 */
instruction_t* block_append_br(block_t* block, struct token_t* token,
        uint32_t true_label, uint32_t false_label);

#endif
