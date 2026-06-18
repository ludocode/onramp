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

#include "block.h"

#include <stdlib.h>
#include <string.h>

#include "instruction.h"
#include "libo-error.h"
#include "libo-otable.h"
#include "libo-vector.h"

static table_t* block_table;

block_t* block_new(const char* name, location_t* location) {
    block_t* block = calloc(1, sizeof(block_t));
    block->name = string_intern_cstr(name);
    block->location = location;
    block->instructions = vector_new();
    block->visited = -1;
    block->parent_blocks = vector_new();
    block->live_temps = otable_new();
    table_put(block_table, &block->entry, string_hash(block->name));
    return block;
}

void block_delete(block_t* block) {
    for (size_t i = vector_count(block->instructions); i-- > 0;) {
        instruction_delete(vector_at(block->instructions, i));
    }
    otable_delete(block->live_temps);
    vector_delete(block->parent_blocks);
    vector_delete(block->instructions);
    location_delete(block->location);
    string_deref(block->name);
    free(block);
}

void blocks_setup(void) {
    block_table = table_new();
}

void blocks_teardown(void) {
    blocks_clear();
    table_delete(block_table);
}

void blocks_clear(void) {
    for (table_entry_t** bucket = table_first_bucket(block_table); bucket;
            bucket = table_next_bucket(block_table, bucket))
    {
        for (table_entry_t* entry = *bucket; entry;) {
            table_entry_t* next = table_entry_next(entry);
            block_delete((block_t*)entry);
            entry = next;
        }
    }
    table_remove_all(block_table);
}

block_t* block_find(const string_t* name) {
    for (table_entry_t* entry = table_bucket(block_table, string_hash(name));
            entry; entry = table_entry_next(entry))
    {
        block_t* block = (block_t*)entry;
        if (string_equal(block->name, name)) {
            return block;
        }
    }
    fatal("Block not found");
}
