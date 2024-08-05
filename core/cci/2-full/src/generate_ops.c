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

#include "generate_ops.h"

#include "common.h"
#include "function.h"
#include "generate.h"
#include "node.h"
#include "block.h"
#include "type.h"
#include "token.h"

/**
 * Generates an arithmetic or other binary calculation that must be done with a
 * libc function. This is used for long long, float and double.
 *
 * The reg_out register must already contain (or point to) the value for the
 * left node. We only need to generate the right node.
 */
static void generate_binary_function(node_t* node, int reg_out, const char* function_name) {
    size_t size = type_size(node->type);
    assert(size >= 4); // should already be promoted

    // push all registers (except for the return register)
    int last_pushed_register = register_loop_count ? R9 : reg_out;
    for (int i = R0; i != last_pushed_register; ++i) {
        if (i != reg_out) {
            block_append(current_block, node->token, PUSH, i);
        }
    }

    // the left value goes in r0
    block_append(current_block, node->token, MOV, R0, reg_out);

    // for 64-bit math, arguments must point to storage space. make room for a
    // temporary
    if (size != 4) {
        assert(size == 8);
        block_append(current_block, node->token, SUB, RSP, RSP, size);
        block_append(current_block, node->token, MOV, R1, RSP);
    }

    // generate right node into r1
    generate_node(node->last_child, R1);

    // generate function call
    block_append(current_block, node->token, CALL, '^', function_name);

    // move return value into output register
    block_append(current_block, node->token, MOV, reg_out, R0);

    // pop stack space used for temporary
    if (size != 4) {
        block_append(current_block, node->token, ADD, RSP, RSP, 8);
    }

    // pop registers
    for (int i = last_pushed_register; i-- != R0;) {
        if (i != reg_out) {
            block_append(current_block, node->token, POP, i);
        }
    }
}

/**
 * Generates a simple arithmetic calculation.
 */
static void generate_simple_arithmetic(node_t* node, int reg_left,
        opcode_t opcode, const char* llong_func,
        const char* float_func, const char* double_func)
{
    type_t* type = node->type;

    const char* function =
            type_is_long_long(type)              ? "__llong_add" :
            type_matches_base(type, BASE_FLOAT)  ? "__float_add" :
            type_matches_base(type, BASE_DOUBLE) ? "__double_add" :
            NULL;

    generate_node(node->first_child, reg_left);
    if (function) {
        generate_binary_function(node, reg_left, function);
    } else {
        int reg_right = register_alloc(node->token);
        generate_node(node->last_child, reg_right);
        block_append(current_block, node->token, opcode, reg_left, reg_left, reg_right);
        register_free(node->token, reg_right);
    }
}

/**
 * Same as generate_pointer_add_sub() except the left value has already been
 * generated.
 *
 * This is called directly by the compound assignment operators.
 */
static void generate_pointer_add_sub_impl(node_t* node, opcode_t op, int reg_left) {
    int reg_right = register_alloc(node->token);
    generate_node(node->last_child, reg_right);

    // One side is a pointer and the other side is an int offset. The offset
    // needs to be shifted or multiplied by the pointer size.

    // Figure out the size of the pointed-to type
    bool is_left_ptr = type_is_indirection(node->first_child->type);
    type_t* ptr_type = (is_left_ptr ? node->first_child : node->last_child)->type;
    size_t size = type_size(ptr_type->ref);
    int reg_int = is_left_ptr ? (reg_right) : reg_left;

    // Shift or multiply the offset
    // TODO there's a GCC extension for a zero-size struct. should figure out if/how pointer arithmetic works with it
    if (size != 1) {
        if (is_pow2(size)) {
            // TODO use fls() or clz() or similar function
            int shift = 0;
            while (size) {
                size >>= 1;
                ++shift;
            }
            block_append(current_block, node->token, SHL, reg_int, reg_int, shift - 1);
        } else {
            block_append_op_imm(current_block, node->token, MUL, reg_int, reg_int, size);
        }
    }

    // Perform the addition or subtraction
    block_append(current_block, node->token, op, reg_left, reg_left, reg_right);

    register_free(node->token, reg_right);
}

void generate_pointer_add_sub(node_t* node, int reg_left) {
    generate_node(node->first_child, reg_left);

    // This is used for NODE_ADD, NODE_SUB and NODE_ARRAY_SUBSCRIPT (which is
    // also ADD.)
    opcode_t op = node->kind == NODE_SUB ? SUB : ADD;
    generate_pointer_add_sub_impl(node, op, reg_left);
}

static void generate_pointers_sub(node_t* node, int reg_left) {

    // Generate the sides
    int reg_right = register_alloc(node->token);
    generate_node(node->first_child, reg_left);
    generate_node(node->last_child, reg_right);

    // Perform the subtraction
    block_append(current_block, node->token, SUB, reg_left, reg_left, reg_right);

    // Shift or divide the result
    size_t size = type_size(node->first_child->type->ref);
    if (size != 1) {
        if (is_pow2(size)) {
            // TODO use fls() or clz() or similar function
            int shift = 0;
            while (size) {
                size >>= 1;
                ++shift;
            }
            block_append(current_block, node->token, SHRS, reg_left, reg_left, shift - 1);
        } else {
            block_append_op_imm(current_block, node->token, DIVS, reg_left, reg_left, size);
        }
    }

    register_free(node->token, reg_right);
}

void generate_add(node_t* node, int reg_out) {
    if (type_is_indirection(node->type)) {
        generate_pointer_add_sub(node, reg_out);
    } else {
        generate_simple_arithmetic(node, reg_out, ADD, "__llong_add", "__float_add", "__double_add");
    }
}

void generate_sub(node_t* node, int reg_out) {
    if (type_is_indirection(node->type)) {
        generate_pointer_add_sub(node, reg_out);
    } else if (type_is_indirection(node->first_child->type)) {
        generate_pointers_sub(node, reg_out);
    } else {
        generate_simple_arithmetic(node, reg_out, SUB, "__llong_sub", "__float_sub", "__double_sub");
    }
}

void generate_mul(node_t* node, int reg_out) {
    generate_simple_arithmetic(node, reg_out, MUL, "__llong_mul", "__float_mul", "__double_mul");
}

void generate_div(node_t* node, int reg_out) {
    if (type_is_signed_integer(node->type)) {
        generate_simple_arithmetic(node, reg_out, DIVS, "__llong_divs", NULL, NULL);
    } else {
        generate_simple_arithmetic(node, reg_out, DIVU, "__llong_divu", "__float_div", "__double_div");
    }
}

void generate_mod(node_t* node, int reg_out) {
    if (type_is_signed_integer(node->type)) {
        generate_simple_arithmetic(node, reg_out, MODS, "__llong_mods", NULL, NULL);
    } else {
        generate_simple_arithmetic(node, reg_out, MODU, "__llong_modu", "__float_mod", "__double_mod");
    }
}

void generate_shl(node_t* node, int reg_out) {
    generate_simple_arithmetic(node, reg_out, SHL, "__llong_shl", NULL, NULL);
}

void generate_shr(node_t* node, int reg_out) {
    if (type_is_signed_integer(node->type)) {
        generate_simple_arithmetic(node, reg_out, SHRS, "__llong_shrs", NULL, NULL);
    } else {
        generate_simple_arithmetic(node, reg_out, SHRU, "__llong_shru", NULL, NULL);
    }
}

void generate_bit_or(node_t* node, int reg_out) {
    generate_simple_arithmetic(node, reg_out, OR, "__llong_bit_or", NULL, NULL);
}

void generate_bit_and(node_t* node, int reg_out) {
    generate_simple_arithmetic(node, reg_out, AND, "__llong_bit_and", NULL, NULL);
}

void generate_bit_xor(node_t* node, int reg_out) {
    generate_simple_arithmetic(node, reg_out, XOR, "__llong_bit_xor", NULL, NULL);
}

void generate_bit_not(node_t* node, int reg_out) {
    generate_node(node->first_child, reg_out);
    if (type_size(node->type) > 4) {
        fatal("TODO bit not llong");
    } else {
        block_append(current_block, node->token, NOT, reg_out, reg_out);
    }
}

void generate_log_not(node_t* node, int reg_out) {
    if (type_size(node->type) != 4) {
        // TODO
        fatal("TODO log not llong");
    }
    generate_node(node->first_child, reg_out);
    block_append(current_block, node->token, ISZ, reg_out, reg_out);
}

static void generate_logical(node_t* node, int reg_out, bool and) {
    int end_label = next_label++;
    generate_node(node->first_child, reg_out);
    block_append(current_block, node->token, and ? JZ : JNZ, reg_out, '&', JUMP_LABEL_PREFIX, end_label);
    generate_node(node->last_child, reg_out);
    block_append(current_block, node->token, JMP, '&', JUMP_LABEL_PREFIX, end_label);

    current_block = block_new(end_label);
    function_add_block(current_function, current_block);
}

void generate_logical_or(node_t* node, int reg_out) {
    generate_logical(node, reg_out, false);
}

void generate_logical_and(node_t* node, int reg_out) {
    generate_logical(node, reg_out, true);
}




/**
 * Generates an ordered comparison.
 */
static void generate_ordering(node_t* node, int reg_left) {
    int reg_right = register_alloc(node->token);
    type_t* type = node->first_child->type;

    const char* function =
            type_matches_base(type, BASE_SIGNED_LONG_LONG)    ? "__llong_cmps" :
            type_matches_base(type, BASE_UNSIGNED_LONG_LONG)  ? "__llong_cmpu" :
            type_matches_base(type, BASE_FLOAT)               ? "__float_cmp" :
            type_matches_base(type, BASE_DOUBLE)              ? "__double_cmp" :
            NULL;

    generate_node(node->first_child, reg_left);
    if (function) {
        generate_binary_function(node, reg_left, function);
    } else {
        generate_node(node->last_child, reg_right);
        block_append(current_block, node->token,
                type_matches_base(type, BASE_SIGNED_INT) ? CMPS : CMPU,
                reg_left, reg_left, reg_right);
    }

    register_free(node->token, reg_right);
}

void generate_less(node_t* node, int reg_out) {
    generate_ordering(node, reg_out);
    block_append(current_block, node->token, CMPU, reg_out, reg_out, -1);
    block_append(current_block, node->token, ADD, reg_out, reg_out, 1);
    block_append(current_block, node->token, AND, reg_out, reg_out, 1);
}

void generate_greater(node_t* node, int reg_out) {
    generate_ordering(node, reg_out);
    block_append(current_block, node->token, CMPU, reg_out, reg_out, 1);
    block_append(current_block, node->token, ADD, reg_out, reg_out, 1);
    block_append(current_block, node->token, AND, reg_out, reg_out, 1);
}

void generate_less_or_equal(node_t* node, int reg_out) {
    generate_ordering(node, reg_out);
    block_append(current_block, node->token, CMPU, reg_out, reg_out, 1);
    block_append(current_block, node->token, AND, reg_out, reg_out, 1);
}

void generate_greater_or_equal(node_t* node, int reg_out) {
    generate_ordering(node, reg_out);
    block_append(current_block, node->token, CMPU, reg_out, reg_out, -1);
    block_append(current_block, node->token, AND, reg_out, reg_out, 1);
}

/**
 * Generates code for == and != operators. The result is zero if the types
 * match and non-zero otherwise.
 */
static void generate_equality(node_t* node, int reg_left) {
    int reg_right = register_alloc(node->token);
    type_t* type = node->first_child->type;

    generate_node(node->first_child, reg_left);
    if (type_is_long_long(type)) {
        generate_binary_function(node, reg_left, "__llong_neq");
    } else if (type_matches_base(type, BASE_DOUBLE)) {
        generate_binary_function(node, reg_left, "__double_neq");
    } else {
        generate_node(node->last_child, reg_right);
        block_append(current_block, node->token, SUB, reg_left, reg_left, reg_right);
    }

    register_free(node->token, reg_right);
}

void generate_equal(node_t* node, int reg_out) {
    generate_equality(node, reg_out);
    block_append(current_block, node->token, CMPU, reg_out, reg_out, 0);
    block_append(current_block, node->token, ADD, reg_out, reg_out, 1);
    block_append(current_block, node->token, AND, reg_out, reg_out, 1);
}

void generate_not_equal(node_t* node, int reg_out) {
    generate_equality(node, reg_out);
    block_append(current_block, node->token, CMPU, reg_out, reg_out, 0);
    block_append(current_block, node->token, AND, reg_out, reg_out, 1);
}

void generate_store(token_t* token, type_t* type, int reg_loc, int reg_val) {
    switch (type_size(type)) {
        case 1:
            block_append(current_block, token, STB, reg_val, 0, reg_loc);
            break;
        case 2:
            block_append(current_block, token, STS, reg_val, 0, reg_loc);
            break;
        case 4:
            block_append(current_block, token, STW, reg_val, 0, reg_loc);
            break;
            // fallthrough
        default:
            // TODO assigning large structures should call memcpy. eventually
            // we can optimize the case of only memcpy'ing e.g. 8 bytes.
            fatal("large assign not yet implemented");
            break;
    }
}

void generate_assign(node_t* node, int reg_val) {
    generate_node(node->last_child, reg_val);

    int reg_loc = register_alloc(node->token);
    generate_location(node->first_child, reg_loc);
    generate_store(node->token, node->type, reg_loc, reg_val);
    register_free(node->token, reg_loc);
}

void generate_add_sub_assign(node_t* node, int reg_val,
        opcode_t opcode, const char* llong_func,
        const char* float_func, const char* double_func)
{
    // generate the storage location
    int reg_loc = register_alloc(node->token);
    generate_location(node->first_child, reg_loc);

    // load it into the output register
    generate_dereference_impl(node, reg_val, reg_loc, 0);

    // do the operation
    if (type_is_indirection(node->type)) {
        generate_pointer_add_sub_impl(node, opcode, reg_val);
    } else {
        generate_simple_arithmetic(node, reg_val, opcode, llong_func, float_func, double_func);
    }

    // store the result
    generate_store(node->token, node->type, reg_loc, reg_val);
    register_free(node->token, reg_loc);
}

// Generates a compound assignment other than add or sub.
void generate_compound_assign(node_t* node, int reg_val,
        opcode_t opcode, const char* llong_func,
        const char* float_func, const char* double_func)
{
    // generate the storage location
    int reg_loc = register_alloc(node->token);
    generate_location(node->first_child, reg_loc);

    // load it into the output register
    generate_dereference_impl(node, reg_val, reg_loc, 0);

    // do the operation
    generate_simple_arithmetic(node, reg_val, opcode, llong_func, float_func, double_func);

    // store the result
    generate_store(node->token, node->type, reg_loc, reg_val);
    register_free(node->token, reg_loc);
}

void generate_add_assign(node_t* node, int reg_out) {
    generate_add_sub_assign(node, reg_out, ADD, "__llong_add", "__float_add", "__double_add");
}

void generate_sub_assign(struct node_t* node, int reg_out) {
    generate_add_sub_assign(node, reg_out, SUB, "__llong_sub", "__float_sub", "__double_sub");
}

void generate_mul_assign(struct node_t* node, int reg_out) {
    generate_compound_assign(node, reg_out, MUL, "__llong_mul", "__float_mul", "__double_mul");
}

void generate_div_assign(struct node_t* node, int reg_out) {
    if (type_is_signed_integer(node->type)) {
        generate_compound_assign(node, reg_out, DIVS, "__llong_divs", NULL, NULL);
    } else {
        generate_compound_assign(node, reg_out, DIVU, "__llong_divu", "__float_div", "__double_div");
    }
}

void generate_mod_assign(struct node_t* node, int reg_out) {
    if (type_is_signed_integer(node->type)) {
        generate_compound_assign(node, reg_out, MODS, "__llong_mods", NULL, NULL);
    } else {
        generate_compound_assign(node, reg_out, MODU, "__llong_modu", "__float_mod", "__double_mod");
    }
}

void generate_and_assign(struct node_t* node, int reg_out) {
    generate_compound_assign(node, reg_out, AND, "__llong_bit_and", NULL, NULL);
}

void generate_or_assign(struct node_t* node, int reg_out) {
    generate_compound_assign(node, reg_out, OR, "__llong_bit_or", NULL, NULL);
}

void generate_xor_assign(struct node_t* node, int reg_out) {
    generate_compound_assign(node, reg_out, XOR, "__llong_bit_xor", NULL, NULL);
}

void generate_shl_assign(struct node_t* node, int reg_out) {
    generate_compound_assign(node, reg_out, SHL, "__llong_shl", NULL, NULL);
}

void generate_shr_assign(struct node_t* node, int reg_out) {
    if (type_is_signed_integer(node->type)) {
        generate_compound_assign(node, reg_out, SHRS, "__llong_shrs", NULL, NULL);
    } else {
        generate_compound_assign(node, reg_out, SHRU, "__llong_shru", NULL, NULL);
    }
}

static void generate_pre_inc_dec(node_t* node, int reg_val, bool inc) {

    // generate the storage location
    int reg_loc = register_alloc(node->token);
    generate_location(node->first_child, reg_loc);

    // load it into the output register
    generate_dereference_impl(node, reg_val, reg_loc, 0);

    // increment/decrement it
    if (type_size(node->type) == 4) {
        block_append_op_imm(current_block, node->token, inc ? ADD : SUB, reg_val, reg_val,
            type_is_indirection(node->type) ? type_size(node->type->ref) : 1);
    } else if (type_size(node->type) == 8) {
        // TODO we should be able to just inline `inc jz inc` or equivalent.
        // need to create a block, use a temporary register, etc.
        fatal("TODO pre/post inc/dec llong");
    }

    // store it back again
    generate_store(node->token, node->type, reg_loc, reg_val);
    register_free(node->token, reg_loc);
}

static void generate_post_inc_dec(node_t* node, int reg_val, bool inc) {

    // generate the storage location
    int reg_loc = register_alloc(node->token);
    generate_location(node->first_child, reg_loc);

    // load it into the output register
    generate_dereference_impl(node, reg_val, reg_loc, 0);

    // increment/decrement it into a temporary reister
    int reg_temp = register_alloc(node->token);
    if (type_size(node->type) == 4) {
        block_append_op_imm(current_block, node->token, inc ? ADD : SUB, reg_temp, reg_val,
            type_is_indirection(node->type) ? type_size(node->type->ref) : 1);
    } else if (type_size(node->type) == 8) {
        // TODO need to allocate stack space and copy
        // TODO we should be able to just inline `inc jz inc` or equivalent.
        // need to create a block, use a temporary register, etc.
        fatal("TODO pre/post inc/dec llong");
    }

    // store it back again
    generate_store(node->token, node->type, reg_loc, reg_temp);
    register_free(node->token, reg_temp);
    register_free(node->token, reg_loc);
}

void generate_pre_inc(node_t* node, int reg_out) {
    generate_pre_inc_dec(node, reg_out, true);
}

void generate_pre_dec(node_t* node, int reg_out) {
    generate_pre_inc_dec(node, reg_out, false);
}

void generate_post_inc(node_t* node, int reg_out) {
    generate_post_inc_dec(node, reg_out, true);
}

void generate_post_dec(node_t* node, int reg_out) {
    generate_post_inc_dec(node, reg_out, false);
}
