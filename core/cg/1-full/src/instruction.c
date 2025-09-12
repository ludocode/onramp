/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025-2026 Fraser Heavy Software
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

#include "instruction.h"

#include <stdlib.h>

#include "libo-error.h"
#include "argument.h"

instruction_t* instruction_new(const location_t* location, opcode_t opcode) {
    instruction_t* instruction = malloc(sizeof(instruction_t));
    if (!instruction) {
        fatal("Out of memory.");
    }

    instruction->location = location_new_copy(location);
    instruction->opcode = opcode;
    vector_init(&instruction->arguments);

    return instruction;
}

void instruction_delete(instruction_t* instruction) {
    location_delete(instruction->location);
    for (size_t i = 0; i < vector_count(&instruction->arguments); ++i) {
        argument_delete(vector_at(&instruction->arguments, i));
    }
    vector_destroy(&instruction->arguments);
    free(instruction);
}

void instruction_append(instruction_t* instruction, argument_t* argument) {
    vector_append(&instruction->arguments, argument);
}
