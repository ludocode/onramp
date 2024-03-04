#ifndef COMPILE_H_INCLUDED
#define COMPILE_H_INCLUDED

#include "common.h"

void compile_init(void);
void compile_destroy(void);

/**
 * Compiles a global variable with the given type and name.
 */
void compile_global_variable(type_t type, const char* name);

/**
 * Compiles the prologue of a function with the given name and number of
 * arguments.
 */
void compile_function_open(const char* name, int arg_count);

/**
 * Compiles the epilogue of the current function with the given name.
 */
void compile_function_close(const char* name, int arg_count, int frame_size);

#if 0
/**
 * Compiles the start of a block, pushing a new scope.
 */
void compile_block_open(void);

/**
 * Compiles the end of a block, popping the top-most scope.
 */
void compile_block_close(void);
#endif

/**
 * Compiles a binary operation.
 *
 * The left of the operation is in r1 and the right is in r0.
 */
type_t compile_binary_op(const char* op, type_t left, type_t right);

/**
 * Places the given number in r0.
 */
type_t compile_immediate(const char* number);

/**
 * Places the given character in r0.
 */
type_t compile_character_literal(char c);

/**
 * Places the string literal with the given computed label index in r0.
 */
void compile_string_literal_invocation(int label_index);

/**
 * Defines a string literal.
 */
void compile_string_literal_definition(int label_index, const char* string);

/**
 * Places the address of the given variable in r0.
 */
type_t compile_variable(const char* name);

/**
 * Emits code to dereference the value of the given type stored in the given
 * register.
 *
 * You must have already removed an indirection (by decrementing the pointer
 * count, clearing the array, or clearing the l-value flag) before calling
 * this.
 */
void compile_dereference(type_t type, int register_num);

/**
 * Dereferences the variable in the given register if it is an lvalue.
 *
 * The type is returned without the l-value flag.
 */
type_t compile_dereference_if_lvalue(type_t type, int register_num);

type_t compile_not(void);

#endif
