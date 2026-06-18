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

#ifndef GENERATE_H_INCLUDED
#define GENERATE_H_INCLUDED

#include <stdbool.h>

struct block_t;
struct function_t;
struct node_t;
struct string_t;
struct symbol_t;
struct token_t;

// TODO make better names
extern struct function_t* current_function;
extern struct block_t* current_block;
extern int next_label;

#ifndef CCI2_IR
extern int register_next;
extern int register_loop_count;
#endif

void generate_setup(void);
void generate_teardown(void);

#ifdef CCI2_IR
/**
 * Generates a temporary, returning its id.
 *
 * The given name may be null, in which case one is generated (e.g. %1, %2,
 * etc.) If the name is already in use (for example due to shadowing), a number
 * will be prepended (e.g. %2_x).
 *
 * The id corresponding to the temporary is returned.
 *
 * If variable is true, an extra '%' is prepended (e.g. %%1, %%2, etc.) This is
 * typically used for variable storage.
 */
int generate_temporary(struct string_t* /*nullable*/ name, bool variable);
#endif

/**
 * Compiles the parse tree of given the function into a series of basic blocks
 * of bytecode.
 */
void generate_function(struct function_t* function);

#ifndef CCI2_IR
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
#endif

/**
 * Compiles a node recursively.
 *
 * The return value is placed in the given temporary. If the return value is
 * larger than a temporary, the given temporary must contain a pointer to where
 * the return value is to be stored.
 *
 * If the output temporary is -1, the value is ignored. (For example it may be
 * ultimately cast to void, either explicitly or as an unused expression, most
 * commonly an assignment.)
 */
void generate_node(struct node_t* node, int reg_out_opt);

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

/**
 * Generates a variable with static storage duration, i.e. a global variable
 * not marked `extern` or a local variable marked `static`.
 */
void generate_static_variable(struct symbol_t* symbol, struct node_t* /*nullable*/ initializer);

/*
 * This generates the contents of a defer.
 *
 * This is not called via generate_node(); a defer is never generated
 * arbitrarily.
 *
 * When generating a sequence, the sequence generates the defer nodes at the
 * end in reverse order. See generate_sequence(). Furthermore, when a jump
 * (break, continue, goto, return) is generated, it calls generate_defer() here
 * for any defers that are crossed.
 */
void generate_defer(struct node_t* node);

/*
 * This generates all defer nodes for a jump from the source node out of the
 * given container node.
 *
 * The container must be a parent of the node, or the node itself (in which
 * case nothing is generated.)
 */
void generate_exit_defers(struct node_t* node, struct node_t* container);

/**
 * This diagnoses errors on any defer nodes found for a jump into the given
 * node from the given container node.
 */
void generate_diagnose_defers(struct node_t* node, struct node_t* container, struct token_t* error_token);

#endif
