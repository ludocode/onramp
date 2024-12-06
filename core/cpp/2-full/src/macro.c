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

#include "token.h"
#include "stream.h"
#include "strings.h"
#include "preprocess.h"
#include "hideset.h"

static table_t macros;

void macro_setup(void) {
    //printf("macro_setup\n");
    table_init(&macros);
}

void macro_teardown(void) {
    //printf("macro_teardown\n");

    // clear macros
    for (table_entry_t** bucket = table_first_bucket(&macros); bucket;
            bucket = table_next_bucket(&macros, bucket))
    {
        for (table_entry_t* entry = *bucket; entry;) {
//printf("  clearing found an entry\n");
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
}

void macro_define_builtins(void) {
    // TODO check -nostddef
    // TODO for now we're pretending to be cpp/1 until object-like macros and #if expressions are fully implemented
    // TODO define __LINE__, __FILE__, etc.

    macro_define_int("__onramp_cpp__", 1);
    macro_define_int("__onramp_cpp_omc__", 1);
}

void macro_undef(string_t* name) {
    macro_t* macro = macro_find(name);
    if (macro) {
        table_remove(&macros, &macro->entry);
        macro_deref(macro);
    }
}

macro_t* macro_new(token_t* name) {
    macro_undef(name->value);
    //printf("new macro %s\n", name->value->bytes);

    macro_t* macro = malloc(sizeof(macro_t));
    macro->refcount = 1;
    macro->name = token_ref(name);
    macro->params = NULL;
    vector_init(&macro->expansion);
    macro->function = NULL;

    table_put(&macros, &macro->entry, macro_hash(macro));
    return macro;
}

void macro_deref(macro_t* macro) {
    //printf("deref'ing macro %s\n",macro->name->value->bytes);
    if (--macro->refcount != 0)
        return;
    //printf("deleting macro %s\n",macro->name->value->bytes);

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

void macro_expand(stream_t* stream, vector_t* /*nullable*/ output, token_t* token) {
    vector_t stack;
    vector_init(&stack);
    token_ref(token);
    goto start;

    while (!vector_is_empty(&stack)) {
        token = vector_remove_last(&stack);
    start:

        // If the token is in its hideset, skip expansion and output it.
        // (We do this before checking if it's a macro because the hideset is
        // usually empty and probably always smaller than the set of all macros.)
        if (token->hideset && hideset_contains(token->hideset, token->value)) {
            //printf("Token %s is in its own hideset. Outputting.\n", token->value->bytes);
            output_token(output, token);
            token_deref(token);
            continue;
        }

        // Find the macro. If it's not a macro, just output it.
        macro_t* macro = macro_find(token->value);
        if (macro == NULL) {
            output_token(output, token);
            token_deref(token);
            continue;
        }
        //printf("Found macro %s\n", token->value->bytes);

        vector_t* args = NULL; // vector of vectors of tokens
        hideset_t* hideset;

        if (macro->params == NULL) {
            // Generate a hideset for expanded tokens
            hideset = hideset_new(token->hideset, macro->name->value);

        } else {
            //printf("Macro %s takes args\n", token->value->bytes);

            // A macro with parameters is only expanded if it is followed
            // directly by an open parenthesis (with only whitespace allowed;
            // in particular, no directives are allowed before the opening
            // parenthesis.)
            stream_skip_whitespace(stream);
            if (!stream_accept(stream, STR_PAREN_OPEN)) {
                // No parenthesis; just output it.
                output_token(output, token);
                token_deref(token);
                continue;
            }

            // Collect arguments.
            token_t* paren_close = NULL;
            args = vector_new();
            vector_t* arg = vector_new();
            int depth = 0;
            for (;;) {
                token_t* t = stream_take(stream);
                if (t->type == token_type_end) {
                    fatal_token(token, "Unclosed macro argument list: expected `)` at end of macro invocation.");
                }

                bool is_comma = token_is_punctuation(t, STR_COMMA);
                bool is_paren_open = token_is_punctuation(t, STR_PAREN_OPEN);
                bool is_paren_close = token_is_punctuation(t, STR_PAREN_CLOSE);

                // Check for end of argument
                if (depth == 0 && (is_comma || is_paren_close)) {
                    vector_append(args, arg);
                    if (!macro->is_variadic && is_comma && vector_count(args) == vector_count(macro->params)) {
                        fatal_token(t, "Too many arguments for non-variadic macro.");
                    }

                    // Handle end of argument list
                    if (is_paren_close) {
                        paren_close = t;
                        break;
                    }

                    // It's a comma. Start a new argument.
                    arg = vector_new();
                    token_deref(t);
                    continue;
                }

                // This is not the end of the argument. Handle nested parentheses.
                if (is_paren_open)
                    ++depth;
                else if (is_paren_close)
                    --depth;
                vector_append(arg, t);
            }

            // Dave Prosser's algorithm is to intersect the hideset with that of
            // the closing parenthesis.
            hideset = hideset_new_intersection(token->hideset, paren_close->hideset,
                    macro->name->value);
            token_deref(paren_close);
        }

        // Expand tokens, pushing them in reverse order onto the stack.
        // TODO we need to add token pasting and stringify somewhere in here
        void** p = vector_end(&macro->expansion);
        void** start = vector_start(&macro->expansion);
        do {
            --p;
            token_t* t = *p;
            //printf("pushing token: ");
            //token_print(t);

            // Check if it's a parameter
            if (macro->params && t->type == token_type_alphanumeric) {
                for (size_t i = 0; i < vector_count(macro->params); ++i) {
                    if (string_equal(t->value, vector_at(macro->params, i))) {

                        // It's a parameter. Push the argument token list in its place, also in reverse order.
                        // TODO if token pasting, don't push; just output
                        // TODO if stringify, stringify
                        vector_t* arg = vector_at(args, i);
                        for (size_t j = vector_count(arg); j-- > 0;) {
                            vector_append(&stack, token_new_expansion(vector_at(arg, j), &token->location, hideset));
                        }

                        // break and continue
                        goto continue_expand;
                    }
                }
            }

            // Not a parameter; push it
            vector_append(&stack, token_new_expansion(*p, &token->location, hideset));
        continue_expand:;
        } while (p != start);

        // Clean up
        hideset_deref(hideset);
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
        token_deref(token);
    }

    vector_destroy(&stack);
}

void macro_define(stream_t* stream) {
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
    printf("defined macro %s:\n", name->value->bytes);
    for (size_t i = 0; i < vector_count(&macro->expansion); ++i) {
        printf("    ");
        token_print(vector_at(&macro->expansion, i));
    }
    */
}
