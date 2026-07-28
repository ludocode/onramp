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
#include "emit.h"
#include "libo-string.h"
#include "node.h"
#include "record.h"
#include "token.h"
#include "type.h"

typedef struct variable_t {
    int temporary;
    uint32_t size;
    uint32_t alignment;
    token_t* token;
} variable_t;

static variable_t* variable_new(int temporary, struct type_t* type,
        struct token_t* /*nullable*/ token)
{
    variable_t* variable = malloc(sizeof(variable_t));
    if (!variable) {
        fatal("Out of memory.");
    }
    variable->temporary = temporary;
    variable->size = type_size(type);
    variable->alignment = type_alignment(type);
    variable->token = token ? token_ref(token) : NULL;
    return variable;
}

static void variable_delete(variable_t* variable) {
    if (variable->token) {
        token_deref(variable->token);
    }
    free(variable);
}

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

    function->variadic_temporary = TEMPORARY_INVALID;
    function->return_temporary = TEMPORARY_INVALID;

    function->name_label = -1;
    vector_init(&function->records);
    function->strings = vector_new();
    function->variables = vector_new();
    return function;
}

void function_delete(function_t* function) {

    // free variables
    size_t variable_count = vector_count(function->variables);
    for (size_t i = 0; i != variable_count; ++i) {
        variable_delete(vector_at(function->variables, i));
    }
    vector_delete(function->variables);

    // free strings
    size_t string_count = vector_count(function->strings);
    for (size_t i = 0; i < string_count; ++i) {
        string_deref(vector_at(function->strings, i));
    }
    vector_delete(function->strings);

    // free records
    size_t record_count = vector_count(&function->records);
    for (size_t i = 0; i < record_count; ++i) {
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

void function_take_string(function_t* function, string_t* string) {
    vector_append(function->strings, string);
}

void function_add_variable(function_t* function, int temporary,
        struct type_t* type, struct token_t* /*nullable*/ token)
{
    variable_t* variable = variable_new(temporary, type, token);
    vector_append(function->variables, variable);
}

void function_emit_variables(function_t* function) {
    size_t var_count = vector_count(function->variables);
    for (size_t i = 0; i < var_count; ++i) {
        variable_t* variable = vector_at(function->variables, i);
        if (variable->token) {
            emit_source_location(variable->token);
        }
        emit_cstr("  var ");
        emit_string(temporary_name(variable->temporary));
        emit_char(' ');
        emit_number(variable->size);
        emit_char(' ');
        emit_number(variable->alignment);
        emit_char('\n');
    }
}
