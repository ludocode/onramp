#include "parse.h"

#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "emit.h"  // TODO remove all emit calls in this code
#include "common.h"
#include "compile.h"
#include "locals.h"
#include "global.h"
#include "parse-decl.h"
#include "parse-expr.h"

static void parse_block(void);
static void parse_statement(void);

// label generation
static int next_label;
static int while_start_label;
static int while_end_label;

static int function_frame_size;

void parse_init(void) {
    while_start_label = -1;
    while_end_label = -1;
}

void parse_destroy(void) {
}

static void parse_condition(int false_label) {

    // collect the expression
    lexer_expect("(", NULL);
    type_t* type = parse_expression();
    lexer_expect(")", NULL);

    // if the type is an lvalue we need to dereference it
    compile_lvalue_to_rvalue(type, 0);

    // if the expression is false, jump to the given label
    emit_term("jz");
    emit_term("r0");
    emit_computed_label('&', JUMP_LABEL_PREFIX, false_label);
    emit_newline();

    type_delete(type);
}

static void parse_if(void) {

    // parse a condition. if true, skip the if block.
    //emit_term("; if");
    //emit_newline();
    int skip_if_label = next_label;
    next_label = (next_label + 1);
    parse_condition(skip_if_label);

    parse_statement(); // TODO maybe not exactly right, not sure how labels work here

    // check for else
    bool has_else = lexer_accept("else");
    if (has_else) {

        // we're still in the if block, so skip the else block.
        int skip_else_label = next_label;
        next_label = (next_label + 1);
        emit_term("jmp");
        emit_computed_label('&', JUMP_LABEL_PREFIX, skip_else_label);
        emit_newline();

        // our else block starts here, which we do only if we skipped if
        emit_computed_label(':', JUMP_LABEL_PREFIX, skip_if_label);
        //emit_term("; else");
        emit_newline();
        parse_statement(); // TODO same here
        emit_computed_label(':', JUMP_LABEL_PREFIX, skip_else_label);
    }

    // if we don't have else, just emit the skip label.
    if (!has_else) {
        emit_computed_label(':', JUMP_LABEL_PREFIX, skip_if_label);
    }

    //emit_term("; end if");
    emit_newline();
}

static void parse_while(void) {

    // push labels of containing while block
    int old_start_label = while_start_label;
    int old_end_label = while_end_label;
    while_start_label = next_label;
    while_end_label = (next_label + 1);
    next_label = (next_label + 2);

    // emit the start label
    emit_computed_label(':', JUMP_LABEL_PREFIX, while_start_label);
    //emit_term("; while");
    emit_newline();

    // emit the condition check (jumps to the end if false)
    parse_condition(while_end_label);

    // emit the contents of the while
    parse_statement(); // TODO same as if here, not sure if should forbid labels or how

    // jump back to the start
    emit_term("jmp");
    emit_computed_label('&', JUMP_LABEL_PREFIX, while_start_label);
    emit_newline();

    // emit the end label
    emit_computed_label(':', JUMP_LABEL_PREFIX, while_end_label);
    //emit_term("; end while");
    emit_newline();

    // pop labels
    while_start_label = old_start_label;
    while_end_label = old_end_label;
}

static void parse_break(void) {
    if (!lexer_accept(";")) {
        fatal("Expected `;` after break.");
    }
    if (while_end_label == -1) {
        fatal("Cannot break outside of while loop.");
    }
    emit_term("jmp");
    emit_computed_label('&', JUMP_LABEL_PREFIX, while_end_label);
    emit_newline();
}

static void parse_continue(void) {
    if (!lexer_accept(";")) {
        fatal("Expected `;` after continue.");
    }
    if (while_end_label == -1) {
        fatal("Cannot continue outside of while loop.");
    }
    emit_term("jmp");
    emit_computed_label('&', JUMP_LABEL_PREFIX, while_start_label);
    emit_newline();
}

static void parse_return(void) {
    int argument = !lexer_accept(";");

    if (argument) {
        type_t* type = parse_expression();
        compile_lvalue_to_rvalue(type, 0);
        // The expression result is in r0 which is our return value.
        // TODO for now assume it's correct, we should be casting it to the return type
        type_delete(type);
        lexer_expect(";", "Expected `;` at end of `return` statement");
    }
    if (!argument) {
        // Without an argument, we still need to return zero in case this is
        // main().
        emit_term("zero");
        emit_term("r0");
        emit_newline();
    }
    emit_term("leave");
    emit_newline();
    emit_term("ret");
    emit_newline();
}

static void parse_statement(void) {
    if (lexer_accept(";")) {
        // empty statement
        return;
    }

    if (lexer_is("{")) {
        //scope_push();
        parse_block();
        //scope_pop();
        return;
    }

    if (lexer_accept("if")) {
        parse_if();
        return;
    }

    if (lexer_accept("while")) {
        parse_while();
        return;
    }

    if (lexer_accept("break")) {
        parse_break();
        return;
    }

    if (lexer_accept("continue")) {
        parse_continue();
        return;
    }

    if (lexer_accept("return")) {
        parse_return();
        return;
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
    emit_term("push");
    emit_term("r0");
    emit_newline();

    // TODO for now ignoring var_type
    type_delete(var_type);

    // get the assignment expression
    // TODO need to rewrite initializer code to support arrays, structs
    type_t* expr_type = parse_unary_expression();

    // pop the destination
    emit_term("pop");
    emit_term("r1");
    emit_newline();

    // compile an assignment
    type_set_lvalue(type, true);
    type_delete(compile_binary_op("=", type, expr_type));
}

static bool try_parse_block(void) {
    if (!lexer_accept("{")) {
        return false;
    }

    //emit_term("; block");
    //emit_newline();

    int previous_locals_count = locals_count;

    while (!lexer_accept("}")) {

        // Check for a declaration
        // TODO we need to do this separately to handle commas, like parse_global()
        type_t* type;
        char* name;
        if (try_parse_declaration(NULL, &type, &name)) {
            parse_local_declaration(type, name);
            continue;
        }

        // Otherwise it's a statement
        parse_statement();
    }

    // track the maximum extent of the function's stack frame
    int frame_size;
    frame_size = locals_frame_size();
    if (function_frame_size < frame_size) {
        function_frame_size = frame_size;
    }

    locals_pop(previous_locals_count);

    //emit_term("; end block");
    //emit_newline();

    return true;
}

static void parse_block(void) {
    if (!try_parse_block()) {
        fatal("Expected {");
    }
}

// Parses a function declaration (and definition, if provided.)
static void parse_function_declaration(type_t* return_type, char* name, storage_t storage) {
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
        emit_global_divider();
    }

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
        if (name == NULL) {
            // TODO technically a name is not required, GCC and Clang only warn
            // about useless declarations.
            fatal("A name is required for a global declaration.");
        }

        // Check for a typedef
        if (storage == STORAGE_TYPEDEF) {
            typedef_add(name, type);
            // TODO multiple typedefs are supposed to be supported
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
        global_declare_variable(type, name);
        if (storage != STORAGE_EXTERN) {
            compile_global_variable(type, name, storage);
        }

        // TODO check for a comma for multiple declarators, only allowed if none are functions
        if (lexer_accept(",")) {
            fatal("`,` in declaration is not yet implemented");
        }
        lexer_expect(";", "Expected `;` at end of global variable declaration.");
        break;
    }

    type_delete(base_type);
}

void parse(void) {
    while (lexer_type != lexer_type_end) {
        parse_global();
    }

    /*
    while (lexer_type != lexer_type_end) {
        //printf("   token %s %s", lexer_type_to_string(lexer_type), lexer_token);
        if (lexer_type == lexer_type_string)
            //printf("    \"%s\"", lexer_literal_string);
        else if (lexer_type == lexer_type_character)
            //printf("    '%c'", lexer_literal_char);
        putchar('\n');
        lexer_consume();
    }
    */
}
