/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2025 Fraser Heavy Software
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
#include "file.h"
#include "expression.h"

void directive_setup(void) {
}

void directive_teardown(void) {
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
    vector_t buffer;
    vector_init(&buffer);
    macro_expand_stream(stream, &buffer, true, true);

    stream_t expr_stream;
    stream_init(&expr_stream, false, &buffer);
    bool result = expression_evaluate(&expr_stream);
    stream_destroy(&expr_stream);

    for (size_t i = 0; i < vector_count(&buffer); ++i) {
        token_deref(vector_at(&buffer, i));
    }
    vector_destroy(&buffer);

    return result;
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
    //trace("Skipping branch\n");
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
            //trace("  skip push\n");
            ++depth;
        } else if (string_equal(command_str, STR_ENDIF) && depth > 0) {
            //trace("  skip pop\n");
            --depth;
        } else if (depth == 0) {
            //trace("  done skipping\n");
            return stream_take(stream);
        }
    }
}

/**
 * Handles a conditional directive that isn't in a skipped branch.
 *
 * This evaluates the condition and updates the conditional stack. If the
 * condition is false, this also consumes all tokens until the end of the
 * branch.
 */
static void directive_conditional(stream_t* stream, token_t* command) {
    token_ref(command);
    file_t* file = file_current;

    // Loop for skipping false sections
    for (;;) {
        string_t* command_str = command->value;
        //printf("handling conditional %s\n", command_str->bytes);

        // TODO check for whitespace before calling this and warn if missing,
        // as of C99 (I think) it's required after `#if`
        stream_skip_horizontal_space(stream);

        // Manage conditional stack.
        if (string_equal(command_str, STR_IF) ||
                string_equal(command_str, STR_IFDEF) ||
                string_equal(command_str, STR_IFNDEF))
        {
            file_conditional_push(file, command);
        } else if (file->conditionals_count == 0) {
            fatal_token(command, "#%s without a matching #if, #ifdef or #ifndef.", command->value->bytes);
        }

        // If this is `#endif`, we're done.
        if (command_str == STR_ENDIF) {
            file_conditional_pop(file, command);
            break;
        }

        // Parse the rest of the directive and decide whether to take the branch.
        // TODO need correct end-of-line handling for all of these conditionals, we're not checking properly that there isn't extra tokens before the line ending
        conditional_t* conditional = &file->conditionals[file->conditionals_count - 1];
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
    token_deref(name);
    directive_parse_end_of_line(stream, command);
}

// Handles an #include directive.
static void directive_include(stream_t* stream, token_t* command) {
    stream_skip_horizontal_space(stream);
    token_t* token = token_ref(stream_peek(stream));

    // Collect the rest of the tokens on the line. We need to perform a macro
    // expansion pass on them. (This is necessary even if the first token is a
    // string. See test: ./include/include-macro-blank-after.c)

    vector_t buffer;
    vector_init(&buffer);
    macro_expand_stream(stream, &buffer, false, true);
    if (vector_is_empty(&buffer)) {
        goto error;
    }

    // Find the first and last non-whitespace tokens
    // TODO don't we have helpers for this somewhere?
    token_t* first = NULL;
    token_t* last = NULL;
    for (size_t i = 0; i < vector_count(&buffer); ++i) {
        token_t* t = vector_at(&buffer, i);
        if (t->type != token_type_space) {
            first = t;
            break;
        }
    }
    for (size_t i = vector_count(&buffer); i-- > 0;) {
        token_t* t = vector_at(&buffer, i);
        if (t->type != token_type_space) {
            last = t;
            break;
        }
    }
    if (first == NULL || last == NULL) {
        goto error;
    }

    // At this point we should have either a string, an angle-bracketed
    // filename parsed by the lexer, or a tokenized angle-bracketed filename
    // expanded from a macro.

    if (first->type == token_type_string || first->type == token_type_angle_include) {
        if (first != last) {
            // See test: ./include/include-extra-tokens-quote.c
            // TODO enable warning
            //warn(last, "-Wextra-tokens", "Unexpected token at the end of #include directive");
        }
        token_deref(token);
        token = token_ref(first);
    } else if (token_is_punctuation(first, STR_LESS) && token_is_punctuation(last, STR_GREATER)) {
        // Support for this has been removed. There are too many edge cases
        // to make this work properly.
        fatal_token(first, "Macros in #include that expand to an angle-bracketed filename are not supported.");
    } else {
        goto error;
    }

    // Clean up
    for (size_t i = 0; i < vector_count(&buffer); ++i) {
        token_deref(vector_at(&buffer, i));
    }
    vector_destroy(&buffer);

    // Our token is now the path to include. Do the search.
    assert(token);
    directive_parse_end_of_line(stream, command);
    preprocess_include_search(stream, token);
    token_deref(token);
    return;

error:
    fatal_token(token, "An #include directive must contain a filename enclosed in quotes or angle brackets or a list of macros that expand to one.");
}

static void directive_line(stream_t* stream, token_t* command) {
    stream_skip_horizontal_space(stream);
    if (!stream->use_lexer) {
        fatal_token(command, "#line directive is not supported here.");
    }

    // parse line number
    token_t* token = stream_peek(stream);
    if (token->type != token_type_number) {
        fatal_token(token, "Expected a line number after #line");
    }

    // TODO some compilers allow line numbers with 0x prefix (e.g. tcc).
    // popular compilers don't. we should probably share the number parsing
    // code with the expression parser but also warn if this is used.
    // For now we don't bother.
    // TODO test error handling on floats, 0x prefixes, trailing junk in
    // numbers, etc. The below code should handle it all correctly but we have
    // no tests.

    const char* p = string_cstr(token->value);
    int line = 0;
    while (*p) {
        if ((line == 0 && *p == '0')     // octal invalid
                || *p < '0' || *p > '9') // must be decimal digit
        {
            fatal_token(token, "Invalid line number in #line; expected decimal integer.");
        }
        int new_line = line * 10 + *p++ - '0';
        if (new_line <= line) {
            fatal_token(token, "Overflow in line number in #line directive; line number must fit in a signed 32-bit int.");
        }
        line = new_line;
    }
    stream_consume(stream);

    stream_skip_horizontal_space(stream);
    token = stream_peek(stream);
    string_t* filename = NULL;
    if (token->type == token_type_string) {
        if (token->prefix != token_prefix_none) {
            fatal_token(token, "Filename in #line directive cannot have a string prefix.");
        }
        filename = string_ref(token->value);
        stream_consume(stream);
    }

    // We need to set both token and char location because the current token
    // might be the line ending in which case the following token might have
    // already been parsed, see output of test misc/line-escaped-newline.c for
    // example. TODO I intend to change the lexer to not parse on consume, once
    // that's done one of these can be removed.
    lexer_current->token_location.line = line;
    lexer_current->char_location.line = line;
    if (filename) {
        location_set_filename(&lexer_current->token_location, filename);
        location_set_filename(&lexer_current->char_location, filename);
        string_deref(filename);
    }

    directive_parse_end_of_line(stream, command);
}

static void directive_error_or_warning(stream_t* stream, token_t* directive, bool error) {

    // Collect the rest of the tokens on the line and perform a macro expansion
    // pass on them
    vector_t buffer;
    vector_init(&buffer);
    macro_expand_stream(stream, &buffer, false, true);

    // Stringify the result
    token_t* strtok = token_new_stringify(&buffer, NULL);
    // TODO this contains the string without resolving escape characters. We
    // should resolve escapes. e.g. `#error "foo"` prints `#error: \"foo\"`

    // Output it
    if (error) {
        fatal_token(directive, "#error: %s", string_cstr(strtok->value));
    } else {
        // TODO: this should be a warning, not an error, unless -Werror
        fatal_token(directive, "#warning: %s", string_cstr(strtok->value));
    }

    // Clean up
    token_deref(strtok);
    for (size_t i = 0; i < vector_count(&buffer); ++i) {
        token_deref(vector_at(&buffer, i));
    }
    vector_destroy(&buffer);
}

void directive_parse(stream_t* stream, token_t* directive) {
    assert(directive->type == token_type_directive);
    stream_skip_horizontal_space(stream);
    token_t* command = stream_take(stream);
    //trace("Parsing directive #%s\n", command->value->bytes);

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
        macro_parse(stream);
        directive_parse_end_of_line(stream, command);
    } else if (string_equal(command_str, STR_UNDEF)) {
        directive_undef(stream, command);
    } else if (string_equal(command_str, STR_LINE)) {
        directive_line(stream, command);
    } else if (string_equal(command_str, STR_INCLUDE)) {
        directive_include(stream, command);
    } else if (string_equal(command_str, STR_INCLUDE_NEXT)) {
        fatal_token(command, "#include_next is not yet implemented.");
    } else if (string_equal(command_str, STR_PRAGMA)) {
        // TODO implement #pragma
        goto ignore;
        //directive_pragma(stream);
    } else if (string_equal(command_str, STR_ERROR)) {
        directive_error_or_warning(stream, command, true);
    } else if (string_equal(command_str, STR_WARNING)) {
        directive_error_or_warning(stream, command, false);
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
