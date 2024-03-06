#include "parse.h"

#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "emit.h"
#include "common.h"
#include "compile.h"
#include "variable.h"

static void parse_block(void);
static void parse_statement(void);
static type_t* parse_binary_expression(void);
static type_t* parse_expression(void);
static type_t* parse_unary_expression(void);

// label generation
static int next_label;
static int while_start_label;
static int while_end_label;

// string generation
static int next_string;
static char** strings;
static size_t strings_count;
#define STRINGS_MAX 128

// functions
// TODO make a hashtable
static char** function_names;
static type_t** function_return_types;
static size_t functions_count;
static int function_frame_size;
#define FUNCTIONS_MAX 512

void parse_init(void) {
    while_start_label = -1;
    while_end_label = -1;
    strings = malloc(sizeof(char*) * STRINGS_MAX);
    function_names = malloc(sizeof(char*) * FUNCTIONS_MAX);
    function_return_types = malloc(sizeof(type_t*) * FUNCTIONS_MAX);
}

void parse_destroy(void) {
    size_t i = 0;
    while (i < functions_count) {
        free(*(function_names + i));
        type_delete(*(function_return_types + i));
        i = (i + 1);
    }

    free(function_return_types);
    free(function_names);
    free(strings);
}

static type_t* try_parse_type(void) {

    // Ignore const, except that if we find it, this has to be a type
    bool has_const = lexer_accept("const");

    type_t* type;
    if (lexer_accept("int")) {
        type = type_new_blank();
        type_set_base(type, TYPE_BASIC_INT);
    } else if (lexer_accept("char")) {
        type = type_new_blank();
        type_set_base(type, TYPE_BASIC_CHAR);
    } else if (lexer_accept("void")) {
        type = type_new_blank();
        type_set_base(type, TYPE_BASIC_VOID);
    } else {
        const type_t* found = typedef_find(lexer_token);
        if (!found) {
            if (has_const) {
                fatal("Expected type name");
            }
            return NULL;
        }
        type = type_clone(found);
        lexer_consume();
    }

    // Collect additional indirections, each of which may be prefixed by const
    int indirections = type_indirections(type);
    while (lexer_accept("*")) {
        lexer_accept("const");
        ++indirections;
    }
    type_set_indirections(type, indirections);

    // Between the type and the name, we can have both const and restrict in
    // either order.
    // (It seems silly to support all this in the most basic compiler but this
    // is all to make even our intermediate header files as correct and
    // forward-compatible as possible.)
    lexer_accept("const");
    lexer_accept("restrict");
    lexer_accept("const");

    return type;
}

static type_t* parse_type(void) {
    type_t* ret = try_parse_type();
    if (!ret) {
        fatal("Expected type name");
    }
    return ret;
}

static char* parse_alphanumeric(void) {
    if (lexer_type != lexer_type_alphanumeric) {
        fatal("Expected identifier");
    }
    return lexer_take();
}

static type_t* parse_primary_expression(void) {

    // parenthesis
    if (lexer_accept("(")) {

        // if the token is a type name, we have a cast expression.
        type_t* type = try_parse_type();
        if (type) {
            lexer_expect(")", "Expected ) after type in cast");
            type_t* actual_type = parse_unary_expression();

            // if the actual type is an l-value, we have to dereference it. we
            // otherwise ignore the type.
            // TODO fix this, need to compile a cast (sign extension on byte),
            // make it like cci/0
            compile_dereference_if_lvalue(actual_type, 0);

            type_delete(actual_type);
            return type;
        }

        type = parse_expression();
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
        type_t* type = compile_character_literal(lexer_token[0]);
        lexer_consume();
        return type;
    }

    // string
    if (lexer_type == lexer_type_string) {
        if (strings_count == STRINGS_MAX) {
            fatal("Too many strings in this function");
        }
        compile_string_literal_invocation(next_string + strings_count);
        strings[strings_count] = lexer_take();
        strings_count = (strings_count + 1);
        return type_new(TYPE_BASIC_CHAR, 1);
    }

    //fatal("Expected primary expression (i.e. identifier, number, string or open parenthesis)");
    fatal("Expected expression");
}

static type_t* parse_function_call(const char* name) {
    //printf("   parsing function call %s\n", name);
    int arg_count;
    arg_count = 0;

    //emit_term("; function call");
    //emit_newline();

    while (!lexer_accept(")")) {
        if (arg_count > 0)
            lexer_expect(",", NULL);
        ++arg_count;

        if (arg_count > 4) {
            // TODO support more args in stage 1, not stage 0
            fatal("TODO only four arguments are supported.");
        }

        //printf("   current token is %s\n",lexer_token);
        type_t* type = parse_binary_expression();
        //printf("   current token is %s\n",lexer_token);

        // if the argument is an l-value, dereference it
        type = compile_dereference_if_lvalue(type, 0);

        // TODO type-check the argument, for now ignore it
        type_delete(type);

        // TODO a simple optimization here (for stage 1, not stage 0) is, if
        // there are at most 4 args, don't push the last arg, just move it to
        // the correct register. (there's a lot of "push r0 pop r0" for
        // single-arg functions)
        // TODO if we want to make it even simpler, just skip the push/pop for
        // single-argument functions.

        // push the argument to the stack
        emit_term("push");
        emit_term("r0");
        emit_newline();
    }

    // TODO another simple optimization is to not use pop, instead use ldw for
    // each arg, then adjust the stack manually. this will be more important
    // when we have more than 4 args and we can't actually pop the args beyond 4

    // pop the arguments
    if (arg_count > 3) {
        emit_term("pop");
        emit_term("r3");
        emit_newline();
    }
    if (arg_count > 2) {
        emit_term("pop");
        emit_term("r2");
        emit_newline();
    }
    if (arg_count > 1) {
        emit_term("pop");
        emit_term("r1");
        emit_newline();
    }
    if (arg_count > 0) {
        emit_term("pop");
        emit_term("r0");
        emit_newline();
    }

    // emit the call
    emit_term("call");
    emit_label('^', name);
    emit_newline();
    //emit_term("; end function call");
    //emit_newline();

    // find the function
    size_t i = 0;
    while (i < functions_count) {
        if (0 == strcmp(name, *(function_names + i))) {
            break;
        }
        i = (i + 1);
    }
    if (i == functions_count) {
        fatal("Function not declared");
    }

    //printf("   done parsing function call %s\n", name);
    return type_clone(*(function_return_types + i));
}

static type_t* parse_postfix_expression(void) {

    // a non-alphanumeric is a primary expression
    if (lexer_type != lexer_type_alphanumeric) {
        return parse_primary_expression();
    }

    // an alphanumeric is either a variable or a function call
    type_t* ret;
    char* name = lexer_take();
    bool paren = lexer_accept("(");
    if (paren) {
        ret = parse_function_call(name);
    }
    if (!paren) {
        ret = compile_variable(name);
    }

    free(name);
    return ret;
}

static type_t* parse_sizeof(void) {
    bool paren = lexer_accept("(");
    if (paren) {
        type_t* type = try_parse_type();
        if (type) {
            lexer_expect(")", "Expected `)` after `sizeof(type`");
            return compile_sizeof(type);
        }
    }

    // Parse the expression with compilation disabled. We only want to do type
    // resolution.
    bool was_enabled = compile_is_enabled();
    compile_set_enabled(false);
    type_t* type = parse_expression();
    compile_set_enabled(was_enabled);

    if (paren) {
        lexer_expect(")", "Expected `)` after `sizeof(expression`");
    }

    return compile_sizeof(type);
}

static type_t* parse_unary_expression(void) {
    if (lexer_accept("+")) {
        // TODO
        return parse_postfix_expression();
    }

    if (lexer_accept("-")) {
        type_t* type = parse_postfix_expression();
        // TODO is unary - allowed on pointers? maybe we should check that
        // there is no indirection
        // negate r0
        // TODO dereference??
        emit_term("sub");
        emit_term("r0");
        emit_term("0");
        emit_term("r0");
        emit_newline();
        return type;
    }

    if (lexer_accept("*")) {
        type_t* type = parse_postfix_expression();
        if (type_indirections(type) == 0) {
            fatal("Type to dereference is not a pointer");
        }

        // If this is already an lvalue, we dereference it now. Otherwise we
        // make it an lvalue, and it will be dereferenced if and when it is
        // needed.
        if (*type & TYPE_FLAG_LVALUE) {
            type = type_decrement_indirection(type);
            compile_dereference(type, 0);
        } else {
            type = type_decrement_indirection(type);
            *type |= TYPE_FLAG_LVALUE;
        }

        return type;
    }

    if (lexer_accept("&")) {
        type_t* type = parse_postfix_expression();
        if (!(*type & TYPE_FLAG_LVALUE)) {
            fatal("Cannot take the address of an r-value");
        }
        // TODO shouldn't we increment reference??
        //type_increment_indirection(type);
        type_set_lvalue(type, false);
        return type;
    }

    if (lexer_accept("!")) {
        type_t* type = parse_postfix_expression();
        compile_dereference_if_lvalue(type, 0);
        type_delete(type);
        return compile_not();
    }
    
    if (lexer_accept("sizeof")) {
        return parse_sizeof();
    }

    return parse_postfix_expression();
}

static const char* binary_ops[] = {
    "=",
    "-", "+", "*", "/", "%",
    "&", "|", "^", "<<", ">>",
    "==", "!=", "<", ">", "<=", ">=",
};

static bool parse_token_is_binary_op(void) {
    for (size_t i = 0; i < sizeof(binary_ops) / sizeof(binary_ops[0]); ++i) {
        if (0 == strcmp(lexer_token, binary_ops[i])) {
            return true;
        }
    }
    return false;
}

#if 0
// TODO all this is moved to compile_immediate
static void parse_number(const char* number) {
    emit_term("imw");
    emit_term("r0");
    emit_term(number);

    #if 0
    size_t len = strlen(number);
    if (len > 2 && number[0] == '0' && (number[1] == 'x' || number[1] == 'X')) {
        // The number is hexadecimal. The lexer has already verified that it is
        // syntactically correct. Just forward it straight to the output.
        emit_term(number);
    } else {
        emit_term(number);
    }
    #endif

    result_t result;
    result.type.type_basic = type_basic_int;
    result.type.indirection = 0;
    result.lvalue = false;
    return result;
}
#endif

static type_t* parse_binary_expression(void) {
    type_t* left = parse_unary_expression();

    // see if we have a binary operator
    if (!parse_token_is_binary_op())
        return left;
    char* op = lexer_take();

    // push the left side of the expression
    emit_term("push");
    emit_term("r0");
    emit_newline();

    // get the right side into r0
    //printf("    parsing right side of binary expression\n");
    type_t* right = parse_unary_expression();

    // pop the left side into r1
    emit_term("pop");
    emit_term("r1");
    emit_newline();

    // compile it
    //printf("    compiling binary expression\n");
    type_t* ret = compile_binary_op(op, left, right);
    free(op);

    // nicer error messages when parens are missed
    if (parse_token_is_binary_op()) {
        fatal("Multiple binary operators are not allowed in an expression. Add parentheses.");
    }

    return ret;
}

static type_t* parse_expression(void) {
    // "expression" is actually a comma operator expression. It's separate from
    // a binary expression because it can't appear as the argument to a
    // function. I'm not sure yet if I'm going to bother implementing it in the
    // lower stages.
    // TODO yes, implement it in opC, we want full expression syntax
    type_t* type = parse_binary_expression();
    if (lexer_accept(",")) {
        fatal("Comma expression is not supported.");
    }
    return type;
}

static void parse_condition(int false_label) {

    // collect the expression
    lexer_expect("(", NULL);
    type_t* type = parse_expression();
    lexer_expect(")", NULL);

    // if the type is an lvalue we need to dereference it
    compile_dereference_if_lvalue(type, 0);

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
    int skip_if_label = next_label++;
    parse_condition(skip_if_label);

    parse_statement(); // TODO maybe not exactly right, not sure how labels work here

    // check for else
    bool has_else = lexer_accept("else");
    if (has_else) {

        // we're still in the if block, so skip the else block.
        int skip_else_label = next_label++;
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
    while_start_label = next_label++;
    while_end_label = next_label++;

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
        fatal("Expected ; after break.");
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
        fatal("Expected ; after continue.");
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
        compile_dereference_if_lvalue(type, 0);
        // The expression result is in r0 which is our return value.
        // TODO for now assume it's correct, we should be casting it to the return type
        type_delete(type);
        lexer_expect(";", NULL);
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
    lexer_expect(";", NULL);
}

// Takes ownership of the given type
static void parse_local_declaration(type_t* type) {
    char* name = parse_alphanumeric();
    //printf("defining new variable %s\n",name);
    variable_add(name, type, false);

    if (lexer_accept(";")) {
        // No initializer.
        return;
    }
    type = type_clone(type);

    if (!lexer_accept("=")) {
        fatal("Expected ; or = after local variable declaration.");
    }

    // The variable has an initializer. Parse it, then generate an assignment
    // operation.

    // compile and push the variable
    type_t* var_type = compile_variable(name);
    emit_term("push");
    emit_term("r0");
    emit_newline();

    // TODO for now ignoring var_type
    type_delete(var_type);

    // get the assignment expression
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

    int previous_variable_count = variable_count;

    while (!lexer_accept("}")) {
        type_t* type = try_parse_type();
        if (type) {
            parse_local_declaration(type);
        }
        if (!type) {
            parse_statement();
        }
    }

    // track the maximum extent of the function's stack frame
    int local_size;
    local_size = variable_local_size();
    if (function_frame_size < local_size) {
        function_frame_size = local_size;
    }

    variable_pop(previous_variable_count);

    //emit_term("; end block");
    //emit_newline();

    return true;
}

static void parse_block(void) {
    if (!try_parse_block()) {
        fatal("Expected {");
    }
}

static void parse_output_strings(void) {
    for (size_t i = 0; i < strings_count; ++i) {
        emit_newline();
        compile_string_literal_definition(next_string, strings[i]);
        next_string = (next_string + 1);
        free(strings[i]);
    }
    strings_count = 0;
}

// Parses a function declaration (and definition, if provided.)
static void parse_function_declaration(type_t* return_type, char* name) {
    //scope_push();
    int previous_variable_count = variable_count;
    int arg_count = 0;

    if (lexer_accept(")")) {
        // nothing
    } else {
        while (!lexer_accept(")")) {
            if (arg_count > 0)
                lexer_expect(",", "Expected , or ) after argument");

            // parse type
            type_t* type = parse_type();

            // check for (void)
            // TODO can probably remove hack now that names are optional
            if (arg_count == 0 && *type == TYPE_BASIC_VOID && lexer_accept(")")) {
                type_delete(type);
                break;
            }

            // parse optional name
            char* varname;
            if (lexer_type == lexer_type_alphanumeric) {
                varname = parse_alphanumeric();
            } else {
                varname = strdup("");
            }

            // add variable
            variable_add(varname, type, false);
            ++arg_count;
        }
    }

    if (functions_count == FUNCTIONS_MAX) {
        fatal("Too many functions.");
    }
    *(function_names + functions_count) = name;
    *(function_return_types + functions_count) = return_type;
    functions_count = (functions_count + 1);

    //printf("   parsed function declaration %s\n",name);
    if (lexer_accept(";")) {
        // nothing to do
    } else {
        //printf("   parsing function body\n");

        // compile the function
        function_frame_size = 0;
        //printf("==%i\n",previous_variable_count);
        //dump_variables();
        compile_function_open(name, arg_count);
        parse_block();
        compile_function_close(name, arg_count, function_frame_size);

        // output any strings that were used in the function
        parse_output_strings();
        emit_global_divider();
    }

    variable_pop(previous_variable_count);
}

static void parse_typedef(void) {
    type_t* type = parse_type();
    char* name = parse_alphanumeric();
    typedef_add(name, type);
    lexer_expect(";", "Expected ; at end of typedef");
}

void parse_global(void) {
//printf("   parse_global()\n");

    if (lexer_accept("typedef")) {
        parse_typedef();
        return;
    }

    // TODO
    lexer_accept("_Noreturn");

    bool is_extern = false;
    if (lexer_accept("extern")) {
        is_extern = true;
    }

    bool is_static = false;
    if (lexer_accept("static")) {
        is_static = true;
    }
    (void)is_static; // TODO not yet used

    type_t* type = parse_type();
    char* name = parse_alphanumeric();

    // see if this is a global variable declaration
    if (lexer_accept(";")) {
        variable_add(name, type, true);
        //printf("   parsed global %s\n",name);
        if (!is_extern)
            compile_global_variable(type, name);
        return;
    }

    // parse a function declaration or definition
    if (lexer_accept("(")) {
        parse_function_declaration(type, name);
        return;
    }

    fatal("Expected ; or (");
}

void parse(void) {
    while (lexer_type != lexer_type_end) {
        parse_global();
    }

    #if 0
    while (lexer_type != lexer_type_end) {
        //printf("   token %s %s", lexer_type_to_string(lexer_type), lexer_token);
        if (lexer_type == lexer_type_string)
            //printf("    \"%s\"", lexer_literal_string);
        else if (lexer_type == lexer_type_character)
            //printf("    '%c'", lexer_literal_char);
        putchar('\n');
        lexer_consume();
    }
    #endif
}
