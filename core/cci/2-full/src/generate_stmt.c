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
    block_append(current_block, node->token, JMP, '&', string->bytes, -1);
    current_block = block_new_user_label(string);
    function_add_block(current_function, current_block);
    string_deref(string);
}

void generate_case_or_default(node_t* node, int reg_out) {
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, node->jump_label);
    current_block = block_new(node->jump_label);
    function_add_block(current_function, current_block);
}

void generate_goto(node_t* node, int reg_out) {
    // TODO make sure the label actually exists; see parse_label(), need a
    // table of labels in function

    if (node->string == NULL) {
        node->string = generate_label_name(node);
    }

    block_append(current_block, node->token, JMP, '&', node->string->bytes, -1);
}

// The type to use for case_cmp() comparisons. It would be better to use
// qsort_r() but we're trying to stick to standard C.
static type_t* case_cmp_type;

/**
 * Compares the given cases for ordering purposes.
 *
 * We must be careful to properly handle the width and signedness of the
 * switch expression.
 *
 * It would be nice to make separate functions and just pass the proper one to
 * qsort(). Unfortunately we don't have function pointers in opC so we write
 * our comparison function this way instead.
 */
static int case_cmp(void* vleft, void* vright) {
    node_t* left = vleft;
    node_t* right = vright;

    if (type_matches_base(case_cmp_type, BASE_SIGNED_LONG_LONG)) {
        if (llong_lts(&left->start64, &right->start64))
            return -1;
        if (llong_lts(&right->start64, &left->start64))
            return 1;

    } else if (type_matches_base(case_cmp_type, BASE_UNSIGNED_LONG_LONG)) {
        if (llong_ltu(&left->start64, &right->start64))
            return -1;
        if (llong_ltu(&right->start64, &left->start64))
            return 1;

    } else if (type_is_signed_integer(case_cmp_type)) {
        if ((int)left->start32 < (int)right->start32)
            return -1;
        if ((int)left->start32 > (int)right->start32)
            return 1;

    } else {
        if ((unsigned)left->start32 < (unsigned)right->start32)
            return -1;
        if ((unsigned)left->start32 > (unsigned)right->start32)
            return 1;
    }

    return 0;
}

static void check_cases_overlap(node_t* switch_, node_t* left, node_t* right) {
    type_t* type = switch_->first_child->type;

    if (type_matches_base(type, BASE_SIGNED_LONG_LONG)) {
        if (llong_lts(&left->end64, &right->start64))
            return;

    } else if (type_matches_base(type, BASE_UNSIGNED_LONG_LONG)) {
        if (llong_ltu(&left->end64, &right->start64))
            return;

    } else if (type_is_signed_integer(type)) {
        if ((int)left->end32 >= (int)right->start32)
            return;

    } else {
        if ((unsigned)left->end32 >= (unsigned)right->start32)
            return;
    }

    // Note that we're not necessarily reporting the second instance;
    // this might be the first because we've re-ordered them. This is
    // not straightforward to fix and not worth fixing at the moment.
    fatal_token(right->token, "Duplicate (or overlapping range of) `case` label in switch.");
}

/**
 * Sort the list of cases.
 */
void cases_sort(node_t* switch_, node_t** cases, size_t count) {
    if (count <= 1)
        return;
    case_cmp_type = switch_->first_child->type;

    // We prefer qsort() because there may be hundreds of cases. Unfortunately,
    // it's not available during bootstrapping so we need a fallback.

    // opC doesn't have function pointers so we can't use qsort().
    #ifdef __onramp_cci_opc__
        #define CASES_SORT_FALLBACK
    #endif

    // If we're making a debug build, we want to test our fallback.
    #ifdef DEBUG
        #define CASES_SORT_FALLBACK
    #endif

    // TODO for now always insertion sort, we'll get qsort() working later
    #define CASES_SORT_FALLBACK

    // If we can't qsort, we use a little insertion sort.
    #ifdef CASES_SORT_FALLBACK
    for (size_t i = 1; i < count; ++i) {
        node_t* temp = cases[i];
        size_t j = i;
        while (j > 0 && 0 > case_cmp(cases + i, cases + j)) {
            cases[j] = cases[j + 1];
            --j;
        }
        cases[j] = temp;
    }
    #else
    qsort(cases, count, sizeof(*cases), case_cmp);
    #endif
}

void generate_case_match(node_t* switch_, int reg_out, node_t* case_) {
    type_t* type = switch_->first_child->type;

    if (type_size(type) == 8) {
        fatal("TODO compare llong case");
    } else {
        if (case_->start32 == case_->end32) {
            int reg_value = register_alloc(case_->token);
            block_append(current_block, case_->token, IMW, ARGTYPE_NUMBER, reg_value, case_->start32);
            block_append(current_block, case_->token, SUB, reg_value, reg_value, reg_out);
            block_append(current_block, case_->token, JZ, reg_value, '&', JUMP_LABEL_PREFIX, case_->jump_label);
            register_free(case_->token, reg_value);
        } else {
            fatal("TODO compare case range");
        }
    }
}

void generate_switch_linear_search(node_t* switch_, int reg_out, node_t** cases, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        generate_case_match(switch_, reg_out, cases[i]);
    }
}

/*
void generate_switch_binary_search(node_t* switch_, int reg_out, node_t** cases, size_t start, size_t end) {
    if (start == end) {
        generate_case_match(switch_, cases[start]);
        return;
    }

    int i = start + end / 2;
    generate_case_greater(switch_, cases[start]);
}
*/

void generate_switch(node_t* switch_, int reg_out) {
    assert(switch_->kind == NODE_SWITCH);

    // generate the expression into reg_out
    if (type_size(switch_->first_child->type) > 4) {
        // switch has void type so we aren't given space to store our llong, we
        // need to allocate it ourselves
        fatal("TODO switch llong");
    } else {
        generate_node(switch_->first_child, reg_out);
    }

    // count our case/default labels
    size_t count = 0;
    for (node_t* label = switch_->next_case; label; label = label->next_case) {
        if (label->kind == NODE_CASE) {
            ++count;
        }
    }

    // allocate an array to store them
    node_t** cases = malloc(count * sizeof(node_t));
    node_t* default_ = NULL;

    // collect the case/default labels
    size_t i = 0;
    for (node_t* label = switch_->next_case; label; label = label->next_case) {
        if (label->kind == NODE_DEFAULT) {
            if (default_ != NULL) {
                fatal_token(label->token, "Duplicate `default` label in switch.");
            }
            default_ = label;
        } else {
            assert(label->kind == NODE_CASE);
            cases[i++] = label;
        }
        label->jump_label = next_label++;
    }

    if (count > 0) {

        // sort them
        /* TODO debug this later
        cases_sort(switch_, cases, count);

        // check for overlaps
        for (size_t i = 0; i < count - 1; ++i) {
            check_cases_overlap(switch_, cases[i], cases[i + 1]);
        }
        */ (void)check_cases_overlap;

        // emit code to jump to the appropriate case

        // TODO if the cases are highly compressed we should make a jump table. Our
        // fallback is a binary search so for now we just use that all the time.
        //generate_switch_binary_search(switch_, reg_out, cases, 0, count - 1);

        // TODO for now not even doing the binary search, we'll just linear search
        generate_switch_linear_search(switch_, reg_out, cases, count);
    }

    free(cases);

    // if we still haven't found the case and we have a `default` label, jump
    // to it; otherwise jump to the end.
    switch_->break_label = next_label++;
    block_append(current_block, NULL, JMP, '&', JUMP_LABEL_PREFIX,
            default_ ? default_->jump_label : switch_->break_label);

    // Note that we don't create a new block here. If there is any code in the
    // switch before the first label, it is unreachable. Since it's after the
    // above JMP, it will be discarded by optimizations.

    // Generate the contents of the switch. Note that this still happens even
    // if it has no case or default labels because it could contain a named
    // label reachable with `goto`.
    generate_node(switch_->last_child, reg_out);

    // Finally, jump to the exit block and open it. We're done.
    block_append(current_block, NULL, JMP, '&', JUMP_LABEL_PREFIX, switch_->break_label);
    current_block = block_new(switch_->break_label);
    function_add_block(current_function, current_block);
}
