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

#include "parse_stmt.h"

#include "node.h"
#include "common.h"
#include "strings.h"
#include "parse_expr.h"
#include "parse_decl.h"
#include "lexer.h"
#include "token.h"
#include "scope.h"
#include "common.h"
#include "type.h"
#include "function.h"

static node_t* break_container;     // the node to break out of on a `break` statement
static node_t* continue_container;  // the node to restart on a `continue` statement

void parse_stmt_init(void) {
}

void parse_stmt_destroy(void) {
}

static node_t* parse_return(void) {
    type_t* expected = current_function->root->type;
    node_t* node_return = node_new_lexer(NODE_RETURN);

    if (lexer_accept(STR_SEMICOLON)) {
        if (!type_matches_base(expected, BASE_VOID)) {
            fatal_token(node_return->token,
                    "Expected a return value for function with non-`void` return type.");
        }
        node_return->type = type_new_base(BASE_VOID);

    } else {
        if (type_matches_base(expected, BASE_VOID)) {
            fatal_token(node_return->token,
                    "Cannot return a value from a function with `void` return type.");
        }

        node_t* expression = parse_expression();
        if (!type_equal(expression->type, expected)) {
            // need to insert a cast operator
            fatal("TODO return type implicit cast");
        }

        node_return->type = type_ref(expression->type);
        node_append(node_return, expression);
        lexer_expect(STR_SEMICOLON, "Expected `;` at end of `return` statement.");
    }
    return node_return;
}

static node_t* parse_if(void) {
    node_t* node_if = node_new_lexer(NODE_IF);
    node_if->type = type_new_base(BASE_VOID);

    lexer_expect(STR_PAREN_OPEN, "Expected `(` after `if`.");
    node_append(node_if, parse_predicate());
    lexer_expect(STR_PAREN_CLOSE, "Expected `)` after condition for `if`.");

    node_t* node_true = node_new(NODE_SEQUENCE);
    node_true->type = type_new_base(BASE_VOID);
    node_append(node_if, node_true);
    parse_statement(node_true, false);

    if (lexer_accept(STR_ELSE)) {
        node_t* node_false = node_new(NODE_SEQUENCE);
        node_false->type = type_new_base(BASE_VOID);
        node_append(node_if, node_false);
        parse_statement(node_false, false);
    }
    
    return node_if;
}

static node_t* parse_while(void) {
    node_t* node_while = node_new_lexer(NODE_WHILE);
    node_while->type = type_new_base(BASE_VOID);
    lexer_expect(STR_PAREN_OPEN, "Expected `(` after `while`.");
    node_append(node_while, parse_predicate());
    lexer_expect(STR_PAREN_CLOSE, "Expected `)` after condition for `while`.");

    node_t* old_break_container = break_container;
    node_t* old_continue_container = continue_container;
    break_container = node_while;
    continue_container = node_while;

    node_t* body = node_new(NODE_SEQUENCE);
    body->type = type_new_base(BASE_VOID);
    node_append(node_while, body);
    parse_statement(body, false);

    break_container = old_break_container;
    continue_container = old_continue_container;
    return node_while;
}

static node_t* parse_switch(void) {

    // parse up to the body
    node_t* node_switch = node_new_lexer(NODE_SWITCH);
    node_switch->type = type_new_base(BASE_VOID);
    lexer_expect(STR_PAREN_OPEN, "Expected `(` after `switch`.");
    node_t* expression = parse_expression();
    node_append(node_switch, expression);
    lexer_expect(STR_PAREN_CLOSE, "Expected `)` after expression for `switch`.");

    // make sure the expression is an integer type
    if (!type_is_base(expression->type))
        goto bad_type;
    switch (expression->type->base) {
        case BASE_BOOL:
        case BASE_CHAR:
        case BASE_SIGNED_CHAR:
        case BASE_UNSIGNED_CHAR:
        case BASE_SIGNED_SHORT:
        case BASE_UNSIGNED_SHORT:
        case BASE_SIGNED_INT:
        case BASE_UNSIGNED_INT:
        case BASE_SIGNED_LONG:
        case BASE_UNSIGNED_LONG:
        case BASE_SIGNED_LONG_LONG:
        case BASE_UNSIGNED_LONG_LONG:
            break;
        default:
            goto bad_type;
    }

    node_t* old_break_container = break_container;
    break_container = node_switch;

    node_t* body = node_new(NODE_SEQUENCE);
    body->type = type_new_base(BASE_VOID);
    node_append(node_switch, body);
    parse_statement(body, false);

    break_container = old_break_container;
    return node_switch;

bad_type:
    fatal_token(expression->token, "Expected `switch` expression to have integer type.");
}

static void parse_case(node_t* parent, bool declaration_allowed) {
    node_t* node_case = node_new_lexer(NODE_CASE);
    node_case->type = type_new_base(BASE_VOID);
    node_append(node_case, parse_expression());
    node_append(parent, node_case);
    lexer_expect(STR_COLON, "Expected `:` after expression for `case`.");

    // TODO we don't verify yet that the case value is a constant expression.
    // We'll do that in the tree optimization pass.
    // TODO we should do it here in addition to the optimization pass because
    // it can't just optimize to a constant, it must actually be a constant,
    // e.g. we can't use `static const int` variables in it

    // A case label is not technically its own statement; it is followed by a
    // statement. This matters e.g. in an unbraced switch.
        // TODO we've done this recursively like cci/1 for simplicity but this
        // could overflow the stack if there are a lot of consecutive case
        // labels. we should move the label parsing to a loop at the top of
        // parse_statement().
    parse_statement(parent, declaration_allowed);
}

static node_t* parse_break(node_t* parent) {
    if (!break_container)
        fatal_token(lexer_token, "Cannot `break` outside of a loop or switch.");
    node_t* node = node_new_lexer(NODE_BREAK);
    node->container = break_container;
    return node;
}

static node_t* parse_continue(node_t* parent) {
    if (!continue_container)
        fatal_token(lexer_token, "Cannot `continue` outside of a loop.");
    node_t* node = node_new_lexer(NODE_CONTINUE);
    node->container = continue_container;
    return node;
}

/**
 * Parses one statement.
 *
 * A statement can append multiple nodes to the given parent because labels and
 * switch cases are individual nodes. They are independent of the statement to
 * which they were attached in the syntax.
 */
void parse_statement(node_t* parent, bool declaration_allowed) {

    // Check for a declaration. A declaration is not really a statement but we
    // support them at any point in a block, and we support them after a label
    // or case as long as we're in a block.
    if (declaration_allowed) {
        if (try_parse_declaration(parent)) {
            return;
        }
    }

    if (lexer_accept(STR_SEMICOLON)) {
        // empty statement
        return;
    }
    if (lexer_is(STR_BRACE_OPEN)) {
        node_append(parent, parse_compound_statement());
        return;
    }

    if (lexer_token->type == token_type_alphanumeric) {

        // check for keyword statements
        if (lexer_is(STR_IF)) { node_append(parent, parse_if()); return; }
        if (lexer_is(STR_WHILE)) { node_append(parent, parse_while()); return; }
        //if (lexer_is(STR_DO)) { node_append(parent, parse_do()); return; }
        //if (lexer_is(STR_FOR)) { node_append(parent, parse_for()); return; }
        if (lexer_is(STR_SWITCH)) { node_append(parent, parse_switch()); return; }
        if (lexer_is(STR_CASE)) { parse_case(parent, declaration_allowed); return; }
        //if (lexer_is(STR_DEFAULT)) { node_append(parent, parse_default(declaration_allowed); }
        if (lexer_is(STR_BREAK)) { node_append(parent, parse_break(parent)); return; }
        if (lexer_is(STR_CONTINUE)) { node_append(parent, parse_continue(parent)); return; }
        if (lexer_is(STR_RETURN)) { node_append(parent, parse_return()); return; }
        //if (lexer_is(STR_GOTO)) { node_append(parent, parse_goto()); return; }

        // check for a label
        if (declaration_allowed) {
        }
    }

    // parse an expression
    node_append(parent, parse_expression());
    lexer_expect(STR_SEMICOLON, "Expected `;` at end of expression statement.");
}

node_t* parse_compound_statement(void) {
    assert(lexer_is(STR_BRACE_OPEN));
    scope_push();
    node_t* node = node_new_lexer(NODE_SEQUENCE);
    node->type = type_new_base(BASE_VOID);
    while (!lexer_accept(STR_BRACE_CLOSE)) {
        parse_statement(node, true);
    }
    scope_pop();
    return node;
}
