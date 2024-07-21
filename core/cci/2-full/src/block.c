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

#include "block.h"

#include <stdlib.h>

#include "emit.h"
#include "common.h"
#include "token.h"
#include "generate.h"

#define BLOCK_INSTRUCTIONS_MIN 8

block_t* block_new(int label) {
    block_t* block = malloc(sizeof(block_t));
    block->label = label;
    block->instructions = NULL;
    block->instructions_count = 0;
    block->instructions_capacity = 0;
    block->emitted = false;
    return block;
}

void block_delete(block_t* block) {
    for (size_t i = 0; i < block->instructions_count; ++i)
        instruction_destroy(&block->instructions[i]);
    free(block->instructions);
    free(block);
}

void block_append(block_t* block, token_t* token, opcode_t opcode, ...) {
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
    va_list args;
    va_start(args, opcode);
    instruction_init(instruction);
    instruction_vset(instruction, token, opcode, args);
    va_end(args);
}

void block_sub_rsp(block_t* block, token_t* token, size_t offset) {
    // TODO remove this wrapper function
    block_append_op_imm(block, token, SUB, RSP, (int)offset);
}

void block_add_rsp(block_t* block, token_t* token, size_t offset) {
    // TODO remove this wrapper function
    block_append_op_imm(block, token, ADD, RSP, (int)offset);
}

void block_append_op_imm(block_t* block, struct token_t* token, opcode_t opcode,
        int reg_out, int value)
{
    if (value == 0)
        return;

    if (value <= 127 && value >= -112) {
        block_append(block, token, opcode, reg_out, reg_out, value);
        return;
    }

    int reg_value = register_alloc(token);
    block_append(block, token, IMW, ARGTYPE_NUMBER, reg_value, value);
    block_append(block, token, opcode, reg_out, reg_out, reg_value);
    register_free(token, reg_value);
}
