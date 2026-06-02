/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2026 Fraser Heavy Software
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

#include "macro.h"

#include <stdlib.h>

#include "stream.h"
#include "strings.h"
#include "preprocess.h"
#include "hideset.h"
#include "options.h"
#include "directive.h"

static table_t macros;

static macro_t* macro_new(token_t* name);
static void macro_define(macro_t* macro);
static void macro_check(macro_t* macro);
static void macro_delete_arg(vector_t* arg);

static macro_function_t macro_builtin_file;
static macro_function_t macro_builtin_line;
static macro_function_t macro_builtin_counter;
static macro_function_t macro_builtin_pragma;

static int macro_counter = 0;

void macro_setup(void) {
    //trace("macro_setup\n");
    table_init(&macros);
}

void macro_teardown(void) {
    //trace("macro_teardown\n");

    // clear macros
    for (table_entry_t** bucket = table_first_bucket(&macros); bucket;
            bucket = table_next_bucket(&macros, bucket))
    {
        for (table_entry_t* entry = *bucket; entry;) {
            //trace("  clearing found an entry\n");
            table_entry_t* next = table_entry_next(entry);
            macro_deref((macro_t*)entry);
            entry = next;
        }
    }
    table_destroy(&macros);
}

static void macro_define_int(const char* cname, int value) {
    string_t* name = string_intern_cstr(cname);
    token_t* name_token = token_new(token_type_alphanumeric, name, &location_builtin);
    macro_t* macro = macro_new(name_token);
    token_deref(name_token);
    string_deref(name);

    vector_append(&macro->expansion, token_new_int(value, &location_builtin, NULL));

    macro_define(macro);
    macro_deref(macro);
}

/**
 * Defines a macro.
 *
 * If arg_count is -1, this is an object-like macro; otherwise it's a
 * function-like macro with the given number of args.
 */
static macro_t* macro_define_function(const char* cname, macro_function_t* function, int arg_count) {
    string_t* name = string_intern_cstr(cname);
    token_t* name_token = token_new(token_type_alphanumeric, name, &location_builtin);
    macro_t* macro = macro_new(name_token);
    token_deref(name_token);
    string_deref(name);

    macro->function = function;

    if (arg_count >= 0) {
        macro->params = vector_new();
        for (int i = 0; i < arg_count; ++i) {
            vector_append(macro->params, string_intern_cstr(""));
        }
    }

    macro_define(macro);
    macro_deref(macro);
    return macro;
}

void macro_define_builtins(void) {
    if (!options_nostddef) {
        macro_define_int("__onramp_cpp__", 1);
    }

    macro_define_function("__FILE__", macro_builtin_file, -1);
    macro_define_function("__LINE__", macro_builtin_line, -1);
    macro_define_function("__COUNTER__", macro_builtin_counter, -1);
    macro_define_function("_Pragma", macro_builtin_pragma, 1);
}

void macro_undef(string_t* name) {
    macro_t* macro = macro_find(name);
    if (macro) {
        table_remove(&macros, &macro->entry);
        macro_deref(macro);
    }
}

/**
 * Creates a new macro, returning a strong reference to it.
 *
 * This does not actually add it to the macros table. You must call
 * macro_define() (and then deref it.)
 */
static macro_t* macro_new(token_t* name) {
    //trace("new macro %s\n", name->value->bytes);

    macro_t* macro = malloc(sizeof(macro_t));
    macro->refcount = 1;
    macro->name = token_ref(name);
    macro->params = NULL;
    macro->is_variadic = false;
    vector_init(&macro->expansion);
    macro->function = NULL;

    return macro;
}

static void macro_define(macro_t* macro) {
    macro_check(macro);

    // Duplicate define should not be possible. A redefinition with the same
    // name is a different macro.
    assert(macro != macro_find(macro->name->value));

    macro_ref(macro);
    macro_undef(macro->name->value);
    table_put(&macros, &macro->entry, macro_hash(macro));

    //macro_print(macro);
}

void macro_print(macro_t* macro) {
    printf("macro %s", macro->name->value->bytes);
    if (macro->params) {
        fputs(" with params ", stdout);
        for (size_t i = 0; i < vector_count(macro->params); ++i) {
            if (i != 0)
                printf(", ");
            fputs(((string_t*)vector_at(macro->params, i))->bytes, stdout);
        }
    }
    printf(":\n");
    for (size_t i = 0; i < vector_count(&macro->expansion); ++i) {
        printf("    ");
        token_print(vector_at(&macro->expansion, i));
    }
}

void macro_deref(macro_t* macro) {
    //trace("deref'ing macro %s\n",macro->name->value->bytes);
    if (--macro->refcount != 0)
        return;
    //trace("deleting macro %s\n",macro->name->value->bytes);

    token_deref(macro->name);

    if (macro->params) {
        for (size_t i = 0; i < vector_count(macro->params); ++i) {
            string_deref(vector_at(macro->params, i));
        }
        vector_delete(macro->params);
    }

    size_t count = vector_count(&macro->expansion);
    for (size_t i = 0; i < count; ++i)
        token_deref(vector_at(&macro->expansion, i));
    vector_destroy(&macro->expansion);

    free(macro);
}

void macro_append(macro_t* macro, token_t* token) {
    vector_append(&macro->expansion, token_ref(token));
}

macro_t* macro_find(string_t* name) {
    for (table_entry_t* entry = table_bucket(&macros, string_hash(name));
            entry; entry = table_entry_next(entry))
    {
        macro_t* macro = (macro_t*)entry;
        if (string_equal(name, macro->name->value)) {
            return macro;
        }
    }
    return NULL;
}

/*
 * Collects macro arguments.
 *
 * The args parameter is an initially empty vector of vectors of tokens. The
 * tokens for each argument are collected into a vector which is appended to
 * the args vector.
 *
 * Returns the closing parenthesis.
 *
 * If the arguments contain preprocessor directives, the directives will be
 * parsed inline before this returns.
 */
static token_t* macro_collect_args(token_t* invocation, macro_t* macro,
        stream_t* stream, vector_t* args, bool stop_on_newline)
{
    //trace("Collecting args for macro %s\n", macro->name->value->bytes);
    vector_t* arg = vector_new();
    int depth = 0;
    size_t params_count = vector_count(macro->params);
    bool in_variadic_arg = macro->is_variadic && params_count == 1;

    token_t* token;
    for (;;) {
        token = stream_take(stream);
        if (token->type == token_type_end) {
            fatal_token(token, "Unclosed macro argument list: expected `)` at end of macro invocation.");
        }
        if (stop_on_newline && token->type == token_type_newline) {
            fatal_token(token, "Unclosed macro argument list. (A macro argument list in a preprocessor directive cannot span multiple lines.)");
        }

        if (token->type == token_type_directive) {
            // TODO this should require -fgnu-extensions.
            directive_parse(stream, token);
            token_deref(token);
            continue;
        }

        bool is_comma = token_is_punctuation(token, STR_COMMA);
        bool is_paren_open = token_is_punctuation(token, STR_PAREN_OPEN);
        bool is_paren_close = token_is_punctuation(token, STR_PAREN_CLOSE);

        // Check for end of argument. A comma ends the argument unless we are
        // parsing the variadic argument.
        if (depth == 0 && ((is_comma && !in_variadic_arg) || is_paren_close)) {
            vector_append(args, arg);

            // Handle end of argument list
            if (is_paren_close) {
                //trace("Collected args:\n"); for (size_t i = 0; i < vector_count(args); ++i) {
                    //trace("  Arg:\n"); for (size_t j = 0; j < vector_count(vector_at(args, i)); ++j) {//trace("    "); token_print(vector_at(vector_at(args, i), j));}}
                break;
            }

            // Start a new argument.
            arg = vector_new();
            token_deref(token);

            // If this is the variadic argument, we'll collect any remaining commas into it.
            if (macro->is_variadic && vector_count(args) == params_count - 1) {
                in_variadic_arg = true;
            }
            continue;
        }

        // Check for nested parentheses
        if (is_paren_open) {
            ++depth;
        } else if (is_paren_close) {
            --depth;
        }

        // Add the token to the current argument
        vector_append(arg, token);
    }

    // Check that the given number of arguments is correct
    size_t args_count = vector_count(args);
    //trace("collected %zi args for %zi params variadic:%i\n", args_count, params_count, macro->is_variadic);
    if (args_count == 1 && params_count == 0) {
        // A function-like macro without parameters can be given only
        // whitespace.
        vector_t* arg = vector_at(args, 0);
        for (size_t i = 0; i < vector_count(arg); ++i) {
            if (!token_is_whitespace(vector_at(arg, i))) {
                fatal_token(vector_at(arg, i), "The argument list to a function-like macro with no parameters must be empty.");
            }
        }

    } else if (macro->is_variadic && args_count == params_count - 1) {
        // The variadic argument is allowed to be omitted in newer C standards.
        // TODO check C version

    } else if (args_count != params_count) {
        fatal_token(invocation, "Wrong number of macro arguments.");

    } else if (macro->is_variadic && params_count == 1) {
        // If the single variadic argument contains only whitespace, delete it
        // so that comma elision and __VA_OPT__ work as expected.
        bool blank = true;
        vector_t* arg = vector_at(args, 0);
        for (size_t i = 0; i < vector_count(arg); ++i) {
            if (!token_is_whitespace(vector_at(arg, i))) {
                blank = false;
                break;
            }
        }
        if (blank) {
            macro_delete_arg(arg);
            vector_remove_all(args);
        }
    }

    return token;
}

/**
 * Deletes the given argument vector.
 */
static void macro_delete_arg(vector_t* arg) {
    for (size_t j = 0; j < vector_count(arg); ++j) {
        token_deref(vector_at(arg, j));
    }
    vector_delete(arg);
}

/*
 * Deletes the argument list collected by macro_collect_args() (the vector of
 * vector of tokens.)
 */
static void macro_delete_args(vector_t* /*nullable*/ args) {
    if (args) {
        for (size_t i = 0; i < vector_count(args); ++i) {
            macro_delete_arg(vector_at(args, i));
        }
        vector_delete(args);
    }
}

/*
 * Expands the given macro with the given arguments, pushing the tokens into
 * the given stream.
 */
static void macro_expand(token_t* token, macro_t* macro, vector_t* /*nullable*/ args,
        stream_t* stream, hideset_t* hideset, location_t* location,
        bool handle_defined)
{
    //trace("Expanding macro %s\n", macro->name->value->bytes);

    // If this is a builtin macro (e.g. __FILE__, __LINE__), delegate to the
    // function that implements it.
    if (macro->function) {
        macro->function(token, macro, args, stream, hideset, location);
        return;
    }

    size_t args_count = args ? vector_count(args) : 0;
    size_t params_count = args ? vector_count(macro->params) : 0;
    void** end = vector_end(&macro->expansion);
    void** start = vector_start(&macro->expansion);

    // Expand tokens, pushing them in reverse order into the stream.
    for (void** p = end; p != start;) {
        //trace("Stack is now:\n"); stream_print_stack(stream);
        --p;
        token_t* current = *p;
        int param = macro_param(macro, current);
        //trace("Expanding macro token %s\n", current->value->bytes);

        // Find the previous and next non-whitespace tokens. We need to
        // know if they're # or ##.
        void** previous_p = token_previous(p, start);
        void** next_p = token_next(p, end);
        token_t* previous = previous_p ? *previous_p : token_end;
        token_t* next = next_p ? (token_t*)*next_p : token_end;
        //trace(" Next is "); token_print(next);
        //trace(" Previous is "); token_print(previous);

        // Check for `defined`. We have to look backwards for `defined` or `defined` `(`.
        // TODO defined in a macro expansion is a GNU extension. We should warn if not -fgnu-extensions. clang calls it -Wexpansion-to-defined
        bool defined_is_enabled = true; // TODO defined should only be enabled in an #if/#elif directive
        if (defined_is_enabled) {
            bool is_defined = false;
            if (token_is_keyword(previous, STR_DEFINED)) {
                //trace("Token %s preceded by `defined`; not expanding\n", current->value->bytes);
                is_defined = true;
            } else if (token_is_punctuation(previous, STR_PAREN_OPEN)) {
                void** prev_prev = token_previous(previous_p, start);
                if (prev_prev && token_is_keyword(*prev_prev, STR_DEFINED)) {
                    //trace("Token %s preceded by `defined` `(`; not expanding\n", current->value->bytes);
                    is_defined = true;
                }
            }
            if (is_defined) {
                if (current->type != token_type_alphanumeric) {
                    fatal_token(current, "`defined` must be followed by a macro name.");
                }
                // We're preceded by `defined` or `defined` `(`. Don't expand this.
                stream_push(stream, token_new_expansion(current, location, hideset));
                continue;
            }
        }

        if (token_is_punctuation(current, STR_HASH)) {
            // Stringify was handled by the stringified token (see below).
            // There's nothing to do here.
            //trace("Skipping stringify operator, already handled\n");
            continue;
        }

        if (token_is_punctuation(current, STR_HASH_HASH)) {
            // Token pasting will be handled by the left token.
            // There's nothing to do here.
            //trace("Skipping paste operator, to be handled\n");
            continue;
        }

        if (token_is_punctuation(next, STR_HASH_HASH)) {
            // It's followed by ##. Token paste.
            if (current->type == token_type_space)
                continue;
            //trace("Token is followed by ##, doing token paste\n");

            // We support comma elision with `, ## __VA_ARGS__`.
            if (token_is_punctuation(current, STR_COMMA)) {
                void** next_next = token_next(next_p, end);
                if (next_next && token_is_keyword(*next_next, STR_VA_ARGS)) {
                    // Only push the comma if a variadic argument was given.
                    if (args_count < params_count) {
                        continue;
                    }
                }
                // The right token has already been pushed. Push the comma.
                stream_push(stream, token_new_expansion(current, location, hideset));
                continue;
            }

            // The right token to paste (or a placeholder) has already been
            // pushed to the stream. The left token must be expanded. We've
            // ensured that both exist in macro_check().
            while (stream_peek(stream)->type == token_type_space) {
                stream_consume(stream);
            }
            token_t* right = stream_take(stream);
            //trace("Right token is: "); token_print(right);

            // If this is a parameter, the left token to paste is the
            // right-most non-space parameter of the expansion. Otherwise it's
            // just this.
            token_t* left;
            if (param == -1) {
                left = current;
            } else {
                vector_t* arg = vector_at(args, param);
                if (vector_is_empty(arg)) {
                    left = NULL;
                } else {
                    size_t j = vector_count(arg);
                    do {
                        left = vector_at(arg, --j);
                    } while (left->type == token_type_space);

                    // Push the rest of the argument to the stream
                    for (; j-- > 0;) {
                        stream_push(stream,
                                token_new_expansion(vector_at(arg, j), location, hideset));
                    }
                }
            }

            // We keep the type of the left token, unless we don't have one, in
            // which case it's the right token. (And if we have neither, we end
            // up with a placeholder, which is an empty token of type
            // alphanumeric.)
            if (left == NULL) {
                //trace("Left token is NULL");
                stream_push(stream, token_new_expansion(right, location, hideset));
            } else {
                //trace("Left token is: "); token_print(left);
                token_t* token = token_new_expansion(left, location, hideset);
                string_deref(token->value);
                token->value = string_concat(left->value, right->value);
                stream_push(stream, token);
            }

            token_deref(right);
            //trace("Pasted token is: "); token_print(stream_peek(stream));
            continue;
        }

        if (param == -1) {
            // Not a parameter; just push it
            // TODO do we recursively expand object-like macros here? I'm
            // pretty sure we don't but this might be wrong.
            //trace("Pushing %s to stack with new hideset.\n", current->value->bytes);
            stream_push(stream, token_new_expansion(current, location, hideset));
            continue;
        }

        if (token_is_punctuation(previous, STR_HASH)) {
            // It's a parameter preceded by #. Stringify the expansion.
            //trace("Stringifying %s\n", current->value->bytes);
            if ((size_t)param == args_count) {
                // The variadic argument is blank or not provided. It
                // stringifies to the empty string (even if it contained
                // whitespace.)
                token_t* token = token_new(token_type_string, STR_EMPTY, location);
                token->hideset = hideset_ref(hideset);
                stream_push(stream, token);
                continue;
            }
            vector_t* arg = vector_at(args, param);
            stream_push(stream, token_new_stringify(arg, hideset));
            continue;
        }

        if (token_is_punctuation(previous, STR_HASH_HASH)) {
            // It's a parameter preceded by ##. Push the argument without macro
            // expansion; the first token will be pasted.
            //trace("Parameter to be token pasted; pushing arg without expansion\n");

            bool empty = true;

            if ((size_t)param != args_count) { // in case the variadic parameter was omitted
                vector_t* arg = vector_at(args, param);
                for (size_t j = vector_count(arg); j-- > 0;) {
                    token_t* t = vector_at(arg, j);
                    stream_push(stream, token_new_expansion(t, location, hideset));
                    if (t->type != token_type_space)
                        empty = false;
                }
            }
            if (empty) {
                // The argument list is empty. We need to push a placeholder token.
                stream_push(stream, token_new_bytes(token_type_alphanumeric,
                            NULL, 0, &location_builtin));
            }
            continue;
        }

        if (string_equal(current->value, STR_VA_ARGS) && args_count < params_count) {
            // __VA_ARGS__ but the variadic argument was omitted. Nothing to do.
            continue;
        }

        {
            // The parameter is not stringified or token pasted. We need to
            // recursively expand it before pushing it into the stream.
            //trace("Token %s is a parameter with no # or ##.\n", current->value->bytes);

            vector_t* arg = vector_at(args, param);
            stream_t arg_stream;
            stream_init(&arg_stream, false, arg);
            vector_t arg_buffer;
            vector_init(&arg_buffer);

            macro_expand_stream(&arg_stream, &arg_buffer, handle_defined, false);

            // Push the resulting token list in reverse order.
            //trace("Pushing argument replacement list\n");
            for (size_t j = vector_count(&arg_buffer); j-- > 0;) {
                token_t* t = vector_at(&arg_buffer, j);
                stream_push(stream, token_new_expansion(t, location, hideset));
                token_deref(t);
            }
            vector_destroy(&arg_buffer);
            stream_destroy(&arg_stream);
            continue;
        }
    }
    //trace("Done expanding macro %s\n", macro->name->value->bytes);
}

void macro_expand_stream(stream_t* stream, vector_t* /*nullable*/ output, bool handle_defined, bool stop_on_newline) {
    //trace("Starting stream expansion at token: "); token_print(stream_peek(stream));

    for (;;) {
        token_t* token = stream_peek(stream);
        // TODO for now we back out on directives, need to replace preprocess_run() with this
        if (token->type == token_type_directive || token->type == token_type_end) {
            break;
        }
        if (stop_on_newline && token->type == token_type_newline) {
            break;
        }

        token_ref(token);
        stream_consume(stream);
        //trace("  Macro expansion token is: "); token_print(token);
        //trace("  Stack is:\n"); stream_print_stack(stream);

        // Special handling for `defined`
        if (handle_defined && token_is_keyword(token, STR_DEFINED)) {
            output_token(output, token);
            token_deref(token);
            stream_forward_spaces(stream, output);

            // A `(` is allowed after `defined`
            if (stream_is(stream, STR_PAREN_OPEN)) {
                output_token(output, stream_peek(stream));
                stream_consume(stream);
            }
            stream_forward_spaces(stream, output);

            // If it's followed by an identifier, don't macro-expand it
            if (stream_peek(stream)->type == token_type_alphanumeric) {
                output_token(output, stream_peek(stream));
                stream_consume(stream);
            }

            continue;
        }

        // If the token is in its hideset, skip expansion and output it.
        // (We do this before checking if it's a macro because the hideset is
        // usually empty and probably always smaller than the set of all macros.)
        if (token->hideset && hideset_contains(token->hideset, token->value)) {
            //trace("Token %s is in its own hideset. Outputting.\n", token->value->bytes);
            output_token(output, token);
            token_deref(token);
            continue;
        }

        // Find the macro. If it's not a macro, just output it.
        macro_t* macro = (token->type == token_type_alphanumeric) ? macro_find(token->value) : NULL;
        if (macro == NULL) {
            output_token(output, token);
            token_deref(token);
            continue;
        }
        //trace("Found macro %s\n", token->value->bytes);

        vector_t* args = NULL; // vector of vectors of tokens
        hideset_t* hideset;

        if (macro->params == NULL) {

            // Generate a hideset for expanded tokens
            hideset = hideset_new(token->hideset, macro->name->value);

        } else {
            //trace("Macro %s takes args\n", token->value->bytes);

            // A macro with parameters is only expanded if it is followed
            // directly by an open parenthesis (with only whitespace allowed;
            // in particular, no directives are allowed before the opening
            // parenthesis.)
            if (stop_on_newline) {
                stream_skip_horizontal_space(stream);
            } else {
                stream_skip_whitespace(stream);
            }
            if (!stream_accept(stream, STR_PAREN_OPEN)) {
                // No parenthesis; just output it.
                //trace("Object-like macro %s is not followed by an open paren. Outputting as-is\n", token->value->bytes);
                output_token(output, token);
                token_deref(token);
                continue;
            }

            // Collect the arguments, running any embedded directives
            args = vector_new();
            token_t* paren_close = macro_collect_args(token, macro, stream, args, stop_on_newline);

            // Generate the hideset. Dave Prosser's algorithm is to intersect
            // the hideset with that of the closing parenthesis.
            hideset = hideset_new_intersection(token->hideset, paren_close->hideset,
                    macro->name->value);
            token_deref(paren_close);
        }

        // Perform the expansion
        macro_expand(token, macro, args, stream, hideset, &token->location, handle_defined);

        // Clean up
        hideset_deref(hideset);
        macro_delete_args(args);
        token_deref(token);
    }

    //trace("Done stream expansion\n");
}

void macro_parse(stream_t* stream) {
    stream_skip_horizontal_space(stream);

    // Get the name
    token_t* macro_name = stream_take(stream);
    if (macro_name->type != token_type_alphanumeric) {
        //token_print(name);
        fatal_token(macro_name, "Expected an identifier after `#define`.");
    }
    if (string_equal(macro_name->value, STR_DEFINED)) {
        fatal_token(macro_name, "`defined` is not a valid macro name.");
    }

    // Create the macro
    macro_t* macro = macro_new(macro_name);
    token_deref(macro_name);

    // Parse parameter list
    if (stream_accept(stream, STR_PAREN_OPEN)) {
        macro->params = vector_new();
        stream_skip_horizontal_space(stream);
        if (!stream_accept(stream, STR_PAREN_CLOSE)) {
            for (;;) {
                token_t* param_name = stream_peek(stream);

                // Check for variadic macro
                if (token_is_punctuation(param_name, STR_ELLIPSIS)) {
                    macro->is_variadic = true;
                    vector_append(macro->params, string_ref(STR_VA_ARGS));
                    stream_consume(stream);
                    stream_skip_horizontal_space(stream);
                    stream_expect(stream, STR_PAREN_CLOSE, "Expected `)` after `...` in macro parameter list.");
                    break;
                }

                // Found a named parameter
                if (param_name->type != token_type_alphanumeric) {
                    fatal_token(param_name, "Expected a parameter name or `)` or `...` in macro parameter list.");
                }
                vector_append(macro->params, string_ref(param_name->value));

                // Interesting behaviour from compilers if an argument is named
                // __VA_ARGS__. GCC forbids it; Clang and chibicc expand it to
                // the named argument; TinyCC expands it to the variadic
                // arguments. We forbid it as well.
                if (string_equal(param_name->value, STR_VA_ARGS)) {
                    fatal_token(param_name, "A macro parameter cannot be named __VA_ARGS__.");
                }
                stream_consume(stream);

                // Check for end of parameter list
                stream_skip_horizontal_space(stream);
                if (stream_accept(stream, STR_PAREN_CLOSE))
                    break;

                // Otherwise we need a comma
                stream_expect(stream, STR_COMMA, "Expected `,` or `)` after macro parameter name.");
                stream_skip_horizontal_space(stream);
            }
        }

    } else {
        token_t* first = stream_peek(stream);
        if (first->type != token_type_space && first->type != token_type_newline) {
        // Note that we don't require whitespace after the macro name in an
        // object-like define. It's technically required as of C99, but we
        // don't require it for backwards compatibility with C89 and for
        // compatibility with GCC and Clang.
        //TODO enable once we implement warn. also check and improve error message
        //warn(first, "Horizontal whitespace is required after the macro name in an object-like `#define` directive in C99 and later.");
        //TODO test what happens if the define is for a string prefix
        //e.g.:
        //#define FOO"abc" // should work
        //#define u8"abc"  // ??
        // TODO note that we're already skipping horizontal space in directive_parse()
        }
    }

    // We don't want to bother storing (and expanding) an expansion list if
    // it's only going to contain whitespace.
    stream_skip_horizontal_space(stream);

    // Collect rest of line
    for (;;) {
        token_t* token = stream_peek(stream);
        if (token->type == token_type_newline)
            break;
        macro_append(macro, token);
        stream_consume(stream);
    }

    /*
    //trace("defined macro %s:\n", name->value->bytes);
    for (size_t i = 0; i < vector_count(&macro->expansion); ++i) {
        //trace("    ");
        token_print(vector_at(&macro->expansion, i));
    }
    */

    macro_define(macro);
    macro_deref(macro);
}

int macro_param(macro_t* macro, token_t* token) {
    if (!macro->params) {
        return -1;
    }
    if (token->type != token_type_alphanumeric) {
        return -1;
    }
    for (size_t i = 0; i < vector_count(macro->params); ++i) {
        if (string_equal(token->value, vector_at(macro->params, i))) {
            return (int)i;
        }
    }
    return -1;
}

/*
 * Checks that the macro's expansion sequence is valid (e.g. that # and ## are
 * used correctly.)
 *
 * Most preprocessors diagnose these errors even if the macro is not used so
 * we do as well. It is also easier to do these checks beforehand rather than
 * doing them in the macro expansion algorithm.
 */
static void macro_check(macro_t* macro) {
    if (macro->function) {
        //trace("macro_check() function, nothing to do\n");
        return;
    }
    //trace("macro_check() %s\n", macro->name->value->bytes);

    vector_t* expansion = &macro->expansion;
    void** start = vector_start(expansion);
    void** end = vector_end(expansion);
    if (start == end)
        return;

    // Find the first and last non-whitespace tokens
    if (((token_t*)*start)->type == token_type_space) {
        start = token_next(start, end);
    }
    void** last = end - 1;
    if (((token_t*)*last)->type == token_type_space) {
        last = token_previous(last, start);
    }

    // Check that ## has a non-whitespace token on both sides
    if (start && token_is_punctuation(*start, STR_HASH_HASH)) {
        fatal_token(*start, "A macro expansion sequence cannot start with `##`.");
    }
    if (last && token_is_punctuation(*last, STR_HASH_HASH)) {
        fatal_token(*last, "A macro expansion sequence cannot end with `##`.");
    }
    end = last + 1;

    for (void** p = start; p != end; ++p) {
        //trace("  checking token: "); token_print(*p);

        // Check that ## does not appear twice in a row
        if (token_is_punctuation(*p, STR_HASH_HASH)) {
            void** token_loc = token_next(p, end);
            assert(token_loc); // checked above
            if (token_is_punctuation(*token_loc, STR_HASH_HASH)) {
                fatal_token(*token_loc, "The `##` macro operator cannot appear twice in a row.");
            }
        }

        // Check that # is always followed by a parameter
        if (token_is_punctuation(*p, STR_HASH)) {
            void** token_loc = token_next(p, end);
            if (token_loc == NULL || macro_param(macro, *token_loc) == -1) {
                token_t* token = token_loc ? (token_t*)*token_loc : token_end;
                fatal_token((token->type != token_type_end) ? token : *p,
                        "The `#` operator in a macro must be followed by a parameter.");
            }
        }

    }
}

static void macro_builtin_file(token_t* token, macro_t* macro,
        vector_t* /*nullable*/ args, stream_t* stream,
        hideset_t* hideset, location_t* location)
{
    token = token_new(token_type_string, location->filename, location);
    token->hideset = hideset ? hideset_ref(hideset) : NULL;
    stream_push(stream, token);
}

static void macro_builtin_line(token_t* token, macro_t* macro,
        vector_t* /*nullable*/ args, stream_t* stream,
        hideset_t* hideset, location_t* location)
{
    stream_push(stream, token_new_int(location->line, location, hideset));
}

static void macro_builtin_counter(token_t* token, macro_t* macro,
        vector_t* /*nullable*/ args, stream_t* stream,
        hideset_t* hideset, location_t* location)
{
    stream_push(stream, token_new_int(macro_counter++, location, hideset));
}

static void macro_builtin_pragma(token_t* token, macro_t* macro,
        vector_t* /*nullable*/ args, stream_t* stream,
        hideset_t* hideset, location_t* location)
{
    if (vector_count(args) != 1)
        goto error;
    vector_t* arg = vector_at(args, 0);
    void** start = vector_start(arg);
    void** end = vector_end(arg);
    if (start == end)
        goto error;

    // There should be a single non-whitespace token. Find it
    if (((token_t*)*start)->type == token_type_space) {
        start = token_next(start, end);
    }
    if (start == NULL)
        goto error;
    if (token_next(start, end) != NULL)
        goto error;
    token = *start;

    // Make sure it's a string
    if (token->type != token_type_string)
        goto error;

    // Push a pragma token into the stream. It will get emitted as its own
    // #pragma line by the emitter.
    token = token_new(token_type_pragma, token->value, location);
    token->hideset = hideset_ref(hideset);
    stream_push(stream, token);
    return;

error:
    fatal_token(token, "_Pragma() must contain a string.");
}
