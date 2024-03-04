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
static type_t parse_binary_expression(void);
static type_t parse_expression(void);
static type_t parse_unary_expression(void);

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
static type_t* function_return_types;
static size_t functions_count;
static int function_frame_size;
#define FUNCTIONS_MAX 512

// TODO consider putting sizeof in cci/0 to avoid this
#ifdef __onramp__
    #define SIZEOF_CHAR_P 4
    #define SIZEOF_INT 4
#else
    #define SIZEOF_CHAR_P sizeof(char*)
    #define SIZEOF_INT sizeof(int)
#endif

void parse_init(void) {
    while_start_label = -1;
    while_end_label = -1;
    strings = malloc(SIZEOF_CHAR_P * STRINGS_MAX);
    function_names = malloc(SIZEOF_CHAR_P * FUNCTIONS_MAX);
    function_return_types = malloc(TYPE_T_SIZE * FUNCTIONS_MAX);
}

void parse_destroy(void) {
    size_t i = 0;
    while (i < functions_count) {
        free(*(function_names + i));
        i = (i + 1);
    }

    free(function_return_types);
    free(function_names);
    free(strings);
}

static bool try_parse_type(type_t* out) {

    // Ignore const, except that if we find it, this has to be a type
    bool has_const = lexer_try("const");

    type_t type;
    if (lexer_try("int")) {
        type = TYPE_BASIC_INT;
    } else if (lexer_try("char")) {
        type = TYPE_BASIC_CHAR;
    } else if (lexer_try("void")) {
        type = TYPE_BASIC_VOID;
    } else {
        if (!typedef_find(lexer_token, &type)) {
            if (has_const) {
                fatal("Expected type name");
            }
            return false;
        }
        lexer_consume();
    }

    // Collect additional indirections, each of which may be prefixed by const
    int indirection = type_indirection(type);
    while (lexer_try("*")) {
        lexer_try("const");
        ++indirection;
    }

    // Between the type and the name, we can have both const and restrict in
    // either order.
    // (It seems silly to support all this in the most basic compiler but this
    // is all to make even our intermediate header files as correct and
    // forward-compatible as possible.)
    lexer_try("const");
    lexer_try("restrict");
    lexer_try("const");

    *out = type_make(type_basic(type), indirection);
    return true;
}

static type_t parse_type(void) {
    type_t ret;
    if (!try_parse_type(&ret)) {
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

static type_t parse_primary_expression(void) {
    type_t type;

    // parenthesis
    if (lexer_try("(")) {

        // if the token is a type name, we have a cast expression.
        if (try_parse_type(&type)) {
            lexer_expect(")", "Expected ) after type in cast");
            type_t actual_type;
            actual_type = parse_unary_expression();

            // if the actual type is an l-value, we have to dereference it. we
            // otherwise ignore the type.
            compile_dereference_if_lvalue(actual_type, 0);

            return type;
        }

        type = parse_expression();
        lexer_expect(")", "Expected ) after parenthesized expression");
        return type;
    }

    // number
    if (lexer_type == lexer_type_number) {
        type = compile_immediate(lexer_token);
        lexer_consume();
        return type;
    }

    // character literal
    if (lexer_type == lexer_type_character) {
        // We don't support any escape characters yet so we can just send it
        // through.
        type = compile_character_literal(lexer_token[0]);
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
        return type_make(TYPE_BASIC_CHAR, 1);
    }

    //fatal("Expected primary expression (i.e. identifier, number, string or open parenthesis)");
    fatal("Expected expression");
}

static type_t parse_function_call(const char* name) {
    //printf("   parsing function call %s\n", name);
    int arg_count;
    arg_count = 0;

    //emit_term("; function call");
    //emit_newline();

    while (!lexer_try(")")) {
        if (arg_count > 0)
            lexer_expect(",", NULL);
        ++arg_count;

        if (arg_count > 4) {
            // TODO support more args in stage 1, not stage 0
            fatal("TODO only four arguments are supported.");
        }

        //printf("   current token is %s\n",lexer_token);
        type_t type = parse_binary_expression();
        //printf("   current token is %s\n",lexer_token);

        // if the argument is an l-value, dereference it
        compile_dereference_if_lvalue(type, 0);

        // TODO type-check the argument, for now ignore it

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
    return *(function_return_types + i);
}

static type_t parse_postfix_expression(void) {

    // a non-alphanumeric is a primary expression
    if (lexer_type != lexer_type_alphanumeric) {
        return parse_primary_expression();
    }

    // an alphanumeric is either a variable or a function call
    type_t ret;
    char* name = lexer_take();
    if (lexer_try("(")) {
        ret = parse_function_call(name);
    } else {
        ret = compile_variable(name);
    }

    free(name);
    return ret;
}

static type_t parse_unary_expression(void) {
    type_t type;

    if (lexer_try("+")) {
        return parse_postfix_expression();
    }

    if (lexer_try("-")) {
        type = parse_postfix_expression();
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

    if (lexer_try("*")) {
        type = parse_postfix_expression();
        if (type_indirection(type) == 0) {
            fatal("Type to dereference is not a pointer");
        }

        // If this is already an lvalue, we dereference it now. Otherwise we
        // make it an lvalue, and it will be dereferenced if and when it is
        // needed.
        if (type & TYPE_FLAG_LVALUE) {
            // TODO are these in the correct order? probably not again?
            compile_dereference(type & ~TYPE_FLAG_LVALUE, 0);
            type = type_decrement_indirection(type);
        } else {
            type = type_decrement_indirection(type);
            type |= TYPE_FLAG_LVALUE;
        }

        return type;
    }

    if (lexer_try("&")) {
        type = parse_postfix_expression();
        if (!(type & TYPE_FLAG_LVALUE)) {
            fatal("Cannot take the address of an r-value");
        }
        // TODO shouldn't we increment reference??
        return type & ~TYPE_FLAG_LVALUE;
    }

    if (lexer_try("!")) {
        type = parse_postfix_expression();
        compile_dereference_if_lvalue(type, 0);
        return compile_not();
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

static type_t parse_binary_expression(void) {
    type_t left = parse_unary_expression();

    // see if we have a binary operator
    if (!parse_token_is_binary_op())
        return left;
    char* op = lexer_take();  // TODO omC compiler can just push it on the stack as an int

    // push the left side of the expression
    emit_term("push");
    emit_term("r0");
    emit_newline();

    // get the right side into r0
    //printf("    parsing right side of binary expression\n");
    type_t right = parse_unary_expression();

    // pop the left side into r1
    emit_term("pop");
    emit_term("r1");
    emit_newline();

    // compile it
    //printf("    compiling binary expression\n");
    type_t ret = compile_binary_op(op, left, right);
    free(op);

    // nicer error messages when parens are missed
    if (parse_token_is_binary_op()) {
        fatal("Multiple binary operators are not allowed in an expression. Add parentheses.");
    }

    return ret;
}

static type_t parse_expression(void) {
    // "expression" is actually a comma operator expression. It's separate from
    // a binary expression because it can't appear as the argument to a
    // function. I'm not sure yet if I'm going to bother implementing it in the
    // lower stages.
    type_t type = parse_binary_expression();
    if (lexer_try(",")) {
        fatal("Comma expression is not supported.");
    }
    return type;
}

static void parse_condition(int false_label) {

    // collect the expression
    lexer_expect("(", NULL);
    type_t type = parse_expression();
    lexer_expect(")", NULL);

    // if the type is an lvalue we need to dereference it
    compile_dereference_if_lvalue(type, 0);

    // if the expression is false, jump to the given label
    emit_term("jz");
    emit_term("r0");
    emit_computed_label('&', JUMP_LABEL_PREFIX, false_label);
    emit_newline();
}

static void parse_if(void) {

    // parse a condition. if true, skip the if block.
    //emit_term("; if");
    //emit_newline();
    int skip_if_label = next_label++;
    parse_condition(skip_if_label);

    parse_statement(); // TODO maybe not exactly right, not sure how labels work here

    // check for else
    bool has_else = lexer_try("else");
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
    if (!lexer_try(";")) {
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
    if (!lexer_try(";")) {
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
    int argument = !lexer_try(";");

    if (argument) {
        type_t type = parse_expression();
        compile_dereference_if_lvalue(type, 0);
        // The expression result is in r0 which is our return value.
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
    if (lexer_try(";")) {
        // empty statement
        return;
    }

    if (lexer_is("{")) {
        //scope_push();
        parse_block();
        //scope_pop();
        return;
    }

    if (lexer_try("if")) {
        parse_if();
        return;
    }

    if (lexer_try("while")) {
        parse_while();
        return;
    }

    if (lexer_try("break")) {
        parse_break();
        return;
    }

    if (lexer_try("continue")) {
        parse_continue();
        return;
    }

    if (lexer_try("return")) {
        parse_return();
        return;
    }

    // parse an expression, discard the result
    (void)parse_expression();
    lexer_expect(";", NULL);
}

static void parse_local_declaration(type_t type) {
    char* name = parse_alphanumeric();
    //printf("defining new variable %s\n",name);
    variable_add(name, type, false);

    if (lexer_try(";")) {
        // No initializer.
        return;
    }

    if (!lexer_try("=")) {
        fatal("Expected ; or = after local variable declaration.");
    }

    // The variable has an initializer. Parse it, then generate an assignment
    // operation.

    // compile and push the variable
    compile_variable(name);
    emit_term("push");
    emit_term("r0");
    emit_newline();

    // get the assignment expression
    type_t expr_type = parse_unary_expression();

    // pop the destination
    emit_term("pop");
    emit_term("r1");
    emit_newline();

    // compile an assignment
    compile_binary_op("=", type | TYPE_FLAG_LVALUE, expr_type);
}

static bool try_parse_block(void) {
    if (!lexer_try("{")) {
        return false;
    }

    //emit_term("; block");
    //emit_newline();

    int previous_variable_count = variable_count;

    while (!lexer_try("}")) {
        type_t type;
        if (try_parse_type(&type)) {
            parse_local_declaration(type);
        } else  {
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
static void parse_function_declaration(type_t return_type, char* name) {
    //scope_push();
    int previous_variable_count = variable_count;
    int arg_count = 0;

    if (lexer_try(")")) {
        // nothing
    } else {
        while (!lexer_try(")")) {
            if (arg_count > 0)
                lexer_expect(",", "Expected , or ) after argument");

            // parse type
            type_t type;
            type = parse_type();

            // check for (void)
            // TODO can probably remove hack now that names are optional
            if (arg_count == 0 && type == TYPE_BASIC_VOID && lexer_try(")"))
                break;

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
    if (lexer_try(";")) {
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
    type_t type = parse_type();
    char* name = parse_alphanumeric();
    typedef_add(name, type);
    lexer_expect(";", "Expected ; at end of typedef");
}

void parse_global(void) {
//printf("   parse_global()\n");

    if (lexer_try("typedef")) {
        parse_typedef();
        return;
    }

    // TODO
    lexer_try("_Noreturn");

    bool is_extern = false;
    if (lexer_try("extern")) {
        is_extern = true;
    }

    bool is_static = false;
    if (lexer_try("static")) {
        is_static = true;
    }
    (void)is_static; // TODO not yet used

    type_t type = parse_type();
    char* name = parse_alphanumeric();

    // see if this is a global variable declaration
    if (lexer_try(";")) {
        variable_add(name, type, true);
        //printf("   parsed global %s\n",name);
        if (!is_extern)
            compile_global_variable(type, name);
        return;
    }

    // parse a function declaration or definition
    if (lexer_try("(")) {
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
