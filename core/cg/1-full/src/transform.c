/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Fraser Heavy Software
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

#include "transform.h"

#include "argument.h"
#include "block.h"
#include "common.h"
#include "instruction.h"
#include "libo-vector.h"
#include "symbol.h"
#include "variable.h"

void convert_parameters(symbol_t* symbol) {
}

void convert_entry(struct symbol_t* symbol) {
    block_t* start_block = vector_at(symbol->blocks, 0);

    // If the function has no stack frame and -fomit-frame-pointer is given we
    // can skip the enter and leave instructions.
    // TODO for now we don't do this ever since it harms debuggability. A
    // better optimization would be to add function inlining to cci/2 so that
    // small leaf functions disappear entirely.
    //if (symbol->frame_size == 0) {
    //    return;
    //}

    if (symbol->frame_size != 0) {

        // insert `sub rsp rsp N`
        instruction_t* sub = instruction_new(location_new_copy(symbol->location), opcode_sub);
        instruction_append(sub, argument_new_register(RSP));
        instruction_append(sub, argument_new_register(RSP));
        vector_insert(start_block->instructions, 0, sub);

        if (symbol->frame_size < 128) {
            instruction_append(sub, argument_new_integer(symbol->frame_size));
        } else {
            // frame size doesn't fix in mix-type byte. use r9
            instruction_append(sub, argument_new_register(9));

            // insert `imw r9 N`
            instruction_t* imw = instruction_new(location_new_copy(symbol->location), opcode_imw);
            instruction_append(imw, argument_new_register(9));
            instruction_append(imw, argument_new_integer(symbol->frame_size));
            vector_insert(start_block->instructions, 0, imw);
        }
    }

    // insert enter
    instruction_t* enter = instruction_new(location_new_copy(symbol->location), opcode_enter);
    vector_insert(start_block->instructions, 0, enter);
}

static void convert_ret(block_t* block, instruction_t* last) {

    // convert to mov if necessary
    argument_t* retval = vector_last(last->arguments);
    if (retval->type == argument_type_sentinel) {
        // no value. delete the argument.
        argument_delete(retval);
    } else {
        // insert mov r0 <arg>, taking ret's argument
        instruction_t* mov = instruction_new(location_new_copy(last->location), opcode_mov);
        instruction_append(mov, argument_new_number(argument_type_register, 0));
        instruction_append(mov, retval);
        vector_insert(block->instructions, vector_count(block->instructions) - 1, mov);
    }
    vector_remove_last(last->arguments);

    // insert leave
    vector_insert(block->instructions, vector_count(block->instructions) - 1,
            instruction_new(location_new_copy(last->location), opcode_leave));
}

static void convert_br(block_t* block, instruction_t* last) {

    // append a `jmp` instruction, stealing the br's `true` label argument
    instruction_t* jmp = instruction_new(location_new_copy(last->location), opcode_jmp);
    instruction_append(jmp, vector_remove(last->arguments, 1));
    vector_append(block->instructions, jmp);

    // convert `br` to `jz` (its `true` argument has been removed)
    last->opcode = opcode_jz;
}

void convert_control_flow(symbol_t* symbol) {
    for (size_t i = vector_count(symbol->blocks); i-- != 0;) {

        // get the last instruction
        block_t* block = vector_at(symbol->blocks, i);
        instruction_t* last = vector_last(block->instructions);

        // check if it's ret or br
        if (last->opcode == opcode_ret) {
            convert_ret(block, last);
        } else if (last->opcode == opcode_br) {
            convert_br(block, last);
        }
    }
}

void convert_vars(struct symbol_t* symbol) {
    // TODO parameters. each parameter will have a variable generated for it.
    //
    // the first four parameters will have a `stw r.. rfp @var` instruction
    // added.
    //
    // parameters beyond the fourth and varargs params will be assigned a
    // positive offset corresponding to their position above the stack frame.
    //
    // all parameters will then have an `add %.. rfp @var` instruction
    // inserted.
    //
    // optimizations can then clean up any unused temps and vars.

    size_t block_count = vector_count(symbol->blocks);
    for (size_t i = 0; i != block_count; ++i) {
        block_t* block = vector_at(symbol->blocks, i);

        size_t instruction_count = vector_count(block->instructions);
        for (size_t j = 0; j != instruction_count; ++j) {
            instruction_t* instruction = vector_at(block->instructions, j);
            if (instruction->opcode != opcode_var) {
                continue;
            }

            // found a `var` instruction. allocate a variable
            size_t size = argument_number(instruction_argument(instruction, 1));
            size_t alignment = 0;
            argument_t* alignment_arg = instruction_argument(instruction, 2);
            if (alignment_arg->type != argument_type_sentinel) {
                alignment = argument_number(alignment_arg);
            }
            variable_t* variable = variable_new(size, alignment);

            // convert `var %x .. ..` to `add %x rfp @x`
            instruction->opcode = opcode_add;
            argument_set_register(instruction_argument(instruction, 1), RFP);
            argument_set_variable(instruction_argument(instruction, 2), variable);
        }
    }
}
