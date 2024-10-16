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
#include "options.h"
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
static node_t* switch_container;    // the node of the containing `switch` statement
static node_t* switch_list;         // the previous case/default in a `switch`, or the switch itself

static void parse_statement(node_t* parent, bool cast_to_void);

void parse_stmt_init(void) {
}

void parse_stmt_destroy(void) {
}

static node_t* parse_return(void) {
    type_t* expected = current_function->root->type;
    node_t* node_return = node_new_lexer(NODE_RETURN);
    node_return->type = type_new_base(BASE_VOID);

    if (lexer_accept(STR_SEMICOLON)) {
        if (!type_matches_base(expected, BASE_VOID)) {
            fatal_token(node_return->token,
                    "Expected a return value for function with non-`void` return type.");
        }

    } else {
        if (type_matches_base(expected, BASE_VOID)) {
            fatal_token(node_return->token,
                    "Cannot return a value from a function with `void` return type.");
        }

        node_t* expression = parse_expression();
        if (!type_equal(expression->type, expected)) {
            // TODO for now we just insert a cast and let codegen fail if it's
            // bad. Probably we should check here that the implicit cast is
            // valid, at least for better error messages.
            expression = node_cast(expression, expected, NULL);
        }

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
    parse_statement(node_true, true);

    if (lexer_accept(STR_ELSE)) {
        node_t* node_false = node_new(NODE_SEQUENCE);
        node_false->type = type_new_base(BASE_VOID);
        node_append(node_if, node_false);
        parse_statement(node_false, true);
    }

    return node_if;
}

/*
 * Parses the body of a while, do or for loop.
 *
 * A break in the expression or other clauses of a loop (in a statement
 * expression) breaks out of the *outer* loop/switch. We push and pop the break
 * container only around the loop body.
 */
static void parse_loop_body(node_t* loop) {
    node_t* old_break_container = break_container;
    node_t* old_continue_container = continue_container;
    break_container = loop;
    continue_container = loop;

    node_t* body = node_new(NODE_SEQUENCE);
    body->type = type_new_base(BASE_VOID);
    node_append(loop, body);
    parse_statement(body, true);

    break_container = old_break_container;
    continue_container = old_continue_container;
}

static node_t* parse_while(void) {
    node_t* node_while = node_new_lexer(NODE_WHILE);
    node_while->type = type_new_base(BASE_VOID);

    lexer_expect(STR_PAREN_OPEN, "Expected `(` after `while`.");
    node_append(node_while, parse_predicate());
    lexer_expect(STR_PAREN_CLOSE, "Expected `)` after condition for `while`.");

    parse_loop_body(node_while);
    return node_while;
}

static node_t* parse_do(void) {
    node_t* node_do = node_new_lexer(NODE_DO);
    node_do->type = type_new_base(BASE_VOID);

    parse_loop_body(node_do);

    lexer_expect(STR_WHILE, "Expected `while` after statement of `do` loop.");
    lexer_expect(STR_PAREN_OPEN, "Expected `(` after `while`.");
    node_append(node_do, parse_predicate());
    lexer_expect(STR_PAREN_CLOSE, "Expected `)` after condition for `while`.");

    return node_do;
}

static node_t* parse_for(void) {
    node_t* node_for = node_new_lexer(NODE_FOR);
    node_for->type = type_new_base(BASE_VOID);

    lexer_expect(STR_PAREN_OPEN, "Expected `(` after `for`.");

    // parse clause-1, declaration or expression
    scope_push(); // TODO don't push in gnu89
    if (lexer_accept(STR_SEMICOLON)) {
        node_append(node_for, node_new_noop());
    } else {
        // TODO check if commas (multiple declarators) are allowed in declaration
        if (!try_parse_declaration(node_for)) {
            node_append(node_for, parse_expression());
            lexer_expect(STR_SEMICOLON, "Expected `;` after first clause of `for` loop.");
        }
    }

    // parse clause-2, predicate expression
    node_append(node_for, lexer_is(STR_SEMICOLON) ?
        node_new_noop() : parse_predicate());
    lexer_expect(STR_SEMICOLON, "Expected `;` after second clause of `for` loop.");

    // parse clause-3, arbitrary expression (usually increment)
    node_append(node_for, lexer_is(STR_PAREN_CLOSE) ?
        node_new_noop() : parse_expression());
    lexer_expect(STR_PAREN_CLOSE, "Expected `)` after third clause of `for` loop.");

    // break in the for loop clauses (in a statement expression) breaks out of
    // the *outer* loop/switch. we push the break container after parsing it.

    parse_loop_body(node_for);
    scope_pop();
    return node_for;
}

static node_t* parse_switch(void) {

    // parse up to the body
    node_t* node_switch = node_new_lexer(NODE_SWITCH);
    node_switch->type = type_new_base(BASE_VOID);
    lexer_expect(STR_PAREN_OPEN, "Expected `(` after `switch`.");
    node_t* expression = parse_expression();
    lexer_expect(STR_PAREN_CLOSE, "Expected `)` after expression for `switch`.");

    // make sure the expression is an integer or enum type
    if (!type_is_base(expression->type))
        goto bad_type;
    if (!type_matches_base(expression->type, BASE_ENUM) && !type_is_integer(expression->type))
        goto bad_type;

    // promote it
    expression = node_promote(expression);
    node_append(node_switch, expression);

    // `break`, `case`, `default` in a statement expression inside the value
    // expression of a switch associate with the outer container, not this
    // switch. We push the old containers after parsing the expression above.
    node_t* old_break_container = break_container;
    break_container = node_switch;
    node_t* old_switch_container = switch_container;
    switch_container = node_switch;
    node_t* old_switch_list = switch_list;
    switch_list = node_switch;

    node_t* body = node_new(NODE_SEQUENCE);
    body->type = type_new_base(BASE_VOID);
    node_append(node_switch, body);
    parse_statement(body, true);

    switch_container = old_switch_container;
    switch_list = old_switch_list;
    break_container = old_break_container;
    return node_switch;

bad_type:
    fatal_token(expression->token, "Expected `switch` expression to have integer type.");
}

static node_t* parse_break(node_t* parent) {
    if (!break_container)
        fatal_token(lexer_token, "Cannot `break` outside of a loop or switch.");
    node_t* node = node_new_lexer(NODE_BREAK);
    node->container = break_container;
    node->type = type_new_base(BASE_VOID);
    return node;
}

static node_t* parse_continue(node_t* parent) {
    if (!continue_container)
        fatal_token(lexer_token, "Cannot `continue` outside of a loop.");
    node_t* node = node_new_lexer(NODE_CONTINUE);
    node->container = continue_container;
    node->type = type_new_base(BASE_VOID);
    return node;
}

static void parse_case(node_t* parent) {
    node_t* node = node_new_lexer(NODE_CASE);
    node->type = type_new_base(BASE_VOID);
    node_append(parent, node);

    // store our case/default statements in a linked list
    if (switch_container == NULL) {
        fatal_token(lexer_token, "Cannot use `case` outside of a `switch` statement.");
    }
    switch_list->next_case = node;
    switch_list = node;

    // parse constant expression (cast to switch expression type)
    type_t* type = switch_container->first_child->type;
    node_t* start = node_cast(parse_constant_expression(), type, NULL);
    // TODO llong expr, for now assume 32-bit
    node->start32 = node_eval_32(start);

    // parse optional case range
    node_t* end = NULL;
    if (lexer_is(STR_ELLIPSIS)) {
        warn(warning_gnu_case_range, lexer_token, "Case ranges are a GNU extension.");
        lexer_consume();
        end = node_cast(parse_constant_expression(), type, NULL);
        node->end32 = node_eval_32(end);
    } else {
        node->end32 = node->start32;
    }

    // We attach the case expressions for debugging purposes (so the user can
    // view them in a tree dump for example.) If the user requested
    // optimization we discard them right away to reduce memory usage.
    if (optimization) {
        node_delete(start);
        if (end)
            node_delete(end);
    } else {
        node_append(node, start);
        if (end)
            node_append(node, end);
    }

    lexer_expect(STR_COLON, "Expected `:` after expression for `case`.");
}

static void parse_default(node_t* parent) {
    node_t* node = node_new_lexer(NODE_DEFAULT);
    node->type = type_new_base(BASE_VOID);
    node_append(parent, node);

    // store our case/default statements in a linked list
    if (switch_list == NULL) {
        fatal_token(lexer_token, "Cannot use `default` outside of a `switch` statement.");
    }
    switch_list->next_case = node;
    switch_list = node;

    lexer_expect(STR_COLON, "Expected `:` after `default`.");
}

static void parse_label(node_t* parent, token_t* name) {
    // TODO ensure name is not a keyword
    // TODO ensure label is not already defined in this function. We'll need to
    // add a table of labels to the function.
    node_t* node = node_new_token(NODE_LABEL, name);
    node->type = type_new_base(BASE_VOID);
    node_append(parent, node);
}

static bool parse_labels(node_t* parent) {
    bool found_label = false;

    for (;;) {
        if (lexer_is(STR_CASE)) {
            parse_case(parent);
            found_label = true;
            continue;
        }
        if (lexer_is(STR_DEFAULT)) {
            parse_default(parent);
            found_label = true;
            continue;
        }
        if (lexer_token->type == token_type_alphanumeric) {
            token_t* name = lexer_take();
            if (!lexer_accept(STR_COLON)) {
                lexer_push(name);
                break;
            }
            parse_label(parent, name);
            token_deref(name);
            found_label = true;
            continue;
        }
        break;
    }

    if (found_label && lexer_is(STR_BRACE_CLOSE)) {
        // allow labels at the end of a block
        // TODO this is C2x only, warn otherwise
    }

    return found_label;
}

static node_t* parse_goto(void) {
    lexer_consume();
    if (lexer_token->type != token_type_alphanumeric) {
        fatal_token(lexer_token, "`goto` must be followed by a label name.");
    }

    // TODO make sure it's not a keyword

    node_t* node = node_new_lexer(NODE_GOTO);
    node->type = type_new_base(BASE_VOID);
    lexer_expect(STR_SEMICOLON, "Expected `;` at end of `goto`.");
    return node;
}

/**
 * Parses a statement, not including any leading labels (which are parsed by
 * parse_labels().)
 */
static void parse_statement_no_labels(node_t* parent, bool cast_to_void) {

    // Empty statement
    if (lexer_accept(STR_SEMICOLON)) {
        return;
    }

    // Compound statement
    if (lexer_is(STR_BRACE_OPEN)) {
        node_append(parent, parse_compound_statement(true));
        return;
    }

    // Keyword statement
    if (lexer_token->type == token_type_alphanumeric) {
        if (lexer_is(STR_IF)) { node_append(parent, parse_if()); return; }
        if (lexer_is(STR_WHILE)) { node_append(parent, parse_while()); return; }
        if (lexer_is(STR_DO)) { node_append(parent, parse_do()); return; }
        if (lexer_is(STR_FOR)) { node_append(parent, parse_for()); return; }
        if (lexer_is(STR_SWITCH)) { node_append(parent, parse_switch()); return; }
        if (lexer_is(STR_BREAK)) { node_append(parent, parse_break(parent)); return; }
        if (lexer_is(STR_CONTINUE)) { node_append(parent, parse_continue(parent)); return; }
        if (lexer_is(STR_RETURN)) { node_append(parent, parse_return()); return; }
        if (lexer_is(STR_GOTO)) { node_append(parent, parse_goto()); return; }
    }

    // Expression statement
    node_t* expr = parse_expression();
    if (cast_to_void)
        expr = node_cast_base(expr, BASE_VOID, NULL);
    node_append(parent, expr);
    lexer_expect(STR_SEMICOLON, "Expected `;` at end of expression statement.");
}

/**
 * Parses one statement.
 *
 * A statement can append multiple nodes to the given parent because labels
 * (including `case` and `default` labels) are individual nodes.
 *
 * If `cast_to_void` is true, all added nodes will have type void, which is
 * the typical case. This is false in a statement expression since the type of
 * the last statement is the type of the statement expression.
 */
static void parse_statement(node_t* parent, bool cast_to_void) {
    parse_labels(parent);
    parse_statement_no_labels(parent, cast_to_void);
}

void parse_declaration_or_statement(node_t* parent, bool cast_to_void) {
    bool found_label = parse_labels(parent);

    if (try_parse_declaration(parent)) {
        if (found_label) {
            // TODO C2x allows this
            fatal_token(parent->last_child->token,
                    "A label cannot be followed by a declaration. (Add `;` after the label.)");
        }
        return;
    }

    parse_statement_no_labels(parent, cast_to_void);
}

node_t* parse_compound_statement(bool create_scope) {
    assert(lexer_is(STR_BRACE_OPEN));

    if (create_scope) {
        scope_push();
    }

    node_t* node = node_new_lexer(NODE_SEQUENCE);
    node->type = type_new_base(BASE_VOID);
    while (!lexer_is(STR_BRACE_CLOSE)) {
        parse_declaration_or_statement(node, true);
    }
    node->end_token = lexer_take();

    if (create_scope) {
        scope_pop();
    }

    return node;
}
