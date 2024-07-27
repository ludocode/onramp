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

#include "node.h"
#include "block.h"
#include "generate.h"
#include "function.h"

void generate_return(node_t* node) {
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

void generate_break(node_t* node) {
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, node->container->end_label);
}

void generate_continue(node_t* node) {
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, node->container->body_label);
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

void generate_while(node_t* node) {
    node_t* condition = node->first_child;
    node_t* body = condition->right_sibling;

    node->body_label = next_label++;
    node->end_label = next_label++;

    block_t* body_block = block_new(node->body_label);
    block_t* end_block = block_new(node->end_label);
    function_add_block(current_function, body_block);
    function_add_block(current_function, end_block);

    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, body_block->label);

    current_block = body_block;
    generate_node(condition, R0);
    block_append(current_block, node->token, JZ, R0, '&', JUMP_LABEL_PREFIX, end_block->label);
    generate_node(body, R0);
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, body_block->label);

    current_block = end_block;
}
