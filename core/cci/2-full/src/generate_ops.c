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
 * If the return value is 32 bits (i.e. float), it is placed in reg_out. If the
 * return value is 64 bits, reg_out must contain the address of storage for the
 * return value.
 */
static void generate_arithmetic_function(node_t* parent,
        node_t* first, node_t* /*nullable*/ second,
        int reg_out, const char* function_name)
{
    bool ret_indirect = type_is_passed_indirectly(parent->type);
    bool left_indirect = type_is_passed_indirectly(first->type);
    bool right_indirect = second ? type_is_passed_indirectly(second->type) : false;

    // push all registers (except for the return register)
    int last_pushed_register = register_loop_count ? R9 : register_next - 1;
    for (int i = R0; i <= last_pushed_register; ++i) {
        if (i != reg_out) {
            block_append(current_block, parent->token, PUSH, i);
        }
    }

    // clear register allocator
    int old_register_next = register_next;
    int old_register_loop_count = register_loop_count;
    register_next = R0;
    register_loop_count = 0;

    // if the return value is indirect, make it the first argument
    int reg_ret = 0;
    if (ret_indirect) {
        reg_ret = register_alloc(parent->token);
        assert(reg_ret == R0);
        block_append(current_block, parent->token, MOV, reg_ret, reg_out);
    }

    // if args are indirect, we need to make room for temporaries.

    // generate the left arguments
    int reg_left = register_alloc(parent->token);
    if (left_indirect) {
        block_append(current_block, parent->token, SUB, RSP, RSP, 8);
        block_append(current_block, parent->token, MOV, reg_left, RSP);
    }
    generate_node(first, reg_left);

    // generate the right argument
    int reg_right;
    if (second) {
        reg_right = register_alloc(parent->token);
        if (right_indirect) {
            block_append(current_block, parent->token, SUB, RSP, RSP, 8);
            block_append(current_block, parent->token, MOV, reg_right, RSP);
        }
        generate_node(second, reg_right);
    }

    // generate function call
    block_append(current_block, parent->token, CALL, ARGTYPE_NAME, '^', function_name);

    // Move return value into output register
    block_append(current_block, parent->token, MOV, reg_out, R0);

    // pop stack space used for temporaries
    if (left_indirect || right_indirect) {
        block_append(current_block, parent->token, ADD, RSP, RSP,
                (left_indirect ? 8 : 0) + (right_indirect ? 8 : 0));
    }

    // restore the register allocator (we don't bother to free the argument registers first)
    register_next = old_register_next;
    register_loop_count = old_register_loop_count;

    // pop registers
    for (int i = last_pushed_register; i >= R0; --i) {
        if (i != reg_out) {
            block_append(current_block, parent->token, POP, i);
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
            type_is_long_long(type)              ? llong_func :
            type_matches_base(type, BASE_FLOAT)  ? float_func :
            type_matches_base(type, BASE_DOUBLE) ? double_func :
            NULL;

    if (function) {
        generate_arithmetic_function(node, node->first_child, node->last_child, reg_left, function);
    } else {
        generate_node(node->first_child, reg_left);
        int reg_right = register_alloc(node->token);
        assert(!type_is_passed_indirectly(node->last_child->type));
        generate_node(node->last_child, reg_right);
        block_append(current_block, node->token, opcode, reg_left, reg_left, reg_right);
        register_free(node->token, reg_right);
    }
}

/**
 * Adds two values where one side is a pointer or array and the other is an
 * integer.
 *
 * The left value has already been generated in the given register and the
 * result is placed in the same register.
 *
 * (Generating the left value is separate because we may have generated a
 * location for compound assignment. If so, it's already been dereferenced into
 * the given register.)
 */
static void generate_pointer_add_sub_impl(node_t* node, opcode_t op, int reg_left) {
    int reg_right = register_alloc(node->token);
    assert(!type_is_passed_indirectly(node->last_child->type));
    generate_node(node->last_child, reg_right);

    // One side is a pointer and the other side is an int offset. The offset
    // needs to be shifted or multiplied by the pointer size.

    // Figure out the size of the pointed-to type
    bool is_left_ptr = type_is_indirection(node->first_child->type);
    type_t* ptr_type = (is_left_ptr ? node->first_child : node->last_child)->type;
    if (!type_is_complete(ptr_type->ref))
        fatal_token(node->token, "Cannot perform pointer arithmetic on a pointer to an incomplete type.");
    size_t size = type_size(ptr_type->ref);
    int reg_int = is_left_ptr ? (reg_right) : reg_left;

    // Shift or multiply the offset
    if (size == 0) {
        fatal("Internal error: cannot perform arithmetic on pointer to zero-size element");
    }
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

void generate_indirection_add_sub(node_t* node, int reg_out) {
    generate_node(node->first_child, reg_out);

    // This is used for NODE_ADD, NODE_SUB and NODE_ARRAY_SUBSCRIPT (which is
    // also ADD.)
    opcode_t op = node->kind == NODE_SUB ? SUB : ADD;
    generate_pointer_add_sub_impl(node, op, reg_out);
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
    if (size == 0) {
        fatal("Internal error: cannot perform arithmetic on pointer to zero-size element");
    }
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
        generate_indirection_add_sub(node, reg_out);
    } else {
        generate_simple_arithmetic(node, reg_out, ADD, "__llong_add", "__float_add", "__double_add");
    }
}

void generate_sub(node_t* node, int reg_out) {
    if (type_is_indirection(node->type)) {
        generate_indirection_add_sub(node, reg_out);
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
    generate_simple_arithmetic(node, reg_out, OR, "__llong_or", NULL, NULL);
}

void generate_bit_and(node_t* node, int reg_out) {
    generate_simple_arithmetic(node, reg_out, AND, "__llong_and", NULL, NULL);
}

void generate_bit_xor(node_t* node, int reg_out) {
    generate_simple_arithmetic(node, reg_out, XOR, "__llong_xor", NULL, NULL);
}

void generate_bit_not(node_t* node, int reg_out) {
    generate_node(node->first_child, reg_out);

    type_t* type = node->type;
    if (type_is_long_long(type)) {
        int reg_temp = register_alloc(node->token);
        block_append(current_block, node->token, LDW, reg_temp, reg_out, 0);
        block_append(current_block, node->token, NOT, reg_temp, reg_temp);
        block_append(current_block, node->token, STW, reg_temp, reg_out, 0);
        block_append(current_block, node->token, LDW, reg_temp, reg_out, 4);
        block_append(current_block, node->token, NOT, reg_temp, reg_temp);
        block_append(current_block, node->token, STW, reg_temp, reg_out, 4);
        register_free(node->token, reg_temp);
    } else if (type_matches_base(type, BASE_FLOAT)) {
        fatal("Internal error: Cannot use 'bitwise not' on a float");
    } else if (type_matches_base(type, BASE_DOUBLE) || type_matches_base(type, BASE_LONG_DOUBLE)) {
        fatal("Internal error: Cannot use 'bitwise not' on a double");
    } else {
        block_append(current_block, node->token, NOT, reg_out, reg_out);
    }
}

void generate_logical_not(node_t* node, int reg_out) {
    type_t* source_type = node->first_child->type;

    // TODO if the size is 8, make stack space for it. for llong we can just OR
    // the two words together then do ISZ. for float/double we may need a
    // function call.

    if (type_matches_base(source_type, BASE_FLOAT) ||
            type_matches_base(source_type, BASE_DOUBLE) ||
            type_matches_base(source_type, BASE_LONG_DOUBLE)) {
        fatal_token(node->token, "TODO logical not float/double");
    }

    if (!type_is_integer(source_type) && !type_is_pointer(source_type)) {
        fatal_token(node->token, "Internal error: unrecognized type for logical not");
    }

    if (type_is_long_long(source_type)) {

        // generate the value
        block_append(current_block, node->token, SUB, RSP, RSP, 8);
        block_append(current_block, node->token, MOV, reg_out, RSP);
        generate_node(node->first_child, reg_out);

        // or the bytes together
        int reg_temp = register_alloc(node->token);
        block_append(current_block, node->token, LDW, reg_temp, reg_out, 0);
        block_append(current_block, node->token, LDW, reg_out, reg_out, 4);
        block_append(current_block, node->token, OR, reg_out, reg_out, reg_temp);
        register_free(node->token, reg_temp);
        block_append(current_block, node->token, ADD, RSP, RSP, 8);

    } else {
        generate_node(node->first_child, reg_out);

        // it's an integer. expand it to register size.
        // TODO is this necessary? don't we put in a promotion cast on the argument?
        if (type_size(source_type) == 1) {
            block_append(current_block, node->token, TRB, reg_out, reg_out);
        } else if (type_size(source_type) == 2) {
            block_append(current_block, node->token, TRS, reg_out, reg_out);
        }
    }

    // Apply isz (is zero) to the value. This is the logical not instruction.
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
 * Generates (left < right).
 */
static void generate_less_impl(node_t* node, node_t* left, node_t* right, int reg_left) {
    assert(type_equal(left->type, right->type));
    type_t* type = left->type;

    if (type_is_long_long(type)) {
        opcode_t opcode = type_is_signed_integer(type) ? LTS : LTU;

        // This is similar to generate_equality() below. We do the comparison
        // inline without a function call. This might be a bad idea; maybe it
        // would be simpler or shorter to move this to a function.

        // make stack space for both llongs
        block_append(current_block, node->token, SUB, RSP, RSP, 16);

        // generate left
        block_append(current_block, node->token, MOV, reg_left, RSP);
        generate_node(left, reg_left);

        // generate right
        int reg_right = register_alloc(node->token);
        block_append(current_block, node->token, ADD, reg_right, RSP, 8);
        generate_node(right, reg_right);

        // compare the low bytes in reg_left
        int reg_temp1 = register_alloc(node->token);
        block_append(current_block, node->token, LDW, reg_left, RSP, 0);
        block_append(current_block, node->token, LDW, reg_temp1, RSP, 8);
        block_append(current_block, node->token, opcode, reg_left, reg_left, reg_temp1);

        // keep it only if the top bytes match
        int reg_temp2 = register_alloc(node->token);
        block_append(current_block, node->token, LDW, reg_right, RSP, 4);
        block_append(current_block, node->token, LDW, reg_temp2, RSP, 12);
        block_append(current_block, node->token, SUB, reg_temp1, reg_right, reg_temp2);
        block_append(current_block, node->token, ISZ, reg_temp1, reg_temp1);
        block_append(current_block, node->token, AND, reg_left, reg_left, reg_temp1);

        // compare the high bytes in reg_right
        block_append(current_block, node->token, opcode, reg_right, reg_right, reg_temp2);

        // combine it with the result in reg_left
        block_append(current_block, node->token, OR, reg_left, reg_left, reg_right);

        // clean up
        register_free(node->token, reg_temp2);
        register_free(node->token, reg_temp1);
        register_free(node->token, reg_right);
        block_append(current_block, node->token, ADD, RSP, RSP, 16);
    } else if (type_matches_base(type, BASE_FLOAT)) {
        generate_arithmetic_function(node, left, right, reg_left, "__float_lt");
    } else if (type_matches_base(type, BASE_DOUBLE)) {
        generate_arithmetic_function(node, left, right, reg_left, "__double_lt");
    } else {
        generate_node(left, reg_left);
        int reg_right = register_alloc(node->token);
        generate_node(right, reg_right);
        block_append(current_block, node->token,
                type_matches_base(type, BASE_SIGNED_INT) ? LTS : LTU,
                reg_left, reg_left, reg_right);
        register_free(node->token, reg_right);
    }
}

void generate_less(node_t* node, int reg_out) {
    generate_less_impl(node, node->first_child, node->last_child, reg_out);
}

void generate_greater(node_t* node, int reg_out) {
    generate_less_impl(node, node->last_child, node->first_child, reg_out);
}

void generate_less_or_equal(node_t* node, int reg_out) {
    generate_less_impl(node, node->last_child, node->first_child, reg_out);
    block_append(current_block, node->token, SUB, reg_out, 1, reg_out);
}

void generate_greater_or_equal(node_t* node, int reg_out) {
    generate_less_impl(node, node->first_child, node->last_child, reg_out);
    block_append(current_block, node->token, SUB, reg_out, 1, reg_out);
}

/**
 * Generates code for == and != operators. The result is zero if the types
 * match and non-zero otherwise.
 */
static void generate_equality(node_t* node, int reg_left) {
    type_t* type = node->first_child->type;

    if (type_is_long_long(type)) {

        // this is a lot of code to compare llongs but maybe less instructions
        // than generating a function call. (we'd still have to generate into
        // stack space and we'd have to push registers.)

        // make stack space for both llongs
        block_append(current_block, node->token, SUB, RSP, RSP, 16);

        // generate left
        block_append(current_block, node->token, MOV, reg_left, RSP);
        generate_node(node->first_child, reg_left);

        // generate right
        int reg_right = register_alloc(node->token);
        block_append(current_block, node->token, ADD, reg_right, RSP, 8);
        generate_node(node->last_child, reg_right);

        // 'or' differences together
        int reg_temp = register_alloc(node->token);
        block_append(current_block, node->token, LDW, reg_left, RSP, 0);
        block_append(current_block, node->token, LDW, reg_temp, RSP, 8);
        block_append(current_block, node->token, SUB, reg_left, reg_left, reg_temp);
        block_append(current_block, node->token, LDW, reg_right, RSP, 4);
        block_append(current_block, node->token, LDW, reg_temp, RSP, 12);
        block_append(current_block, node->token, SUB, reg_right, reg_right, reg_temp);
        block_append(current_block, node->token, OR, reg_left, reg_left, reg_right);

        // clean up
        register_free(node->token, reg_temp);
        register_free(node->token, reg_right);
        block_append(current_block, node->token, ADD, RSP, RSP, 16);

    } else if (type_matches_base(type, BASE_FLOAT)) {
        generate_arithmetic_function(node, node->first_child, node->last_child, reg_left, "__float_ne");
    } else if (type_matches_base(type, BASE_DOUBLE)) {
        generate_arithmetic_function(node, node->first_child, node->last_child, reg_left, "__double_ne");
    } else {
        generate_node(node->first_child, reg_left);
        int reg_right = register_alloc(node->token);
        generate_node(node->last_child, reg_right);
        block_append(current_block, node->token, SUB, reg_left, reg_left, reg_right);
        register_free(node->token, reg_right);
    }
}

void generate_equal(node_t* node, int reg_out) {
    generate_equality(node, reg_out);
    block_append(current_block, node->token, ISZ, reg_out, reg_out);
}

void generate_not_equal(node_t* node, int reg_out) {
    generate_equality(node, reg_out);
    block_append(current_block, node->token, BOOL, reg_out, reg_out);
}

/*
 * Generates code to zero out memory for the given type with the given number
 * of bytes at the address stored in the given register.
 *
 * We check both size and alignment to decide on a step size because this is
 * used to zero out strings in initializers among other things.
 */
void generate_zero_array(token_t* token, type_t* type, size_t count, int reg_loc) {
    size_t size = type_size(type);
    size_t total = count * size;
    size_t align = type_alignment(type);

    // choose a step size
    size_t step;
    size_t steps;
    int store;
    if (0 == (total & 3) && 0 == (align & 3)) {
        step = 4;
        steps = count >> 2;
        store = STW;
    } else if (0 == (total & 1) && 0 == (align & 1)) {
        step = 2;
        steps = count >> 1;
        store = STS;
    } else {
        step = 1;
        steps = count;
        store = STB;
    }

    // If the number of steps is small, unroll it.
    if (steps <= 8) {
        for (size_t i = 0; i < count; i += step) {
            block_append(current_block, token, store, 0, reg_loc, i);
        }

    // Otherwise insert a loop.
    } else {
        int reg_i = register_alloc(token);
        block_t* loop_block = block_new(next_label++);
        block_t* end_block = block_new(next_label++);
        function_add_block(current_function, loop_block);
        function_add_block(current_function, end_block);

        block_append(current_block, token, IMW, ARGTYPE_NUMBER, reg_i, count);
        block_append(current_block, token, JMP, '&', JUMP_LABEL_PREFIX, loop_block->label);

        current_block = loop_block;
        block_append(current_block, token, JZ, reg_i, '&', JUMP_LABEL_PREFIX, end_block->label);
        block_append(current_block, token, SUB, reg_i, reg_i, step);
        block_append(current_block, token, store, 0, reg_loc, reg_i);
        block_append(current_block, token, JMP, '&', JUMP_LABEL_PREFIX, loop_block->label);

        current_block = end_block;
        register_free(token, reg_i);
    }
}

void generate_zero_scalar(struct token_t* token, struct type_t* type, int reg_base, int offset) {
    if (offset == 0) {
        generate_zero_array(token, type, 1, reg_base);
    } else {
        int reg_loc = register_alloc(token);
        block_append(current_block, token, IMW, ARGTYPE_NUMBER, reg_loc, offset);
        block_append(current_block, token, ADD, reg_loc, reg_loc, reg_base);
        generate_zero_array(token, type, 1, reg_loc);
        register_free(token, reg_loc);
    }
}

void generate_copy(token_t* token, type_t* type, uint32_t count,
        int reg_src, int reg_dest)
{
    int reg_temp = register_alloc(token);
    uint32_t align = type_alignment(type);
    uint32_t total = count * type_size(type);

    // Choose a step size. We check both size and alignment because this is
    // used to copy strings in initializers among other things.
    uint32_t step;
    uint32_t steps;
    int load;
    int store;
    if (0 == (total & 3) && 0 == (align & 3)) {
        step = 4;
        steps = total >> 2;
        load = LDW;
        store = STW;
    } else if (0 == (total & 1) && 0 == (align & 1)) {
        step = 2;
        steps = total >> 1;
        load = LDS;
        store = STS;
    } else {
        step = 1;
        steps = total;
        load = LDB;
        store = STB;
    }

    // If the number of steps is small, unroll it.
    if (steps <= 4) {
        for (uint32_t i = 0; i < total; i += step) {
            block_append(current_block, token, load, reg_temp, reg_src, i);
            block_append(current_block, token, store, reg_temp, reg_dest, i);
        }

    // Otherwise insert a loop.
    } else {
        int reg_i = register_alloc(token);
        block_t* loop_block = block_new(next_label++);
        block_t* end_block = block_new(next_label++);
        function_add_block(current_function, loop_block);
        function_add_block(current_function, end_block);

        block_append(current_block, token, IMW, ARGTYPE_NUMBER, reg_i, total);
        block_append(current_block, token, JMP, '&', JUMP_LABEL_PREFIX, loop_block->label);

        current_block = loop_block;
        block_append(current_block, token, JZ, reg_i, '&', JUMP_LABEL_PREFIX, end_block->label);
        block_append(current_block, token, SUB, reg_i, reg_i, step);
        block_append(current_block, token, load, reg_temp, reg_src, reg_i);
        block_append(current_block, token, store, reg_temp, reg_dest, reg_i);
        block_append(current_block, token, JMP, '&', JUMP_LABEL_PREFIX, loop_block->label);

        current_block = end_block;
        register_free(token, reg_i);
    }

    register_free(token, reg_temp);
}

// Generates a store for a direct value in reg_val into the address in reg_loc
// plus the given small offset.
static void generate_store_direct(token_t* token, size_t size, int reg_val, int reg_base, int offset) {
    assert(offset < 128 && offset >= -112);
    int opcode;
    switch (size) {
        case 1: opcode = STB; break;
        case 2: opcode = STS; break;
        case 4: opcode = STW; break;
        default:
            fatal_token(token, "Internal error: impossible size for direct store: %i", (int)size);
            break;
    }
    block_append(current_block, token, opcode, reg_val, reg_base, offset);
}

void generate_store_offset(token_t* token, type_t* type, int reg_val, int reg_base, int offset) {
    assert(!type_is_array(type));

    size_t size = type_size(type);
    bool indirect = type_is_passed_indirectly(type);

    // See if we can do the offset inline
    if (indirect) {
        if (offset == 0) {
            generate_copy(token, type, 1, reg_val, reg_base);
            return;
        }
    } else {
        if (offset < 128 && offset >= -112) {
            generate_store_direct(token, size, reg_val, reg_base, offset);
            return;
        }
    }

    // We have to add the offset and base into a temporary register
    int reg_loc = register_alloc(token);
    block_append(current_block, token, IMW, ARGTYPE_NUMBER, reg_loc, offset);
    block_append(current_block, token, ADD, reg_loc, reg_loc, reg_base);
    if (indirect)
        generate_copy(token, type, 1, reg_val, reg_loc);
    else
        generate_store_direct(token, size, reg_val, reg_loc, 0);
    register_free(token, reg_loc);
}

void generate_store(token_t* token, type_t* type, int reg_val, int reg_loc) {
    generate_store_offset(token, type, reg_val, reg_loc, 0);
}

void generate_assign(node_t* node, int reg_val) {
    generate_node(node->last_child, reg_val);

    int reg_loc = register_alloc(node->token);
    generate_location(node->first_child, reg_loc);
    generate_store(node->token, node->type, reg_val, reg_loc);
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
    generate_store(node->token, node->type, reg_val, reg_loc);
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
    generate_store(node->token, node->type, reg_val, reg_loc);
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

static void generate_inc_dec(node_t* node, int reg_in, int reg_out, bool inc) {
    if (type_is_long_long(node->type)) {
        // long long increment/decrement is done branchless. we modify the low
        // byte, then create a mask on overflow and use it to conditionally
        // modify the high byte.
        // we would probably produce faster code if we made this branch on
        // overflow. could be worth revisiting at some point.
        int reg_temp1 = register_alloc(node->token);
        int reg_temp2 = register_alloc(node->token);
        if (inc) {
            // increment low byte
            block_append(current_block, node->token, LDW, reg_temp1, reg_in, 0);
            block_append(current_block, node->token, INC, reg_temp1, reg_temp1);
            block_append(current_block, node->token, STW, reg_temp1, reg_out, 0);
            // check for overflow
            block_append(current_block, node->token, ISZ, reg_temp1, reg_temp1);
            // increment high byte if overflow
            block_append(current_block, node->token, LDW, reg_temp2, reg_in, 4);
            block_append(current_block, node->token, ADD, reg_temp2, reg_temp2, reg_temp1);
            block_append(current_block, node->token, STW, reg_temp2, reg_out, 4);
        } else {
            // decrement low byte
            block_append(current_block, node->token, LDW, reg_temp1, reg_in, 0);
            block_append(current_block, node->token, DEC, reg_temp1, reg_temp1);
            block_append(current_block, node->token, STW, reg_temp1, reg_out, 0);
            // check for overflow
            block_append(current_block, node->token, SUB, reg_temp1, reg_temp1, 0xFF);
            block_append(current_block, node->token, ISZ, reg_temp1, reg_temp1);
            // decrement high byte if overflow
            block_append(current_block, node->token, LDW, reg_temp2, reg_in, 4);
            block_append(current_block, node->token, SUB, reg_temp2, reg_temp2, reg_temp1);
            block_append(current_block, node->token, STW, reg_temp2, reg_out, 4);
        }
        register_free(node->token, reg_temp2);
        register_free(node->token, reg_temp1);

    } else if (type_is_integer(node->type) || type_matches_base(node->type, BASE_ENUM)) {
        // Note: We don't need to do anything special to handle wrapping for
        // char or short. Only the byte or short will be stored, and the
        // register will be truncated or sign extended if and when it is cast
        // to int.
        block_append_op_imm(current_block, node->token, inc ? ADD : SUB, reg_out, reg_in, 1);

    } else if (type_is_indirection(node->type)) {
        block_append_op_imm(current_block, node->token, inc ? ADD : SUB,
                reg_out, reg_in, type_size(node->type->ref));

    } else {
        fatal("Internal error: cannot generate pre/post increment on non-integer non-pointer type.");
    }
}

static void generate_pre_inc_dec(node_t* node, int reg_val, bool inc) {

    // generate the storage location
    int reg_loc = register_alloc(node->token);
    generate_location(node->first_child, reg_loc);

    // load it into the output
    generate_dereference_impl(node, reg_val, reg_loc, 0);

    // increment/decrement it
    generate_inc_dec(node, reg_val, reg_val, inc);

    // store it back again
    generate_store(node->token, node->type, reg_val, reg_loc);
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
    bool indirect = type_is_passed_indirectly(node->type);
    if (indirect) {
        block_append(current_block, node->token, SUB, RSP, RSP, 8);
        block_append(current_block, node->token, MOV, reg_temp, RSP);
    }
    generate_inc_dec(node, reg_val, reg_temp, inc);

    // store it back again
    generate_store(node->token, node->type, reg_temp, reg_loc);

    // clean up
    if (indirect) {
        block_append(current_block, node->token, ADD, RSP, RSP, 8);
    }
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

void generate_unary_plus(node_t* node, int reg_out) {
    // We don't need to do anything besides generate. Probably unary plus
    // should be an implicit cast, but we should actually insert this cast into
    // the tree, not do it in codegen.
    generate_node(node->first_child, reg_out);
}

void generate_unary_minus(node_t* node, int reg_out) {
    type_t* type = node->type;

    char* func;
    if (type_is_long_long(type)) {
        // TODO we could probably do this one inline pretty easily, just use
        // ltu to get borrow bit
        func = "__llong_negate";
    } else if (type_matches_base(type, BASE_FLOAT)) {
        // TODO this could be done inline, just flip sign bit (maybe? or do we
        // need to signal on negating a NaN?)
        func = "__float_negate";
    } else if (type_matches_base(type, BASE_DOUBLE)) {
        // TODO this could be done inline, or not, same as float
        func = "__double_negate";
    } else {
        func = NULL;
    }

    if (func) {
        generate_arithmetic_function(node, node->first_child, NULL, reg_out, func);
    } else {
        generate_node(node->first_child, reg_out);
        block_append(current_block, node->token, SUB, reg_out, 0, reg_out);
    }
}
