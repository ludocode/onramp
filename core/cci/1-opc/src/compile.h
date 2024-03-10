#ifndef COMPILE_H_INCLUDED
#define COMPILE_H_INCLUDED

#include "common.h"
#include "parse-decl.h"

void compile_init(void);
void compile_destroy(void);

/**
 * Toggles compilation on or off.
 *
 * When off, nothing is emitted when compile functions are called, but type
 * manipulations are still performed. This is used to implement sizeof().
 *
 * Compilation is on by default. This is forwarded to emit_set_enabled(); this
 * is just here so that we have a clean separation between parse, compile and
 * emit.
 */
void compile_set_enabled(bool enabled);

bool compile_is_enabled(void);

/**
 * Compiles a global variable with the given type and name.
 */
void compile_global_variable(const type_t* type, const char* name, storage_t storage);

/**
 * Compiles the prologue of a function with the given name and number of
 * arguments.
 */
void compile_function_open(const char* name, int arg_count);

/**
 * Compiles the epilogue of the current function with the given name.
 */
void compile_function_close(const char* name, int arg_count, int frame_size, storage_t storage);

/**
 * Compiles a binary operation.
 *
 * The left of the operation is in r1 and the right is in r0.
 */
type_t* compile_binary_op(const char* op, type_t* left, type_t* right);

/**
 * Places the given number in r0.
 */
type_t* compile_immediate(const char* number);

/**
 * Places the given number in r0.
 */
type_t* compile_immediate_int(int x);

/**
 * Places the size of the given type in r0, returning unsigned int.
 */
type_t* compile_sizeof(type_t* type);

/**
 * Places the given character in r0.
 */
type_t* compile_character_literal(char c);

/**
 * Places the string literal with the given computed label index in r0.
 */
void compile_string_literal_invocation(int label_index);

/**
 * Defines a string literal.
 */
void compile_string_literal_definition(int label_index, const char* string);

/**
 * Emits code to load the variable with the given name as an l-value into r0,
 * returning its type.
 */
type_t* compile_load_variable(const char* name);

/**
 * Emits code to dereference the value of the given type stored in the given
 * register.
 *
 * In other words, this loads a value of the given type whose address is in the
 * given register into the given register.
 *
 * You must have already removed an indirection (by decrementing the pointer
 * count, clearing the array, or clearing the l-value flag) before calling
 * this. It is an error to call this on an l-value, on an array, or on a value
 * larger than a register.
 */
void compile_dereference(type_t* type, int register_num);

/**
 * Converts the l-value in the given register into an r-value.
 *
 * The type is returned without the l-value flag and with any other necessary
 * conversions (e.g. an array decays to a pointer).
 */
type_t* compile_lvalue_to_rvalue(type_t* type, int register_num);

type_t* compile_boolean_not(void);

type_t* compile_bitwise_not(void);

/**
 * Compiles a cast of the value in r0 from its current type to the desired type.
 *
 * Both types must be r-values.
 */
type_t* compile_cast(type_t* current_type, type_t* desired_type, int register_num);

#endif
