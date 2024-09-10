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

#ifndef GENERATE_OPS_H_INCLUDED
#define GENERATE_OPS_H_INCLUDED

/*
 * Code generation for binary and unary operators and other simple expressions
 */

#include <stddef.h>
#include <stdint.h>

struct node_t;
struct type_t;
struct token_t;

void generate_add(struct node_t* node, int reg_out);
void generate_sub(struct node_t* node, int reg_out);
void generate_mul(struct node_t* node, int reg_out);
void generate_div(struct node_t* node, int reg_out);
void generate_mod(struct node_t* node, int reg_out);

void generate_shl(struct node_t* node, int reg_out);
void generate_shr(struct node_t* node, int reg_out);
void generate_bit_or(struct node_t* node, int reg_out);
void generate_bit_and(struct node_t* node, int reg_out);
void generate_bit_xor(struct node_t* node, int reg_out);
void generate_bit_not(struct node_t* node, int reg_out);

void generate_logical_not(struct node_t* node, int reg_out);
void generate_logical_or(struct node_t* node, int reg_out);
void generate_logical_and(struct node_t* node, int reg_out);

void generate_equal(struct node_t* node, int reg_out);
void generate_not_equal(struct node_t* node, int reg_out);
void generate_less(struct node_t* node, int reg_out);
void generate_greater(struct node_t* node, int reg_out);
void generate_less_or_equal(struct node_t* node, int reg_out);
void generate_greater_or_equal(struct node_t* node, int reg_out);

void generate_store(struct token_t* /*nullable*/ token, struct type_t* type,
        int register_location, int register_value);

void generate_store_offset(struct token_t* /*nullable*/ token, struct type_t* type,
        int register_location, int register_value, int offset);

/**
 * Generates code to copy count values of the given type from reg_src to
 * reg_dest.
 *
 * This is used to generate indirect loads and stores of structs,
 * initialization of arrays, etc.
 */
void generate_copy(struct token_t* token, struct type_t* type, uint32_t count,
        int reg_src, int reg_dest);

/**
 * Generates code to zero out count values of the given type at the address in
 * the given register.
 */
void generate_zero_array(struct token_t* token, struct type_t* type, size_t count, int reg_loc);

/**
 * Generates code to zero out a value of the given type at the given location
 * with the given offset.
 */
void generate_zero_scalar(struct token_t* token, struct type_t* type, int reg_base, int offset);

void generate_assign(struct node_t* node, int reg_out);
void generate_add_assign(struct node_t* node, int reg_out);
void generate_sub_assign(struct node_t* node, int reg_out);
void generate_mul_assign(struct node_t* node, int reg_out);
void generate_div_assign(struct node_t* node, int reg_out);
void generate_mod_assign(struct node_t* node, int reg_out);
void generate_and_assign(struct node_t* node, int reg_out);
void generate_or_assign(struct node_t* node, int reg_out);
void generate_xor_assign(struct node_t* node, int reg_out);
void generate_shl_assign(struct node_t* node, int reg_out);
void generate_shr_assign(struct node_t* node, int reg_out);

void generate_pre_inc(struct node_t* node, int reg_out);
void generate_pre_dec(struct node_t* node, int reg_out);
void generate_post_inc(struct node_t* node, int reg_out);
void generate_post_dec(struct node_t* node, int reg_out);

/**
 * Generates an add or subtract of a pointer and an integer. The integer is
 * multiplied by the size of the pointed-to type.
 *
 * This is also used for calculating the location of an array subscript
 * operation.
 *
 * One child of the node must be a pointer and the other side must be a (signed
 * or unsigned) 32-bit integer.
 */
void generate_pointer_add_sub(struct node_t* node, int reg_out);

void generate_unary_plus(struct node_t* node, int reg_out);
void generate_unary_minus(struct node_t* node, int reg_out);

#endif
