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

#include "generate.h"
#include "node.h"
#include "block.h"
#include "type.h"

/**
 * Generates an arithmetic or other binary calculation that must be done with a
 * libc function. This is used for long long, float and double.
 */
static void generate_binary_function(node_t* node, int register_num, const char* function_name) {

    // push registers
    for (int i = register_num; i > R0;) {
        block_add(current_block, PUSH, --i);
    }

    // for 64-bit math, arguments must point to storage space. make room for a
    // temporary
    size_t size = type_size(node->type);
    assert(size >= 4); // should already be promoted
    if (size != 4) {
        if (register_num != R0)
            block_add(current_block, MOV, R0, register_num);
        block_add(current_block, SUB, RSP, RSP, size);
        block_add(current_block, MOV, R1, RSP);
    }

    // generate arguments
    generate_node(node->first_child, R0);
    generate_node(node->last_child, R1);

    // generate function call
    block_add(current_block, CALL, '^', function_name);

    // move return value into output register
    if (register_num != R0)
        block_add(current_block, MOV, register_num, R0);

    // pop registers
    for (int i = 0; i < register_num; ++i) {
        block_add(current_block, PUSH, i);
    }

    // pop stack space used for temporary
    if (size != 4) {
        block_add(current_block, ADD, RSP, RSP, 8);
    }
}

/**
 * Generates a simple arithmetic calculation.
 */
static void generate_simple_arithmetic(node_t* node, int register_num,
        opcode_t opcode, const char* llong_func,
        const char* float_func, const char* double_func)
{
    bool pushed = generate_register_push(&register_num);
    type_t* type = node->type;

    const char* function =
            type_is_long_long(type)              ? "__llong_add" :
            type_matches_base(type, BASE_FLOAT)  ? "__float_add" :
            type_matches_base(type, BASE_DOUBLE) ? "__double_add" :
            NULL;

    if (function) {
        generate_binary_function(node, register_num, function);
    } else {
        generate_node(node->first_child, register_num);
        generate_node(node->last_child, register_num + 1);
        block_add(current_block, opcode, register_num, register_num, register_num + 1);
    }

    generate_register_pop(pushed);
}

void generate_add(node_t* node, int register_num) {
    generate_simple_arithmetic(node, register_num, ADD, "__llong_add", "__float_add", "__double_add");
}

void generate_sub(node_t* node, int register_num) {
    generate_simple_arithmetic(node, register_num, SUB, "__llong_sub", "__float_sub", "__double_sub");
}

void generate_mul(node_t* node, int register_num) {
    generate_simple_arithmetic(node, register_num, MUL, "__llong_mul", "__float_mul", "__double_mul");
}

void generate_div(node_t* node, int register_num) {
    if (type_is_signed_integer(node->type)) {
        generate_simple_arithmetic(node, register_num, DIVS, "__llong_divs", NULL, NULL);
    } else {
        generate_simple_arithmetic(node, register_num, DIVU, "__llong_divu", "__float_div", "__double_div");
    }
}

void generate_mod(node_t* node, int register_num) {
    if (type_is_signed_integer(node->type)) {
        generate_simple_arithmetic(node, register_num, MODS, "__llong_mods", NULL, NULL);
    } else {
        generate_simple_arithmetic(node, register_num, MODU, "__llong_modu", "__float_mod", "__double_mod");
    }
}

void generate_shl(node_t* node, int register_num) {
    generate_simple_arithmetic(node, register_num, SHL, "__llong_shl", NULL, NULL);
}

void generate_shr(node_t* node, int register_num) {
    if (type_is_signed_integer(node->type)) {
        generate_simple_arithmetic(node, register_num, SHRS, "__llong_shrs", NULL, NULL);
    } else {
        generate_simple_arithmetic(node, register_num, SHRU, "__llong_shru", NULL, NULL);
    }
}

void generate_bit_or(node_t* node, int register_num) {
    generate_simple_arithmetic(node, register_num, OR, "__llong_bit_or", NULL, NULL);
}



void generate_bit_not(node_t* node, int register_num) {
    if (type_size(node->type) != 4) {
        // TODO
        fatal("TODO bit not llong");
    }
    block_add(current_block, SUB, register_num, 0xFF, register_num);
}

void generate_log_not(node_t* node, int register_num) {
    if (type_size(node->type) != 4) {
        // TODO
        fatal("TODO log not llong");
    }
    block_add(current_block, ISZ, register_num, register_num);
}

/**
 * Generates an ordered comparison.
 */
static void generate_ordering(node_t* node, int register_num) {
    bool pushed = generate_register_push(&register_num);
    type_t* type = node->type;

    const char* function =
            type_matches_base(type, BASE_SIGNED_LONG_LONG)    ? "__llong_cmps" :
            type_matches_base(type, BASE_UNSIGNED_LONG_LONG)  ? "__llong_cmpu" :
            type_matches_base(type, BASE_FLOAT)               ? "__float_cmp" :
            type_matches_base(type, BASE_DOUBLE)              ? "__double_cmp" :
            NULL;

    if (function) {
        generate_binary_function(node, register_num, function);
    } else {
        generate_node(node->first_child, register_num);
        generate_node(node->last_child, register_num + 1);
        block_add(current_block,
                type_matches_base(type, BASE_SIGNED_INT) ? CMPS : CMPU,
                register_num, register_num, register_num + 1);
    }

    generate_register_pop(pushed);
}

void generate_less(node_t* node, int register_num) {
    generate_ordering(node, register_num);
    block_add(current_block, CMPU, register_num, register_num, -1);
    block_add(current_block, ADD, register_num, register_num, 1);
    block_add(current_block, AND, register_num, register_num, 1);
}

void generate_greater(node_t* node, int register_num) {
    generate_ordering(node, register_num);
    block_add(current_block, CMPU, register_num, register_num, 1);
    block_add(current_block, ADD, register_num, register_num, 1);
    block_add(current_block, AND, register_num, register_num, 1);
}

void generate_less_or_equal(node_t* node, int register_num) {
    generate_ordering(node, register_num);
    block_add(current_block, CMPU, register_num, register_num, 1);
    block_add(current_block, AND, register_num, register_num, 1);
}

void generate_greater_or_equal(node_t* node, int register_num) {
    generate_ordering(node, register_num);
    block_add(current_block, CMPU, register_num, register_num, -1);
    block_add(current_block, AND, register_num, register_num, 1);
}

/**
 * Generates code for == and != operators. The result is zero if the types
 * match and non-zero otherwise.
 */
static void generate_equality(node_t* node, int register_num) {
    bool pushed = generate_register_push(&register_num);
    type_t* type = node->type;

    if (type_is_long_long(type) || type_matches_base(type, BASE_DOUBLE)) {
        generate_binary_function(node, register_num, "__llong_double_neq");
    } else {
        generate_node(node->first_child, register_num);
        generate_node(node->last_child, register_num + 1);
        block_add(current_block, SUB, register_num, register_num, register_num + 1);
    }

    generate_register_pop(pushed);
}

void generate_equal(node_t* node, int register_num) {
    generate_equality(node, register_num);
    block_add(current_block, CMPU, register_num, register_num, 0);
    block_add(current_block, ADD, register_num, register_num, 1);
    block_add(current_block, AND, register_num, register_num, 1);
}

void generate_not_equal(node_t* node, int register_num) {
    generate_equality(node, register_num);
    block_add(current_block, CMPU, register_num, register_num, 0);
    block_add(current_block, AND, register_num, register_num, 1);
}

void generate_store(type_t* type, int register_location, int register_value) {
    switch (type_size(type)) {
        case 1:
            block_add(current_block, STB, register_value, 0, register_location);
            break;
        case 2:
            block_add(current_block, STS, register_value, 0, register_location);
            break;
        case 4:
            block_add(current_block, STW, register_value, 0, register_location);
            break;
            // fallthrough
        default:
            // TODO assigning large structures should call memcpy. eventually
            // we can optimize the case of only memcpy'ing e.g. 8 bytes.
            fatal("large assign not yet implemented");
            break;
    }
}

void generate_assign(node_t* node, int register_num) {
    bool pushed = generate_register_push(&register_num);

    generate_node(node->last_child, register_num);
    generate_location(node->first_child, register_num + 1);
    generate_store(node->type, register_num + 1, register_num);

    generate_register_pop(pushed);
}
