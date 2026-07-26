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

#include "optimize.h"

#include "argument.h"
#include "block.h"
#include "common.h"
#include "instruction.h"
#include "symbol.h"
#include "temporary.h"

int optimize;

/**
 * Scans all reachable instructions to determine which temporaries are only
 * assigned once.
 *
 * We can do much better optimizations with temporaries that are not
 * reassigned, but we need to detect them first. Eventually I'd like to convert
 * the IR to pure SSA form so this wouldn't be necessary, but for now most
 * temporaries can still be reassigned.
 */
static void optimize_scan_temporaries(symbol_t* symbol, block_t* block, int visited) {
    assert(block);
    if (block->visited == visited) {
        return;
    }
    block->visited = visited;

    // for each instruction
    size_t instruction_count = vector_count(block->instructions);
    for (size_t i = 0; i != instruction_count; ++i) {
        instruction_t* instruction = vector_at(block->instructions, i);
        if (instruction_mode(instruction) == argument_mode_read) {
            continue;
        }

        // get the output temporary
        argument_t* argument = instruction_argument(instruction, 0);
        if (argument->type != argument_type_temporary) {
            // the call instruction allows a sentinel output.
            continue;
        }
        temporary_t* temporary = argument_temporary(argument);

        if (!temporary->instruction) {
            // this is the first output to this temporary
            temporary->instruction = instruction;
            temporary->is_ssa = true;
        } else {
            // this is an additional output to this temporary.
            // (We don't store the source block; we only do during forward
            // propagation so we don't use a non-SSA temporary before it is
            // assigned.)
            temporary->is_ssa = false;
        }
    }

    // Continue to any blocks reachable from this one
    instruction_t* last = vector_last(block->instructions);
    if (last->opcode == opcode_jmp) {
        string_t* label = argument_label(instruction_argument(last, 0));
        optimize_scan_temporaries(symbol, block_find(label), visited);
    } else if (last->opcode == opcode_br) {
        string_t* true_label = argument_label(instruction_argument(last, 1));
        optimize_scan_temporaries(symbol, block_find(true_label), visited);
        string_t* false_label = argument_label(instruction_argument(last, 2));
        optimize_scan_temporaries(symbol, block_find(false_label), visited);
    }
}

/**
 * If all arguments to the given instruction are constants, try to compute it
 * now and replace it with mov.
 */
static void optimize_constant_instruction(instruction_t* instruction) {
    argument_mode_t mode = instruction_mode(instruction);
    if (mode != argument_mode_write) {
        return;
    }
    size_t arg_count = vector_count(instruction->arguments);

    if (arg_count == 3) {
        argument_t* arg_a = instruction_argument(instruction, 1);
        if (!argument_is_integer(arg_a)) {
            return;
        }

        argument_t* arg_b = instruction_argument(instruction, 2);
        if (!argument_is_integer(arg_b)) {
            return;
        }

        uint32_t val_a = argument_number(arg_a);
        uint32_t val_b = argument_number(arg_b);

        switch (instruction->opcode) {
            case opcode_add:
                argument_set_integer(arg_a, val_a + val_b);
                break;
            case opcode_sub:
                argument_set_integer(arg_a, val_a - val_b);
                break;
            case opcode_mul:
                argument_set_integer(arg_a, val_a * val_b);
                break;
            case opcode_divu:
                argument_set_integer(arg_a, val_a / val_b);
                break;
            case opcode_divs:
                argument_set_integer(arg_a, (uint32_t)((int32_t)val_a / (int32_t)val_b));
                break;
            case opcode_modu:
                argument_set_integer(arg_a, val_a % val_b);
                break;
            case opcode_mods:
                argument_set_integer(arg_a, (uint32_t)((int32_t)val_a % (int32_t)val_b));
                break;
            case opcode_and:
                argument_set_integer(arg_a, val_a & val_b);
                break;
            case opcode_or:
                argument_set_integer(arg_a, val_a | val_b);
                break;
            case opcode_xor:
                argument_set_integer(arg_a, val_a ^ val_b);
                break;
            case opcode_shl:
                if (val_b & 0xFFFFFFE0) {
                    fatal_loc(instruction->location, "shl bits out of range.");
                }
                argument_set_integer(arg_a, val_a << val_b);
                break;
            case opcode_shru:
                if (val_b & 0xFFFFFFE0) {
                    fatal_loc(instruction->location, "shru bits out of range.");
                }
                argument_set_integer(arg_a, val_a >> val_b);
                break;
            case opcode_shrs:
                if (val_b & 0xFFFFFFE0) {
                    fatal_loc(instruction->location, "shrs bits out of range.");
                }
                argument_set_integer(arg_a, (uint32_t)((int32_t)val_a >> val_b));
                break;
            case opcode_ltu:
                argument_set_integer(arg_a, val_a < val_b);
                break;
            case opcode_lts:
                argument_set_integer(arg_a, (int32_t)val_a < (int32_t)val_b);
                break;
            case opcode_rol:
                val_b &= 0x0000001F;
                if (val_b != 0) {
                    argument_set_integer(arg_a, (val_a << val_b) | (val_a >> (32 - val_b)));
                }
                break;
            case opcode_ror:
                val_b &= 0x0000001F;
                if (val_b != 0) {
                    argument_set_integer(arg_a, (val_a >> val_b) | (val_a << (32 - val_b)));
                }
                break;
            default:
                return;
        }

        argument_delete(arg_b);
        vector_remove(instruction->arguments, 2);
        instruction->opcode = opcode_mov;
        return;
    }

    if (arg_count == 2) {
        argument_t* arg = instruction_argument(instruction, 1);
        if (!argument_is_integer(arg)) {
            return;
        }
        uint32_t val = argument_number(arg);

        switch (instruction->opcode) {
            case opcode_sxs:
                argument_set_integer(arg, (uint32_t)(int32_t)(int16_t)val);
                break;
            case opcode_sxb:
                argument_set_integer(arg, (uint32_t)(int32_t)(int8_t)val);
                break;
            case opcode_trs:
                argument_set_integer(arg, (uint32_t)(uint16_t)val);
                break;
            case opcode_trb:
                argument_set_integer(arg, (uint32_t)(uint8_t)val);
                break;
            case opcode_not:
                argument_set_integer(arg, ~val);
                break;
            case opcode_bool:
                argument_set_integer(arg, !!val);
                break;
            case opcode_isz:
                argument_set_integer(arg, !val);
                break;
            default:
                return;
        }

        instruction->opcode = opcode_mov;
        return;
    }
}

/**
 * Run forward propagation on the given instruction.
 */
static void optimize_forward_instruction(symbol_t* symbol,
        block_t* block, instruction_t* instruction)
{
    argument_mode_t mode = instruction_mode(instruction);
    size_t i = (mode == argument_mode_write) ? 1 : 0;
    size_t arg_count = vector_count(instruction->arguments);

    // Replace any temporaries with constants
    for (; i < arg_count; ++i) {
        argument_t* argument = instruction_argument(instruction, i);
        if (!argument_is_temporary(argument)) {
            continue;
        }
        temporary_t* temporary = argument_temporary(argument);

        // We have a temporary input. Check if it was assigned a constant
        // value.
        if (!temporary->is_ssa && temporary->block != block) {
            // Non-ssa temporary was not previously assigned in this block. We
            // can't use it.
            continue;
        }
        instruction_t* src = temporary->instruction;
        assert(src);
        assert(argument_temporary(instruction_argument(src, 0)) == temporary);
        if (src->opcode != opcode_mov) {
            continue;
        }
        argument_t* src_arg = instruction_argument(src, 1);
        if (!argument_is_integer(src_arg)) {
            continue;
        }

        // It's a constant input. Replace it.
        argument_set_integer(argument, argument_number(src_arg));
    }

    // If all arguments are constants, try to resolve it at compile time.
    optimize_constant_instruction(instruction);

    // If this instruction outputs a non-ssa temporary, mark this as the last
    // instruction that modified it.
    if (mode != argument_mode_read) {
        argument_t* argument = instruction_argument(instruction, 0);
        if (argument->type == argument_type_temporary) {
            temporary_t* temporary = argument_temporary(argument);
            if (!temporary->is_ssa) {
                temporary->instruction = instruction;
                temporary->block = block;
            }
        }
    }
}

static void optimize_forward_block(symbol_t* symbol, block_t* block, int visited) {
    assert(block);
    if (block->visited == visited) {
        return;
    }
    block->visited = visited;

    // Run forward propagation on each instruction
    size_t count = vector_count(block->instructions);
    for (size_t i = 0; i != count; ++i) {
        instruction_t* instruction = vector_at(block->instructions, i);
        optimize_forward_instruction(symbol, block, instruction);
    }

    // Continue to any blocks reachable from this one
    instruction_t* last = vector_last(block->instructions);
    if (last->opcode == opcode_jmp) {
        string_t* label = argument_label(instruction_argument(last, 0));
        optimize_forward_block(symbol, block_find(label), visited);
    } else if (last->opcode == opcode_br) {
        string_t* true_label = argument_label(instruction_argument(last, 1));
        optimize_forward_block(symbol, block_find(true_label), visited);
        string_t* false_label = argument_label(instruction_argument(last, 2));
        optimize_forward_block(symbol, block_find(false_label), visited);
    }
}

void optimize_forward(symbol_t* symbol) {
    optimize_scan_temporaries(symbol, vector_first(symbol->blocks), pass_id++);

    // TODO optimizations in the forward pass:
    // - [x] constant propagation: replace any constant temporary inputs with their values
    // - [ ] base+offset arguments (add, loads, stores)
    //     - if one argument is zero, can replace with any add
    //     - if one argument is constant and other is base+offset, can combine offsets
    // - [x] any math op with constant args, replace with mov
    // - [ ] any temporary arg, if mov, replace with previous temporary (as long as both are ssa or neither has been modified)
    // - [ ] br arguments: bool or isz, can be optimized out
    // - [ ] br constant, replace with jmp, orphan other block recursively
    // - [ ] jmp, merge block if sole parent

    optimize_forward_block(symbol, vector_first(symbol->blocks), pass_id++);
}

/*
 * Scan all instructions in the given block, marking any input temporaries as used.
 */
static void optimize_dead_store_scan(symbol_t* symbol, block_t* block, int visited) {
    assert(block);
    if (block->visited == visited) {
        return;
    }
    block->visited = visited;

    // Check each instruction for temporary inputs
    size_t count = vector_count(block->instructions);
    for (size_t i = 0; i != count; ++i) {
        instruction_t* instruction = vector_at(block->instructions, i);
        size_t j = (instruction_mode(instruction) == argument_mode_write) ? 1 : 0;
        size_t count = vector_count(instruction->arguments);
        for (; j < count; ++j) {
            argument_t* argument = instruction_argument(instruction, j);
            if (argument_is_temporary(argument)) {
                argument_temporary(argument)->is_used = true;
            }
        }
    }

    // Continue to any blocks reachable from this one
    instruction_t* last = vector_last(block->instructions);
    if (last->opcode == opcode_jmp) {
        string_t* label = argument_label(instruction_argument(last, 0));
        optimize_dead_store_scan(symbol, block_find(label), visited);
    } else if (last->opcode == opcode_br) {
        string_t* true_label = argument_label(instruction_argument(last, 1));
        optimize_dead_store_scan(symbol, block_find(true_label), visited);
        string_t* false_label = argument_label(instruction_argument(last, 2));
        optimize_dead_store_scan(symbol, block_find(false_label), visited);
    }
}

/*
 * Remove all dead store instructions in the given block.
 *
 * For instructions with side-effects (e.g. `call`), the output temporary is
 * replaced with a sentinel. Otherwise the instruction opcode is changed to
 * nop.
 *
 * TODO this should be done in reverse order with liveness analysis, and/or we
 * should iterate until nothing more is deleted. This is just a quick first
 * pass implementation.
 */
static void optimize_dead_store_remove(symbol_t* symbol, block_t* block, int visited) {
    assert(block);
    if (block->visited == visited) {
        return;
    }
    block->visited = visited;

    // Check each instruction for a temporary output
    size_t count = vector_count(block->instructions);
    for (size_t i = 0; i != count; ++i) {
        instruction_t* instruction = vector_at(block->instructions, i);
        if (instruction_mode(instruction) == argument_mode_read) {
            continue;
        }
        argument_t* argument = instruction_argument(instruction, 0);
        if (!argument_is_temporary(argument)) {
            continue;
        }
        if (!argument_temporary(argument)->is_used) {
            // This is a dead store.
            if (instruction->opcode == opcode_call) {
                // TODO if this is a pure function we can delete the call. We'd
                // have to pass the pure attribute into the IR.
                argument_set_sentinel(argument);
            } else {
                instruction_set_nop(instruction);
            }
        }
    }

    // Continue to any blocks reachable from this one
    instruction_t* last = vector_last(block->instructions);
    if (last->opcode == opcode_jmp) {
        string_t* label = argument_label(instruction_argument(last, 0));
        optimize_dead_store_remove(symbol, block_find(label), visited);
    } else if (last->opcode == opcode_br) {
        string_t* true_label = argument_label(instruction_argument(last, 1));
        optimize_dead_store_remove(symbol, block_find(true_label), visited);
        string_t* false_label = argument_label(instruction_argument(last, 2));
        optimize_dead_store_remove(symbol, block_find(false_label), visited);
    }
}

void optimize_dead_store(symbol_t* symbol) {
    optimize_dead_store_scan(symbol, vector_first(symbol->blocks), pass_id++);
    optimize_dead_store_remove(symbol, vector_first(symbol->blocks), pass_id++);
}
