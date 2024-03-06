#include "compile.h"

#include <stdlib.h>
#include <string.h>

#include "emit.h"
#include "variable.h"

void compile_init(void) {
}

void compile_destroy(void) {
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
    int i = 0;
    while (i < size) {
        if ((i > 0) & ((i & 15) == 0)) {
            emit_newline();
        }
        emit_quoted_byte(0x00);
        i = (i + 1);
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
    int i = 0;
    while (i < arg_count) {
        if (i < 4) {
            emit_term("stw");
            emit_register(i);
            emit_term("rfp");
            emit_int(-(i + 1) * 4);
            emit_newline();
        }
        if (i >= 4) {
            emit_term("ldw");
            emit_term("r9");
            emit_term("rfp");
            emit_int((i - 3) * 4);
            emit_newline();

            emit_term("stw");
            emit_term("r9");
            emit_term("rfp");
            emit_int(-(i + 1) * 4);
            emit_newline();
        }
        i = (i + 1);
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

    }

    if (!(offset == 0) & !(offset >= 0x80)) {
        // the offset fits in a mix-type byte
        emit_term("add");
        emit_term("r0");
        emit_term("rfp");
        emit_int(offset);
    }

    if (offset >= 0x80) {
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
    *ret = (*type | TYPE_FLAG_LVALUE);
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

/**
 * Emits code to dereference the value of the given type stored in the given
 * register.
 */
void compile_dereference(type_t* type, int register_num) {
    int size = type_size(type);
    if (size == 1) {
        //printf("compiling ldb for %x\n",type);
        emit_term("ldb");
    }
    if (size == 4) {
        emit_term("ldw");
    }
    emit_register(register_num);
    emit_term("0");
    emit_register(register_num);
    emit_newline();
}

type_t* compile_dereference_if_lvalue(type_t* type, int register_num) {
    if (*type & TYPE_FLAG_LVALUE) {
        *type = (*type & ~TYPE_FLAG_LVALUE);
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
        *right = (*right & ~TYPE_FLAG_LVALUE);
        compile_dereference(right, 0);
    }

    left = type_set_lvalue(left, false);
    size_t size = type_size(left);

    // cast the types (performing sign extension, etc.)
    type_t* result = compile_cast(right, left, 0);

    // assign stores the right value into the left.
    if (size == 1) {
        emit_term("stb");
    }
    if (size == 4) {
        emit_term("stw");
    }
    emit_term("r0");
    emit_term("0");
    emit_term("r1");
    emit_newline();

    return result;
}

/**
 * Calculates the arithmetic factor of the given type and emits it for the given
 * register.
 *
 * Pointers of word size (e.g. `int*`) have an arithmetic factor of four, which
 * means the other operand in an addition or multiplication must be multiplied
 * by four.
 *
 * For pointers this is essentially the sizeof() of the pointed-to type. We
 * handle a few extra special cases here.
 */
static void compile_arithmetic_factor(const type_t* type, int register_num, bool multiply) {

    // We can't perform arithmetic on void or void*.
    if (*type == TYPE_BASIC_VOID) {
        fatal("Cannot perform arithmetic on value of `void` type.");
    }
    if (*type == (TYPE_BASIC_VOID|1)) {
        fatal("Cannot perform arithmetic on value of `void*` type.");
    }

    // All other scalar types have arithmetic factor of 1.
    if (type_indirections(type) == 0) {
        return;
    }

    // char* has a factor of 1.
    if (*type == (TYPE_BASIC_CHAR|1)) {
        return;
    }

    // Otherwise it's a pointer to something other than char (i.e. it could be
    // int*, char**, void**, etc.) The arithmetic factor is 4 so we shift
    // (signed) by 2. If we're multiplying, emit shl; if we're diving, emit
    // shrs.
    if (multiply) {
        emit_term("shl");
    }
    if (!multiply) {
        emit_term("shrs");
    }
    emit_register(register_num);
    emit_register(register_num);
    emit_term("2");
    emit_newline();
}

type_t* compile_add_sub(bool add, type_t* left, type_t* right) {
    int left_indirections = type_indirections(left);
    int right_indirections = type_indirections(right);

    // If both sides are pointers, the types must match. We adjust for the
    // arithmetic factor afterwards.
    bool both = ((left_indirections > 0) & (right_indirections > 0));

    // Otherwise, at most one side is a pointer. Emit the arithmetic factors
    // now.
    if (!both) {
        // right is in register 0; multiply it by the arithmetic factor of left
        compile_arithmetic_factor(left, 0, true);
        // left is in register 1; multiply it by the arithmetic factor of right
        compile_arithmetic_factor(right, 1, true);
    }

    // Compute the result into r0
    if (add) {
        emit_term("add");
    }
    if (!add) {
        emit_term("sub");
    }
    emit_term("r0");
    emit_term("r1");
    emit_term("r0");
    emit_newline();

    // If both sides were pointers, emit the arithmetic factor on the result.
    if (both) {
        compile_arithmetic_factor(left, 0, false);

        // the return type is int.
        type_delete(left);
        type_delete(right);
        return type_new(TYPE_BASIC_INT, 0);
    }

    // Return whichever side is a pointer type.
    if (left_indirections) {
        type_delete(right);
        return left;
    }
    if (right_indirections) {
        type_delete(left);
        return right;
    }

    // If neither is a pointer, promote to int.
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

type_t* compile_boolean_not(void) {
    // TODO use isz
    emit_term("cmpu");
    emit_term("r0");
    emit_term("r0");
    emit_term("0");
    emit_newline();
    compile_cmp_to_true(); // TODO is this even necessary?
    return type_new(TYPE_BASIC_INT, 0);
}

type_t* compile_bitwise_not(void) {
    emit_term("not");
    emit_term("r0");
    emit_term("r0");
    emit_newline();
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

    // The right-hand side is always an r-value.
    right = compile_dereference_if_lvalue(right, 0);

    // Handle assignment first because it has a lot of special behaviour.
    if (0 == strcmp(op, "=")) {
        return compile_assign(left, right);
    }

    // For all remaining operations, the left type is also an r-value, and both
    // types should be promoted from char to int.
    left = compile_dereference_if_lvalue(left, 1);
    if (*left == TYPE_BASIC_CHAR) {
        left = compile_cast(left, type_new(TYPE_BASIC_INT, 0), 1);
    }
    if (*right == TYPE_BASIC_CHAR) {
        right = compile_cast(right, type_new(TYPE_BASIC_INT, 0), 0);
    }

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
        return compile_basic_op("divs");
    }
    if (0 == strcmp(op, "%")) {
        return compile_basic_op("mods");
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

type_t* compile_cast(type_t* current_type, type_t* desired_type, int register_num) {

    // if neither type is char, we do nothing.
    if ((*current_type != TYPE_BASIC_CHAR) & (*desired_type != TYPE_BASIC_CHAR)) {
        type_delete(current_type);
        return desired_type;
    }

    // casting to or from char does sign extension.
    
    // TODO use sxb in assembly, not implemented yet
    emit_term("shl");
    emit_register(register_num);
    emit_register(register_num);
    emit_term("24");
    emit_newline();
    emit_term("shrs");
    emit_register(register_num);
    emit_register(register_num);
    emit_term("24");
    emit_newline();

    type_delete(current_type);
    return desired_type;
}
