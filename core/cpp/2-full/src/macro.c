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

#include "macro.h"

#include <stdlib.h>

#include "stream.h"
#include "strings.h"
#include "preprocess.h"
#include "hideset.h"
#include "options.h"

static table_t macros;

static macro_t* macro_new(token_t* name);
static void macro_define(macro_t* macro);
static void macro_check(macro_t* macro);

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
    location_t location;
    location_init_builtin(&location);

    string_t* name = string_intern_cstr(cname);
    token_t* name_token = token_new(token_type_alphanumeric, name, &location);
    macro_t* macro = macro_new(name_token);
    token_deref(name_token);
    string_deref(name);

    char buf[16];
    sprintf(buf, "%i", value);
    string_t* numstr = string_intern_cstr(buf);
    vector_append(&macro->expansion, token_new(token_type_number, numstr, &location));
    string_deref(numstr);
    location_destroy(&location);

    macro_define(macro);
    macro_deref(macro);
}

void macro_define_builtins(void) {
    if (!options_nostddef) {
        macro_define_int("__onramp_cpp__", 1);
        // TODO for now we're pretending to be cpp/1 until object-like macros and #if expressions are fully implemented
        macro_define_int("__onramp_cpp_omc__", 1);
    }

    // TODO define __LINE__, __FILE__, etc.
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
 */
static token_t* macro_collect_args(macro_t* macro, stream_t* stream, vector_t* args) {
    //trace("Collecting args for macro %s\n", macro->name->value->bytes);
    vector_t* arg = vector_new();
    int depth = 0;
    for (;;) {
        token_t* token = stream_take(stream);
        if (token->type == token_type_end) {
            fatal_token(token, "Unclosed macro argument list: expected `)` at end of macro invocation.");
        }

        bool is_comma = token_is_punctuation(token, STR_COMMA);
        bool is_paren_open = token_is_punctuation(token, STR_PAREN_OPEN);
        bool is_paren_close = token_is_punctuation(token, STR_PAREN_CLOSE);

        // Check for end of argument
        if (depth == 0 && (is_comma || is_paren_close)) {
            vector_append(args, arg);

            // Handle end of argument list
            if (is_paren_close) {
                //trace("Collected args:\n"); for (size_t i = 0; i < vector_count(args); ++i) {
                    //trace("  Arg:\n"); for (size_t j = 0; j < vector_count(vector_at(args, i)); ++j) {trace("    "); token_print(vector_at(vector_at(args, i), j));}}
                return token;
            }

            // It's a comma. Check for too many arguments
            if (!macro->is_variadic && vector_count(args) == vector_count(macro->params)) {
                fatal_token(token, "Too many arguments for non-variadic macro.");
            }

            // Start a new argument.
            arg = vector_new();
            token_deref(token);
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
}

/*
 * Destroys the argument list collected by macro_collect_args() (the vector of
 * vector of tokens.)
 */
static void macro_destroy_args(vector_t* /*nullable*/ args) {
    if (args) {
        for (size_t i = 0; i < vector_count(args); ++i) {
            vector_t* arg = vector_at(args, i);
            for (size_t j = 0; j < vector_count(arg); ++j) {
                token_deref(vector_at(arg, j));
            }
            vector_delete(arg);
        }
        vector_delete(args);
    }
}

/*
 * Expands the given macro the given arguments onto the given stack.
 */
static void macro_expand_impl(macro_t* macro, vector_t* /*nullable*/ args,
        stream_t* stream, hideset_t* hideset, location_t* location)
{
    assert((args == NULL) == (macro->params == NULL));

    // Expand tokens, pushing them in reverse order into the stream.
    //trace("Expanding macro %s\n", macro->name->value->bytes);
    void** end = vector_end(&macro->expansion);
    void** start = vector_start(&macro->expansion);
    void** p = end;

    while (p != start) {
        --p;
        token_t* current = *p;
        int param = macro_param(macro, current);
        //trace("Expanding macro token %s\n", current->value->bytes);

        // Find the previous and next non-whitespace tokens. We need to
        // know if they're # or ##.
        token_t* previous = token_previous(p, start);
        token_t* next = token_next(p, end);
        //trace(" Next is "); token_print(next);
        //trace(" Previous is "); token_print(previous);

        if (token_is_punctuation(current, STR_HASH)) {
            // Stringify was handled by the stringified token (see below).
            // There's nothing to do here.
            //trace("Skipping stringify operator, already handled\n");

        } else if (token_is_punctuation(current, STR_HASH_HASH)) {
            // Token pasting will be handled by the left token.
            // There's nothing to do here.
            //trace("Skipping paste operator, to be handled\n");

        } else if (token_is_punctuation(next, STR_HASH_HASH)) {
            // It's followed by ##. Token paste.
            if (current->type == token_type_space)
                continue;
            //trace("Token is followed by ##, doing token paste\n");

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

        } else if (param == -1) {
            // Not a parameter; just push it
            // TODO do we recursively expand object-like macros here? I'm
            // pretty sure we don't but this might be wrong.
            //trace("Pushing %s to stack with new hideset.\n", current->value->bytes);
            stream_push(stream, token_new_expansion(current, location, hideset));

        } else if (token_is_punctuation(previous, STR_HASH)) {
            // It's a parameter preceded by #. Stringify the expansion.
            //trace("Stringifying %s\n", current->value->bytes);
            vector_t* arg = vector_at(args, param);
            stream_push(stream, token_new_stringify(arg, hideset));

        } else if (token_is_punctuation(previous, STR_HASH_HASH)) {
            // It's a parameter preceded by ##. Push the argument without macro
            // expansion; the first token will be pasted.
            //trace("Parameter to be token pasted; pushing arg without expansion\n");

            bool empty = true;

            vector_t* arg = vector_at(args, param);
            for (size_t j = vector_count(arg); j-- > 0;) {
                token_t* t = vector_at(arg, j);
                stream_push(stream, token_new_expansion(t, location, hideset));
                if (t->type != token_type_space)
                    empty = false;
            }
            if (empty) {
                // The argument list is empty. We need to push a placeholder token.
                stream_push(stream, token_new_bytes(token_type_alphanumeric,
                            NULL, 0, &location_builtin));
            }

        } else {
            // The parameter is not stringified or token pasted. We need to
            // recursively expand it before pushing it into the stream.
            //trace("Token %s is a parameter with no # or ##.\n", current->value->bytes);

            vector_t* arg = vector_at(args, param);
            stream_t arg_stream;
            stream_init(&arg_stream, false, arg);
            vector_t arg_buffer;
            vector_init(&arg_buffer);

            // TODO loop shouldn't be necessary, need to fix macro_expand() loop
            while (stream_peek(&arg_stream)->type != token_type_end) {
                token_t* t = stream_take(&arg_stream);
                macro_expand(&arg_stream, &arg_buffer, t); 
                token_deref(t);
            }

            // Push the resulting token list in reverse order.
            //trace("Pushing argument replacement list\n");
            for (size_t j = vector_count(&arg_buffer); j-- > 0;) {
                token_t* t = vector_at(&arg_buffer, j);
                stream_push(stream, token_new_expansion(t, location, hideset));
                token_deref(t);
            }
            vector_destroy(&arg_buffer);
            stream_destroy(&arg_stream);
        }

        //trace("Stack is now:\n"); stream_print_stack(stream);
    }
    //trace("Done expanding macro %s\n", macro->name->value->bytes);
}

void macro_expand(stream_t* stream, vector_t* /*nullable*/ output, token_t* token) {
    //trace("Starting macro expansion at token %s\n", token->value->bytes);

    token_ref(token);
    /*
    goto start;

    while (!vector_is_empty(&stack)) {
        token = vector_remove_last(&stack);
    start:
    */
        //trace("  Macro expansion token is: "); token_print(token);
        //trace("  Stack is:\n"); for (size_t i = vector_count(&stack); i-- > 0;) {trace("    "); token_print(vector_at(&stack, i));}

        // If the token is in its hideset, skip expansion and output it.
        // (We do this before checking if it's a macro because the hideset is
        // usually empty and probably always smaller than the set of all macros.)
        if (token->hideset && hideset_contains(token->hideset, token->value)) {
            //trace("Token %s is in its own hideset. Outputting.\n", token->value->bytes);
            output_token(output, token);
            token_deref(token);
            return;//continue;
        }

        // Find the macro. If it's not a macro, just output it.
        macro_t* macro = (token->type == token_type_alphanumeric) ? macro_find(token->value) : NULL;
        if (macro == NULL) {
            output_token(output, token);
            token_deref(token);
            return;//continue;
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
            stream_skip_whitespace(stream);
            if (!stream_accept(stream, STR_PAREN_OPEN)) {
                // No parenthesis; just output it.
                //trace("Object-like macro %s is not followed by an open paren. Outputting as-is\n", token->value->bytes);
                output_token(output, token);
                token_deref(token);
                return;//continue;
            }

            // Collect the arguments
            args = vector_new();
            token_t* paren_close = macro_collect_args(macro, stream, args);

            // Generate the hideset. Dave Prosser's algorithm is to intersect
            // the hideset with that of the closing parenthesis.
            hideset = hideset_new_intersection(token->hideset, paren_close->hideset,
                    macro->name->value);
            token_deref(paren_close);
        }

        // Perform the expansion
        macro_expand_impl(macro, args, stream, hideset, &token->location);

        // Clean up
        hideset_deref(hideset);
        macro_destroy_args(args);
        token_deref(token);
        /*
    }

    //trace("Done token macro expansion\n");
    */
}

void macro_parse(stream_t* stream) {
    stream_skip_horizontal_space(stream);

    // Get the name
    token_t* name = stream_take(stream);
    if (name->type != token_type_alphanumeric) {
        //token_print(name);
        fatal_token(name, "Expected an identifier after `#define`.");
    }

    // Create the macro
    macro_t* macro = macro_new(name);
    token_deref(name);

    // Parse parameter list
    if (stream_accept(stream, STR_PAREN_OPEN)) {
        macro->params = vector_new();
        stream_skip_horizontal_space(stream);
        if (!stream_accept(stream, STR_PAREN_CLOSE)) {
            for (;;) {

                // Check for variadic macro
                if (token_is_punctuation(name, STR_ELLIPSIS)) {
                    macro->is_variadic = true;
                    stream_skip_horizontal_space(stream);
                    stream_expect(stream, STR_PAREN_CLOSE, "Expected `)` after `...` in macro parameter list.");
                    break;
                }

                // Found a named parameter
                token_t* name = stream_peek(stream);
                if (name->type != token_type_alphanumeric) {
                    fatal_token(name, "Expected a parameter name or `)` or `...` in macro parameter list.");
                }
                vector_append(macro->params, string_ref(name->value));
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
    if (macro->function)
        return;

    vector_t* expansion = &macro->expansion;
    void** start = vector_start(expansion);
    void** end = vector_end(expansion);
    if (start == end)
        return;

    // Find the first and last non-whitespace tokens
    token_t* first = *start;
    token_t* last = *(end - 1);
    if (first->type == token_type_space) {
        first = token_next(start, end);
    }
    if (last->type == token_type_space) {
        last = token_previous(end - 1, start);
    }

    // Check that ## has a non-whitespace token on both sides
    if (token_is_punctuation(first, STR_HASH_HASH)) {
        fatal_token(first, "A macro expansion sequence cannot start with `##`.");
    }
    if (token_is_punctuation(last, STR_HASH_HASH)) {
        fatal_token(last, "A macro expansion sequence cannot end with `##`.");
    }

    for (void** p = start; p != end; ++p) {

        // Check that ## does not appear twice in a row
        if (token_is_punctuation(*p, STR_HASH_HASH)) {
            token_t* token = token_next(p, end);
            if (token_is_punctuation(token, STR_HASH_HASH)) {
                fatal_token(token, "The `##` macro operator cannot appear twice in a row.");
            }
        }

        // Check that # is always followed by a parameter
        if (token_is_punctuation(*p, STR_HASH)) {
            token_t* token = token_next(p, end);
            int param = macro_param(macro, token);
            if (param == -1) {
                fatal_token((token->type != token_type_end) ? token : *p,
                        "The `#` operator in a macro must be followed by a parameter.");
            }
        }

    }
}
