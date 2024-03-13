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

static type_t* parse_assignment_expression(void);
static type_t* parse_identifier_expression(void);

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
 * operator.
 */
static int binary_operator_precedence(void) {
    if (lexer_type != lexer_type_punctuation) {
        return -1;
    }
    const char* op = lexer_token;
    if (0 == strcmp(op, "||")) {return 1;}  // logical or
    if (0 == strcmp(op, "&&")) {return 2;}  // logical and
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
    if (0 == strcmp(op, "%"))  {return 10;} // ...              // TODO maybe remove
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
        // TODO move this into an integer literal parsing function
        *type = type_new_base(BASE_SIGNED_INT);
        char* end;
        errno = 0;
        *value = strtol(lexer_token, &end, 0);
        if (errno != 0) {
            fatal("Error parsing number");
        }
        if (*end != 0) {
            fatal("Unrecognized characters after number (number literal suffixes are not supported in opC)");
        }
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
        //if (0 == strcmp(op, "%"))  {*value = (left_value %  right_value); continue;} // TODO maybe remove
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
        type_t* type = compile_immediate(lexer_token);
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
    int arg_count;
    arg_count = 0;

    while (!lexer_accept(")")) {
        if (arg_count > 0) {
            lexer_expect(",", NULL);
        }
        arg_count = (arg_count + 1);

        if (arg_count > 4) {
            // TODO support more args in stage 1, not stage 0
            fatal("TODO only four arguments are supported.");
        }

        //printf("   current token is %s\n",lexer_token);
        type_t* type = parse_assignment_expression();
        //printf("   current token is %s\n",lexer_token);

        // if the argument is an l-value, dereference it
        type = compile_lvalue_to_rvalue(type, 0);

        // TODO type-check the argument, for now ignore it
        type_delete(type);

        // TODO a simple optimization here (for stage 1, not stage 0) is, if
        // there are at most 4 args, don't push the last arg, just move it to
        // the correct register. (there's a lot of "push r0 pop r0" for
        // single-arg functions)
        // TODO if we want to make it even simpler, just skip the push/pop for
        // single-argument functions.

        // push the argument to the stack
        compile_push(0);
    }

    // TODO another simple optimization is to not use pop, instead use ldw for
    // each arg, then adjust the stack manually. this will be more important
    // when we have more than 4 args and we can't actually pop the args beyond 4

    // pop the arguments
    if (arg_count > 3) {
        compile_pop(3);
    }
    if (arg_count > 2) {
        compile_pop(2);
    }
    if (arg_count > 1) {
        compile_pop(1);
    }
    if (arg_count > 0) {
        compile_pop(0);
    }

    // emit the call
    emit_term("call");
    emit_label('^', name);
    emit_newline();

    // find the function
    global_t* global = global_find(name);
    if (global == NULL) {
        fatal_2("Function not declared: ", name);
    }
    if (!global_is_function(global)) {
        fatal_2("Called symbol is not a function: ", name);
    }

    //printf("   done parsing function call %s\n", name);
    return type_clone(global_type(global));
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
        ret = parse_function_call(name);
    }
    if (!paren) {
        ret = compile_load_variable(name);
    }
    free(name);
    return ret;
}

static type_t* parse_record_access(type_t* type) {
    const field_t* field = record_find_field(type_record(type), lexer_token);
    lexer_consume();
    compile_offset(field_offset(field));

    type_delete(type);
    type = type_clone(field_type(field));
    type_set_lvalue(type, true);
    return type;
}

static type_t* parse_record_value_access(type_t* type) {
    if (lexer_type != lexer_type_alphanumeric) {
        fatal("Expected a struct or union field name after `.`");
    }
    if ((!type_is_lvalue(type) | (type_indirections(type) != 0)) | !type_is_record(type)) {
        fatal("`.` can only be used on a struct or union pointer.");
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
    if ((!type_is_lvalue(type) | (type_indirections(type) != 1)) | !type_is_record(type)) {
        fatal("`.` can only be used on a struct or union pointer.");
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
            fatal("++ not yet implemented.");
            continue;
        }

        // post-decrement
        if (lexer_accept("--")) {
            fatal("-- not yet implemented.");
            continue;
        }

        break;
    }

    return type;
}

static type_t* parse_sizeof(void) {

    // Check for sizeof(type). The type must be in parentheses. (The type
    // declaration must be abstract.)
    bool paren = lexer_accept("(");
    if (paren) {
        type_t* type;
        if (try_parse_declaration(NULL, &type, NULL)) {
            lexer_expect(")", "Expected `)` after `sizeof(type`");
            return compile_sizeof(type);
        }
    }

    // Otherwise it's a (possibly parenthesized) expression. Parse it with
    // compilation disabled; we only want to do type resolution.
    // Note that the expression in sizeof isn't allowed to be a cast. We don't
    // bother to check this at the moment.
    bool was_enabled = compile_is_enabled();
    compile_set_enabled(false);
    type_t* type = parse_expression();
    compile_set_enabled(was_enabled);
    if (paren) {
        lexer_expect(")", "Expected `)` after `sizeof(expression`");
    }

    return compile_sizeof(type);
}

type_t* parse_unary_expression(void) {
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
        type_t* type = parse_unary_expression();
        compile_lvalue_to_rvalue(type, 0);
        type_delete(type);
        return compile_boolean_not();
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
        fatal("++ not yet implemented.");
    }

    if (lexer_accept("--")) {
        fatal("-- not yet implemented.");
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
        type_t* right = parse_unary_expression();
        compile_pop(1);

        // compile it
        left = compile_binary_op(op, left, right);
        free(op);
    }

    return left;
}

type_t* parse_conditional_expression(void) {
    type_t* predicate_type = parse_binary_expression(0);
    if (!lexer_accept("?")) {
        return predicate_type;
    }

    // we have a conditional expression.
    int false_label = parse_generate_label();
    int end_label = parse_generate_label();

    // if the type is an lvalue we need to dereference it
    compile_lvalue_to_rvalue(predicate_type, 0);
    // TODO compile a cast to int, need to convert e.g. llong to int, fail on
    // structs, etc.
    type_delete(predicate_type);

    // parse true branch
    compile_jump_if_zero(false_label);
    type_t* true_type = parse_expression();
    compile_jump(end_label);
    lexer_expect(":", "Expected `:` in conditional expression");

    // parse false branch
    compile_label(false_label);
    type_t* false_type = parse_conditional_expression();
    compile_label(end_label);

    // TODO make sure types are compatible
    type_delete(false_type);

    // the return value cannot be assigned.
    //compile_lvalue_to_rvalue(true_type, 0);
    return true_type;
}

static type_t* parse_assignment_expression(void) {
    type_t* left = parse_conditional_expression();
    if (!is_assign_op()) {
        return left;
    }
    char* op = lexer_take();

    // The left side of an assignment must be an l-value and can only be a
    // unary expression. Luckily, nothing in between this and a unary
    // expression can return an l-value so we can just parse a conditional
    // expression and compile_assign() will check that it's an l-value.

    // parse the right-hand side
    // the right side will be in r0, the left will be in r1
    compile_push(0);
    type_t* right = parse_unary_expression();
    compile_pop(1);

    type_t* type = compile_assign(op, left, right);
    free(op);
    return type;
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
