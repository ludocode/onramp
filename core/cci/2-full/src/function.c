/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2026 Fraser Heavy Software
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

#include "function.h"

#include <stdlib.h>

#include "block.h"
#include "node.h"
#include "record.h"
#include "token.h"
#include "type.h"

typedef struct label_node_t {
    table_entry_t entry;
    string_t* string;
    node_t* node;
} label_node_t;

label_node_t* label_node_new(string_t* string, node_t* node) {
    label_node_t* ln = malloc(sizeof(label_node_t));
    ln->string = string_ref(string);
    // TODO we need to reference-count nodes to make this safe
    ln->node = node;
    return ln;
}

void label_node_delete(label_node_t* ln) {
    string_deref(ln->string);
    free(ln);
}

function_t* function_new(type_t* type, token_t* name,
        string_t* asm_name, node_t* root)
{
    function_t* function = malloc(sizeof(function_t));
    function->type = type_ref(type);
    function->name = token_ref(name);
    function->asm_name = string_ref(asm_name);
    function->root = root;
    vector_init(&function->blocks);
    table_init(&function->labels);

    #ifndef CCI2_IR
    function->variadic_offset = -1;
    #endif
    #ifdef CCI2_IR
    function->variadic_temporary = -1;
    #endif

    function->name_label = -1;
    vector_init(&function->records);
    return function;
}

void function_delete(function_t* function) {

    // free records
    for (size_t i = 0; i < vector_count(&function->records); ++i) {
        record_deref(vector_at(&function->records, i));
    }
    vector_destroy(&function->records);

    // free labels
    for (table_entry_t** bucket = table_first_bucket(&function->labels);
            bucket; bucket = table_next_bucket(&function->labels, bucket))
    {
        for (table_entry_t* entry = *bucket; entry;) {
            table_entry_t* next = table_entry_next(entry);
            label_node_delete((label_node_t*)entry);
            entry = next;
        }
    }
    table_destroy(&function->labels);

    // free blocks
    size_t count = vector_count(&function->blocks);
    for (size_t i = 0; i < count; ++i) {
        block_delete(vector_at(&function->blocks, i));
    }
    vector_destroy(&function->blocks);

    node_delete(function->root);
    string_deref(function->asm_name);
    token_deref(function->name);
    type_deref(function->type);
    free(function);
}

void function_add_block(function_t* function, block_t* block) {
    vector_append(&function->blocks, block);
}

void function_add_label(function_t* function, node_t* label) {
    assert(label->kind == NODE_LABEL);

    // make sure the label doesn't already exist
    string_t* string = label->token->value;
    if (NULL != function_find_label(function, string)) {
        fatal_token(label->token, "Duplicate label.");
    }

    label_node_t* ln = label_node_new(string, label);
    uint32_t hash = string_hash(string);

    table_put(&function->labels, &ln->entry, hash);
}

node_t* function_find_label(function_t* function, string_t* name) {
    table_entry_t* entry = table_bucket(&function->labels, string_hash(name));
    for (; entry; entry = table_entry_next(entry)) {
        label_node_t* ln = (label_node_t*)entry;
        if (string_equal(name, ln->string)) {
            return ln->node;
        }
    }
    return NULL;
}

void function_add_record(function_t* function, record_t* record) {
    vector_append(&function->records, record_ref(record));
}
