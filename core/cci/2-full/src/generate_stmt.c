/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Fraser Heavy Software
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

#include "generate_stmt.h"

#include <stdlib.h>

#include "node.h"
#include "token.h"
#include "block.h"
#include "generate.h"
#include "function.h"

void generate_return(node_t* node, int reg_out) {

    // reg_out is ignored. We aren't returning a value to the parent
    // expression; we are exiting the function entirely.
    (void)reg_out;

    assert(node->kind == NODE_RETURN);
    if (node->first_child) {
        if (type_is_passed_indirectly(current_function->root->type)) {
            // The pointer to storage for the return value was pushed just
            // above the return address. We load it so we can generate the
            // return value directly into it.
            block_append(current_block, node->token, LDW, R0, RFP, 8);
        }
        generate_node(node->first_child, R0);
    } else {
        // No return value. If the function is main, we have to implicitly
        // return zero.
        if (string_equal_cstr(current_function->asm_name, "main")) {
            block_append(current_block, node->token, ZERO, R0);
        }
    }
    block_append(current_block, node->token, LEAVE);
    block_append(current_block, node->token, RET);
}

void generate_break(node_t* node, int reg_out) {
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, node->container->break_label);
}

void generate_continue(node_t* node, int reg_out) {
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, node->container->continue_label);
}

void generate_if(node_t* node, int reg_out) {
    node_t* condition = node->first_child;
    node_t* true_node = condition->right_sibling;
    node_t* false_node = true_node->right_sibling;

    block_t* true_block = block_new(next_label++);
    block_t* false_block = false_node ? block_new(next_label++) : 0;
    block_t* end_block = block_new(next_label++);

    function_add_block(current_function, true_block);
    if (false_block)
        function_add_block(current_function, false_block);
    function_add_block(current_function, end_block);

    bool indirect = type_is_passed_indirectly(node->type);
    int pred_register = indirect ? register_alloc(node->token) : reg_out;
    generate_node(condition, pred_register);

    block_append(current_block, node->token, JNZ, pred_register, '&', JUMP_LABEL_PREFIX, true_block->label);
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, false_node ? false_block->label : end_block->label);

    if (indirect)
        register_free(node->token, pred_register);

    current_block = true_block;
    generate_node(true_node, reg_out);
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, end_block->label);

    if (false_node) {
        current_block = false_block;
        generate_node(false_node, reg_out);
        block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, end_block->label);
    }

    current_block = end_block;
}

void generate_while(node_t* node, int reg_out) {
    node_t* condition = node->first_child;
    node_t* body = condition->right_sibling;

    node->continue_label = next_label++;
    node->break_label = next_label++;

    block_t* body_block = block_new(node->continue_label);
    block_t* end_block = block_new(node->break_label);
    function_add_block(current_function, body_block);
    function_add_block(current_function, end_block);

    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, body_block->label);

    current_block = body_block;
    generate_node(condition, reg_out);
    block_append(current_block, node->token, JZ, reg_out, '&', JUMP_LABEL_PREFIX, end_block->label);
    generate_node(body, reg_out);
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, body_block->label);

    current_block = end_block;
}

void generate_do(node_t* node, int reg_out) {
    node_t* body = node->first_child;
    node_t* condition = body->right_sibling;

    node->continue_label = next_label++;
    node->break_label = next_label++;

    block_t* body_block = block_new(node->continue_label);
    block_t* end_block = block_new(node->break_label);
    function_add_block(current_function, body_block);
    function_add_block(current_function, end_block);

    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, body_block->label);

    current_block = body_block;
    generate_node(body, reg_out);
    generate_node(condition, reg_out);
    block_append(current_block, node->token, JZ, reg_out, '&', JUMP_LABEL_PREFIX, end_block->label);
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, body_block->label);

    current_block = end_block;
}

void generate_for(node_t* node, int reg_out) {
    node_t* initialization = node->first_child;
    node_t* condition = initialization->right_sibling;
    node_t* increment = condition->right_sibling;
    node_t* body = increment->right_sibling;

    int body_label = next_label++;
    node->continue_label = next_label++;
    node->break_label = next_label++;

    block_t* increment_block = block_new(node->continue_label);
    block_t* body_block = block_new(body_label);
    block_t* end_block = block_new(node->break_label);
    function_add_block(current_function, increment_block);
    function_add_block(current_function, body_block);
    function_add_block(current_function, end_block);

    generate_node(initialization, reg_out);
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, body_label);

    current_block = increment_block;
    generate_node(increment, reg_out);
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, body_label);

    current_block = body_block;
    if (condition->kind != NODE_NOOP) {
        generate_node(condition, reg_out);
        block_append(current_block, node->token, JZ, reg_out, '&', JUMP_LABEL_PREFIX, end_block->label);
    }
    generate_node(body, reg_out);
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, increment_block->label);

    current_block = end_block;
}

// TODO this is really horrible. We need to make it explicit in the object code
// and assembly that labels are local to symbols, that way we don't need to
// do anything (except check for duplicates) to ensure that the names are unique.
static string_t* generate_label_name(node_t* node) {
    string_t* func_name = current_function->name;
    string_t* label_name = node->token->value;

    char prefix[32];
    snprintf(prefix, sizeof(prefix), "%u", (unsigned)string_length(func_name));

    size_t len = strlen(USER_LABEL_PREFIX) + strlen(prefix) + 1 +
            string_length(func_name) + 1 + string_length(label_name);
    char* cstr = malloc(len + 1);

    strcpy(cstr, USER_LABEL_PREFIX);
    strcat(cstr, prefix);
    strcat(cstr, "_");
    strcat(cstr, func_name->bytes);
    strcat(cstr, "_");
    strcat(cstr, label_name->bytes);

    string_t* string = string_intern_bytes(cstr, len);
    free(cstr);
    return string;
}

void generate_label(node_t* node, int reg_out) {
    string_t* string = generate_label_name(node);
    current_block = block_new_user_label(string);
    function_add_block(current_function, current_block);
    string_deref(string);
}

void generate_goto(node_t* node, int reg_out) {
    // TODO make sure the label actually exists; see parse_label(), need a
    // table of labels in function

    if (node->string == NULL) {
        node->string = generate_label_name(node);
    }

    block_append(current_block, node->token, JMP, '&', node->string->bytes, -1);
}
