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

instruction_t* instruction_new(location_t* location, opcode_t opcode) {
    instruction_t* instruction = malloc(sizeof(instruction_t));
    if (!instruction) {
        fatal("Out of memory.");
    }

    instruction->location = location;
    instruction->opcode = opcode;
    instruction->arguments = vector_new();

    return instruction;
}

void instruction_delete(instruction_t* instruction) {
    location_delete(instruction->location);
    for (size_t i = 0; i < vector_count(instruction->arguments); ++i) {
        argument_delete(vector_at(instruction->arguments, i));
    }
    vector_delete(instruction->arguments);
    free(instruction);
}

argument_mode_t instruction_mode(instruction_t* instruction) {
    switch (instruction->opcode) {

        // arithmetic
        case opcode_add:
        case opcode_sub:
        case opcode_mul:
        case opcode_divu:
        case opcode_divs:
        case opcode_modu:
        case opcode_mods:
        case opcode_zero:
            return argument_mode_write;
        case opcode_inc:
        case opcode_dec:
            return argument_mode_read_write;
        case opcode_sxs:
        case opcode_sxb:
        case opcode_trs:
        case opcode_trb:
            return argument_mode_write;

        // logic
        case opcode_and:
        case opcode_or:
        case opcode_xor:
        case opcode_not:
        case opcode_shl:
        case opcode_shru:
        case opcode_shrs:
        case opcode_rol:
        case opcode_ror:
        case opcode_mov:
        case opcode_bool:
        case opcode_isz:
        case opcode_ltu:
        case opcode_lts:
            return argument_mode_write;

        // memory
        case opcode_ldw:
        case opcode_lds:
        case opcode_ldb:
            return argument_mode_write;
        case opcode_stw:
        case opcode_sts:
        case opcode_stb:
            return argument_mode_read;

        // control
        case opcode_enter:
        case opcode_leave:
            return argument_mode_read;
        case opcode_var:
        case opcode_alloc:
        case opcode_free:
        case opcode_call:
        case opcode_volatile:
            return argument_mode_write;
        case opcode_ret:
        case opcode_jmp:
        case opcode_br:
        case opcode_jz:
            return argument_mode_read;

    }
    fatal("Internal error: unrecognized opcode in instruction_mode()");
}
