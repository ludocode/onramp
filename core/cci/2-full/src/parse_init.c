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

#include "parse_init.h"

#include "parse_expr.h"
#include "node.h"
#include "record.h"
#include "type.h"
#include "options.h"
#include "lexer.h"
#include "strings.h"
#include "token.h"

/**
 * If there's a previous initializer for this node, delete it, and issue a
 * warning at the current token that an initializer is being overridden.
 */
static void parse_init_clear(node_t* node, size_t index) {
    if (index >= vector_count(&node->initializers))
        return;

    void** old_initializer = vector_address(&node->initializers, index);
    if (*old_initializer) {
        warn(warning_initializer_overrides, lexer_token,
                "An initializer overrides a previous initializer for the same location.");
        node_delete(*old_initializer);
        *old_initializer = NULL;
    }
}

/**
 * Parses part of an initializer list, filling the given struct or array
 * starting at the given index.
 *
 * If we reach a nested struct or array, we recurse (whether or not it has
 * braces.)
 *
 * If root is true, this is the root object of a possibly nested initializer
 * list (i.e. this object is the most recent `{`, but not necessarily the
 * outer-most `{`). In this case we can accept designators because they are
 * relative to this object.
 *
 * If root if false, we're a nested object without a `{`. We can start with a
 * designator (since we might be continuing a designator from the parent, and
 * it would have handled it otherwise), but if we hit a designator after the
 * first element we'll back out and let the parent handle it.
 */
static void parse_initializer_list_part(type_t* type, node_t* node, size_t index, bool is_object) {
    bool first = true;

    for (;;) {

        // check for designators

        if (lexer_is(STR_SQUARE_OPEN)) {
            if (!is_object && !first)
                break;
            if (!type_is_array(type)) {
                fatal_token(lexer_token, "Cannot use an array designator on something that isn't an array.");
            }
            fatal_token(lexer_token, "TODO array designator not yet implemented.");

        } else if (lexer_is(STR_DOT)) {
            if (!is_object && !first)
                break;
            if (!type_matches_base(type, BASE_RECORD))
                fatal_token(lexer_token, "Cannot use a member designator on something that isn't a struct or union.");
            fatal_token(lexer_token, "TODO member designator not yet implemented.");
        }



        // TODO check if the current position is valid. in bounds of array,
        // etc. For now we accept any number of objects.


        // Make room for our object
        if (vector_count(&node->initializers) <= index + 1) {
            vector_resize(&node->initializers, index + 1);
        }

        // Get the type of child we're initializing next
        type_t* child_type;
        if (type_matches_base(type, BASE_RECORD)) {
            child_type = vector_at(&type->record->member_list, index);
        } else if (type_is_array(type)) {
            child_type = type->ref;
        } else {
            fatal("Internal error: cannot parse initializer part for non-compound type");
        }

        // If it's a struct or array, we recurse.
        if (type_matches_base(child_type, BASE_RECORD) || type_is_array(child_type)) {
            if (lexer_is(STR_BRACE_OPEN)) {
                // In the case of a nested brace, we replace the child object
                // entirely, overriding any previous initializers for it even
                // if they will be empty-initialized by this.
                //     see test: init/init-overrides-zeroing.c
                parse_init_clear(node, index);
                vector_set(&node->initializers, index, parse_initializer(child_type));

            } else if (lexer_token->type == token_type_string && type_is_array(child_type)) {
                fatal("TODO initialize array with string");

            } else {
                // Otherwise, we create the initializer list if necessary, and
                // if it already exists, we keep it, because we are *not*
                // necessarily overriding parts of it that have already been
                // initialized by other designators.
                //     see test: init/init-partial-out-of-order.c
                if (vector_at(&node->initializers, index) == NULL) {
                    vector_set(&node->initializers, index, node_new(NODE_INITIALIZER_LIST));
                }
                parse_initializer_list_part(child_type, vector_at(&node->initializers, index), 0, false);
            }

        } else {
            // It's a scalar.
            parse_init_clear(node, index);
            node_t* child = parse_initializer(child_type);
            // TODO it would be nice to evaluate constant expressions, or at
            // least number nodes, right away and free the nodes to minimize
            // memory usage. Constant expression values could be written
            // directly into a block of memory at this point. This could
            // drastically reduce memory usage in parsing char arrays
            // especially anywhere #embed is used (a node per char multiplies
            // memory usage by sizeof(node_t) which is like 70x.) For now we
            // don't bother.
            vector_set(&node->initializers, index, child);
        }

        // Move to the next member.
        ++index;
        // TODO if index is out of bounds, break


        // TODO trailing comma is C99
        if (lexer_accept(STR_BRACE_CLOSE))
            break;
        lexer_expect(STR_COMMA, "Expected `,` or `}` after initializer list expression.");
    }

}

node_t* parse_initializer_list(type_t* type) {
    node_t* root = node_new_lexer(NODE_INITIALIZER_LIST);

    // Constant initializer data
    // TODO this should only be allocated for the root initializer, we need to
    // pass it around (or just store it in global vars?)
    //uint8_t* bytes = 0;
    //size_t bytes_count = 0;
    //size_t bytes_capacity = 0;

    parse_initializer_list_part(type, root, 0, true);

    // TODO empty initializer list is C23, should check that it's not empty
    // here in earlier language standards

    return root;
}

node_t* parse_initializer(type_t* type) {
    bool brace = false;
    if (lexer_is(STR_BRACE_OPEN)) {

        // braces for a struct or array are an initializer list.
        if (type_matches_base(type, BASE_RECORD) || type_is_array(type)) {
            return parse_initializer_list(type);
        }

        // braced scalars are otherwise allowed.
        brace = true;
        lexer_consume();
    }

    node_t* node = parse_assignment_expression();

    // TODO we could use some checks to make sure the conversion is valid.
    // For now we just implicitly cast.
    node = node_cast(node, type, NULL);

    if (brace)
        lexer_expect(STR_BRACE_CLOSE, "Expected `}` to match `{` around this scalar initializer.");
    return node;
}
