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
void generate_log_not(struct node_t* node, int reg_out);

void generate_equal(struct node_t* node, int reg_out);
void generate_not_equal(struct node_t* node, int reg_out);
void generate_less(struct node_t* node, int reg_out);
void generate_greater(struct node_t* node, int reg_out);
void generate_less_or_equal(struct node_t* node, int reg_out);
void generate_greater_or_equal(struct node_t* node, int reg_out);

void generate_store(struct token_t* /*nullable*/ token, struct type_t* type,
        int register_location, int register_value);

void generate_assign(struct node_t* node, int reg_out);
void generate_assign_add(struct node_t* node, int reg_out);
void generate_assign_sub(struct node_t* node, int reg_out);
void generate_assign_mul(struct node_t* node, int reg_out);
void generate_assign_div(struct node_t* node, int reg_out);

#endif
