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

#include "convert.h"

#include "argument.h"
#include "block.h"
#include "instruction.h"
#include "libo-vector.h"
#include "symbol.h"

void convert_entry(symbol_t* symbol) {

    // insert enter
    block_t* block = vector_at(symbol->blocks, 0);
    vector_insert(block->instructions, 0,
            instruction_new(symbol->location, opcode_enter));

    // TODO insert sub rsp stack space

    // TODO store arguments

}

void convert_ret(symbol_t* symbol) {
    for (size_t i = vector_count(symbol->blocks); i-- != 0;) {

        // get the last instruction. if it's not ret, ignore it.
        block_t* block = vector_at(symbol->blocks, i);
        if (vector_count(block->instructions) == 0) {
            continue;
        }
        instruction_t* last = vector_last(block->instructions);
        if (last->opcode != opcode_ret) {
            continue;
        }

        // convert ret argument to mov if necessary
        argument_t* retval = vector_last(&last->arguments);
        if (retval->type == argument_type_sentinel) {
            // no value. delete the argument.
            argument_delete(retval);
        } else {
            // insert mov r0 <arg>, taking ret's argument
            instruction_t* mov = instruction_new(last->location, opcode_mov);
            instruction_append(mov, argument_new_number(argument_type_register, 0));
            instruction_append(mov, retval);
            vector_insert(block->instructions, vector_count(block->instructions) - 1, mov);
        }
        vector_remove_last(&last->arguments);

        // insert leave
        vector_insert(block->instructions, vector_count(block->instructions) - 1,
                instruction_new(last->location, opcode_leave));
    }
}
