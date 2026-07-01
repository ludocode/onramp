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

#include "generate_stmt.h"

#include <stdlib.h>

#include "node.h"
#include "token.h"
#include "block.h"
#include "generate.h"
#include "function.h"

#ifndef CCI2_IR
void generate_return(node_t* node, int reg_out) {

    // reg_out is ignored. We aren't returning a value to the parent
    // expression; we are exiting the function entirely.
    (void)reg_out;

    int return_reg = R0;

    assert(node->kind == NODE_RETURN);
    if (node->first_child) {
        if (type_is_passed_indirectly(current_function->root->type)) {
            // The pointer to storage for the return value was pushed just
            // above the return address. We load it so we can generate the
            // return value directly into it.
            block_append(current_block, node->token, LDW, R0, RFP, 8);
        }
        generate_node(node->first_child, return_reg);
    } else {
        // No return value. If the function is main, we have to implicitly
        // return zero.
        // TODO this probably isn't true for return statements without
        // arguments. We only need to return 0 implicitly if control falls off
        // the end of main(), which we do in generate_function().
        if (string_equal_cstr(current_function->asm_name, "main")) {
            block_append(current_block, node->token, ZERO, R0);
        }
    }

    // generate defer statements
    generate_exit_defers(node, current_function->root);

    block_append(current_block, node->token, LEAVE);
    block_append(current_block, node->token, RET);
}
#endif // !CCI2_IR

#ifdef CCI2_IR
void generate_return(node_t* node, int temp_out) {

    // reg_out is ignored. We aren't returning a value to the parent
    // expression; we are exiting the function entirely.
    (void)temp_out;

    instruction_t* instruction;
    int retval = -1;

    assert(node->kind == NODE_RETURN);
    if (node->first_child) {
        retval = generate_temporary(NULL);
        if (type_is_passed_indirectly(current_function->root->type)) {
            #ifndef CCI2_IR
            // The pointer to storage for the return value was pushed just
            // above the return address. We load it so we can generate the
            // return value directly into it.
            block_append(current_block, node->token, LDW, R0, RFP, 8);
            #endif
            #ifdef CCI2_IR
            fatal("TODO IR indirect return");
            #endif
        }
        generate_node(node->first_child, retval);
    }

    // generate defer statements
    generate_exit_defers(node, current_function->root);

    // generate ret instruction
    instruction = block_append(current_block, node->token, RET, 1);
    if (node->first_child) {
        instruction_set_arg_temporary(instruction, 0, retval);  // ret %retval
    } else {
        // No return value. If the function is main, we have to implicitly
        // return zero.
        // TODO this probably isn't true for return statements without
        // arguments. We only need to return 0 implicitly if control falls off
        // the end of main(), which we do in generate_function().
        if (string_equal_cstr(current_function->asm_name, "main")) {
            instruction_set_arg_number(instruction, 0, 0); // ret 0
        } else {
            instruction_set_arg_sentinel(instruction, 0); // ret %
        }
    }

    // in case unreachable code follows the return, generate an orphan block
    // for it
    current_block = block_new(next_label++);
    function_add_block(current_function, current_block);

}
#endif // CCI2_IR

void generate_break(node_t* node, int reg_out) {
    generate_exit_defers(node, node->container);
    #ifndef CCI2_IR
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, node->container->break_label);
    #endif
    #ifdef CCI2_IR
    block_append_jmp(current_block, node->token, node->container->break_label);

    // generate a new block in case there are instructions after the break (we
    // can't add instructions after a jmp in IR)
    current_block = block_new(next_label++);
    function_add_block(current_function, current_block);
    #endif
}

void generate_continue(node_t* node, int reg_out) {
    generate_exit_defers(node, node->container);
    #ifndef CCI2_IR
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, node->container->continue_label);
    #endif
    #ifdef CCI2_IR
    block_append_jmp(current_block, node->token, node->container->continue_label);

    // generate a new block in case there are instructions after the continue
    // (we can't add instructions after a jmp in IR)
    current_block = block_new(next_label++);
    function_add_block(current_function, current_block);
    #endif
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

    #ifndef CCI2_IR
    bool indirect = type_is_passed_indirectly(node->type);
    int pred_register = indirect ? register_alloc(node->token) : reg_out;
    #endif
    #ifdef CCI2_IR
    int pred_register = generate_temporary(NULL);
    #endif
    generate_node(condition, pred_register);

    #ifndef CCI2_IR
    block_append(current_block, node->token, JNZ, pred_register, '&', JUMP_LABEL_PREFIX, true_block->label);
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, false_node ? false_block->label : end_block->label);
    #endif
    #ifdef CCI2_IR
    instruction_t* instruction = block_append_br(current_block, node->token,
            true_block->label,
            false_block ? false_block->label : end_block->label);
    instruction_set_arg_temporary(instruction, 0, pred_register);
    #endif

    #ifndef CCI2_IR
    if (indirect)
        register_free(node->token, pred_register);
    #endif

    current_block = true_block;
    generate_node(true_node, reg_out);
    #ifndef CCI2_IR
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, end_block->label);
    #endif
    #ifdef CCI2_IR
    block_append_jmp(current_block, node->token, end_block->label);
    #endif

    if (false_node) {
        current_block = false_block;
        generate_node(false_node, reg_out);
        #ifndef CCI2_IR
        block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, end_block->label);
        #endif
        #ifdef CCI2_IR
        block_append_jmp(current_block, node->token, end_block->label);
        #endif
    }

    current_block = end_block;
}

void generate_while(node_t* node, int reg_out) {
    node_t* condition = node->first_child;
    node_t* body = condition->right_sibling;

    node->continue_label = next_label++; // label of condition block
    node->break_label = next_label++; // label of end block
    int body_label = next_label++; // label of body block

    block_t* condition_block = block_new(node->continue_label);
    block_t* body_block = block_new(body_label);
    block_t* end_block = block_new(node->break_label);
    function_add_block(current_function, condition_block);
    function_add_block(current_function, body_block);
    function_add_block(current_function, end_block);

    // jump to condition
    #ifndef CCI2_IR
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, condition_block->label);
    #endif
    #ifdef CCI2_IR
    block_append_jmp(current_block, node->token, condition_block->label);
    #endif

    // generate condition
    // TODO this doesn't look like it handles 64-bit predicate correctly...
    current_block = condition_block;
    generate_node(condition, reg_out);

    // branch to body or end
    #ifndef CCI2_IR
    block_append(current_block, node->token, JZ, reg_out, '&', JUMP_LABEL_PREFIX, end_block->label);
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, body_block->label);
    #endif
    #ifdef CCI2_IR
    instruction_t* instruction = block_append_br(current_block, node->token,
            body_block->label, end_block->label);
    instruction_set_arg_temporary(instruction, 0, reg_out);
    #endif

    // generate body
    current_block = body_block;
    generate_node(body, reg_out);

    // jump back to condition
    #ifndef CCI2_IR
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, condition_block->label);
    #endif
    #ifdef CCI2_IR
    block_append_jmp(current_block, node->token, condition_block->label);
    #endif

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

    #ifndef CCI2_IR
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, body_block->label);
    #endif
    #ifdef CCI2_IR
    block_append_jmp(current_block, node->token, body_block->label);
    #endif

    current_block = body_block;
    generate_node(body, reg_out);
    generate_node(condition, reg_out);

    #ifndef CCI2_IR
    block_append(current_block, node->token, JZ, reg_out, '&', JUMP_LABEL_PREFIX, end_block->label);
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, body_block->label);
    #endif
    #ifdef CCI2_IR
    instruction_t* instruction = block_append_br(current_block, node->token,
            body_block->label, end_block->label);
    instruction_set_arg_temporary(instruction, 0, reg_out);
    #endif

    current_block = end_block;
}

void generate_for(node_t* node, int reg_out) {
    node_t* initialization = node->first_child;
    node_t* condition = initialization->right_sibling;
    node_t* increment = condition->right_sibling;
    node_t* body = increment->right_sibling;

    int condition_label = next_label++;
    node->continue_label = next_label++; // increment
    node->break_label = next_label++; // end
    int body_label = next_label++;

    block_t* increment_block = block_new(node->continue_label);
    block_t* condition_block = block_new(condition_label);
    block_t* body_block = block_new(body_label);
    block_t* end_block = block_new(node->break_label);
    function_add_block(current_function, increment_block);
    function_add_block(current_function, condition_block);
    function_add_block(current_function, body_block);
    function_add_block(current_function, end_block);

    // generate initialization (into the current block)
    generate_node(initialization, -1);
    #ifndef CCI2_IR
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, condition_label);
    #endif
    #ifdef CCI2_IR
    block_append_jmp(current_block, node->token, condition_label);
    #endif

    // generate increment
    current_block = increment_block;
    generate_node(increment, -1);
    #ifndef CCI2_IR
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, condition_label);
    #endif
    #ifdef CCI2_IR
    block_append_jmp(current_block, node->token, condition_label);
    #endif

    // generate condition
    current_block = condition_block;
    if (condition->kind == NODE_NOOP) {
        #ifndef CCI2_IR
        block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, body_label);
        #endif
        #ifdef CCI2_IR
        block_append_jmp(current_block, node->token, body_label);
        #endif
    } else {
        generate_node(condition, reg_out);
        #ifndef CCI2_IR
        block_append(current_block, node->token, JZ, reg_out, '&', JUMP_LABEL_PREFIX, end_block->label);
        block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, body_label);
        #endif
        #ifdef CCI2_IR
        instruction_t* instruction = block_append_br(current_block, node->token,
                body_block->label, end_block->label);
        instruction_set_arg_temporary(instruction, 0, reg_out);
        #endif
    }

    // generate body
    current_block = body_block;
    generate_node(body, -1);
    #ifndef CCI2_IR
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, increment_block->label);
    #endif
    #ifdef CCI2_IR
    block_append_jmp(current_block, node->token, increment_block->label);
    #endif

    current_block = end_block;
}

#ifndef CCI2_IR
// TODO this is really horrible. We need to make it explicit in the object code
// and assembly that labels are local to symbols, that way we don't need to
// do anything (except check for duplicates) to ensure that the names are unique.
static string_t* generate_label_name(node_t* node) {
    string_t* func_name = current_function->name->value;
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
#endif

void generate_label(node_t* node, int reg_out) {
    #ifndef CCI2_IR
    string_t* string = generate_label_name(node);
    block_append(current_block, node->token, JMP, '&', string->bytes, -1);
    current_block = block_new_user_label(string);
    function_add_block(current_function, current_block);
    string_deref(string);
    #endif
    #ifdef CCI2_IR
    block_append_jmp(current_block, node->token, node->jump_label);
    current_block = block_new(node->jump_label);
    current_block->user_label = string_ref(node->token->value);
    function_add_block(current_function, current_block);
    #endif
}

void generate_case_or_default(node_t* node, int reg_out) {
    generate_diagnose_defers(node, node->container, node->token);

    #ifndef CCI2_IR
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, node->jump_label);
    #endif
    #ifdef CCI2_IR
    block_append_jmp(current_block, node->token, node->jump_label);
    #endif
    current_block = block_new(node->jump_label);
    function_add_block(current_function, current_block);
}

void generate_goto(node_t* goto_node, int reg_out) {

    // find the destination label node
    node_t* label_node = function_find_label(current_function, goto_node->token->value);
    if (label_node == NULL) {
        fatal_token(goto_node->token, "Label in `goto` does not exist.");
    }

    // collect goto parents
    vector_t goto_parents;
    vector_init(&goto_parents);
    for (node_t* node = goto_node; node; node = node->parent) {
        vector_append(&goto_parents, node);
    }

    // collect label parents
    vector_t label_parents;
    vector_init(&label_parents);
    for (node_t* node = label_node; node; node = node->parent) {
        vector_append(&label_parents, node);
    }

    // find the direct children of the nearest common ancestor
    size_t goto_i = vector_count(&goto_parents);
    size_t label_i = vector_count(&label_parents);
    while (1) {
        --goto_i;
        --label_i;
        if (vector_at(&goto_parents, goto_i) != vector_at(&label_parents, label_i)) {
            break;
        }
    }
    node_t* goto_ancestor_child = vector_at(&goto_parents, goto_i);
    node_t* label_ancestor_child = vector_at(&label_parents, label_i);

    // make sure we're not jumping out of a defer
    for (size_t i = 0; i <= goto_i; ++i) {
        if (((node_t*)vector_at(&goto_parents, i))->kind == NODE_DEFER) {
            fatal_token(goto_node->token, "Cannot `goto` out of a `defer` statement.");
        }
    }

    // make sure we're not jumping into a defer, or into a node that follows a
    // defer
    generate_diagnose_defers(label_node, label_ancestor_child, goto_node->token);

    // generate defers out to the direct child of the common ancestor
    generate_exit_defers(goto_node, goto_ancestor_child);

    // figure out if we're jumping backwards or forwards in the common ancestor
    bool backwards = false;
    for (node_t* node = goto_ancestor_child->left_sibling; node; node = node->left_sibling) {
        if (node == label_ancestor_child) {
            backwards = true;
            break;
        }
    }

    // if we're jumping backwards in the common ancestor, generate defers
    if (backwards) {
        node_t* node = goto_ancestor_child->left_sibling;
        for (; node != label_ancestor_child; node = node->left_sibling) {
            assert(node != NULL);
            if (node->kind == NODE_DEFER) {
                generate_defer(node);
            }
        }
    }

    // if we're jumping forwards in the common ancestor, make sure we're not
    // crossing a defer node
    if (!backwards) {
        node_t* node = goto_ancestor_child->right_sibling;
        for (; node && node != label_ancestor_child; node = node->right_sibling) {
            assert(node != NULL);
            if (node->kind == NODE_DEFER) {
                fatal_token(label_node->token, "Cannot `goto` forward across a `defer` statement.");
            }
        }
    }

    // generate the jump
    #ifndef CCI2_IR
    if (goto_node->string == NULL) {
        goto_node->string = generate_label_name(goto_node);
    }
    block_append(current_block, goto_node->token, JMP, '&', goto_node->string->bytes, -1);
    #endif
    #ifdef CCI2_IR
    block_append_jmp(current_block, goto_node->token, label_node->jump_label);

    // generate a new block in case there are instructions after the goto (we
    // can't add instructions after a jmp in IR)
    current_block = block_new(next_label++);
    function_add_block(current_function, current_block);
    #endif

    vector_destroy(&label_parents);
    vector_destroy(&goto_parents);
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
static int case_cmp(const void* vleft, const void* vright) {
    const node_t* left = vleft;
    const node_t* right = vright;

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

/*
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
*/

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

    // Use libc qsort() if we can
    #ifndef CASES_SORT_FALLBACK
    qsort(cases, count, sizeof(*cases), case_cmp);
    #endif

    // If we can't, our fallback a little insertion sort.
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
    #endif
}

void generate_case_match(node_t* switch_, int reg_out, node_t* case_) {
    type_t* type = switch_->first_child->type;

    if (type_size(type) == 8) {
        fatal("TODO compare llong case");
    } else {
        if (case_->start32 == case_->end32) {
            #ifndef CCI2_IR
            int reg_value = register_alloc(case_->token);
            block_append(current_block, case_->token, IMW, ARGTYPE_NUMBER, reg_value, case_->start32);
            block_append(current_block, case_->token, SUB, reg_value, reg_value, reg_out);
            block_append(current_block, case_->token, JZ, reg_value, '&', JUMP_LABEL_PREFIX, case_->jump_label);
            register_free(case_->token, reg_value);
            #endif
            #ifdef CCI2_IR
            int result = generate_temporary(NULL);

            instruction_t* instruction = block_append(current_block, case_->token, SUB, 3);
            instruction_set_arg_temporary(instruction, 0, result);
            instruction_set_arg_temporary(instruction, 1, reg_out);
            instruction_set_arg_number(instruction, 2, case_->start32);

            instruction = block_append_br(current_block, case_->token,
                    case_->jump_label, next_label);
            instruction_set_arg_temporary(instruction, 0, result);

            current_block = block_new(next_label++);
            function_add_block(current_function, current_block);
            #endif
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
        */// (void)check_cases_overlap;

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
    #ifndef CCI2_IR
    block_append(current_block, NULL, JMP, '&', JUMP_LABEL_PREFIX,
            default_ ? default_->jump_label : switch_->break_label);
    #endif
    #ifdef CCI2_IR
    block_append_jmp(current_block, switch_->token,
            default_ ? default_->jump_label : switch_->break_label);
    #endif

    // Generate a new block in case there is any unreachable code in the switch
    // before the first label. (We can't put anything after the jmp in IR.)
    current_block = block_new(next_label++);
    function_add_block(current_function, current_block);

    // Generate the contents of the switch. Note that this still happens even
    // if it has no case or default labels because it could contain a named
    // label reachable with `goto`.
    generate_node(switch_->last_child, reg_out);

    // Finally, jump to the exit block and open it. We're done.
    #ifndef CCI2_IR
    block_append(current_block, NULL, JMP, '&', JUMP_LABEL_PREFIX, switch_->break_label);
    #endif
    #ifdef CCI2_IR
    block_append_jmp(current_block, switch_->token, switch_->break_label);
    #endif
    current_block = block_new(switch_->break_label);
    function_add_block(current_function, current_block);
}
