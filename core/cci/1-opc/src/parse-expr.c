/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
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

#include "parse-expr.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "common.h"
#include "global.h"
#include "parse-stmt.h"
#include "compile.h"
#include "lexer.h"
#include "emit.h"  // TODO remove all emit calls in this code
#include "record.h"
#include "field.h"

static type_t* parse_unary_expression(void);
static type_t* parse_identifier_expression(void);
static type_t* parse_post_inc_dec_operator(type_t* var_type, const char* binary_op);

static char* stashed_identifier;

void parse_expr_init(void) {
    // nothing
}

void parse_expr_destroy(void) {
    if (stashed_identifier != NULL) {
        fatal("Internal error: leaked expression identifier");
    }
}

void parse_expr_stash_identifier(char* identifier) {
    if (stashed_identifier != NULL) {
        fatal("Internal error: leaked expression identifier");
    }
    stashed_identifier = identifier;
}



/**
 * Returns the precedence of the current token, or -1 if it's not a binary
 * precedence operator.
 *
 * Note that we don't include &&, ||, or any assignment operators. We don't
 * consider these to be binary precedence operators because they have special
 * handling.
 */
static int binary_operator_precedence(void) {
    if (lexer_type != lexer_type_punctuation) {
        return -1;
    }
    const char* op = lexer_token;
    if (0 == strcmp(op, "|"))  {return 3;}  // inclusive or
    if (0 == strcmp(op, "^"))  {return 4;}  // exclusive or
    if (0 == strcmp(op, "&"))  {return 5;}  // bitwise and
    if (0 == strcmp(op, "==")) {return 6;}  // equality
    if (0 == strcmp(op, "!=")) {return 6;}  // ...
    if (0 == strcmp(op, "<=")) {return 7;}  // relational
    if (0 == strcmp(op, ">=")) {return 7;}  // ...
    if (0 == strcmp(op, "<"))  {return 7;}  // ...
    if (0 == strcmp(op, ">"))  {return 7;}  // ...
    if (0 == strcmp(op, "<<")) {return 8;}  // bitwise shift
    if (0 == strcmp(op, ">>")) {return 8;}  // ...
    if (0 == strcmp(op, "+"))  {return 9;}  // additive
    if (0 == strcmp(op, "-"))  {return 9;}  // ...
    if (0 == strcmp(op, "*"))  {return 10;} // multiplicative
    if (0 == strcmp(op, "/"))  {return 10;} // ...
    if (0 == strcmp(op, "%"))  {return 10;} // ...
    return -1;
}

/**
 * Returns true if the current token is an assignment operator.
 */
static bool is_assign_op(void) {
    if (lexer_type != lexer_type_punctuation) {
        return false;
    }
    const char* op = lexer_token;
    if (0 == strcmp(op, "=")) {return true;}
    if (0 == strcmp(op, "+=")) {return true;}
    if (0 == strcmp(op, "-=")) {return true;}
    if (0 == strcmp(op, "*=")) {return true;}
    if (0 == strcmp(op, "/=")) {return true;}
    if (0 == strcmp(op, "%=")) {return true;}
    if (0 == strcmp(op, "&=")) {return true;}
    if (0 == strcmp(op, "|=")) {return true;}
    if (0 == strcmp(op, "^=")) {return true;}
    if (0 == strcmp(op, "<<=")) {return true;}
    if (0 == strcmp(op, ">>=")) {return true;}
    return false;
}

/**
 * Returns true if the current token is a unary operator.
 */
static bool is_unary_operator(void) {
    if (lexer_type != lexer_type_punctuation) {
        return false;
    }
    const char* op = lexer_token;
    if (0 == strcmp(op, "&")) {return true;}
    if (0 == strcmp(op, "*")) {return true;}
    if (0 == strcmp(op, "+")) {return true;}
    if (0 == strcmp(op, "-")) {return true;}
    if (0 == strcmp(op, "~")) {return true;}
    if (0 == strcmp(op, "!")) {return true;}
    return false;
}

/**
 * Parses a number.
 *
 * omC doesn't support unsigned so unsigned numbers will be wrapped to the
 * range of a signed int. We don't currently bother to check that the range is
 * valid.
 *
 * This implementation is based mostly on strtol() with some simplications and
 * the addition of suffix parsing.
 */
static void parse_number(const char* p, type_t** out_type, int* out_value) {
    int base = 0;

    // detecting leading 0x/0X for hex
    if (*p == '0') {
        char x = *(p + 1);
        if ((x == 'x') | (x == 'X')) {
            base = 16;
            p = (p + 2);
        }
    }

    // detect leading binary 0b/0B for binary
    if (base == 0) {
        if (*p == '0') {
            char b = *(p + 1);
            if ((b == 'b') | (b == 'B')) {
                base = 2;
                p = (p + 2);
            }
        }
    }

    // detect leading 0 for octal
    if (base == 0) {
        if (*p == '0') {
            base = 8;
        }
    }

    // otherwise assume decimal
    if (base == 0) {
        base = 10;
    }

    // accumulate digits
    long value = 0;
    while (1) {
        // TODO hex_to_int in libo
        long digit = 99;
        if ((*p >= '0') & (*p <= '9')) {
            digit = (*p - '0');
        }
        if ((*p >= 'a') & (*p <= 'f')) {
            digit = ((*p - 'a') + 10);
        }
        if ((*p >= 'A') & (*p <= 'F')) {
            digit = ((*p - 'A') + 10);
        }
        if (digit >= base) {
            break;
        }

        // Although omC doesn't support unsigned, this still works for unsigned
        // numbers because of two's complement. We just can't (easily) check
        // for overflow so we don't bother.
        value = ((value * base) + digit);

        p = (p + 1);
    }

    // parse out the suffix
    bool suffix_unsigned = false;
    bool suffix_long = false;
    while (*p) {

        // parse long
        if ((*p == 'l') | (*p == 'L')) {
            if (suffix_long) {
                fatal("Integer literal suffix for `long long` is not supported.");
            }
            suffix_long = true;
            p = (p + 1);
            continue;
        }

        // parse unsigned
        if ((*p == 'u') | (*p == 'U')) {
            if (suffix_unsigned) {
                fatal("Redundant `u` suffix on integer literal.");
            }
            suffix_unsigned = true;
            p = (p + 1);
            continue;
        }

        // unrecognized. try to give slightly better error mesagge
        if (((*p == '.') | ((*p == 'e') | (*p == 'E'))) |
                ((*p == 'p') | (*p == 'P')))
        {
            fatal("Floating point literals are not supported.");
        }
        fatal("Malformed number literal.");
    }

    // we only care about signed/unsigned.
    if (suffix_unsigned) {
        *out_type = type_new_base(BASE_UNSIGNED_INT);
    }
    if (!suffix_unsigned) {
        *out_type = type_new_base(BASE_SIGNED_INT);
    }
    *out_value = value;
}



/*
 * Constant Expressions
 */

// Parses a parenthesized expression. The opening paren has already been consumed. 
bool parse_constant_parenthesized_expression(type_t** type, int* value) {
    if (!try_parse_constant_expression(type, value)) {
        fatal("Expected a constant expression.");
    }
    lexer_expect(")", "Expected ) after parenthesized constant expression");
    return true;
}

bool try_parse_constant_primary_expression(type_t** type, int* value) {

    // check for a number
    if (lexer_type == lexer_type_number) {
        parse_number(lexer_token, type, value);
        lexer_consume();
        return true;
    }

    // check for a character literal
    if (lexer_type == lexer_type_character) {
        *type = type_new_base(BASE_SIGNED_INT);
        *value = *lexer_token;
        lexer_consume();
        return true;
    }

    // check for a parenthesized expression
    if (lexer_accept("(")) {
        return parse_constant_parenthesized_expression(type, value);
    }

    return false;
}

bool try_parse_constant_postfix_expression(type_t** type, int* value) {

    // TODO check for array and struct dereference

    return try_parse_constant_primary_expression(type, value);
}

bool try_parse_constant_unary_expression(type_t** type, int* value) {
    if (is_unary_operator()) {
        char* op = lexer_take();

        type_t* right_type;
        int right_value;
        if (!try_parse_constant_unary_expression(&right_type, &right_value)) {
            fatal_2("Expected a constant expression after unary operator ", op);
        }

        if (0 == strcmp(op, "&")) {
            fatal("TODO unary & in constant expression");
        }
        if (0 == strcmp(op, "*")) {
            fatal("TODO unary * in constant expression");
        }
        if (0 == strcmp(op, "+")) {
            *type = type_set_lvalue(right_type, false);
        }
        if (0 == strcmp(op, "-")) {
            *type = type_set_lvalue(right_type, false);
            *value = -right_value;
        }
        if (0 == strcmp(op, "~")) {
            *type = type_set_lvalue(right_type, false);
            *value = ~right_value;
        }
        if (0 == strcmp(op, "!")) {
            *type = type_set_lvalue(right_type, false);
            *value = ~right_value;
        }
        free(op);
        return true;
    }

    if (lexer_accept("sizeof")) {
        lexer_expect("(", "`sizeof` must be followed by `(` in a constant expression.");
        type_t* sizeof_type;
        if (!try_parse_declaration(NULL, &sizeof_type, NULL)) {
            fatal("`sizeof(` must be followed by a type in a constant expression.");
        }
        lexer_expect(")", "`sizeof(type` must be followed by `)` in a constant expression.");
        *type = type_new_base(BASE_SIGNED_INT); // TODO unsigned
        *value = type_size(sizeof_type);
        type_delete(sizeof_type);
        return true;
    }

    return try_parse_constant_postfix_expression(type, value);
}

bool try_parse_constant_cast_expression(type_t** type, int* value) {
    if (!lexer_accept("(")) {
        return try_parse_constant_unary_expression(type, value);
    }

    // we have parens. see if they contain a type declaration
    type_t* desired_type;
    if (!try_parse_declaration(NULL, &desired_type, NULL)) {
        // not a type; it's a parenthesize expression.
        return parse_constant_parenthesized_expression(type, value);
    }

    // we have a cast expression.
    type_t* actual_type;
    if (!try_parse_constant_cast_expression(&actual_type, value)) {
        fatal("Expected a constant expression.");
    }

    // TODO make sure types are compatible. For now we assume they are.
    *type = desired_type;
    type_delete(actual_type);
    return true;
}

bool try_parse_constant_binary_expression(type_t** type, int* value, int min_precedence) {
    if (!try_parse_constant_cast_expression(type, value)) {
        return false;
    }

    // In order to simplify our memory management here, we always jump back to
    // the top of the loop and we free the previous op at the top.
    char* op = NULL;
    while (1) {
        free(op);
        int left_value = *value;

        // get a binary operator (of appropriate precedence)
        int op_precedence = binary_operator_precedence();
        if (op_precedence < min_precedence) {
            break;
        }
        op = lexer_take();

        // get the right side
        type_t* right_type;
        int right_value;
        if (!try_parse_constant_binary_expression(&right_type, &right_value, op_precedence + 1)) {
            fatal_2("Expected constant expression after operator: ", op);
        }

        // promote types
        // TODO for now just return int
        type_delete(*type);
        type_delete(right_type);
        *type = type_new_base(BASE_SIGNED_INT);

        // do the math
        if ((0 == strcmp(op, "|")) | (0 == strcmp(op, "||"))) {
            *value = (!!left_value | !!right_value);
            continue;
        }
        if ((0 == strcmp(op, "&")) | (0 == strcmp(op, "&&"))) {
            *value = (!!left_value & !!right_value);
            continue;
        }
        // TODO if both types are unsigned int this should be an unsigned
        // operation, not a signed one. Many of these (comparisons, right
        // shift, divide and modulus) will give incorrect values in some cases
        // for very large unsigned numbers. We don't actually have unsigned
        // operations in omC so we'd probably have to make wrapper functions in
        // assembly.
        if (0 == strcmp(op, "^"))  {*value = (left_value ^  right_value); continue;}
        if (0 == strcmp(op, "==")) {*value = (left_value == right_value); continue;}
        if (0 == strcmp(op, "!=")) {*value = (left_value != right_value); continue;}
        if (0 == strcmp(op, "<=")) {*value = (left_value <= right_value); continue;}
        if (0 == strcmp(op, ">=")) {*value = (left_value >= right_value); continue;}
        if (0 == strcmp(op, "<"))  {*value = (left_value <  right_value); continue;}
        if (0 == strcmp(op, ">"))  {*value = (left_value >  right_value); continue;}
        if (0 == strcmp(op, "+"))  {*value = (left_value +  right_value); continue;}
        if (0 == strcmp(op, "-"))  {*value = (left_value -  right_value); continue;}
        if (0 == strcmp(op, "*"))  {*value = (left_value *  right_value); continue;}
        if (0 == strcmp(op, "/"))  {*value = (left_value /  right_value); continue;}
        if (0 == strcmp(op, "%"))  {*value = (left_value %  right_value); continue;}
        if (0 == strcmp(op, "<<")) {*value = (left_value << right_value); continue;}
        if (0 == strcmp(op, ">>")) {*value = (left_value >> right_value); continue;}
        fatal_2("Internal error: invalid binary operator: ", op);
    }

    return true;
}

bool try_parse_constant_conditional_expression(type_t** type, int* value) {
    if (!try_parse_constant_binary_expression(type, value, 0)) {
        return false;
    }
    if (!lexer_accept("?")) {
        return true;
    }
    // We have a conditional expression.
    type_delete(*type);
    int predicate = *value;

    // Parse the left side
    type_t* left_type;
    int left_value;
    if (!try_parse_constant_expression(&left_type, &left_value)) {
        fatal("`?` here must be followed by a constant expression.");
    }

    lexer_expect(":", "Expected `:` in constant conditional expression");

    // Parse the right side
    type_t* right_type;
    int right_value;
    if (!try_parse_constant_conditional_expression(&right_type, &right_value)) {
        fatal("`:` here must be followed by a constant conditional expression.");
    }

    // Return left if the predicate is true, right otherwise.
    if (predicate) {
        type_delete(right_type);
        *type = left_type;
        *value = left_value;
        return true;
    }
    type_delete(left_type);
    *type = right_type;
    *value = right_value;
    return true;
}

bool try_parse_constant_expression(type_t** type, int* value) {
    return try_parse_constant_conditional_expression(type, value);
}



/*
 * Normal (non-constant) Expressions
 */

static type_t* parse_primary_expression(void) {

    // an alphanumeric is the name of a variable or a function call
    if ((stashed_identifier != NULL) | (lexer_type == lexer_type_alphanumeric)) {
        return parse_identifier_expression();
    }

    // parenthesis
    if (lexer_accept("(")) {

        // Check for a cast expression. (The type declaration must be abstract.)
        type_t* desired_type;
        if (try_parse_declaration(NULL, &desired_type, NULL)) {
            lexer_expect(")", "Expected ) after type in cast");

            // Parse the expression to be cast and cast it
            type_t* current_type = parse_unary_expression();
            current_type = compile_lvalue_to_rvalue(current_type, 0);
            return compile_cast(current_type, desired_type, 0);
        }

        // Otherwise we have a parenthesized expression.
        type_t* type = parse_expression();
        lexer_expect(")", "Expected ) after parenthesized expression");
        return type;
    }

    // number
    if (lexer_type == lexer_type_number) {
        int value;
        type_t* type;
        parse_number(lexer_token, &type, &value);
        compile_immediate(value);
        lexer_consume();
        return type;
    }

    // character literal
    if (lexer_type == lexer_type_character) {
        // We don't support any escape characters yet so we can just send it
        // through.
        type_t* type = compile_character_literal(*lexer_token);
        lexer_consume();
        return type;
    }

    // string
    if (lexer_type == lexer_type_string) {
        compile_string_literal_invocation(store_string_literal());
        return type_increment_pointers(type_new_base(BASE_SIGNED_CHAR));
    }

    fatal("Expected expression");
}

static type_t* parse_function_call(const char* name) {
    //printf("   parsing function call %s\n", name);

    // find the function
    const global_t* global = global_find(name);
    if (global == NULL) {
        fatal_2("No such function: ", name);
    }
    if (!global_is_function(global)) {
        fatal_2("Called global is not a function: ", name);
    }
    int param_count = global_function_param_count(global);
    bool is_variadic = global_function_is_variadic(global);

    // all our arguments will be pushed to the stack. we're going to track how
    // many words we've pushed so we can find our argument list as we go.
    int stack_count = 0;

    // collect the arguments onto the stack
    int index = 0;
    while (!lexer_accept(")")) {
        if (index > 0) {
            lexer_expect(",", NULL);
        }

        // get the argument
        type_t* type = parse_assignment_expression();
        type = compile_promote(type, 0);
        compile_push(0);
        stack_count = (stack_count + 1);

        // check for errors
        if (index < param_count) {
            if (!type_is_compatible(type, global_function_param_type(global, index))) {
                fatal("Incompatible argument in function call.");
            }
        }
        if (index >= param_count) {
            if (!is_variadic) {
                fatal("Too many arguments in function call.");
            }
        }

        type_delete(type);
        index = (index + 1);
    }
    int arg_count = index;

    // place leading non-variadic arguments into registers
    index = 0;
    while (index < 4) {
        if (index == param_count) {
            break;
        }
        if (index == arg_count) {
            break;
        }
        compile_stack_offset(true, ((stack_count - index) - 1) << 2, index);
        index = (index + 1);
    }

    // all remaining arguments are pushed to the stack in reverse order
    int register_args = index;
    index = arg_count;
    while (index > register_args) {
        index = (index - 1);
        compile_stack_offset(true, ((stack_count - index) - 1) << 2, 9);
        compile_push(9);
        stack_count = (stack_count + 1);
    }

    // emit the call
    emit_term("call");
    emit_label('^', name);
    emit_newline();

    // pop everything from the stack
    compile_stack_shift(stack_count << 2);

    //printf("   done parsing function call %s\n", name);
    return type_clone(global_type(global));
}

static type_t* parse_va_list(void) {
    type_t* type = parse_assignment_expression();
    if (!type_is_base_pointer(type, BASE_SIGNED_INT)) {
        fatal("Expected a va_list.");
    }
    return type;
}

static type_t* parse_builtin_va_arg(void) {
    type_t* list = parse_va_list();
    if (!type_is_lvalue(list)) {
        fatal("Expected a va_list l-value.");
    }

    lexer_expect(",", "Expected a second argument to va_arg().");

    type_t* desired_type;
    if (!try_parse_declaration(NULL, &desired_type, NULL)) {
        fatal("Expected a type as the second argument to va_arg().");
    }

    // decrement the list
    compile_push(0);
    compile_mov(1, 0);
    type_t* int_type = compile_immediate_signed_int(1);
    type_t* value_type = compile_binary_op("+", type_clone(list), int_type);
    compile_pop(1);
    type_delete(compile_assign(type_clone(list), value_type));

    // load the value at the list of the given type
    list = type_decrement_indirection(list);
    compile_dereference(list, 0);

    lexer_expect(")", "Expected `)` after the second argument to va_arg().");
    return compile_cast(list, desired_type, 0);
}

static type_t* parse_builtin_va_start(void) {
    type_t* list = parse_va_list();
    if (!type_is_lvalue(list)) {
        fatal("Expected a va_list l-value.");
    }

    lexer_expect(",", "Expected a second argument to va_start().");

    // For now we ignore the second argument without compiling it. We'd have to
    // check that it's a local and that it matches the correct parameter. We
    // don't currently track the name of the last parameter or which locals
    // match which parameters and it's not worth doing just for this.
    compile_inhibit_push();
    type_delete(parse_expression());
    compile_inhibit_pop();

    // Store the address of one below the first variadic argument.
    int offset = (global_function_param_count(current_function) - 4);
    if (offset < 0) {
        offset = 0;
    }
    offset = ((offset + 1) << 2);
    compile_mov(1, 0);
    compile_frame_offset(false, offset, 0);
    type_delete(compile_assign(list, type_set_lvalue(type_clone(list), false)));

    lexer_expect(")", "Expected `)` after the second argument to va_start().");
    return type_new_base(BASE_VOID);
}

static type_t* parse_builtin_va_end(void) {
    // This does nothing. We still evaluate the argument for side-effects.
    type_delete(parse_va_list());
    lexer_expect(")", "Expected `)` after the argument to va_end().");
    return type_new_base(BASE_VOID);
}

static type_t* parse_builtin_va_copy(void) {
    type_t* left = parse_va_list();
    if (!type_is_lvalue(left)) {
        fatal("Expected a va_list l-value.");
    }

    lexer_expect(",", "Expected a second argument to va_start().");

    compile_push(0);
    type_t* right = parse_va_list();
    compile_pop(1);
    type_delete(compile_assign(left, right));

    lexer_expect(")", "Expected `)` after the second argument to va_copy().");
    return type_new_base(BASE_VOID);
}

static bool try_parse_builtin(const char* name, type_t** out_type) {
    if (0 == strcmp(name, "__builtin_va_arg")) {
        *out_type = parse_builtin_va_arg();
        return true;
    }
    if (0 == strcmp(name, "__builtin_va_start")) {
        *out_type = parse_builtin_va_start();
        return true;
    }
    if (0 == strcmp(name, "__builtin_va_end")) {
        *out_type = parse_builtin_va_end();
        return true;
    }
    if (0 == strcmp(name, "__builtin_va_copy")) {
        *out_type = parse_builtin_va_copy();
        return true;
    }
    return false;
}

static type_t* parse_identifier_expression(void) {

    // if we've stashed an identifier due to tentatively parsing a label, we
    // use it here.
    char* name;
    if (stashed_identifier == NULL) {
        name = lexer_take();
    }
    if (stashed_identifier != NULL) {
        name = stashed_identifier;
        stashed_identifier = NULL;
    }

    type_t* ret;
    bool paren = lexer_accept("(");
    if (paren) {
        if (!try_parse_builtin(name, &ret)) {
            ret = parse_function_call(name);
        }
    }
    if (!paren) {
        ret = compile_load_variable(name);
    }
    free(name);
    return ret;
}

static type_t* parse_record_access(type_t* type) {
    size_t offset;
    const field_t* field = record_find_field(type_record(type), lexer_token, &offset);
    if (field == NULL) {
        fatal_2("Struct or union has no such field: ", lexer_token);
    }
    lexer_consume();
    compile_offset(offset);

    type_delete(type);
    type = type_clone(field_type(field));
    type_set_lvalue(type, true);
    return type;
}

static type_t* parse_record_value_access(type_t* type) {
    if (lexer_type != lexer_type_alphanumeric) {
        fatal("Expected a struct or union field name after `.`");
    }
    if ((!type_is_lvalue(type) | (type_indirections(type) != 0)) | (type_base(type) != BASE_RECORD)) {
        fatal("`.` can only be used on a struct or union value.");
    }
    return parse_record_access(type);
}

static type_t* parse_record_pointer_access(type_t* type) {
    if (lexer_type != lexer_type_alphanumeric) {
        fatal("Expected a struct or union field name after `.`");
    }
    // Note: We just check that indirections is 1, which means `->` works on an
    // array of structs. Apparently this is normal as neither GCC and Clang
    // complain when `->` is used this way, but strictly speaking, the spec
    // says it's supposed to only be used on pointers.
    if ((type_indirections(type) != 1) | (type_base(type) != BASE_RECORD)) {
        fatal("`->` can only be used on a struct or union pointer.");
    }

    type = compile_operator_dereference(type);
    return parse_record_access(type);
}

static type_t* parse_postfix_expression(void) {

    // A postfix expression starts with a primary expression. (This also
    // handles function calls since we don't support function pointers.)
    type_t* type = parse_primary_expression();

    // Check for postfix operators
    while (1) {

        // array indexing
        if (lexer_accept("[")) {
            compile_push(0);
            type_t* right = parse_expression();
            lexer_expect("]", "Expected `]` after `[` expression.");
            compile_pop(1);
            type = compile_binary_op("+", type, right);
            type = compile_operator_dereference(type);
            continue;
        }

        // record value access
        if (lexer_accept(".")) {
            type = parse_record_value_access(type);
            continue;
        }

        // record pointer access
        if (lexer_accept("->")) {
            type = parse_record_pointer_access(type);
            continue;
        }

        // post-increment
        if (lexer_accept("++")) {
            type = parse_post_inc_dec_operator(type, "+");
            continue;
        }

        // post-decrement
        if (lexer_accept("--")) {
            type = parse_post_inc_dec_operator(type, "-");
            continue;
        }

        break;
    }

    return type;
}

static type_t* parse_sizeof(void) {

    // sizeof without parens has high precedence. We only consume a unary
    // expression.
    if (!lexer_accept("(")) {
        compile_inhibit_push();
        type_t* type = parse_unary_expression();
        compile_inhibit_pop();
        return compile_sizeof(type);
    }

    // Check for sizeof(type). The type declaration must be abstract.
    type_t* type;
    if (try_parse_declaration(NULL, &type, NULL)) {
        lexer_expect(")", "Expected `)` after `sizeof(type`");
        return compile_sizeof(type);
    }

    // Otherwise it's a parenthesized expression.
    compile_inhibit_push();
    type = parse_expression();
    compile_inhibit_pop();
    lexer_expect(")", "Expected `)` after `sizeof(expression`");
    return compile_sizeof(type);

}

// Parses post-increment and post-decrement operators.
static type_t* parse_post_inc_dec_operator(type_t* var_type, const char* binary_op) {
    if (!type_is_lvalue(var_type)) {
        fatal("Cannot post-increment or post-decrement an r-value.");
    }

    // store the original value
    compile_mov(1, 0);
    type_t* ret = compile_lvalue_to_rvalue(type_clone(var_type), 0);
    compile_push(0);

    // do the increment
    compile_push(1);
    type_t* int_type = compile_immediate_signed_int(1);
    type_t* value_type = compile_binary_op(binary_op, type_clone(var_type), int_type);
    compile_pop(1);
    type_delete(compile_assign(var_type, value_type));

    // return the original value
    compile_pop(0);
    return ret;
}

// Parses pre-increment and pre-decrement operators.
static type_t* parse_pre_inc_dec_operator(const char* binary_op) {
    type_t* var_type = parse_unary_expression();
    if (!type_is_lvalue(var_type)) {
        fatal("Cannot pre-increment or pre-decrement an r-value.");
    }

    compile_push(0);
    compile_mov(1, 0);
    type_t* int_type = compile_immediate_signed_int(1);
    type_t* value_type = compile_binary_op(binary_op, type_clone(var_type), int_type);
    compile_pop(1);
    return compile_assign(var_type, value_type);
}

static type_t* parse_unary_expression(void) {
    if (stashed_identifier) {
        return parse_postfix_expression();
    }

    if (lexer_accept("+")) {
        // TODO we should remove l-value and promote to int
        return parse_unary_expression();
    }

    if (lexer_accept("-")) {
        // TODO unary minus is not allowed on pointers. We don't bother to
        // check this; we just return the same type.
        type_t* type = parse_unary_expression();
        compile_lvalue_to_rvalue(type, 0);
        emit_term("sub");
        emit_term("r0");
        emit_term("0");
        emit_term("r0");
        emit_newline();
        return type;
    }

    if (lexer_accept("*")) {
        type_t* type = parse_unary_expression();
        if (type_indirections(type) == 0) {
            fatal("Type to dereference is not a pointer");
        }
        type = compile_operator_dereference(type);
        return type;
    }

    if (lexer_accept("&")) {
        type_t* type = parse_unary_expression();
        if (!type_is_lvalue(type)) {
            fatal("Cannot take the address of an r-value");
        }
        type_set_lvalue(type, false);
        if (type_is_array(type)) {
            // The `&` operator on an array just converts it to a pointer; the
            // overall indirection count stays the same.
            type_set_array_length(type, TYPE_ARRAY_NONE);
        }
        type_increment_pointers(type);
        return type;
    }

    if (lexer_accept("!")) {
        // We optimize `!!` here.
        bool doubled = lexer_accept("!");
        type_t* type = parse_unary_expression();
        compile_lvalue_to_rvalue(type, 0);
        type_delete(type);
        if (doubled) {
            compile_boolean_cast();
        }
        if (!doubled) {
            compile_boolean_not();
        }
        return type_new_base(BASE_SIGNED_INT);
    }

    if (lexer_accept("~")) {
        type_t* type = parse_unary_expression();
        compile_lvalue_to_rvalue(type, 0);
        type_delete(type);
        return compile_bitwise_not();
    }
    
    if (lexer_accept("sizeof")) {
        return parse_sizeof();
    }

    if (lexer_accept("++")) {
        return parse_pre_inc_dec_operator("+");
    }

    if (lexer_accept("--")) {
        return parse_pre_inc_dec_operator("-");
    }

    return parse_postfix_expression();
}

static type_t* parse_binary_expression(int min_precedence) {
    type_t* left = parse_unary_expression();

    while (1) {
        // get a binary operator (of appropriate precedence)
        int op_precedence = binary_operator_precedence();
        if (op_precedence < min_precedence) {
            break;
        }
        char* op = lexer_take();

        // parse the right-hand side
        // the right side will be in r0, the left will be in r1
        compile_push(0);
        type_t* right = parse_binary_expression(op_precedence + 1);
        compile_pop(1);

        // compile it
        left = compile_binary_op(op, left, right);
        free(op);
    }

    return left;
}

type_t* parse_logical_and_expression(void) {
    type_t* left = parse_binary_expression(0);
    if (!lexer_is("&&")) {
        return left;
    }

    int skip_label = parse_generate_label();

    while (lexer_accept("&&")) {
        left = compile_promote(left, 0);
        compile_jump_if_zero(skip_label);

        type_t* right = compile_promote(parse_logical_and_expression(), 0);

        if (!type_is_compatible(left, right)) {
            fatal("Incompatible types in `||` operator.");
        }

        type_delete(left);
        left = right;
    }

    compile_label(skip_label);
    compile_boolean_cast();
    type_delete(left);
    return type_new_base(BASE_SIGNED_INT);
}

type_t* parse_logical_or_expression(void) {
    type_t* left = parse_logical_and_expression();
    if (!lexer_is("||")) {
        return left;
    }

    int skip_label = parse_generate_label();

    while (lexer_accept("||")) {
        left = compile_promote(left, 0);
        compile_jump_if_not_zero(skip_label);

        type_t* right = compile_promote(parse_logical_and_expression(), 0);

        if (!type_is_compatible(left, right)) {
            fatal("Incompatible types in `||` operator.");
        }

        type_delete(left);
        left = right;
    }

    compile_label(skip_label);
    compile_boolean_cast();
    type_delete(left);
    return type_new_base(BASE_SIGNED_INT);
}

type_t* parse_conditional_expression(void) {
    type_t* predicate_type = parse_logical_or_expression();
    if (!lexer_accept("?")) {
        return predicate_type;
    }

    // we have a conditional expression.
    int false_label = parse_generate_label();
    int end_label = parse_generate_label();

    // if the type is an lvalue we need to dereference it
    type_delete(compile_lvalue_to_rvalue(predicate_type, 0));
    compile_jump_if_zero(false_label);

    // parse true branch
    type_t* true_type = compile_promote(parse_expression(), 0);
    compile_jump(end_label);
    lexer_expect(":", "Expected `:` in conditional expression");

    // parse false branch
    compile_label(false_label);
    type_t* false_type = compile_promote(parse_conditional_expression(), 0);
    compile_label(end_label);

    if (!type_is_compatible(true_type, false_type)) {
        fatal("Incompatible types in branches of conditional expression.");
    }

    // We should preserve pointer types and we should be signed if either side
    // was signed. Otherwise we don't care because we already promoted both.
    if (type_indirections(false_type) > 0) {
        type_delete(true_type);
        return false_type;
    }
    if (type_is_signed(false_type)) {
        type_delete(true_type);
        return false_type;
    }
    type_delete(false_type);
    return true_type;
}

type_t* parse_assignment_expression(void) {
    type_t* left = parse_conditional_expression();
    if (!is_assign_op()) {
        return left;
    }
    char* op = lexer_take();

    // The left side of an assignment must be an l-value and can only be a
    // unary expression. Luckily, nothing in between this and a unary
    // expression can return an l-value so we can just parse a conditional
    // expression and compile_assign() will check that it's an l-value.

    // Parse the right-hand side
    compile_push(0);
    type_t* right = parse_assignment_expression();

    // If it's a compound assigment, calculate the result
    if (0 != strcmp(op, "=")) {
        type_t* temp = type_clone(left);
        compile_stack_offset(true, 0, 1);
        *(op + (strlen(op) - 1)) = 0; // remove the `=`
        right = compile_binary_op(op, temp, right);
    }
    free(op);

    // Do the assignment
    compile_pop(1);
    return compile_assign(left, right);
}

static type_t* parse_comma_expression(void) {
    type_t* type = parse_assignment_expression();
    while (lexer_accept(",")) {
        type_delete(type);
        type = parse_assignment_expression();
    }
    return type;
}

type_t* parse_expression(void) {
    return parse_comma_expression();
}
