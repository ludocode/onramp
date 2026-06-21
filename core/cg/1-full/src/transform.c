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
#include "libo-error.h"
#include "libo-vector.h"
#include "symbol.h"
#include "temporary.h"
#include "variable.h"

void transform_parameters(symbol_t* symbol) {
    vector_t* parameters = symbol->parameters;
    if (vector_is_empty(parameters)) {
        return;
    }

    vector_t* preamble = vector_new();

    // The first four parameters are passed in registers. Generate a variable
    // for each one and append a store instruction.
    size_t count = vector_count(parameters);
    size_t register_count = (count > 4) ? 4 : count;
    for (size_t i = 0; i < register_count; ++i) {
        temporary_t* temporary = vector_at(parameters, i);

        // insert `var %name 4 %`
        instruction_t* var = instruction_new(location_new_copy(symbol->location), opcode_var);
        instruction_append(var, argument_new_temporary(temporary));
        instruction_append(var, argument_new_integer(4));
        instruction_append(var, argument_new_sentinel());
        vector_append(preamble, var);

        // insert `stw rN %name`
        instruction_t* stw = instruction_new(location_new_copy(symbol->location), opcode_stw);
        instruction_append(stw, argument_new_register(i));
        instruction_append(stw, argument_new_temporary(temporary));
        vector_append(preamble, stw);
    }

    // Any additional parameters are passed on the stack. Each one is turned
    // into a variable with positive frame offset.
    int offset = 4;
    for (size_t i = 4; i < count; ++i) {
        // insert `add %name rfp <offset>`
        instruction_t* add = instruction_new(location_new_copy(symbol->location), opcode_add);
        instruction_append(add, argument_new_temporary(vector_at(parameters, i)));
        instruction_append(add, argument_new_register(RFP));
        instruction_append(add, argument_new_integer(offset));
        vector_append(preamble, add);
        offset += 4;
    }

    // If there is a variadic parameter, assign it now.
    if (symbol->varargs) {

        // insert `add %_Vargs rfp <offset>`
        instruction_t* add = instruction_new(location_new_copy(symbol->location), opcode_add);
        instruction_append(add, argument_new_temporary(symbol->varargs));
        instruction_append(add, argument_new_register(RFP));
        // We don't need to handle mix-type, it will be split out later
        //if (offset < 128) {
            instruction_append(add, argument_new_integer(offset));
        /*
        } else {
            // offset doesn't fix in mix-type byte. use r9
            instruction_append(sub, argument_new_register(9));

            // insert `imw r9 N`
            instruction_t* imw = instruction_new(location_new_copy(symbol->location), opcode_imw);
            instruction_append(imw, argument_new_register(9));
            instruction_append(imw, argument_new_integer(offset));
            vector_append(preamble, imw);
        }
        */
        vector_append(preamble, add);
    }

    // Insert all generated instructions at the front of the first block.
    // TODO we need a vector bulk insert function to do this much more efficiently
    block_t* start_block = vector_at(symbol->blocks, 0);
    for (size_t i = vector_count(preamble); i-- != 0;) {
        vector_insert(start_block->instructions, 0, vector_at(preamble, i));
    }

    vector_delete(preamble);
}

void transform_entry(struct symbol_t* symbol) {
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

static void transform_ret(block_t* block, instruction_t* last) {

    // convert to mov if necessary
    argument_t* retval = vector_last(last->arguments);
    if (retval->type == argument_type_sentinel) {
        // no value. delete the argument.
        argument_delete(retval);
    } else {
        // insert mov r0 <arg>, taking ret's argument
        instruction_t* mov = instruction_new(location_new_copy(last->location), opcode_mov);
        instruction_append(mov, argument_new_register(0));
        instruction_append(mov, retval);
        vector_insert(block->instructions, vector_count(block->instructions) - 1, mov);
    }
    vector_remove_last(last->arguments);

    // insert leave
    vector_insert(block->instructions, vector_count(block->instructions) - 1,
            instruction_new(location_new_copy(last->location), opcode_leave));
}

static void transform_br(block_t* block, instruction_t* last) {

    // append a `jmp` instruction, stealing the br's `true` label argument
    instruction_t* jmp = instruction_new(location_new_copy(last->location), opcode_jmp);
    instruction_append(jmp, vector_remove(last->arguments, 1));
    vector_append(block->instructions, jmp);

    // convert `br` to `jz` (its `true` argument has been removed)
    last->opcode = opcode_jz;
}

void transform_control_flow(symbol_t* symbol) {
    for (size_t i = vector_count(symbol->blocks); i-- != 0;) {

        // get the last instruction
        block_t* block = vector_at(symbol->blocks, i);
        instruction_t* last = vector_last(block->instructions);

        // check if it's ret or br
        if (last->opcode == opcode_ret) {
            transform_ret(block, last);
        } else if (last->opcode == opcode_br) {
            transform_br(block, last);
        }
    }
}

void transform_load_store_sym(symbol_t* symbol) {
    size_t block_count = vector_count(symbol->blocks);
    for (size_t i = 0; i != block_count; ++i) {
        block_t* block = vector_at(symbol->blocks, i);

        size_t instruction_count = vector_count(block->instructions);
        for (size_t j = 0; j != instruction_count; ++j) {
            instruction_t* instruction = vector_at(block->instructions, j);

            switch (instruction->opcode) {
                case opcode_sym: {
                    // `sym %1 ^foo` --> `imw %2 ^foo  add %1 rpp %2`

                    temporary_t* temporary = temporary_new_anonymous();
                    instruction_t* imw = instruction_new(location_new_copy(instruction->location), opcode_imw);
                    instruction_append(imw, argument_new_temporary(temporary));
                    instruction_append(imw, instruction_argument(instruction, 1));

                    vector_set(instruction->arguments, 1, argument_new_temporary(temporary));
                    vector_insert(instruction->arguments, 1, argument_new_register(RPP));
                    instruction->opcode = opcode_add;

                    vector_insert(block->instructions, j, imw);
                    ++j;
                    ++instruction_count;
                    break;
                }

                case opcode_ldw:
                case opcode_lds:
                case opcode_ldb:
                case opcode_stw:
                case opcode_sts:
                case opcode_stb: {
                    argument_t* argument = instruction_argument(instruction, 1);
                    if (argument->type == argument_type_absolute) {
                        // `op %1 ^foo` --> `imw %2 ^foo  op %1 rpp %2`

                        temporary_t* temporary = temporary_new_anonymous();
                        instruction_t* imw = instruction_new(location_new_copy(instruction->location), opcode_imw);
                        instruction_append(imw, argument_new_temporary(temporary));
                        instruction_append(imw, argument);

                        vector_set(instruction->arguments, 1, argument_new_temporary(temporary));
                        vector_insert(instruction->arguments, 1, argument_new_register(RPP));

                        vector_insert(block->instructions, j, imw);
                        ++j;
                        ++instruction_count;
                    } else {
                        // `op %1 x` --> `op %1 x 0`
                        instruction_append(instruction, argument_new_integer(0));
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }
}

void transform_vars(symbol_t* symbol) {
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

/**
 * Transform registers and variables for a "normal" instruction.
 *
 * These instructions have at most one output and at most two inputs.
 *
 * Spilled outputs use r0. Spilled inputs, or variable inputs whose offsets
 * don't fit in a mix-type byte, use registers r0 and r1.
 */
static size_t transform_registers_normal(block_t* block, instruction_t* instruction, size_t index) {
    argument_mode_t mode = instruction_mode(instruction);

    // Check for temporary or variable inputs
    size_t reg = 0;
    size_t j = (mode == argument_mode_write) ? 1 : 0;
    size_t count = vector_count(instruction->arguments);
    for (; j < count; ++j) {
        argument_t* argument = instruction_argument(instruction, j);
        if (argument->type == argument_type_temporary) {
            temporary_t* temporary = argument_temporary(argument);
            if (temporary->reg == -1) {
                // temporary is spilled. load it into `reg` (r0 or r1.)
                assert(reg != 2);
                argument_set_register(argument, reg);
                instruction_t* ldw = instruction_new(location_new_copy(instruction->location), opcode_ldw);
                instruction_append(ldw, argument_new_register(reg));
                instruction_append(ldw, argument_new_register(RFP));
                vector_insert(block->instructions, index++, ldw);

                // use the same register if the offset doesn't fit in a mix-type byte.
                variable_t* variable = temporary->variable;
                if (variable_offset_fits_in_mix_type(variable)) {
                    instruction_append(ldw, argument_new_integer(variable->offset));
                } else {
                    instruction_append(ldw, argument_new_register(reg));
                    instruction_t* imw = instruction_new(location_new_copy(instruction->location), opcode_imw);
                    instruction_append(imw, argument_new_register(reg));
                    instruction_append(imw, argument_new_integer(variable->offset));
                    vector_insert(block->instructions, index++, imw);
                }
                ++reg;
            } else {
                // temporary is in a register. replace with the register.
                argument_set_register(argument, temporary->reg);
            }

        } else if (argument->type == argument_type_variable) {
            variable_t* variable = argument_variable(argument);
            if (variable_offset_fits_in_mix_type(variable)) {
                // variable fits. replace it with its offset.
                argument_set_integer(argument, variable->offset);
            } else {
                // variable doesn't fit. use `reg` (r0 or r1.)
                argument_set_register(argument, reg);
                instruction_t* imw = instruction_new(location_new_copy(instruction->location), opcode_imw);
                instruction_append(imw, argument_new_register(reg));
                instruction_append(imw, argument_new_integer(variable->offset));
                vector_insert(block->instructions, index++, imw);
                ++reg;
            }
        }
    }

    // Check for a temporary output
    if (mode != argument_mode_read) {
        argument_t* argument = instruction_argument(instruction, 0);
        if (argument->type == argument_type_temporary) {
            temporary_t* temporary = argument_temporary(argument);
            if (temporary->reg == -1) {
                // temporary is spilled. replace with r0 and append a store
                // instruction.
                argument_set_register(argument, 0);
                instruction_t* stw = instruction_new(location_new_copy(instruction->location), opcode_stw);
                instruction_append(stw, argument_new_register(0));
                instruction_append(stw, argument_new_register(RFP));

                // use r1 if the offset doesn't fit in a mix-type byte.
                variable_t* variable = temporary->variable;
                if (variable_offset_fits_in_mix_type(variable)) {
                    instruction_append(stw, argument_new_integer(variable->offset));
                } else {
                    instruction_append(stw, argument_new_register(1));
                    instruction_t* imw = instruction_new(location_new_copy(instruction->location), opcode_imw);
                    instruction_append(imw, argument_new_register(1));
                    instruction_append(imw, argument_new_integer(variable->offset));
                    vector_insert(block->instructions, ++index, imw);
                }
                vector_insert(block->instructions, ++index, stw);
            } else {
                // temporary is in a register. replace with the register.
                argument_set_register(argument, temporary->reg);
            }

        }
    }

    return index;
}

/**
 * Transform registers and variables for a store instruction.
 *
 * These instructions need to be handled separately because they have three
 * inputs, all of which may be spilled (or may not fit in a mix-type byte), but
 * we only have two extra registers (r0 and r1.)
 *
 * In case we need more registers, we perform the addition separately.
 */
static size_t transform_registers_store(block_t* block, instruction_t* store, size_t index) {

    // Check the three arguments. If any of them don't need a spill register,
    // we'll have at most two spills so we can forward to
    // transform_registers_normal().
    for (size_t i = 0; i < 3; ++i) {
        argument_t* argument = instruction_argument(store, i);
        if (argument->type == argument_type_temporary) {
            if (argument_temporary(argument)->reg != -1) {
                // temporary is in a register
                return transform_registers_normal(block, store, index);
            }
        } else if (argument->type == argument_type_variable) {
            if (variable_offset_fits_in_mix_type(argument_variable(argument))) {
                // variable offset fits; don't need a spill register
                return transform_registers_normal(block, store, index);
            }
        } else {
            // otherwise it's a register or constant
            return transform_registers_normal(block, store, index);
        }
    }

    // We have three spills. Separate the addition.
    instruction_t* add = instruction_new(location_new_copy(store->location), opcode_add);
    instruction_append(add, argument_new_register(1));
    instruction_append(add, vector_at(store->arguments, 1));
    instruction_append(add, vector_at(store->arguments, 2));
    vector_insert(block->instructions, index, add);
    vector_set(store->arguments, 1, argument_new_integer(0));
    vector_set(store->arguments, 2, argument_new_register(1));

    // Perform transform on each instruction separately.
    index = transform_registers_normal(block, add, index);
    index = transform_registers_normal(block, store, ++index);
    return index;
}

/**
 * Transform registers and variables for a call instruction.
 */
static size_t transform_registers_call(block_t* block, instruction_t* instruction, size_t index) {
    fatal("TODO transform call instruction");
    // need to assign variables for all live temporaries on a call instruction
    // even if they will be stored in registers (making sure to not include
    // temporaries that end as inputs to the call!) do this during register
    // allocation analysis
    //
    // all temporaries in registers must be spilled to their variables then
    // restored afterwards
    //
    // if function to call is itself a temporary, place it in r9. place third
    // and fourth arguments in r2 and r3. will need to do this carefully,
    // permute using r0/r1 where needed.
    //
    // finally place first two arguments in r0, r1
}

void transform_registers(symbol_t* symbol) {
    size_t block_count = vector_count(symbol->blocks);
    for (size_t i = 0; i != block_count; ++i) {
        block_t* block = vector_at(symbol->blocks, i);

        // The instruction count will change as we walk through the block.
        for (size_t j = 0; j != vector_count(block->instructions); ++j) {
            instruction_t* instruction = vector_at(block->instructions, j);

            // Some instructions have to be handled specially.
            switch (instruction->opcode) {
                case opcode_call:
                    j = transform_registers_call(block, instruction, j);
                    break;

                case opcode_stw:
                case opcode_sts:
                case opcode_stb:
                    j = transform_registers_store(block, instruction, j);
                    break;

                default:
                    j = transform_registers_normal(block, instruction, j);
                    break;
            }
        }
    }
}
