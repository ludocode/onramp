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

#include "analyze.h"

#include <stdlib.h>

#include "block.h"
#include "common.h"
#include "instruction.h"
#include "libo-error.h"
#include "libo-otable.h"
#include "libo-vector.h"
#include "symbol.h"
#include "temporary.h"
#include "variable.h"

void analyze_block_parents(symbol_t* symbol) {
    size_t count = vector_count(symbol->blocks);
    for (size_t i = 0; i < count; ++i) {
        block_t* block = vector_at(symbol->blocks, i);
        //printf("%s() block %s\n", __func__, block->name->bytes);
        instruction_t* last = vector_last(block->instructions);

        switch (last->opcode) {
            block_t* child;

            case opcode_ret:
                //printf("found ret block %s\n", block->name->bytes);
                // no child blocks
                break;

            case opcode_br:
                //printf("br looking up true block %s\n", argument_label(instruction_argument(last, 1))->bytes);
                child = block_find(argument_label(instruction_argument(last, 1)));
                vector_append(child->parent_blocks, block);

                //printf("br looking up false block %s\n", argument_label(instruction_argument(last, 2))->bytes);
                child = block_find(argument_label(instruction_argument(last, 2)));
                vector_append(child->parent_blocks, block);
                break;

            case opcode_jmp:
                //printf("jmp looking up block %s\n", argument_label(instruction_argument(last, 0))->bytes);
                child = block_find(argument_label(instruction_argument(last, 0)));
                vector_append(child->parent_blocks, block);
                break;

            default:
                // this should have been checked during parsing
                fatal("Internal error: invalid end of block");
        }
    }
}

/**
 * Collects all blocks that end in a `ret` instruction into the given
 * ret_blocks vector, marking them visited.
 */
static void analyze_collect_ret_blocks(symbol_t* symbol, vector_t* ret_blocks, int visited) {
    size_t count = vector_count(symbol->blocks);
    for (size_t i = 0; i < count; ++i) {
        block_t* block = vector_at(symbol->blocks, i);
        instruction_t* last = vector_last(block->instructions);
        if (last->opcode == opcode_ret) {
            #ifdef LOG_REGISTER_ALLOCATOR
            printf("Found ret block %s\n", block->name->bytes);
            #endif
            vector_append(ret_blocks, block);
            block->visited = visited;
        }
    }
}

/**
 * A recursive depth-first search from the start block to find blocks reachable
 * from start but not reachable backwards from a `ret` instruction (i.e.
 * infinite loops.)
 *
 * Any blocks found this way are inserted at the front of the vector so that
 * their order is reversed.
 */
static void analyze_liveness_block_order_forward(vector_t* blocks,
        block_t* block, int visited)
{
    if (block->visited == visited) {
        return;
    }

    block->visited = visited;
    vector_insert(blocks, 0, block);

    instruction_t* last = vector_last(block->instructions);
    switch (last->opcode) {
        block_t* child;

        case opcode_br:
            child = block_find(instruction_argument(last, 1)->string);
            analyze_liveness_block_order_forward(blocks, child, visited);

            child = block_find(instruction_argument(last, 2)->string);
            analyze_liveness_block_order_forward(blocks, child, visited);
            break;

        case opcode_jmp:
            child = block_find(instruction_argument(last, 0)->string);
            analyze_liveness_block_order_forward(blocks, child, visited);
            break;

        default:
            break;
    }
}

/**
 * Generates a block ordering on which to perform each iteration of liveness
 * analysis.
 *
 * Our liveness analysis algorithm will yield the same results regardless of
 * block order, but it will converge much more quickly if we order the blocks
 * properly. Liveness analysis is performed in reverse order, so we walk
 * backwards breadth-first from each `ret` block until all reachable blocks are
 * found.
 */
static vector_t* analyze_liveness_block_order(symbol_t* symbol) {
    int visited = pass_id++;

    vector_t* blocks = vector_new();
    vector_reserve(blocks, vector_count(symbol->blocks));

    // Start with all `ret` blocks.
    analyze_collect_ret_blocks(symbol, blocks, visited);

    // Now we step through the blocks array, appending any parent nodes that
    // have not been visited yet. Note that the vector count here grows as we
    // walk through it.
    for (size_t i = 0; i < vector_count(blocks); ++i) {
        block_t* block = vector_at(blocks, i);
        #ifdef LOG_REGISTER_ALLOCATOR
        printf("Block order visiting: %s\n", block->name->bytes);
        #endif
        for (size_t j = vector_count(block->parent_blocks); j-- != 0;) {
            block_t* parent = vector_at(block->parent_blocks, j);
            if (parent->visited != visited) {
                parent->visited = visited;
                vector_append(blocks, parent);
            }
        }
    }

    // Now we walk forward from the start block to find any blocks that were
    // missed. They get inserted in reverse order.
    block_t* start = vector_first(symbol->blocks);
    analyze_liveness_block_order_forward(blocks, start, visited);

    // The blocks array now contains all reachable blocks in a good order for
    // iteration of liveness analysis.
    return blocks;
}

void analyze_liveness_instruction(otable_t* live_temps, instruction_t* instruction) {
    //printf("  instruction %s\n", opcode_to_string(instruction->opcode));
    argument_mode_t mode = instruction_mode(instruction);

    // Check for a temporary output
    if (mode != argument_mode_read) {
        // The instruction writes to its first argument (which must exist.)
        // If the temporary was live, it becomes dead.
        argument_t* argument = instruction_argument(instruction, 0);
        if (argument->type == argument_type_temporary) {
            temporary_t* temporary = argument_temporary(argument);
            //printf("  found write arg %s\n", temporary->name->bytes);
            otable_remove(live_temps, temporary, temporary_hash(temporary));
        }
    }

    // Look for temporary inputs
    size_t first_read_arg = (mode == argument_mode_write) ? 1 : 0;
    size_t count = vector_count(instruction->arguments);
    for (size_t j = first_read_arg; j < count; ++j) {
        argument_t* argument = instruction_argument(instruction, j);
        if (argument->type == argument_type_temporary) {
            temporary_t* temporary = argument_temporary(argument);
            //printf("  found read arg at %zu: %s\n", j, temporary->name->bytes);
            // The instruction is reading this temporary. It becomes live.
            otable_put(live_temps, temporary, temporary_hash(temporary));
        }
    }
}

void analyze_liveness(symbol_t* symbol) {

    // Generate a good block order
    vector_t* blocks = analyze_liveness_block_order(symbol);
    size_t block_count = vector_count(blocks);

    #ifdef LOG_REGISTER_ALLOCATOR
    for (size_t i = 0; i < block_count; ++i) {
        printf("Final block order: %s\n", ((block_t*)vector_at(blocks,i))->name->bytes);
    }
    #endif

    // We iterate until nothing changes. Each iteration can only grow the set
    // of live temporaries, and since there are a finite number of temporaries,
    // this is guaranteed to eventually terminate.
    for (;;) {
        bool changed = false;
        #ifdef LOG_REGISTER_ALLOCATOR
        printf("\n=========================\nStarting liveness iteration\n");
        #endif

        // We're walking through the blocks in the order given by
        // analyze_liveness_block_order(), which is backwards breadth-first
        // from `ret` blocks.
        for (size_t i = 0; i < block_count; ++i) {
            block_t* block = vector_at(blocks, i);
            otable_t* live_temps = otable_new_copy(block->live_temps);

            #ifdef LOG_REGISTER_ALLOCATOR
            printf("Block %s end live_temps:", block->name->bytes);
            temporaries_print_table(live_temps);
            putchar('\n');
            #endif

            // Walk backwards through the instructions, updating this block's
            // live_temps as we go.
            for (size_t j = vector_count(block->instructions); j-- != 0;) {
                instruction_t* instruction = vector_at(block->instructions, j);
                analyze_liveness_instruction(live_temps, instruction);
            }

            #ifdef LOG_REGISTER_ALLOCATOR
            printf("Block %s start live_temps:", block->name->bytes);
            temporaries_print_table(live_temps);
            putchar('\n');
            #endif

            // The result is the set of live temps at the start of the block.
            // Merge it into the live temps at the end of every parent.
            for (size_t j = vector_count(block->parent_blocks); j-- != 0;) {
                block_t* parent = vector_at(block->parent_blocks, j);
                #ifdef LOG_REGISTER_ALLOCATOR
                printf("  Propagating to end of parent: %s\n", parent->name->bytes);
                #endif
                size_t old_count = otable_count(parent->live_temps);
                otable_union(parent->live_temps, live_temps);
                if (old_count != otable_count(parent->live_temps)) {
                    // We've grown a set of temporaries. We'll need another pass.
                    changed = true;
                }
            }
            otable_delete(live_temps);
        }

        if (!changed) {
            #ifdef LOG_REGISTER_ALLOCATOR
            printf("No changes. Done.\n\n");
            #endif
            break;
        }
        #ifdef LOG_REGISTER_ALLOCATOR
        printf("Changes. Looping.\n");
        #endif
    }

    vector_delete(blocks);
}

/**
 * Number all instructions in the block for the purpose of live interval
 * analysis.
 *
 * We walk in depth-first order, following false blocks before true blocks in
 * branches.
 */
static size_t analyze_number_instructions(block_t* block, size_t index, int visited) {

    // only scan blocks once
    if (block->visited == visited) {
        return index;
    }
    block->visited = visited;
    //printf("analyze_number_instructions() visiting block %s\n", block->name->bytes);

    // number all instructions in the block
    size_t count = vector_count(block->instructions);
    instruction_t* instruction;
    for (size_t i = 0; i < count; ++i) {
        instruction = vector_at(block->instructions, i);
        instruction->index = index++;
        //printf("analyze_number_instructions() numbering instruction %zu: %s\n",
        //        instruction->index, opcode_to_string(instruction->opcode));
    }

    switch (instruction->opcode) {
        block_t* child;

        case opcode_ret:
            //printf("ret no children\n");
            // no child blocks
            break;

        case opcode_br:
            //printf("br looking up false block %s\n", argument_label(instruction_argument(instruction, 2))->bytes);
            child = block_find(argument_label(instruction_argument(instruction, 2)));
            if (!child) {
                fatal("Block not found");
            }
            vector_append(child->parent_blocks, block);
            index = analyze_number_instructions(child, index, visited);

            //printf("br looking up true block %s\n", argument_label(instruction_argument(instruction, 1))->bytes);
            child = block_find(argument_label(instruction_argument(instruction, 1)));
            if (!child) {
                fatal("Block not found");
            }
            vector_append(child->parent_blocks, block);
            index = analyze_number_instructions(child, index, visited);
            break;

        case opcode_jmp:
            //printf("jmp looking up dest block %s\n", argument_label(instruction_argument(instruction, 0))->bytes);
            child = block_find(argument_label(instruction_argument(instruction, 0)));
            if (!child) {
                fatal("Block not found");
            }
            vector_append(child->parent_blocks, block);
            index = analyze_number_instructions(child, index, visited);
            break;

        default:
            // this should have been checked during parsing
            fatal("Internal error: invalid end of block");
    }

    return index;
}

/**
 * Expand the interval of the given temporary.
 */
static void analyze_expand_interval(temporary_t* temporary, size_t index) {
    if (temporary->interval_start == TEMPORARY_INTERVAL_INVALID) {
        //printf("initial interval for %s: %zu\n", temporary->name->bytes, index);
        temporary->interval_start = index;
        temporary->interval_end = index;
    } else {
        //printf("expanding interval for %s: %zu\n", temporary->name->bytes, index);
        if (temporary->interval_start > index) {
            //printf("lowering interval for %s: %zu\n", temporary->name->bytes, index);
            temporary->interval_start = index;
        }
        if (temporary->interval_end < index) {
            //printf("raising interval for %s: %zu\n", temporary->name->bytes, index);
            temporary->interval_end = index;
        }
    }
}

/**
 * Expand the interval of all temporaries in the given set to include the given
 * instruction index.
 */
static void analyze_expand_live_intervals(otable_t* live_temps, size_t index) {
    for (void** temp = otable_begin(live_temps); temp; temp = otable_next(live_temps, temp)) {
        analyze_expand_interval(*temp, index);
    }
}

static void analyze_live_intervals(block_t* block, int visited) {

    // Only visit each block once
    if (block->visited == visited) {
        return;
    }
    block->visited = visited;

    // Start with the table of live temporaries at the end of this block
    otable_t* live_temps = otable_new_copy(block->live_temps);

    // Any live temporaries at the end of this block must have their live
    // interval expanded to include it.
    size_t end_index = ((instruction_t*)vector_last(block->instructions))->index;
    #ifdef LOG_REGISTER_ALLOCATOR
    printf("Visiting block: %s\n", block->name->bytes);
    printf("Expanding live intervals at end %zu for temporaries:", end_index);
    temporaries_print_table(live_temps);
    putchar('\n');
    #endif
    analyze_expand_live_intervals(live_temps, end_index);

    // Walk backwards through the block. We're looking for instructions that
    // make temporaries live or dead.
    for (size_t i = vector_count(block->instructions); i-- != 0;) {
        instruction_t* instruction = vector_at(block->instructions, i);
        argument_mode_t mode = instruction_mode(instruction);

        // Look for temporary outputs
        if (mode != argument_mode_read) {
            // The instruction writes to its first argument (which must exist.)
            // If the temporary was live, it becomes dead.
            argument_t* argument = instruction_argument(instruction, 0);
            if (argument->type == argument_type_temporary) {
                temporary_t* temporary = argument_temporary(argument);
                otable_remove(live_temps, temporary, temporary_hash(temporary));
                analyze_expand_interval(temporary, instruction->index);
            }
        }

        // Look for call instructions. All live temporaries must be preserved
        // across the call so we make sure each one has stack space reserved.
        if (instruction->opcode == opcode_call) {
            for (void** entry = otable_begin(live_temps); entry;
                    entry = otable_next(live_temps, entry))
            {
                temporary_t* temporary = *entry;
                if (temporary->variable == NULL) {
                    temporary->variable = variable_new(4, 4);
                }
            }

            // Store the set of live temps in the instruction so we can
            // preserve them.
            assert(!instruction->live_temps);
            instruction->live_temps = otable_new_copy(live_temps);
        }

        // Look for temporary inputs
        size_t j = (mode == argument_mode_write) ? 1 : 0;
        size_t count = vector_count(instruction->arguments);
        for (; j < count; ++j) {
            argument_t* argument = instruction_argument(instruction, j);
            if (argument->type == argument_type_temporary) {
                // The instruction is reading this temporary. It is live from
                // the *previous* instruction.
                //
                // (If it is never read again, it becomes dead here after we
                // read from it. We don't include this instruction because the
                // output of this instruction may become live; this way it can
                // use the same register.)
                temporary_t* temporary = argument_temporary(argument);
                otable_put(live_temps, temporary, temporary_hash(temporary));
                assert(instruction->index != 0);
                analyze_expand_interval(temporary, instruction->index - 1);
            }
        }

    }

    // Any live temporaries at the start of this block must have their live
    // interval expanded to include it.
    size_t start_index = ((instruction_t*)vector_first(block->instructions))->index;
    #ifdef LOG_REGISTER_ALLOCATOR
    printf("Expanding live intervals at start %zu for temporaries:", start_index);
    temporaries_print_table(live_temps);
    putchar('\n');
    #endif
    analyze_expand_live_intervals(live_temps, start_index);

    otable_delete(live_temps);

    // Continue to any blocks reachable from this one
    instruction_t* last = vector_last(block->instructions);
    if (last->opcode == opcode_jmp) {
        string_t* label = argument_label(instruction_argument(last, 0));
        analyze_live_intervals(block_find(label), visited);
    } else if (last->opcode == opcode_br) {
        string_t* true_label = argument_label(instruction_argument(last, 1));
        analyze_live_intervals(block_find(true_label), visited);
        string_t* false_label = argument_label(instruction_argument(last, 2));
        analyze_live_intervals(block_find(false_label), visited);
    }
}

/**
 * Performs a linear scan.
 */
static void analyze_linear_scan(symbol_t* symbol) {

    // Collect all temporaries
    vector_t* temporaries = vector_new();
    temporaries_list_all(temporaries);
    size_t count = vector_count(temporaries);

    #ifndef __onramp_cci_opc
    #ifndef __onramp_cci_omc
    #define HAVE_QSORT
    #endif
    #endif

    // Sort the list of temporaries by increasing start index. (During
    // bootstrapping we do a simple insertion sort.)
    #ifndef HAVE_QSORT
    for (size_t i = 0; i < count; ++i) {
        for (size_t j = i + 1; j-- != 1;) {
            temporary_t* t2 = vector_at(temporaries, j);
            temporary_t* t1 = vector_at(temporaries, j - 1);
            if (0 >= temporary_compare_live_interval(&t1, &t2)) {
                break;
            }
            vector_set(temporaries, j, t1);
            vector_set(temporaries, j - 1, t2);
        }
    }
    #endif
    #ifdef HAVE_QSORT
    qsort(temporaries->elements, count, sizeof(void*), temporary_compare_live_interval);
    #endif

    temporary_t** registers = calloc(AVAILABLE_LIVE_REGISTERS, sizeof(temporary_t*));
    size_t registers_used = 0;

    for (size_t i = 0; i < vector_count(temporaries); ++i) {
        temporary_t* temporary = vector_at(temporaries, i);
        size_t length = temporary_interval_length(temporary);

        #ifdef LOG_REGISTER_ALLOCATOR
        if (temporary->interval_start == TEMPORARY_INTERVAL_INVALID) {
            printf("Temporary %s not used.\n", temporary->name->bytes);
        } else {
            printf("Considering temporary %s with interval %zu-%zu\n", temporary->name->bytes,
                    temporary->interval_start, temporary->interval_end);
        }
        #endif

        // If any of the live temporaries have an end interval earlier than the
        // start of the current temporary, we can clear them. These register
        // assignments become permanent.
        size_t j;
        for (j = 0; j < AVAILABLE_LIVE_REGISTERS; ++j) {
            if (registers[j] != NULL) {
                temporary_t* reg = registers[j];
                if (reg->interval_end < temporary->interval_start) {
                    #ifdef LOG_REGISTER_ALLOCATOR
                    printf("Assigned temporary %s register r%zu\n",
                            reg->name->bytes, FIRST_LIVE_REGISTER + j);
                    #endif
                    registers[j] = NULL;
                    --registers_used;
                }
            }
        }

        // Find a register in the list that is either unused, or that has a
        // longer interval that this temporary (for us to replace.)
        for (j = 0; j < AVAILABLE_LIVE_REGISTERS; ++j) {
            if ((registers_used != AVAILABLE_LIVE_REGISTERS) ? (registers[j] == NULL)
                   : (temporary_interval_length(registers[j]) > length))
            {
                break;
            }
        }

        if (j == AVAILABLE_LIVE_REGISTERS) {
            //printf("  spilling current %s\n", temporary->name->bytes);
            // No register could be found. Spill this temporary.
            if (temporary->variable == NULL) {
                temporary->variable = variable_new(4, 4);
            }
            temporary->reg = -1;
            #ifdef LOG_REGISTER_ALLOCATOR
            printf("Spilled temporary %s to variable @%zu\n",
                    temporary->name->bytes, temporary->variable->id);
            #endif
            continue;
        }

        // A register was found. If it already contains a temporary, spill it.
        if (registers[j]) {
            temporary_t* spill = registers[j];
            //printf("  spilling reg %zu %s\n", FIRST_LIVE_REGISTER + j, spill->name->bytes);
            spill->reg = -1;
            if (spill->variable == NULL) {
                spill->variable = variable_new(4, 4);
            }
            --registers_used;
            #ifdef LOG_REGISTER_ALLOCATOR
            printf("Spilled temporary %s to variable @%zu\n",
                    spill->name->bytes, spill->variable->id);
            #endif
        }

        // Assign this temporary to the register.
        //printf("  assigning reg %zu to %s\n", FIRST_LIVE_REGISTER + j, temporary->name->bytes);
        registers[j] = temporary;
        temporary->reg = FIRST_LIVE_REGISTER + j;
        ++registers_used;
    }

    #ifdef LOG_REGISTER_ALLOCATOR
    // Print any open register assignments that become permanent
    for (size_t j = 0; j < AVAILABLE_LIVE_REGISTERS; ++j) {
        if (registers[j] != NULL) {
            temporary_t* reg = registers[j];
            printf("Assigned temporary %s register r%zu (end)\n",
                    reg->name->bytes, FIRST_LIVE_REGISTER + j);
        }
    }
    #endif

    free(registers);
    vector_delete(temporaries);
}

void analyze_register_allocation(symbol_t* symbol) {

    // Assign a unique index to all instructions.
    analyze_number_instructions(vector_first(symbol->blocks), 1, pass_id++);

    // Convert the liveness data to live intervals.
    analyze_live_intervals(vector_first(symbol->blocks), pass_id++);

    // Perform linear scan
    analyze_linear_scan(symbol);

}

static void analyze_variable_assign(variable_t* variable, size_t* frame_size) {

    // Found a variable argument. If it hasn't been assigned stack
    // space yet, assign it now.
    if (variable->offset == 0) {
        size_t alignment_mask = variable->alignment - 1;
        *frame_size = (*frame_size + alignment_mask) & ~alignment_mask;
        *frame_size += variable->size;
        variable->offset = -(int)*frame_size;
        #ifdef LOG_REGISTER_ALLOCATOR
        printf("Assigned variable @%zu stack address %i, new frame size %zu\n",
                variable->id, variable->offset, *frame_size);
        #endif
    }
}

static void analyze_variable_offsets_block(block_t* block, int visited, size_t* frame_size) {

    // Only visit each block once
    if (block->visited == visited) {
        return;
    }
    block->visited = visited;

    // For each instruction, look for variable inputs
    for (size_t i = vector_count(block->instructions); i-- != 0;) {
        instruction_t* instruction = vector_at(block->instructions, i);

        argument_mode_t mode = instruction_mode(instruction);
        size_t j = (mode == argument_mode_write) ? 1 : 0;
        size_t count = vector_count(instruction->arguments);
        for (; j < count; ++j) {
            argument_t* argument = instruction_argument(instruction, j);

            if (argument->type == argument_type_variable) {
                analyze_variable_assign(argument_variable(argument), frame_size);
            } else if (argument->type == argument_type_temporary) {
                variable_t* variable = argument_temporary(argument)->variable;
                if (variable) {
                    analyze_variable_assign(variable, frame_size);
                }
            }
        }
    }

    // Continue to any blocks reachable from this one
    instruction_t* last = vector_last(block->instructions);
    if (last->opcode == opcode_jmp) {
        string_t* label = argument_label(instruction_argument(last, 0));
        analyze_variable_offsets_block(block_find(label), visited, frame_size);
    } else if (last->opcode == opcode_br) {
        string_t* true_label = argument_label(instruction_argument(last, 1));
        analyze_variable_offsets_block(block_find(true_label), visited, frame_size);
        string_t* false_label = argument_label(instruction_argument(last, 2));
        analyze_variable_offsets_block(block_find(false_label), visited, frame_size);
    }
}

void analyze_stack_frame(symbol_t* symbol) {
    // Walk through each instruction to find used variables. We want to avoid
    // assigning stack space to variables that have been eliminated.
    analyze_variable_offsets_block(vector_first(symbol->blocks), pass_id++,
            &symbol->frame_size);

    // Align the stack
    symbol->frame_size = (symbol->frame_size + 3u) & ~(size_t)3u;
}
