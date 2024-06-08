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
    free(block->instructions);
    free(block);
}

void block_add(block_t* block, opcode_t opcode, ...) {
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

    va_list args;
    va_start(args, opcode);
    instruction_vset(block->instructions + block->instructions_count++, opcode, args);
    va_end(args);
}

void block_sub_rsp_r9(block_t* block, size_t offset) {
    if (offset == 0)
        return;
    if (offset > 0x7F) {
        block_add(block, IMW, ARGTYPE_NUMBER, R9, (int)offset);
        block_add(block, SUB, RSP, RSP, R9);
    } else {
        block_add(block, SUB, RSP, RSP, (int)offset);
    }
}

void block_add_rsp_r9(block_t* block, size_t offset) {
    if (offset == 0)
        return;
    if (offset > 0x7F) {
        block_add(block, IMW, ARGTYPE_NUMBER, R9, (int)offset);
        block_add(block, ADD, RSP, RSP, R9);
    } else {
        block_add(block, ADD, RSP, RSP, (int)offset);
    }
}
