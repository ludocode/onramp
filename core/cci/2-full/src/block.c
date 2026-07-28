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

#include "block.h"

#include <stdlib.h>

#include "common.h"
#include "emit.h"
#include "generate.h"
#include "token.h"
#include "type.h"

#define BLOCK_INSTRUCTIONS_MIN 8

static block_t* block_new_impl(void) {
    block_t* block = malloc(sizeof(block_t));
    block->label = -1;
    block->user_label = NULL;
    block->instructions = NULL;
    block->instructions_count = 0;
    block->instructions_capacity = 0;
    block->emitted = false;
    return block;
}

block_t* block_new(int label) {
    assert(label != -1);
    block_t* block = block_new_impl();
    block->label = label;
    return block;
}

void block_delete(block_t* block) {
    for (size_t i = 0; i < block->instructions_count; ++i)
        instruction_destroy(&block->instructions[i]);
    if (block->user_label)
        string_deref(block->user_label);
    free(block->instructions);
    free(block);
}

instruction_t* block_append(block_t* block, token_t* token, opcode_t opcode,
        size_t arg_count
) {
    // TODO use vector_t, would simplify this a lot
    if (block->instructions_count == block->instructions_capacity) {
        // grow
        size_t new_capacity = block->instructions_capacity * 2;
        if (new_capacity < BLOCK_INSTRUCTIONS_MIN) {
            new_capacity = BLOCK_INSTRUCTIONS_MIN;
        }
        if (new_capacity <= block->instructions_capacity) {
            fatal("Out of memory.");
        }
        block->instructions_capacity = new_capacity;
        block->instructions = realloc(block->instructions, new_capacity * sizeof(instruction_t));
        if (block->instructions == NULL) {
            fatal("Out of memory.");
        }
    }

    instruction_t* instruction = block->instructions + block->instructions_count++;
    instruction_init(instruction, token, opcode, arg_count);
    return instruction;
}

instruction_t* block_append_jmp(block_t* block, token_t* token, uint32_t label) {
    instruction_t* instruction = block_append(block, token, JMP, 1);
    instruction_set_arg_relative(instruction, 0, label);
    return instruction;
}

instruction_t* block_append_br(block_t* block, token_t* token,
        uint32_t true_label, uint32_t false_label)
{
    instruction_t* instruction = block_append(block, token, BR, 3);
    // arg 0 not set
    instruction_set_arg_relative(instruction, 1, true_label);
    instruction_set_arg_relative(instruction, 2, false_label);
    return instruction;
}
