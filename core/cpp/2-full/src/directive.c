#include "directive.h"

#include <stdlib.h>
#include <assert.h>

#include "token.h"
#include "strings.h"
#include "stream.h"
#include "libo-vector.h"
#include "macro.h"
#include "emit.h"
#include "lexer.h"
#include "preprocess.h"

/*
 * A conditional branch (`#if`, `#ifdef` or `#ifndef`.)
 *
 * We use a global conditional stack, not a per-file stack. Each conditional
 * points to the lexer that contained it so we can make sure conditionals don't
 * cross file boundaries.
 */
typedef struct conditional_t {
    token_t* token;
    bool taken;
    //lexer_t* lexer; // TODO make sure conditionals don't cross file boundaries
} conditional_t;

static conditional_t* conditionals;
static size_t conditionals_capacity;
static size_t conditionals_count;

void directive_setup(void) {
    #ifdef DEBUG
    // start small to test growth
    conditionals_capacity = 1;
    #endif
    #ifndef DEBUG
    conditionals_capacity = 8;
    #endif

    conditionals = malloc(conditionals_capacity * sizeof(conditional_t));
    if (conditionals == NULL) {
        fatal("Out of memory.");
    }
}

void directive_teardown(void) {
    //assert(conditionals_count == 0); // TODO turn this on
if (conditionals_count != 0) fatal("Unclosed #if"); // TODO this is temporary, unclosed conditionals should be checked when each file is closed
    free(conditionals);
}

static bool directive_command_is_conditional(string_t* command) {
    if (string_equal(command, STR_IF) ||
        string_equal(command, STR_IFDEF) ||
        string_equal(command, STR_IFNDEF) ||
        string_equal(command, STR_ELIF) ||
        string_equal(command, STR_ELIFDEF) ||
        string_equal(command, STR_ELIFNDEF) ||
        string_equal(command, STR_ELSE) ||
        string_equal(command, STR_ENDIF))
    {
        return true;
    }
    return false;
}

static void directive_conditional_push(token_t* token) {
    if (conditionals_count == conditionals_capacity) {
        size_t new_capacity = conditionals_capacity * 2;
//printf("previous cap %zi new cap %zi\n",conditionals_capacity,new_capacity);
        if (new_capacity <= conditionals_capacity) {
            fatal("Out of memory.");
        }
        conditionals = realloc(conditionals, new_capacity * sizeof(conditional_t));
        if (conditionals == NULL) {
            fatal("Out of memory.");
        }
        conditionals_capacity = new_capacity;
    }

    conditionals[conditionals_count].token = token_ref(token);
    conditionals[conditionals_count].taken = false;
    ++conditionals_count;
}

static void directive_conditional_pop(token_t* endif) {
    assert(conditionals_count != 0); // this was checked already
    token_deref(conditionals[conditionals_count - 1].token);
    --conditionals_count;
}

/**
 * Ensures that the rest of the line is blank and consumes it including the
 * line ending.
 */
static void directive_parse_end_of_line(stream_t* stream, token_t* command) {
    stream_skip_horizontal_space(stream);
    token_t* end = stream_peek(stream);
    if (end->type != token_type_newline) {
        fatal_token(end, "Unexpected token `%s` at end of line in `#%s` directive.", end->value->bytes, command->value->bytes);
    }
    stream_consume(stream);
}

/**
 * Checks the expression of an `#if` or `#elif` directive, returning its value.
 */
static bool directive_parse_if(stream_t* stream, token_t* command) {
    fatal("TODO #if/#elif");

    directive_parse_end_of_line(stream, command);
}

/**
 * Checks if the macro following an `#ifdef`, `#ifndef`, `#elifdef` or
 * `#elifndef` directive is defined, returning true if it is.
 */
static bool directive_parse_ifdef(stream_t* stream, token_t* command) {
    stream_skip_horizontal_space(stream);

    // check if the given macro exists
    token_t* name = stream_peek(stream);
    if (name->type != token_type_alphanumeric) {
        fatal_token(name, "`#%s` must be followed by an identifier.", command->value->bytes);
    }
    bool found = NULL != macro_find(name->value);
    //printf("consuming identifier %s\n",name->value->bytes);
    stream_consume(stream);

    // make sure there's nothing else on the line
    directive_parse_end_of_line(stream, command);
    return found;
}

static void directive_parse_else(stream_t* stream, token_t* command) {
    // make sure there's nothing on the line
    directive_parse_end_of_line(stream, command);
}

/**
 * Skips the contents of a false branch until the next conditional directive.
 *
 * The new directive command is consumed and returned as a strong reference.
 */
static token_t* directive_skip_branch(stream_t* stream, token_t* src) {
    int depth = 0;
    for (;;) {

        // skip until we reach a directive
        token_type_t type = stream_peek(stream)->type;
        if (type == token_type_end) {
            fatal_token(src, "Unclosed branch directive: expected `#endif` to match this `#%s`.",
                    src->value->bytes);
        }
        stream_consume(stream);
        if (type != token_type_directive) {
            continue;
        }
        stream_skip_horizontal_space(stream);

        // check if it's a conditional
        token_t* command = stream_peek(stream);
        if (command->type != token_type_alphanumeric)
            continue;
        string_t* command_str = command->value;
        if (!directive_command_is_conditional(command_str))
            continue;

        // exit if we found an elif/else/endif at depth zero, otherwise we
        // push/pop depth.
        if (string_equal(command_str, STR_IF) ||
                string_equal(command_str, STR_IFDEF) ||
                string_equal(command_str, STR_IFNDEF))
        {
            ++depth;
        } else if (string_equal(command_str, STR_ENDIF) && depth > 0) {
            --depth;
        } else if (depth == 0) {
            return stream_take(stream);
        }
    }
}

/**
 * Handles a conditional that isn't in a skipped branch.
 *
 * Nested conditionals in skipped branches are handled in directive_skip_branch().
 */
static void directive_conditional(stream_t* stream, token_t* command) {
    token_ref(command);

    // Loop for skipping false sections
    for (;;) {
        string_t* command_str = command->value;
        //printf("handling conditional %s\n", command_str->bytes);

        // TODO we're doing this here because directive_parse() does it before
        // calling us
        stream_skip_horizontal_space(stream);

        // Manage conditional stack.
        if (string_equal(command_str, STR_IF) ||
                string_equal(command_str, STR_IFDEF) ||
                string_equal(command_str, STR_IFNDEF))
        {
            directive_conditional_push(command);
        } else if (conditionals_count == 0) {
            fatal_token(command, "`#%s` without a matching #if, #ifdef or #ifndef.", command->value->bytes);
        }

        // If this is `#endif`, we're done.
        if (command_str == STR_ENDIF) {
            directive_conditional_pop(command);
            break;
        }

        // Parse the rest of the directive and decide whether to take the branch.
        conditional_t* conditional = &conditionals[conditionals_count - 1];
        bool take_branch = false;
        if (string_equal(command_str, STR_IF)) {
            take_branch = directive_parse_if(stream, command);
        } else if (string_equal(command_str, STR_ELIF)) {
            if (!conditional->taken) {
                take_branch = directive_parse_if(stream, command);
            }
        } else if (string_equal(command_str, STR_IFDEF) || string_equal(command_str, STR_IFNDEF)) {
            take_branch = (string_equal(command_str, STR_IFDEF)) == directive_parse_ifdef(stream, command);
        } else if (string_equal(command_str, STR_ELIFDEF) || string_equal(command_str, STR_ELIFNDEF)) {
            if (!conditional->taken) {
                take_branch = (string_equal(command_str, STR_ELIFDEF)) == directive_parse_ifdef(stream, command);
            }
        } else if (string_equal(command_str, STR_ELSE)) {
            directive_parse_else(stream, command);
            take_branch = !conditional->taken;
        } else {
            // unreachable
            fatal("Internal error: unrecognized conditional directive.");
        }

        // If we're taking the branch, there's nothing else to do.
        if (take_branch) {
            assert(!conditional->taken);
            conditional->taken = true;
            break;
        }

        // We're not taking the branch. Skip until we find another conditional
        // directive (at the same depth) and loop around.
        token_t* new_command = directive_skip_branch(stream, command);
        token_deref(command);
        command = new_command;
        assert(command->type == token_type_alphanumeric);
        assert(directive_command_is_conditional(command->value));
    }

    token_deref(command);
}

static void directive_undef(stream_t* stream, token_t* command) {
    token_t* name = stream_take(stream);
    if (name->type != token_type_alphanumeric) {
        fatal_token(name, "Expected an identifier after `#undef`.");
    }
    macro_undef(name->value);
    directive_parse_end_of_line(stream, command);
}

// Handles an #include directive.
static void directive_include(stream_t* stream, token_t* command) {
    stream_skip_horizontal_space(stream);
    token_t* token = stream_take(stream);

    // If the next token is a string, we've found our filename.
    if (token->type == token_type_string || token->type == token_type_angle_include) {
        if (token->prefix != token_prefix_none) {
            fatal_token(token, "String prefixes are not supported on `#include` directives.");
        }
    } else {

        // Otherwise we need to collect the rest of the tokens on the line and
        // perform a macro expansion pass on them.

        if (token->type != token_type_alphanumeric) {
            fatal_token(token, "An #include directive must contain a filename enclosed in quotes or angle brackets or a list of macros that expand to one.");
        }

        fatal_token(token, "TODO #include macro");

        // At this point we should have either a string or a tokenized
        // angle-bracketed filename. If it's angle-bracketed, we need to
        // convert it to a string and wrap it in a token with the same location
        // as the first macro. TODO probably share stringify code

        // TODO make sure rest of line is clear
    }

    directive_parse_end_of_line(stream, command);
    preprocess_include_search(stream, token);
    token_deref(token);
}

void directive_parse(stream_t* stream, token_t* token) {
    stream_skip_horizontal_space(stream);
    token_t* command = stream_take(stream);

    // ignore blank preprocessor directives
    if (command->type == token_type_newline) {
        token_deref(command);
        return;
    }

    // a directive must start with a command
    // (TODO we could support GNU linemarkers as input, in which case we should
    // parse a number command here.)
    if (command->type != token_type_alphanumeric) {
        fatal_token(command, "A preprocessor directive must start with a command.");
    }

    // TODO we're skipping horizontal space after every command but we should
    // actually do this conditionally per-command. e.g. object-like #define
    // requires whitespace in C99 and later but not in C89.
    stream_skip_horizontal_space(stream);

    string_t* command_str = command->value;
    if (string_equal(command_str, STR_DEFINE)) {
        macro_define(stream);
        directive_parse_end_of_line(stream, command);
    } else if (string_equal(command_str, STR_UNDEF)) {
        directive_undef(stream, command);
    } else if (string_equal(command_str, STR_LINE)) {
        // TODO implement #line
        fatal_token(command, "#line is not yet implemented.");
        //directive_line(stream);
    } else if (string_equal(command_str, STR_INCLUDE)) {
        directive_include(stream, command);
    } else if (string_equal(command_str, STR_INCLUDE_NEXT)) {
        fatal_token(command, "#include_next is not yet implemented.");
    } else if (string_equal(command_str, STR_PRAGMA)) {
        // TODO implement #pragma
        goto ignore;
        //directive_pragma(stream);
    } else if (string_equal(command_str, STR_ERROR)) {
        fatal_token(command, "#error is not yet implemented.");
        //directive_error_or_warning(stream, true);
    } else if (string_equal(command_str, STR_WARNING)) {
        fatal_token(command, "#warning is not yet implemented.");
        //directive_error_or_warning(stream, false);
    } else if (string_equal(command_str, STR_EMBED)) {
        fatal_token(command, "#embed is not yet implemented.");
        //directive_embed(stream);
    } else if (directive_command_is_conditional(command_str)) {
        directive_conditional(stream, command);
    } else {
        //token_print(command);
        fatal_token(command, "Unrecognized preprocessor directive: %s.", command_str->bytes);
    }

    token_deref(command);
    return;

ignore:
    token_deref(command);
    for (;;) {
        token_t* token = stream_peek(stream);
        if (token->type == token_type_newline)
            break;
        stream_consume(stream);
    }
}
