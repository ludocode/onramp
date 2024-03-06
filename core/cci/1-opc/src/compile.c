#include "compile.h"

#include <stdlib.h>
#include <string.h>

#include "emit.h"
#include "variable.h"

#if 0
// This tracks the maximum extent of the total number of variables throughout a
// function. It lets us calculate its frame size.
// TODO: remove this, track this in parse when a block closes, just call it
// parse_function_size
static int compile_function_max_variable_count;

/**
 * The current scope depth. Global variables are at scope 0. A function's
 * arguments and top-level variables are at scope 1. Nested blocks are at
 * deeper scopes.
 */
static int compile_scope;

/**
 * The number of variables at each scope depth.
 *
 * This is used to pop variables when a scope ends.
 *
 * TODO get rid of this. block scope variable counts should be stored on the
 * stack. compile shouldn't have anything to do with this anyway, it should be
 * strictly codegen, parse should pop vars.
 */
static int* compile_scope_variable_count;
#endif

#define COMPILE_SCOPE_MAX 16

void compile_init(void) {
    //compile_scope_variable_count = malloc(COMPILE_SCOPE_MAX * 4);
}

void compile_destroy(void) {
    //free(compile_scope_variable_count);
}

void compile_set_enabled(bool enabled) {
    emit_set_enabled(enabled);
}

bool compile_is_enabled(void) {
    return emit_is_enabled();
}

void compile_global_variable(const type_t* type, const char* name) {
    emit_label('=', name);
    emit_newline();

    int size = type_size(type);
    for (int i = 0; i < size; ++i) {
        if (i > 0 && ((i % 16) == 0)) { // TODO this isn't necessary in cci/0, only supports char and int
            emit_newline();
        }
        emit_quoted_byte(0x00);
    }
    emit_newline();
    emit_global_divider();
}

void compile_function_open(const char* name, int arg_count) {
    //printf("   compiling function open %s with %i args\n",name, arg_count);

    // we don't know the stack frame size yet so we'll emit the function
    // prologue at the end and then jump back here. (technically we're
    // compiling most of the function as a different symbol.)
    emit_prefixed_label('@', "_F_", name);
    emit_newline();

    // we'll track the function's frame size as we go.
    //printf("starting function, %i vars\n", variable_count - arg_count);
    //compile_function_max_variable_count = variable_count - arg_count;
    //printf("open varcount %i\n", compile_function_max_variable_count);

    // the first four arguments arrive in registers, the rest come on the
    // stack. we copy them to their new offsets.
    for (int i = 0; i < arg_count; ++i) {
        if (i > 4) {
            emit_term("ldw");
            emit_term("r9");
            emit_term("rfp");
            emit_int((i - 4) * 4);
            emit_newline();

            emit_term("stw");
            emit_term("r9");
            emit_term("rfp");
            emit_int(-(i + 1) * 4);
            emit_newline();
        } else {
            emit_term("stw");
            emit_register(i);
            emit_term("rfp");
            emit_int(-(i + 1) * 4);
            emit_newline();
        }
    }
}

void compile_function_close(const char* name, int arg_count, int frame_size) {
    //printf("close varcount %i\n", compile_function_max_variable_count);

    // add a return to the function in case it didn't return on its own
    // main() needs to return 0 if execution reaches the end of the function
    // without a return statement (provided it's declared with a return type of
    // int.) We do this for all functions to make the behaviour consistent.
    emit_term("zero");
    emit_term("r0");
    emit_newline();
    emit_term("leave");
    emit_newline();
    emit_term("ret");
    emit_newline();

    // emit the function prologue
    emit_newline();
    emit_label('=', name); // TODO NO, if static use @
    emit_newline();
    emit_term("enter");
    emit_newline();

    // set up the stack frame (now that we know its size)
    //variable_pop(variable_count - arg_count);
    //printf("closing function, %i vars left, %i max, %i vars in function\n", variable_count, compile_function_max_variable_count, compile_function_max_variable_count - variable_count);
    if ((frame_size > 0) & (frame_size < 0x80)) {
        // the frame size fits in a mix-type byte
        emit_term("sub");
        emit_term("rsp");
        emit_term("rsp");
        emit_int(frame_size);
        emit_newline();
    }
    if (frame_size >= 0x80) {
        // the frame size needs to go in a temporary register
        // TODO test this
        emit_term("imw");
        emit_term("r9");
        emit_int(frame_size);
        emit_newline();
        emit_term("sub");
        emit_term("rsp");
        emit_term("rsp");
        emit_term("r9");
        emit_newline();
    }

    // jump to the top of the function
    emit_term("jmp");
    emit_prefixed_label('^', "_F_", name);
    emit_global_divider();
}

type_t* compile_immediate(const char* number) {
    // TODO an obvious optimization here is to output numbers in range
    // -160 to +254 using add instead of imw
    emit_term("imw");
    emit_term("r0");
    // TODO we should parse out this number, make sure it matches the expected
    // type! e.g. if assigning to char, need to truncate (or do we?)
    emit_term(number);
    emit_newline();
    return type_new(TYPE_BASIC_INT, 0);
}

type_t* compile_immediate_int(int x) {
    // TODO see above optimization potential. compile_immediate() should call
    // this, also should have better names for these two functions
    emit_term("imw");
    emit_term("r0");
    emit_int(x);
    emit_newline();
    return type_new(TYPE_BASIC_INT, 0);
}

type_t* compile_sizeof(type_t* type) {
    size_t size = type_size(type);
    type_delete(type);
    return compile_immediate_int(size);
}

type_t* compile_character_literal(char c) {
    emit_term("mov");
    emit_term("r0");
    emit_character_literal(c);
    emit_newline();
    return type_new(TYPE_BASIC_INT, 0);
}

#if 0
void compile_block_open(void) {
    if (compile_scope == COMPILE_SCOPE_MAX) {
        fatal("Maximum block depth exceeded.");
    }
    compile_scope_variable_count[compile_scope] = variable_count;
    ++compile_scope;
}

void compile_block_close(void) {
    // TODO remove this whole function, do this in parse
    //printf("closing block, %i vars\n", variable_count);
    if (variable_count > compile_function_max_variable_count) {
        compile_function_max_variable_count = variable_count;
    }
    --compile_scope;
    variable_pop(compile_scope_variable_count[compile_scope]);
}
#endif

type_t* compile_variable(const char* name) {

    // find the variable
    const type_t* type;
    int offset;
    if (!variable_find(name, &type, &offset)) {
        fatal_2("Variable not found: ", name);
    }

    // put its address into r0
    if (offset == 0) {
        // it's a global variable
        emit_term("imw");
        emit_term("r0");
        emit_label('^', name);
        emit_newline();
        emit_term("add");
        emit_term("r0");
        emit_term("rpp");
        emit_term("r0");

    } else if (offset < 0x80) {
        // the offset fits in a mix-type byte
        emit_term("add");
        emit_term("r0");
        emit_term("rfp");
        emit_int(offset);

    } else {
        // the offset needs to go in a temporary register
        emit_term("imw");
        emit_term("r0");
        emit_int(offset);
        emit_newline();
        emit_term("add");
        emit_term("r0");
        emit_term("rfp");
        emit_term("r0");
    }
    emit_newline();

    // return it as an lvalue
    type_t* ret = type_new_blank();
    *ret = *type | TYPE_FLAG_LVALUE;
    return ret;
}

void compile_string_literal_invocation(int label_index) {
    emit_term("imw");
    emit_term("r0");
    emit_computed_label('^', STRING_LABEL_PREFIX, label_index);
    emit_newline();
    emit_term("add");
    emit_term("r0");
    emit_term("rpp");
    emit_term("r0");
    emit_newline();
}

void compile_string_literal_definition(int label_index, const char* string) {
    emit_computed_label('@', STRING_LABEL_PREFIX, label_index);
    emit_newline();
    emit_string_literal(string);
    emit_quoted_byte(0x00); // null-terminator
    emit_newline();
}

#if 0
static void compile_load_or_store(const char* name, bool load) {

    // find the variable
    variable_t* variable = scope_find(name);
    if (variable == 0) {
        fatal_2("Undeclared variable", name);
    }

    // emit the correct load/store instruction
    if (type_size(variable->type) == 1) {
        emit_term(load ? "ldb" : "stb");
    } else {
        emit_term(load ? "ldw" : "stw");
    }

    // load it into r0
    emit_term("r0");
    emit_label('^', name);
    emit_newline();

}

void compile_load(const char* name) {
    compile_load_or_store(name, true);
}

void compile_store(const char* name) {
    compile_load_or_store(name, false);
}
#endif

/**
 * Emits code to dereference the value of the given type stored in the given
 * register.
 */
void compile_dereference(type_t* type, int register_num) {
    if (type_size(type) == 1) {
        //printf("compiling ldb for %x\n",type);
        emit_term("ldb");
    } else {
        emit_term("ldw");
    }
    emit_register(register_num);
    emit_term("0");
    emit_register(register_num);
    emit_newline();
}

type_t* compile_dereference_if_lvalue(type_t* type, int register_num) {
    if (*type & TYPE_FLAG_LVALUE) {
        *type &= ~TYPE_FLAG_LVALUE;
        compile_dereference(type, register_num);
    }
    return type;
}

type_t* compile_assign(type_t* left, type_t* right) {

    // We're storing into the left. It must be an lvalue.
    if (!(*left & TYPE_FLAG_LVALUE)) {
        fatal("Assignment location is not an lvalue.");
    }

    // If the right is an lvalue, dereference it.
    if (*right & TYPE_FLAG_LVALUE) {
        *right &= ~TYPE_FLAG_LVALUE;
        compile_dereference(right, 0);
    }

    // assign stores the right value into the left.
    //if (type_size(*left & ~TYPE_FLAG_LVALUE) == 1) {
    if (type_size(left) == 1) {
        emit_term("stb");
    } else {
        emit_term("stw");
    }
    emit_term("r0");
    emit_term("0");
    emit_term("r1");
    emit_newline();

    // TODO for now we're returning right, this is probably wrong, if left is
    // char we maybe are supposed to truncate to char but also promote to int?
    type_delete(left);
    return right;
}

// Returns the factor to use for the other term for arithmetic with this type.
//
// If this is a pointer to anything but char, this returns 4, because the other
// term must be multiplied by 4. Otherwise it returns 1.
static int compile_arithmetic_factor(const type_t* type) {

    // A char* has a factor of 1.
    if (*type == (TYPE_BASIC_CHAR|1))
        return 1;

    // A non-pointer has a factor of 1.
    if (type_indirections(type) == 0)
        return 1;

    // TODO we should disallow arithmetic on void and void* (but not void**)

    // Otherwise it's a pointer to something other than char (i.e. it could be
    // int*, char**, void**, etc.) We need to multiply by 4.
    return 4;
}

type_t* compile_add_sub(bool add, type_t* left, type_t* right) {
    int left_indirection = type_indirections(left);
    int right_indirection = type_indirections(right);

    // Only one side is allowed to be a pointer.
    if (left_indirection > 0 && right_indirection > 0) {
        fatal("Cannot add or subtract two pointers");
    }

    // If either side is a word pointer, multiply the other by four
    // TODO use shl
    int right_factor = compile_arithmetic_factor(left);
    if (right_factor == 4) {
        emit_term("mul");
        emit_term("r0");
        emit_term("r0");
        emit_term("4");
        emit_newline();
    }
    int left_factor = compile_arithmetic_factor(right);
    if (left_factor == 4) {
        emit_term("mul");
        emit_term("r1");
        emit_term("r1");
        emit_term("4");
        emit_newline();
    }

    // Compute the result into r0
    if (add)
        emit_term("add");
    else
        emit_term("sub");
    emit_term("r0");
    emit_term("r1");
    emit_term("r0");
    emit_newline();

    // Return whichever side is a pointer type.
    //printf("addsub %02x %02x\n",left,right);
    if (left_indirection > 0) {
        //printf("returning left\n");
        type_delete(right);
        return left;
    }
    if (right_indirection > 0) {
        type_delete(left);
        return right;
    }

    // If neither is, promote to int.
    type_delete(left);
    type_delete(right);
    return type_new(TYPE_BASIC_INT, 0);
}

// If the result of a comparison in r0 is 0, r0 is set to 1; otherwise it's set
// to 0.
static void compile_cmp_to_true(void) {
    emit_term("and");
    emit_term("r0");
    emit_term("r0");
    emit_term("1");
    emit_newline();
    emit_term("xor");
    emit_term("r0");
    emit_term("r0");
    emit_term("1");
    emit_newline();
}

// If the result of a comparison in r0 is 0, r0 is set to 0; otherwise it's set
// to 1.
static void compile_cmp_to_false(void) {
    emit_term("and");
    emit_term("r0");
    emit_term("r0");
    emit_term("1");
    emit_newline();
}

static void compile_compare_signed(void) {
    emit_term("cmps");
    emit_term("r0");
    emit_term("r1");
    emit_term("r0");
    emit_newline();
}

static void compile_compare_unsigned(void) {
    emit_term("cmpu");
    emit_term("r0");
    emit_term("r1");
    emit_term("r0");
    emit_newline();
}

static void compile_compare_term(const char* term) {
    emit_term("cmpu");
    emit_term("r0");
    emit_term("r0");
    emit_term(term);
    emit_newline();
}

type_t* compile_lt(void) {
    compile_compare_signed();
    compile_compare_term("-1");
    compile_cmp_to_true();
    return type_new(TYPE_BASIC_INT, 0);
}

type_t* compile_gt(void) {
    compile_compare_signed();
    compile_compare_term("1");
    compile_cmp_to_true();
    return type_new(TYPE_BASIC_INT, 0);
}

type_t* compile_ge(void) {
    compile_compare_signed();
    compile_compare_term("-1");
    compile_cmp_to_false();
    return type_new(TYPE_BASIC_INT, 0);
}

type_t* compile_le(void) {
    compile_compare_signed();
    compile_compare_term("1");
    compile_cmp_to_false();
    return type_new(TYPE_BASIC_INT, 0);
}

type_t* compile_eq(void) {
    compile_compare_unsigned();
    compile_cmp_to_true();
    return type_new(TYPE_BASIC_INT, 0);
}

type_t* compile_ne(void) {
    compile_compare_unsigned();
    compile_cmp_to_false();
    return type_new(TYPE_BASIC_INT, 0);
}

type_t* compile_not(void) {
    emit_term("cmpu");
    emit_term("r0");
    emit_term("r0");
    emit_term("0");
    emit_newline();
    compile_cmp_to_true();
    return type_new(TYPE_BASIC_INT, 0);
}

type_t* compile_basic_op(const char* op) {
    emit_term(op);
    emit_term("r0");
    emit_term("r1");
    emit_term("r0");
    emit_newline();
    // TODO ignore pointers, just return int for now
    return type_new(TYPE_BASIC_INT, 0);
}

// Compiles a binary operation.
// The result of the left expression is in r1.
// The result of the right expression is in r0.
type_t* compile_binary_op(const char* op, type_t* left, type_t* right) {
    if (0 == strcmp(op, "=")) {
        return compile_assign(left, right);
    }

    // For the remaining operations, we need to dereference both to compute the
    // result
    left = compile_dereference_if_lvalue(left, 1);  // TODO move this one up above compile_assign()
    right = compile_dereference_if_lvalue(right, 0);

    // add/sub
    if (0 == strcmp(op, "+")) {
        return compile_add_sub(true, left, right);
    }
    if (0 == strcmp(op, "-")) {
        return compile_add_sub(false, left, right);
    }

    // all the rest of these ignore the types
    // TODO need to cast/promote
    type_delete(left);
    type_delete(right);

    // mul/div/mod
    if (0 == strcmp(op, "*")) {
        return compile_basic_op("mul");
    }
    if (0 == strcmp(op, "/")) {
        return compile_basic_op("div");
    }
    if (0 == strcmp(op, "%")) {
        // TODO this doesn't exist
        return compile_basic_op("mod");
    }

    // bitwise ops
    if (0 == strcmp(op, "|")) {
        return compile_basic_op("or");
    }
    if (0 == strcmp(op, "&")) {
        return compile_basic_op("and");
    }
    if (0 == strcmp(op, "^")) {
        return compile_basic_op("xor");
    }
    if (0 == strcmp(op, "<<")) {
        return compile_basic_op("shl");
    }
    if (0 == strcmp(op, ">>")) {
        return compile_basic_op("shrs"); // we only support signed
    }

    // comparisons
    if (0 == strcmp(op, "==")) {
        return compile_eq();
    }
    if (0 == strcmp(op, "!=")) {
        return compile_ne();
    }
    if (0 == strcmp(op, "<")) {
        return compile_lt();
    }
    if (0 == strcmp(op, ">")) {
        return compile_gt();
    }
    if (0 == strcmp(op, "<=")) {
        return compile_le();
    }
    if (0 == strcmp(op, ">=")) {
        return compile_ge();
    }

    fatal_2("op not yet implemented: ", op);
}
