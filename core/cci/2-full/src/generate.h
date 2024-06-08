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

#ifndef GENERATE_H_INCLUDED
#define GENERATE_H_INCLUDED

#include <stdbool.h>

struct node_t;
struct function_t;
struct block_t;
struct symbol_t;

// TODO make better names
extern struct function_t* current_function;
extern struct block_t* current_block;
extern int next_label;

/**
 * Compiles the parse tree of given the function into a series of basic blocks
 * of bytecode.
 */
void generate_function(struct function_t* function);

/**
 * Makes room for an extra register, if needed.
 *
 * This handles the case of all registers being full. A register is spilled to
 * the stack to make room.
 *
 * If register_num is less than 9, there is already an extra register, so this
 * does nothing and returns false.
 *
 * If register_num is 9, this pushes r8 and moves r9 to r8, thus clearing an
 * extra register and keeping the registers in order. In this case this returns
 * true.
 *
 * The return value should be passed to a subsequent call to
 * generate_register_pop().
 */
bool generate_register_push(int* register_num);

/**
 * Undoes a register spill that was done earlier, if needed.
 *
 * This undoes an earlier call to generate_register_push().
 */
void generate_register_pop(bool pushed);

/**
 * Compiles a node recursively.
 *
 * The return value is placed in the given register. If the return value is
 * larger than a register, the given register must contain a pointer to where
 * the return value is to be stored.
 */
void generate_node(struct node_t* node, int register_num);

/**
 * Compiles an l-value node, for example as the left-hand side of the
 * assignment. This emits code to place the address of the value in the given
 * register instead of the value itself.
 */
void generate_location(struct node_t* node, int register_num);

void generate_global_variable(struct symbol_t* symbol, struct node_t* /*nullable*/ initializer);

#endif
