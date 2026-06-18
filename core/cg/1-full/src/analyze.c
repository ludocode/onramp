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

#include "block.h"
#include "common.h"
#include "instruction.h"
#include "libo-error.h"
#include "libo-otable.h"
#include "libo-vector.h"
#include "symbol.h"
#include "temporary.h"

static void print_live_temps(otable_t* temps) {
    if (temps->count == 0) {
        fputs("(none)", stdout);
        return;
    }
    size_t j = 0;
    for (void** p = otable_begin(temps); p; p = otable_next(temps, p)) {
        temporary_t* temporary = *p;
        if (temporary) {
            if (j++ != 0) {
                putchar(' ');
            }
            fputs(temporary->name->bytes, stdout);
        }
    }
}

/**
 * Fills out the back pointers from each block to its parent blocks (i.e. the
 * parent_blocks list of each block), and collects all blocks that end in
 * `ret`, marking them visited.
 */
static void analyze_block_parents(symbol_t* symbol, vector_t* ret_blocks, int visited) {
    size_t count = vector_count(symbol->blocks);
    for (size_t i = 0; i < count; ++i) {
        block_t* block = vector_at(symbol->blocks, i);
        instruction_t* last = vector_last(block->instructions);

        switch (last->opcode) {
            block_t* child;

            case opcode_ret:
                //printf("found ret block %s\n", block->name->bytes);
                vector_append(ret_blocks, block);
                block->visited = visited;
                break;

            case opcode_br:
                //printf("looking up arg1 block %s\n", instruction_argument(last, 1)->string->bytes);
                child = block_find(instruction_argument(last, 1)->string);
                vector_append(child->parent_blocks, block);

                //printf("looking up arg2 block %s\n", instruction_argument(last, 2)->string->bytes);
                child = block_find(instruction_argument(last, 2)->string);
                vector_append(child->parent_blocks, block);
                break;

            case opcode_jmp:
                //printf("looking up arg0 block %s\n", instruction_argument(last, 0)->string->bytes);
                child = block_find(instruction_argument(last, 0)->string);
                vector_append(child->parent_blocks, block);
                break;

            default:
                // this should have been checked during parsing
                fatal("Internal error: invalid end of block");
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

    // Generate back pointer links, and also put all `ret` blocks in blocks,
    // marking them visited.
    analyze_block_parents(symbol, blocks, visited);

    // Now we step through the blocks array, appending any parent nodes that
    // have not been visited yet. Note that the vector count here grows as we
    // walk through it.
    for (size_t i = 0; i < vector_count(blocks); ++i) {
        block_t* block = vector_at(blocks, i);
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
            //printf("  found write arg %s\n", argument->temporary->name->bytes);
            otable_remove(live_temps, argument->temporary,
                    string_hash(argument->temporary->name));
        }
    }

    // Look for temporary inputs
    size_t first_read_arg = (mode == argument_mode_write) ? 1 : 0;
    size_t count = vector_count(instruction->arguments);
    for (size_t j = first_read_arg; j < count; ++j) {
        argument_t* argument = instruction_argument(instruction, j);
        if (argument->type == argument_type_temporary) {
            //printf("  found read arg at %zu: %s\n", j, argument->temporary->name->bytes);
            // The instruction is reading this temporary. It becomes live.
            otable_put(live_temps, argument->temporary,
                    string_hash(argument->temporary->name));
        }
    }
}

void analyze_liveness(symbol_t* symbol) {

    // Generate a good block order
    vector_t* blocks = analyze_liveness_block_order(symbol);
    size_t block_count = vector_count(blocks);

    //for (size_t i = 0; i < block_count; ++i) {
    //    printf("  block order: %s\n", ((block_t*)vector_at(blocks,i))->name->bytes);
    //}

    // We iterate until nothing changes. Each iteration can only grow the set
    // of live temporaries, and since there are a finite number of temporaries,
    // this is guaranteed to eventually terminate.
    for (;;) {
        bool changed = false;
        //printf("\n=========================\nstarting liveness iteration\n");

        // We're walking through the blocks in the order given by
        // analyze_liveness_block_order(), which is backwards breadth-first
        // from `ret` blocks.
        for (size_t i = 0; i < block_count; ++i) {
            block_t* block = vector_at(blocks, i);
            otable_t* live_temps = otable_new_copy(block->live_temps);

            //printf("block %s starting live_temps: ", block->name->bytes);
            //print_live_temps(live_temps);
            //putchar('\n');

            // Walk backwards through the instructions, updating this block's
            // live_temps as we go.
            for (size_t j = vector_count(block->instructions); j-- != 0;) {
                instruction_t* instruction = vector_at(block->instructions, j);
                analyze_liveness_instruction(live_temps, instruction);
            }

            //printf("block %s ending live_temps: ", block->name->bytes);
            //print_live_temps(live_temps);
            //putchar('\n');

            // The result is the set of live temps at the start of the block.
            // Merge it into the live temps at the end of every parent.
            for (size_t j = vector_count(block->parent_blocks); j-- != 0;) {
                block_t* parent = vector_at(block->parent_blocks, j);
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
            //printf("no changes. done\n\n");
            break;
        }
        //printf("changes. looping\n");
    }

    vector_delete(blocks);
}
