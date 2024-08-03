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
struct token_t;

// TODO make better names
extern struct function_t* current_function;
extern struct block_t* current_block;
extern int next_label;

extern int register_next;
extern int register_loop_count;

void generate_init(void);
void generate_destroy(void);

/**
 * Compiles the parse tree of given the function into a series of basic blocks
 * of bytecode.
 */
void generate_function(struct function_t* function);

/**
 * Allocates a register, returning its number (i.e. the value 0x80-0x89
 * corresponding to the register r0-r9.)
 *
 * The register must be passed to a subsequent call to
 * register_free(). Registers must be freed in reverse order of
 * allocation.
 *
 * Registers are allocated sequentially from r0-r9. If additional registers are
 * needed, we loop back around to r0 and push the existing value to make room.
 * (This means only the last 10 allocated registers can be used. This is not a
 * problem because most operations use at most four allocated registers.)
 *
 * The given token is used to emit source location information if a push is
 * needed.
 */
int register_alloc(struct token_t* /*nullable*/ token);

/**
 * Frees a register.
 *
 * The register given must be the last allocated register that has not yet been
 * freed.
 *
 * The given token is used to emit source location information if a pop is
 * needed.
 */
void register_free(struct token_t* /*nullable*/ token, int reg);

/**
 * Compiles a node recursively.
 *
 * The return value is placed in the given register. If the return value is
 * larger than a register, the given register must contain a pointer to where
 * the return value is to be stored.
 */
void generate_node(struct node_t* node, int reg_out);

/**
 * Compiles an l-value node, for example as the left-hand side of the
 * assignment. This emits code to place the address of the value in the given
 * register instead of the value itself.
 */
void generate_location(struct node_t* node, int reg_out);

/**
 * Given a pointer and member offset, performs a dereference operation. Lots of
 * things use this:
 *
 * - unary `*` (dereference)
 * - binary `.` (struct value access)
 * - binary `->` (struct pointer access)
 * - array subscript (after addition)
 * - va_arg()
 * - the compound assignment operators
 *
 * The pointer in reg_ptr is shifted by the given member offset, then loaded
 * into reg_out.
 */
void generate_dereference_impl(struct node_t* node, int reg_out, int reg_ptr, int offset);

void generate_global_variable(struct symbol_t* symbol, struct node_t* /*nullable*/ initializer);

void generate_builtin(struct node_t* node, int reg_out);

#endif
