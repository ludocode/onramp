#include "parse-expr.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "common.h"
#include "parse-decl.h"
#include "lexer.h"

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
