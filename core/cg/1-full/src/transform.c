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

#include <stdlib.h>

#include "analyze.h"
#include "argument.h"
#include "block.h"
#include "common.h"
#include "instruction.h"
#include "libo-error.h"
#include "libo-otable.h"
#include "libo-vector.h"
#include "symbol.h"
#include "temporary.h"
#include "variable.h"

/**
 * Generate a mov from one register to another, appending it to the given vector.
 */
static void transform_register_mov(vector_t* instructions, location_t* location, size_t dest, size_t src) {
    assert(dest < REGISTER_COUNT);
    assert(src < REGISTER_COUNT);
    if (dest == src) {
        return;
    }
    instruction_t* mov = instruction_new(location_new_copy(location), opcode_mov);
    instruction_append(mov, argument_new_register(dest));
    instruction_append(mov, argument_new_register(src));
    vector_append(instructions, mov);
}

/**
 * Appends the given 3-argument instruction to the given vector, with an
 * additional instruction to convert its final argument to mix-type if
 * necessary using the given register.
 *
 * If the instruction's last argument does not fit in a mix-type byte, an imw
 * instruction is also inserted to load it into the given register.
 *
 * Returns the number of instructions inserted (1 or 2.)
 */
static size_t transform_insert_instruction_mix(
        instruction_t* instruction, vector_t* vector, size_t index, int reg)
{
    vector_insert(vector, index, instruction);

    argument_t* argument = instruction_argument(instruction, 2);
    assert(argument->type == argument_type_number);
    uint32_t value = argument_number(argument);

    if (!mix_type_fits(value)) {
        instruction_t* imw = instruction_new(location_new_copy(instruction->location), opcode_imw);
        instruction_append(imw, argument_new_register(reg));
        instruction_append(imw, argument);
        vector_insert(vector, index, imw);

        vector_set(instruction->arguments, 2, argument_new_register(reg));
        return 2;
    }
    return 1;
}

void transform_parameters(symbol_t* symbol) {
    vector_t* parameters = symbol->parameters;
    if (vector_is_empty(parameters)) {
        return;
    }

    vector_t* preamble = vector_new();
    vector_t* variables = vector_new();

    // The first four parameters are passed in registers. Generate a variable
    // for each one and store it.
    size_t count = vector_count(parameters);
    size_t register_count = (count > 4) ? 4 : count;
    for (size_t i = 0; i != register_count; ++i) {
        if (vector_at(parameters, i) == NULL) {
            // sentinel; ignored parameter
            vector_append(variables, NULL);
            continue;
        }
        variable_t* variable = variable_new(4, 4);
        vector_append(variables, variable);

        // insert `stw rN rfp @var`
        instruction_t* stw = instruction_new(location_new_copy(symbol->location), opcode_stw);
        instruction_append(stw, argument_new_register(i));
        instruction_append(stw, argument_new_register(RFP));
        instruction_append(stw, argument_new_variable(variable));
        vector_append(preamble, stw);
    }

    // Now create a temporary for the location of each variable.
    for (size_t i = 0; i != register_count; ++i) {
        temporary_t* temporary = vector_at(parameters, i);
        if (temporary == NULL) {
            // sentinel; ignored parameter
            continue;
        }
        variable_t* variable = vector_at(variables, i);

        // insert `add %name rfp @var`
        instruction_t* add = instruction_new(location_new_copy(symbol->location), opcode_add);
        instruction_append(add, argument_new_temporary(temporary));
        instruction_append(add, argument_new_register(RFP));
        instruction_append(add, argument_new_variable(variable));
        vector_append(preamble, add);
    }

    // Any additional parameters are passed on the stack. Each one is turned
    // into a variable with positive frame offset.
    // (The frame pointer points to the previous frame pointer. The word above
    // it is the return address. Stack-passed arguments start above that.)
    int offset = 8;
    if (count > 4) {
        for (size_t i = 4; i != count; ++i) {
            temporary_t* temporary = vector_at(parameters, i);
            if (temporary != NULL) {
                // insert `add %name rfp <offset>`
                instruction_t* add = instruction_new(location_new_copy(symbol->location), opcode_add);
                instruction_append(add, argument_new_temporary(vector_at(parameters, i)));
                instruction_append(add, argument_new_register(RFP));
                instruction_append(add, argument_new_integer(offset));
                vector_append(preamble, add);
            }
            offset += 4;
        }
    }

    // If there is a variadic parameter, assign it now.
    if (symbol->varargs) {

        // insert `add %_Vargs rfp <offset>`
        instruction_t* add = instruction_new(location_new_copy(symbol->location), opcode_add);
        instruction_append(add, argument_new_temporary(symbol->varargs));
        instruction_append(add, argument_new_register(RFP));
        instruction_append(add, argument_new_integer(offset));
        vector_append(preamble, add);
    }

    // Insert all generated instructions at the front of the first block.
    // TODO we need a vector bulk insert function to do this much more efficiently
    block_t* start_block = vector_at(symbol->blocks, 0);
    for (size_t i = vector_count(preamble); i-- != 0;) {
        vector_insert(start_block->instructions, 0, vector_at(preamble, i));
    }

    vector_delete(variables);
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
        instruction_append(sub, argument_new_integer(symbol->frame_size));
        vector_insert(start_block->instructions, 0, sub);
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

void transform_variables(symbol_t* symbol) {
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
            #ifdef LOG_REGISTER_ALLOCATOR
            printf("Generated var for temporary %s variable @%zu\n",
                    argument_temporary(instruction_argument(instruction, 0))->name->bytes,
                    variable->id);
            #endif

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
 * Spilled outputs use r8. Spilled inputs, or variable inputs whose offsets
 * don't fit in a mix-type byte, use registers r8 and r9.
 */
static size_t transform_registers_normal(block_t* block, instruction_t* instruction, size_t index) {
    argument_mode_t mode = instruction_mode(instruction);

    // Check for temporary or variable inputs
    size_t reg = FIRST_SPILL_REGISTER;
    size_t j = (mode == argument_mode_write) ? 1 : 0;
    size_t count = vector_count(instruction->arguments);
    for (; j < count; ++j) {
        argument_t* argument = instruction_argument(instruction, j);

        if (argument->type == argument_type_temporary) {
            temporary_t* temporary = argument_temporary(argument);
            if (temporary->reg == TEMPORARY_REGISTER_INVALID) {
                // temporary is spilled. load it into spill register `reg` (r8 or r9.)
                assert(reg != FIRST_SPILL_REGISTER + AVAILABLE_SPILL_REGISTERS);
                assert(temporary->variable);
                assert(temporary->variable->offset != 0);

                argument_set_register(argument, reg);
                instruction_t* ldw = instruction_new(location_new_copy(instruction->location), opcode_ldw);
                instruction_append(ldw, argument_new_register(reg));
                instruction_append(ldw, argument_new_register(RFP));
                instruction_append(ldw, argument_new_integer(temporary->variable->offset));
                // use the same register if the offset doesn't fit in a mix-type byte.
                index += transform_insert_instruction_mix(ldw, block->instructions, index, reg);

                ++reg;
            } else {
                // temporary is in a register. replace with the register.
                argument_set_register(argument, temporary->reg);
            }

        } else if (argument->type == argument_type_variable) {
            variable_t* variable = argument_variable(argument);
            assert(variable->offset != 0);

            if (mix_type_fits((uint32_t)variable->offset)) {
                // variable fits. replace it with its offset.
                argument_set_integer(argument, variable->offset);
            } else {
                // variable doesn't fit. use spill register `reg` (r8 or r9.)
                argument_set_register(argument, reg);
                instruction_t* imw = instruction_new(location_new_copy(instruction->location), opcode_imw);
                instruction_append(imw, argument_new_register(reg));
                instruction_append(imw, argument_new_integer(variable->offset));
                vector_insert(block->instructions, index++, imw);
                ++reg;
            }

        } else if (argument->type == argument_type_number) {
            opcode_t opcode = instruction->opcode;
            if (opcode == opcode_mov) {
                // mov is being used for a large number. change mov to imw.
                instruction->opcode = opcode_imw;
            } else if (opcode != opcode_imw) {
                uint32_t integer = argument_number(argument);
                if (!mix_type_fits(integer)) {
                    // integer argument doesn't fit in mix-type byte. use spill register `reg` (r8 or r9.)
                    argument_set_register(argument, reg);
                    instruction_t* imw = instruction_new(location_new_copy(instruction->location), opcode_imw);
                    instruction_append(imw, argument_new_register(reg));
                    instruction_append(imw, argument_new_integer(integer));
                    vector_insert(block->instructions, index++, imw);
                    ++reg;
                }
            }
        }
    }

    // Check for a temporary output
    if (mode != argument_mode_read) {
        argument_t* argument = instruction_argument(instruction, 0);
        if (argument->type == argument_type_temporary) {
            temporary_t* temporary = argument_temporary(argument);
            if (temporary->reg == TEMPORARY_REGISTER_INVALID) {
                // temporary is spilled. replace with r8 and append a store
                // instruction.
                printf("SPILLED TEMPORARY OUTPUT %s\n", temporary->name->bytes);
                assert(temporary->variable);
                assert(temporary->variable->offset != 0);

                argument_set_register(argument, FIRST_SPILL_REGISTER);
                instruction_t* stw = instruction_new(location_new_copy(instruction->location), opcode_stw);
                instruction_append(stw, argument_new_register(FIRST_SPILL_REGISTER));
                instruction_append(stw, argument_new_register(RFP));
                instruction_append(stw, argument_new_integer(temporary->variable->offset));
                // use r9 if the offset doesn't fit in a mix-type byte.
                index += transform_insert_instruction_mix(stw, block->instructions,
                        index + 1, FIRST_SPILL_REGISTER + 1);
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
 * we only have two extra registers (r8 and r9.)
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
            if (mix_type_fits((uint32_t)argument_variable(argument)->offset)) {
                // variable offset fits; don't need a spill register
                return transform_registers_normal(block, store, index);
            }
        } else if (argument->type == argument_type_number) {
            if (mix_type_fits(argument_number(argument))) {
                // mix-type integer argument fits; don't need a register
                return transform_registers_normal(block, store, index);
            }
        } else {
            // otherwise it's a register or constant
            return transform_registers_normal(block, store, index);
        }
    }

    // We have three spills. Separate the addition.
    // (Use the second spill register for the addition result because the first
    // may be needed for the value.)
    instruction_t* add = instruction_new(location_new_copy(store->location), opcode_add);
    instruction_append(add, argument_new_register(FIRST_SPILL_REGISTER + 1));
    instruction_append(add, vector_at(store->arguments, 1));
    instruction_append(add, vector_at(store->arguments, 2));
    vector_insert(block->instructions, index, add);
    vector_set(store->arguments, 1, argument_new_integer(0));
    vector_set(store->arguments, 2, argument_new_register(FIRST_SPILL_REGISTER + 1));

    // Perform transform on each instruction separately.
    index = transform_registers_normal(block, add, index);
    index = transform_registers_normal(block, store, ++index);
    return index;
}

// Loads or stores all live temporaries around a call instruction.
static void transform_preserve_call(instruction_t* instruction, vector_t* instructions, bool load) {
    assert(instruction->opcode == opcode_call);
    assert(instruction->live_temps != NULL);
    otable_t* live_temps = instruction->live_temps;
    for (void** entry = otable_begin(live_temps); entry; entry = otable_next(live_temps, entry)) {
        temporary_t* temporary = *entry;
        if (temporary->reg == -1) {
            // temporary is already spilled.
            continue;
        }
        assert(temporary->variable);
        assert(temporary->variable->offset != 0);

        instruction_t* mem = instruction_new(location_new_copy(instruction->location),
               load ? opcode_ldw : opcode_stw);
        instruction_append(mem, argument_new_register(temporary->reg));
        instruction_append(mem, argument_new_register(RFP));
        instruction_append(mem, argument_new_integer(temporary->variable->offset));
        // use r8 if the offset doesn't fit in a mix-type byte
        transform_insert_instruction_mix(mem, instructions, vector_count(instructions), 8);
    }
}

/**
 * For the given call instruction, generates instructions to push or pop
 * stack-passed arguments.
 *
 * See transform_call().
 */
static void transform_call_stack_args(vector_t* instructions, instruction_t* instruction, bool push) {
    vector_t* arguments = instruction->arguments;
    size_t arg_count = vector_count(arguments) - 2; // first two instruction args are retval and function

    // Up to four arguments can be passed in registers. Variadic arguments are
    // never passed in registers.
    size_t reg_count = 4;
    if (instruction->varargs_index != VARARGS_INDEX_INVALID) {
        reg_count = instruction->varargs_index;
    }
    if (arg_count < reg_count) {
        // No arguments are passed on the stack.
        return;
    }

    // Arguments beyond the fourth are pushed in reverse order (i.e. the fifth
    // argument is lowest on the stack.)

    // Create (or free) stack space
    size_t stack_space = 4 * (arg_count - reg_count);
    instruction_t* addsub = instruction_new(location_new_copy(instruction->location),
            push ? opcode_sub : opcode_add);
    instruction_append(addsub, argument_new_register(RSP));
    instruction_append(addsub, argument_new_register(RSP));
    instruction_append(addsub, argument_new_integer(stack_space));
    // use r8 if it doesn't fit in a mix-type byte
    transform_insert_instruction_mix(addsub, instructions, vector_count(instructions), 8);

    if (!push) {
        return;
    }

    // Push arguments
    for (size_t i = reg_count; i < arg_count; ++i) {
        argument_t* argument = vector_at(instruction->arguments, i + 2);
        instruction_t* stw = instruction_new(location_new_copy(instruction->location), opcode_stw);

        if (argument->type == argument_type_number || argument->type == argument_type_variable) {
            uint32_t value = (argument->type == argument_type_number) ?
                argument_number(argument) : (uint32_t)argument_variable(argument)->offset;
            if (mix_type_fits(value)) {
                // value fits in a mix-type byte. we can store it directly.
                instruction_append(stw, argument_new_integer(value));
            } else {
                // value doesn't fit. load it into r8.
                instruction_t* imw = instruction_new(location_new_copy(instruction->location), opcode_imw);
                instruction_append(imw, argument_new_register(8));
                instruction_append(imw, argument_new_integer(value));
                vector_append(instructions, imw);
                instruction_append(stw, argument_new_register(8));
            }

        } else if (argument->type == argument_type_temporary) {
            temporary_t* temporary = argument_temporary(argument);
            if (temporary->reg != -1) {
                // argument is live. we can store straight from its register.
                instruction_append(stw, argument_new_register(temporary->reg));
            } else {
                // argument is spilled. load it into r8.
                instruction_t* ldw = instruction_new(location_new_copy(instruction->location), opcode_ldw);
                instruction_append(ldw, argument_new_register(8));
                instruction_append(ldw, argument_new_register(RFP));
                instruction_append(ldw, argument_new_integer(temporary->variable->offset));
                transform_insert_instruction_mix(ldw, instructions, vector_count(instructions), 8);
                instruction_append(stw, argument_new_register(8));
            }

        } else if (argument->type == argument_type_sentinel) {
            // nothing to do; argument is left uninitialized
            instruction_delete(stw);
            continue;

        } else {
            fatal("Internal error: invalid argument type to call instruction");
        }

        // store the argument to the stack
        instruction_append(stw, argument_new_register(RSP));
        instruction_append(stw, argument_new_integer((i - reg_count) * 4));
        // use r9 if the offset doesn't fit in a mix-type byte
        transform_insert_instruction_mix(stw, instructions, vector_count(instructions), 9);
    }
}

/**
 * Determine the source live register for the given argument.
 *
 * If the given argument is a live temporary, this returns the register that
 * contains it. Otherwise it returns TEMPORARY_REGISTER_INVALID.
 */
static int transform_call_live_register(instruction_t* instruction, vector_t* arguments, size_t index) {
    if (index >= vector_count(arguments)) {
        return TEMPORARY_REGISTER_INVALID;
    }
    if (instruction->varargs_index != VARARGS_INDEX_INVALID && index >= 2 + instruction->varargs_index) {
        // this variadic argument is not passed in a register
        return TEMPORARY_REGISTER_INVALID;
    }
    argument_t* argument = vector_at(arguments, index);

    if (argument_is_register(argument)) {
        // could be rfp, rsp or even rpp; could happen due to optimizations
        return argument_register(argument);
    }

    if (argument_is_temporary(argument)) {
        temporary_t* temporary = argument_temporary(argument);
        return temporary->reg;
    }

    return TEMPORARY_REGISTER_INVALID;
}

/**
 * For the given call instruction, generates instructions to place the first
 * four arguments in r0-r3 and the function pointer (if any) in r4, if those
 * arguments are live (or are otherwise already in registers.)
 *
 * We only care about live arguments first because we have to permute them
 * carefully without clobbering any registers we need.
 *
 * See transform_call().
 */
static void transform_call_live_args(vector_t* instructions, instruction_t* instruction) {
    vector_t* arguments = instruction->arguments;
    location_t* location = instruction->location;

    // Register that originally contained the value needed in the indexed register
    // i.e. need[3] == 4 means the live temporary assigned to r4 needs to be
    // moved into the argument slot r3
    int* need = malloc(sizeof(int) * 5);
    need[0] = transform_call_live_register(instruction, arguments, 2);
    need[1] = transform_call_live_register(instruction, arguments, 3);
    need[2] = transform_call_live_register(instruction, arguments, 4);
    need[3] = transform_call_live_register(instruction, arguments, 5);
    need[4] = transform_call_live_register(instruction, arguments, 1); // function pointer arg
    //printf("\n====================== %s()\n", __func__);

    // Current location of live value that was originally in the indexed register
    // i.e. loc[4] == 1 means the live temporary assigned to r4 is currently in r1
    int* loc = malloc(sizeof(int) * 5);

    for (int i = 0; i != 5; ++i) {
        // Initialize locations
        loc[i] = i;
        //printf("need[%i] == %i\n", i, need[i]);

        // If any arguments are already where they belong, we don't need to move them
        if (need[i] == i) {
            //printf("%i is already where it goes\n",i);
            need[i] = TEMPORARY_REGISTER_INVALID;
        }
    }

    int spill = FIRST_SPILL_REGISTER;
    for (;;) {

        // find the highest-numbered register that needs an argument to be
        // moved into it
        // TODO change all these int to size_t, argument->reg should be unsigned
        int target = TEMPORARY_REGISTER_INVALID;
        for (int i = 5; i-- != 0;) {
            if (need[i] != TEMPORARY_REGISTER_INVALID) {
                target = i;
                break;
            }
        }
        if (target == TEMPORARY_REGISTER_INVALID) {
            // we're done!
            break;
        }
        int original_target = target;
        //printf("\nstarting iteration at target %i, needs register %i\n", target, need[target]);

        // See if another argument needs the contents of this register. If so,
        // we'd clobber it if we moved this one first, so move that one first
        // instead. We do this check in a loop until we find a register that
        // can be moved without clobbering anything.
        for (;;) {
            if (loc[target] != target) {
                // The target register's data has already been moved elsewhere
                // so we wouldn't clobber anything by assigning it.
                //printf("target %i already moved to %i\n", target, loc[target]);
                break;
            }

            int other = TEMPORARY_REGISTER_INVALID;
            for (int i = 0; i < 5; ++i) {
                if (i != target && need[i] == target) {
                    // We've found another argument that needs this register
                    other = i;
                    //printf("other %i needs register %i\n",other, target);
                    break;
                }
            }
            if (other == TEMPORARY_REGISTER_INVALID) {
                // Nothing needs to contents of this register. We can assign it.
                //printf("no other register needs target %i\n",target);
                break;
            }

            target = other;
            if (target == original_target) {
                // We have a cycle. There can be at most two cycles (since we
                // have up to five live registers) so just move the contents
                // into the next spill register to break the cycle.
                //printf("found cycle, moving loc[%i] to spill %i\n",target,spill);
                assert(spill != FIRST_SPILL_REGISTER + AVAILABLE_SPILL_REGISTERS);
                transform_register_mov(instructions, location, spill, target);
                loc[target] = spill;
                ++spill;
                break;
            }

            // Loop around on the new register and check again.
        }

        // We've found a register to move.
        int src_reg = need[target];
        //printf("moving src %i to target %i\n", src_reg, target);
        transform_register_mov(instructions, location, target,
                (src_reg < 5) ? loc[src_reg] : src_reg);
        need[target] = TEMPORARY_REGISTER_INVALID;
        if (loc[target] == target) {
            loc[target] = TEMPORARY_REGISTER_INVALID;
        }
    }

    //printf("%s() done\n\n", __func__);
    free(loc);
    free(need);
}

/**
 * For the given call instruction, generates instructions to place the first
 * four arguments in r0-r3 and the function pointer (if any) in r4, if those
 * arguments are spilled, or are otherwise not in other registers.
 *
 * See transform_call().
 */
static void transform_call_spilled_args(vector_t* instructions, instruction_t* instruction) {
    vector_t* arguments = instruction->arguments;
    location_t* location = instruction->location;

    size_t count = vector_count(arguments);
    if (count > 6) {
        count = 6;
    }
    if (instruction->varargs_index != VARARGS_INDEX_INVALID && count >= 2 + instruction->varargs_index) {
        count = 2 + instruction->varargs_index;
    }

    for (size_t i = 1; i < count; ++i) {
        argument_t* argument = vector_at(arguments, i);

        // The second argument to the call instruction is the function pointer;
        // the next four arguments go in registers r0-r3.
        int dest_reg = (i == 1) ? 4 : (i - 2);

        switch (argument->type) {
            case argument_type_sentinel: // nothing to do
            case argument_type_register: // handled in live
            case argument_type_absolute: // nothing to do
                continue;

            case argument_type_relative:
                // shouldn't have been able to parse this
                fatal("Invalid relative argument to call instruction.");
                break;

            case argument_type_number:
                instruction_t* imw = instruction_new(location_new_copy(location), opcode_imw);
                instruction_append(imw, argument_new_register(dest_reg));
                instruction_append(imw, argument_new_integer(argument->number));
                vector_append(instructions, imw);
                break;

            case argument_type_temporary: {
                temporary_t* temporary = argument_temporary(argument);
                if (temporary->reg != TEMPORARY_REGISTER_INVALID) {
                    // live arguments have already been handled
                    break;
                }
                assert(temporary->variable != NULL);
                assert(temporary->variable->offset != 0);
                instruction_t* ldw = instruction_new(location_new_copy(location), opcode_ldw);
                instruction_append(ldw, argument_new_register(dest_reg));
                instruction_append(ldw, argument_new_register(RFP));
                instruction_append(ldw, argument_new_integer(temporary->variable->offset));
                // use the same register if the offset doesn't fit in a mix-type byte.
                transform_insert_instruction_mix(ldw, instructions, vector_count(instructions), dest_reg);
                break;
            }

            case argument_type_variable: {
                variable_t* variable = argument_variable(argument);
                assert(variable->offset != 0);
                instruction_t* imw = instruction_new(location_new_copy(location), opcode_imw);
                instruction_append(imw, argument_new_register(dest_reg));
                instruction_append(imw, argument_new_integer(variable->offset));
                vector_append(instructions, imw);
                break;
            }
        }
    }
}

/**
 * Places the return value where it goes after a call instruction.
 *
 * If the return value is spilled, it is stored to the stack. If the return
 * value is live, it is moved to its appropriate register.
 */
static void transform_call_return(vector_t* instructions, instruction_t* instruction) {
    argument_t* ret_arg = vector_first(instruction->arguments);
    if (ret_arg->type == argument_type_sentinel) {
        return;
    }

    temporary_t* temporary = argument_temporary(ret_arg);

    if (temporary->reg != -1) {
        // return value is in a register. move it there
        transform_register_mov(instructions, instruction->location, temporary->reg, 0);
    } else {
        // return value is spilled. store it
        assert(temporary->variable->offset != 0);
        instruction_t* stw = instruction_new(location_new_copy(instruction->location), opcode_stw);
        instruction_append(stw, argument_new_register(0));
        instruction_append(stw, argument_new_register(RFP));
        instruction_append(stw, argument_new_integer(temporary->variable->offset));
        // use r8 if the offset doesn't fit in a mix-type byte.
        transform_insert_instruction_mix(stw, instructions, vector_count(instructions), FIRST_SPILL_REGISTER);
    }
}

/**
 * Transform registers and variables for a call instruction.
 */
static size_t transform_call(block_t* block, instruction_t* instruction, size_t index) {
    vector_t* instructions = vector_new();

    // All live temporaries must be preserved.
    transform_preserve_call(instruction, instructions, false);

    // Arguments beyond the fourth get pushed to the stack.
    transform_call_stack_args(instructions, instruction, true);

    // The first four arguments go in r0-r3, and the function pointer goes in r9.
    transform_call_live_args(instructions, instruction);
    transform_call_spilled_args(instructions, instruction);

    // Transform the call instruction itself
    instruction_t* new_call = instruction_new(
            location_new_copy(instruction->location), opcode_call);
    argument_t* function_name = instruction_argument(instruction, 1);
    instruction_append(new_call, argument_is_absolute(function_name) ?
            argument_new_copy(function_name) : argument_new_register(4)); // function pointers in r4
    vector_append(instructions, new_call);

    // Pop args
    transform_call_stack_args(instructions, instruction, false);

    // Put the return value where it goes
    transform_call_return(instructions, instruction);

    // All live temporaries must be restored.
    transform_preserve_call(instruction, instructions, true);

    // Finally, we replace the original call instruction in the block with all
    // of the instructions we've generated.
    vector_remove(block->instructions, index);
    // TODO vector bulk insert
    for (size_t i = 0; i < vector_count(instructions); ++i) {
        vector_insert(block->instructions, index + i, vector_at(instructions, i));
    }
    instruction_delete(instruction);
    size_t count = vector_count(instructions);
    vector_delete(instructions);
    return index + count - 1;
}

static void transform_registers_block(symbol_t* symbol, block_t* block, int visited) {
    if (block->visited == visited) {
        return;
    }
    block->visited = visited;

    // The instruction count will change as we walk through the block.
    for (size_t j = 0; j != vector_count(block->instructions); ++j) {
        instruction_t* instruction = vector_at(block->instructions, j);

        // Some instructions have to be handled specially.
        switch (instruction->opcode) {
            case opcode_call:
                j = transform_call(block, instruction, j);
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

    // Continue to any blocks reachable from this one
    instruction_t* last = vector_last(block->instructions);
    if (last->opcode == opcode_jmp) {
        string_t* label = argument_label(instruction_argument(last, 0));
        transform_registers_block(symbol, block_find(label), visited);
    } else if (last->opcode == opcode_br) {
        string_t* true_label = argument_label(instruction_argument(last, 1));
        transform_registers_block(symbol, block_find(true_label), visited);
        string_t* false_label = argument_label(instruction_argument(last, 2));
        transform_registers_block(symbol, block_find(false_label), visited);
    }
}

void transform_registers(symbol_t* symbol) {
    // Transform only reachable blocks. (Unreachable temporaries are not
    // considered by the register allocator.)
    transform_registers_block(symbol, vector_first(symbol->blocks), pass_id++);
}
