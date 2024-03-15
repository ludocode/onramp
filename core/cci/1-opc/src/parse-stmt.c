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

#include "parse-stmt.h"

#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "lexer.h"
#include "locals.h"
#include "parse-decl.h"
#include "global.h"
#include "compile.h"
#include "parse-expr.h"
#include "types.h"

static void parse_block(void);
static void parse_statement(bool declaration_allowed);
static bool try_parse_local_declaration(void);

// statement state
static int last_label;
static int continue_label;
static int break_label;
static int case_label;
static int default_label;
static int switch_offset;
static bool default_used;

// string generation
static int next_string;
static char** strings;
static size_t strings_count;
#define STRINGS_MAX 128

// function generation
static int function_frame_size;
static bool inside_function;
static const char* function_name;

void parse_stmt_init(void) {
    last_label = -1;
    continue_label = -1;
    break_label = -1;
    case_label = -1;
    default_label = -1;
    strings = malloc(sizeof(char*) * STRINGS_MAX);
}

void parse_stmt_destroy(void) {
    free(strings);
}

int parse_generate_label(void) {
    last_label = (last_label + 1);
    return last_label;
}



/*
 * String Literals
 */

int store_string_literal(void) {
    if (strings_count == STRINGS_MAX) {
        fatal("Too many strings in this function");
    }
    int string_id = (next_string + strings_count);
    *(strings + strings_count) = lexer_take();
    strings_count = (strings_count + 1);
    return string_id;
}

void output_string_literals(void) {
    size_t i = 0;
    while (i < strings_count) {
        compile_string_literal_definition(next_string, *(strings + i));
        next_string = (next_string + 1);
        free(*(strings + i));
        i = (i + 1);
    }
    strings_count = 0;
}



/*
 * Statements
 */

static void parse_condition(int false_label) {

    // collect the expression
    lexer_expect("(", "Expected `(` to start condition of branch or loop");
    type_t* type = parse_expression();
    lexer_expect(")", "Expected `)` to end condition of branch or loop");

    // if the type is an lvalue we need to dereference it
    compile_lvalue_to_rvalue(type, 0);

    // if the expression is false, jump to the given label
    compile_jump_if_zero(false_label);

    type_delete(type);
}

static void parse_if(void) {

    // parse a condition. if true, skip the if block.
    int skip_if_label = parse_generate_label();
    parse_condition(skip_if_label);

    parse_statement(false);

    // check for else
    bool has_else = lexer_accept("else");
    if (has_else) {

        // we're still in the if block, so skip the else block.
        int skip_else_label = parse_generate_label();
        compile_jump(skip_else_label);

        // our else block starts here, which we do only if we skipped if
        compile_label(skip_if_label);
        parse_statement(false);
        compile_label(skip_else_label);
    }

    // if we don't have else, just emit the skip label.
    if (!has_else) {
        compile_label(skip_if_label);
    }
}

static void parse_while(void) {

    // push labels of containing loop (for break/continue)
    int old_start_label = continue_label;
    int old_end_label = break_label;
    continue_label = parse_generate_label();
    break_label = parse_generate_label();

    // parse and compile the loop
    compile_label(continue_label);
    parse_condition(break_label);
    parse_statement(false);
    compile_jump(continue_label);
    compile_label(break_label);

    // pop labels
    continue_label = old_start_label;
    break_label = old_end_label;
}

static void parse_do(void) {

    // push labels of containing loop
    int old_start_label = continue_label;
    int old_end_label = break_label;
    continue_label = parse_generate_label();
    break_label = parse_generate_label();

    // parse and compile the loop
    compile_label(continue_label);
    parse_statement(false);
    lexer_expect("while", "Expected `while` after `do` statement.");
    parse_condition(break_label);
    lexer_expect(";", "Expected `;` after `do`-`while` condition.");
    compile_jump(continue_label);
    compile_label(break_label);

    // pop labels
    continue_label = old_start_label;
    break_label = old_end_label;
}

static void parse_for(void) {

    // push labels of containing loop
    int old_start_label = continue_label;
    int old_end_label = break_label;
    continue_label = parse_generate_label();
    break_label = parse_generate_label();

    // push locals count. we want to pop our variable definition when done.
    int previous_locals_count = locals_count;

    // parse the initialization clause
    lexer_expect("(", "Expected `(` after `for`");
    if (!lexer_accept(";")) {
        if (!try_parse_local_declaration()) {
            type_delete(parse_expression());
        }
        lexer_expect(";", "Expected `(` after initialization clause of `for`");
    }

    // parse the condition expression
    compile_label(continue_label);
    if (!lexer_accept(";")) {
        type_t* type = parse_expression();
        compile_lvalue_to_rvalue(type, 0);
        compile_jump_if_zero(break_label);
        type_delete(type);
        lexer_expect(";", "Expected `(` after condition clause of `for`");
    }

    // Parse the increment expression. This only happens at the end of the loop
    // but we need to emit it now because we're compiling in a single pass, so
    // we set up these circuitous jumps around it to run the increment at the
    // appropriate time.
    int loop_contents_label = parse_generate_label();
    int loop_increment_label = parse_generate_label();
    compile_jump(loop_contents_label);
    compile_label(loop_increment_label);
    if (!lexer_accept(")")) {
        type_delete(parse_expression());
        lexer_expect(")", "Expected `)` after increment clause of `for`");
    }
    compile_jump(continue_label);

    // parse and compile the loop
    compile_label(loop_contents_label);
    parse_statement(false);
    compile_jump(loop_increment_label);
    compile_label(break_label);

    // pop locals
    locals_pop(previous_locals_count);

    // pop labels
    continue_label = old_start_label;
    break_label = old_end_label;

}

static void parse_break(void) {
    if (!lexer_accept(";")) {
        fatal("Expected `;` after break.");
    }
    if (break_label == -1) {
        fatal("Cannot `break` outside of loop or switch.");
    }
    compile_jump(break_label);
}

static void parse_continue(void) {
    if (!lexer_accept(";")) {
        fatal("Expected `;` after continue.");
    }
    if (continue_label == -1) {
        fatal("Cannot `continue` outside of loop.");
    }
    compile_jump(continue_label);
}

static void parse_return(void) {
    int argument = !lexer_is(";");

    if (argument) {
        type_t* type = parse_expression();
        compile_lvalue_to_rvalue(type, 0);
        // The expression result is in r0 which is our return value.
        // TODO for now assume it's correct, we should be casting it to the return type
        type_delete(type);
    }
    if (!argument) {
        // Without an argument, we still have to return zero if this is main().
        if (0 == strcmp(function_name, "main")) {
            compile_zero();
        }
    }
    compile_return();
    lexer_expect(";", "Expected `;` at end of `return` statement");
}

static void parse_switch(void) {

    // Push the outer info
    int previous_switch_offset = switch_offset;
    int previous_default_label = default_label;
    bool previous_default_used = default_used;
    bool previous_break_label = break_label;
    default_label = -1;
    default_used = -1;
    break_label = parse_generate_label();

    // Compile the expression into r0
    lexer_expect("(", "Expected `(` after `switch`");
    type_t* expr_type = parse_expression();
    lexer_expect(")", "Expected `)` after expression of `switch`");

    // We need to store the expression result so we can check it against each
    // case statement. For this we use an anonymous variable.
    switch_offset = locals_add(strdup_checked(""), type_new_base(BASE_SIGNED_INT));
    compile_load_frame_offset(switch_offset, 1);
    type_t* var = type_new_base(BASE_SIGNED_INT);
    type_set_lvalue(var, true);
    type_delete(compile_assign(var, expr_type));

    // Emit a jump to the first `case` statement. The cases form a linked list;
    // each one tests the expression and either runs the code or jumps to the
    // next case statement.
    case_label = parse_generate_label();
    compile_jump(case_label);

    // Parse and compile the contents of the switch
    parse_statement(false);
    compile_jump(break_label);

    // If we found a default label and none of the cases matched, jump to it
    // now. We're done.
    compile_label(case_label);
    if (default_label != -1) {
        compile_jump(default_label);
    }
    compile_label(break_label);

    // Pop outer info
    break_label = previous_break_label;
    default_used = previous_default_used;
    default_label = previous_default_label;
    switch_offset = previous_switch_offset;
}

static void parse_case(bool declaration_allowed) {
    
    // A case statement compares its constant expression to the stored
    // expression of the switch. Since code passing through the switch needs to
    // fallthrough case statements, we also emit a jump around it.
    int run_label = parse_generate_label();
    compile_jump(run_label);

    // Case statements form a linked list.
    compile_label(case_label);
    case_label = parse_generate_label();

    // Parse the constant expression of the case statement, put it in r0
    type_t* case_type;
    int value;
    if (!try_parse_constant_expression(&case_type, &value)) {
        fatal("Expected a constant expression after `case`.");
    }
    lexer_expect(":", "Expected `:` after `case` expression.");
    type_delete(compile_immediate_int(value));

    // Get the switch value in r1
    compile_load_frame_offset(switch_offset, 1);
    type_t* switch_type = type_new_base(BASE_SIGNED_INT);
    type_set_lvalue(switch_type, true);
    compile_lvalue_to_rvalue(switch_type, 1);

    // Compare them
    type_delete(compile_binary_op("!=", switch_type, case_type));
    compile_jump_if_zero(run_label);

    // If this case didn't match, we jump to the next one.
    compile_jump(case_label);
    compile_label(run_label);

    // As with labels, a case can be followed by a statement and together they
    // form one statement. This is important when a case is used in an unbraced
    // switch or other statement. We also allow case at the end of blocks so
    // as a hack we just check for a closing brace.
    if (!lexer_is("}")) {
        parse_statement(declaration_allowed);
    }
}

static void parse_default(void) {
    if (case_label == -1) {
        fatal("Cannot use `default` outside of a switch.");
    }
    if (default_label != -1) {
        fatal("`default` has already been used in this switch.");
    }
    default_label = parse_generate_label();
    compile_label(default_label);
    lexer_expect(":", "Expected `:` after `default`.");
}

static void parse_goto(void) {
    if (lexer_type != lexer_type_alphanumeric) {
        fatal("Expected an identifier after `goto`");
    }
    compile_goto(function_name, lexer_token);
    lexer_consume();
    lexer_expect(";", "Expected `;` after `goto` label");
}

static void parse_statement(bool declaration_allowed) {

    // Check for a declaration. A declaration is not really a statement but we
    // support them at any point in a block, and we support them after a label
    // or case as long as we're in a block.
    if (declaration_allowed) {
        if (try_parse_local_declaration()) {
            return;
        }
    }

    if (lexer_accept(";")) {
        // empty statement
        return;
    }
    if (lexer_is("{")) {
        parse_block();
        return;
    }

    if (lexer_type == lexer_type_alphanumeric) {

        // check for keyword statements
        if (lexer_accept("if")) { parse_if(); return; }
        if (lexer_accept("while")) { parse_while(); return; }
        if (lexer_accept("do")) { parse_do(); return; }
        if (lexer_accept("for")) { parse_for(); return; }
        if (lexer_accept("switch")) { parse_switch(); return; }
        if (lexer_accept("case")) { parse_case(declaration_allowed); return; }
        if (lexer_accept("default")) { parse_default(); return; }
        if (lexer_accept("break")) { parse_break(); return; }
        if (lexer_accept("continue")) { parse_continue(); return; }
        if (lexer_accept("return")) { parse_return(); return; }
        if (lexer_accept("goto")) { parse_goto(); return; }

        // check for a label
        char* name = lexer_take();
        if (lexer_accept(":")) {
            compile_user_label(function_name, name);
            free(name);

            // A label can be followed by a statement, and together they form
            // one statement. This is important when a label is used in an
            // unbraced if, switch, etc. We also want labels to be allowed at
            // the end of blocks so as a hack we just check for a closing
            // brace.
            if (!lexer_is("}")) {
                parse_statement(declaration_allowed);
            }
            return;
        }
        
        // It's not a keyword or label. It's an expression that begins with an
        // identifier. Stash the identifier in the expression parser so that it
        // will find it.
        parse_expr_stash_identifier(name);
    }

    // parse an expression, discard the result
    type_delete(parse_expression());
    lexer_expect(";", "Expected `;` at end of expression statement.");
}

// Takes ownership of the given type
static void parse_local_declaration(type_t* type, char* /*nullable*/ name) {
    if (name == NULL) {
        // We ignore useless variable declarations.
        type_delete(type);
    }

    //printf("defining new variable %s\n",name);
    locals_add(name, type);

    if (lexer_accept(";")) {
        // No initializer.
        return;
    }
    type = type_clone(type);

    if (!lexer_accept("=")) {
        fatal("Expected `;` or `=` after local variable declaration.");
    }

    // The variable has an initializer. Parse it, then generate an assignment
    // operation.

    // compile and push the variable
    type_t* var_type = compile_load_variable(name);
    compile_push(0);

    // TODO for now ignoring var_type
    type_delete(var_type);

    // get the assignment expression
    // TODO need to rewrite initializer code to support arrays, structs
    type_t* expr_type = parse_assignment_expression();

    // pop the destination
    compile_pop(1);

    // compile an assignment
    type_set_lvalue(type, true);
    type_delete(compile_assign(type, expr_type));
}

// Tries to parse a declaration of a local variable plus its initializer.
static bool try_parse_local_declaration(void) {
    // TODO we need to do this separately to handle commas, like parse_global()
    type_t* type;
    char* name;
    if (try_parse_declaration(NULL, &type, &name)) {
        parse_local_declaration(type, name);
        return true;
    }
    return false;
}

static bool try_parse_block(void) {
    if (!lexer_accept("{")) {
        return false;
    }

    int previous_locals_count = locals_count;

    while (!lexer_accept("}")) {
        parse_statement(true);
    }

    // track the maximum extent of the function's stack frame
    int frame_size;
    frame_size = locals_frame_size();
    if (function_frame_size < frame_size) {
        function_frame_size = frame_size;
    }

    locals_pop(previous_locals_count);

    return true;
}

static void parse_block(void) {
    if (!try_parse_block()) {
        fatal("Expected {");
    }
}

// Parses a function declaration (and definition, if provided.)
static void parse_function_declaration(type_t* return_type, char* name, storage_t storage) {
    inside_function = true;
    int arg_count = 0;

    while (!lexer_accept(")")) {
        if (arg_count > 0) {
            lexer_expect(",", "Expected , or ) after argument");
        }

        // parse parameter
        type_t* arg_type;
        char* arg_name;
        if (!try_parse_declaration(NULL, &arg_type, &arg_name)) {
            fatal("Expected a function parameter declaration");
        }

        // check for (void) parameter list
        if (((arg_name == NULL) & (arg_count == 0)) & type_is_base(arg_type, BASE_VOID)) {
            if (lexer_accept(")")) {
                type_delete(arg_type);
                break;
            }
        }

        // If we don't have a name, for now we give it an anonymous name. We'll
        // still reserve stack space for it and move the argument into it. This
        // is simpler than trying to optimize away stack storage for unnamed
        // parameters.
        if (arg_name == NULL) {
            arg_name = strdup("");
        }

        // add variable
        locals_add(arg_name, arg_type);
        arg_count = (arg_count + 1);
    }

    // TODO pass arg_types, arg_count to function
    const global_t* global = global_declare_function(return_type, name, 0, NULL);
    function_name = global_name(global);

    //printf("   parsed function declaration %s\n",name);
    if (!lexer_accept(";")) {
        //printf("   parsing function body\n");

        // compile the function
        function_frame_size = 0;
        //printf("==%i\n",previous_locals_count);
        //dump_variables();
        compile_function_open(global_name(global), arg_count);
        parse_block();
        compile_function_close(global_name(global), arg_count, function_frame_size, storage);

        // output any strings that were used in the function
        output_string_literals();
        compile_global_divider();
    }

    function_name = NULL;
    inside_function = false;
    locals_pop(0);
}

void parse_global(void) {
    storage_t storage;
    type_t* base_type;
    if (!try_parse_declaration_specifiers(&base_type, &storage)) {
        fatal("Expected a global declaration.");
    }

    while (1) {
        type_t* type;
        char* name;
        if (!try_parse_declarator(base_type, &type, &name)) {
            fatal("Expected a declarator for this global declaration.");
        }

        // A no-name declaration is not necessarily an error, for example it
        // could be an enum.
        if (name == NULL) {
            type_delete(type);
        }
        if (name != NULL) {

            // Check for a typedef
            if (storage == STORAGE_TYPEDEF) {
                types_add_typedef(name, type);
                lexer_expect(";", "Expected `;` at end of typedef");
                break;
            }

            // Check for a function
            if (lexer_accept("(")) {
                parse_function_declaration(type, name, storage);
                break;
            }

            // Otherwise it's a global variable declaration
            if (lexer_accept("=")) {
                fatal("Global variable initializer is not yet implemented");
            }
            if (storage != STORAGE_EXTERN) {
                compile_global_variable(type, name, storage);
            }
            global_declare_variable(type, name);

        }

        // TODO check for a comma for multiple declarators, only allowed if none are functions
        if (lexer_is(",")) {
            fatal("Multiple declarators with `,` are not yet implemented.");
        }
        lexer_expect(";", "Expected `;` at end of global variable declaration.");
        break;
    }

    type_delete(base_type);
}

bool parse_is_inside_function(void) {
    return inside_function;
}
