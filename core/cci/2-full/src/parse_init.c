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
 * Returns true if the given expression is a valid string literal initializer
 * for the given array type.
 */
static bool valid_string_array_initializer(type_t* array, node_t* scalar) {
    assert(type_is_array(array));

    if (scalar->kind != NODE_STRING)
        return false;

    // TODO check matching type of string literal. For now
    // we just check if it's a char type.
    type_t* element_type = array->ref;
    if (type_matches_base(element_type, BASE_CHAR) ||
            type_matches_base(element_type, BASE_SIGNED_CHAR) ||
            type_matches_base(element_type, BASE_UNSIGNED_CHAR))
    {
        return true;
    }

    return false;
}

/**
 * Returns the type of the child at the given index for nested structs and
 * arrays.
 */
static type_t* initializer_child_type(type_t* type, size_t index) {
    if (type_is_array(type))
        return type->ref;
    if (type_matches_base(type, BASE_RECORD))
        return record_member_type_at(type->record, index);
    return type;
}

/**
 * Parses an initializer list.
 *
 * This is used to parse a braced initializer for a struct, union, array or
 * scalar.
 *
 * We accept designators for structs, unions and arrays, which changes the
 * next subobject to be initialized within the initializer list.
 *
 * We accept multiple elements for structs and arrays. Unions and scalars allow
 * only one element.
 *
 * (A braced scalar is still wrapped in an initializer list. This matches the
 * grammar and it's the easiest way to parse it.)
 *
 * We only recurse if we hit a nested brace. Otherwise the parsing is entirely
 * in this closed loop, even for nested objects.
 */
node_t* parse_initializer_list(type_t* root_type) {
    assert(lexer_is(STR_BRACE_OPEN));

    // The root initializer list is what the spec calls the "current object".
    // It is fixed throughout parsing this list. When we see a nested brace, we
    // get a new "current object" by recursing.
    node_t* const root = node_new_lexer(NODE_INITIALIZER_LIST);
    root->type = type_ref(root_type);
    root->index = SIZE_MAX;

    // The current initialization position consists of an index into a
    // particular initialization list which is nested somewhere in the tree of
    // our "current object". We use this to walk around the tree.
    node_t* node = root;
    size_t index = 0;

    if (lexer_accept(STR_BRACE_CLOSE)) {
        // TODO warn, {} is C23
        return root;
    }

    for (;;) {


        // check for designators
        /*
        for (;;) {

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
        }
        */


        type_t* child_type = initializer_child_type(node->type, index);

        if (lexer_is(STR_BRACE_OPEN)) {
            // In the case of a nested brace, we replace the child object
            // entirely, overriding any previous initializers for it even if
            // they will be empty-initialized by this.
            //     See test: init/init-overrides-zeroing.c
            // This handles nested compound objects and nested braced scalars.

            // Note: If we're already in a scalar initializer, this is
            // *another* set of braces, but the grammar suggests it is allowed.
            // Popular compilers (except MSVC) accept it but Clang warns
            // -Wmany-braces-around-scalar-init. We don't bother warning.

            vector_ensure_size(&node->initializers, index + 1);
            parse_init_clear(node, index);
            node_t* list = parse_initializer_list(child_type);
            list->index = index;
            list->parent = node;
            vector_set(&node->initializers, index, list);

        } else {

            // It's an unbraced scalar. Parse it
            node_t* scalar = parse_assignment_expression();

            // Walk down the type tree to find what this scalar is
            // initializing. We break out of this loop once we've found the
            // correct position to insert this scalar.
            for (;;) {
                vector_ensure_size(&node->initializers, index + 1);

                if (type_matches_base(child_type, BASE_RECORD)) {
                    if (type_equal(child_type, scalar->type)) {
                        // This record member is being initialized by a record
                        // of matching type.
                        break;
                    }
                    goto walk_down;
                }

                if (type_is_array(child_type)) {
                    if (valid_string_array_initializer(child_type, scalar)) {
                        // We're initializing a character array with a string literal.
                        break;
                    }
                    goto walk_down;
                }

                // We've found the position.
                break;

            walk_down:;
                // This record member or array element is itself a record or
                // array and is being initialized by its first element
                // (potentially recursively.) Create the child initializer list
                // if necessary, and if it already exists, we keep it, because
                // we are *not* necessarily overriding parts of it that have
                // already been initialized by other designators.
                //     See test: init/init-partial-out-of-order.c
                node_t* child_node = vector_at(&node->initializers, index);
                if (child_node == NULL) {
                    child_node = node_new(NODE_INITIALIZER_LIST);
                    child_node->type = type_ref(child_type);
                    child_node->index = index;
                    child_node->parent = node;
                    vector_set(&node->initializers, index, child_node);
                }

                // Move down and keep walking
                node = child_node;
                index = 0;
                child_type = initializer_child_type(node->type, index);
                continue;
            }

            // Assign the scalar, replacing any existing initializer in case
            // designators have caused overlap.
            parse_init_clear(node, index);
            scalar->parent = node;
            vector_set(&node->initializers, index, scalar);

        }

        // Check for the end of the initializer list
        if (!lexer_accept(STR_COMMA)) {
            lexer_expect(STR_BRACE_CLOSE, "Expected `,` or `}` after initializer list expression.");
            break;
        }
        if (lexer_accept(STR_BRACE_CLOSE)) {
            break;
        }

        // Walk to the next element, up the tree if necessary
        for (;;) {
            if (type_is_array(node->type)) {
                // We allow unlimited elements in an array even if there are
                // too many. The excess are ignored. We could probably optimize
                // this by discarding the excess without putting them in the
                // tree but right now we don't bother.
                ++index;
                break;
            }

            if (type_matches_base(node->type, BASE_RECORD)) {
                ++index;
                record_t* record = node->type->record;
                if (record->is_struct && index < record_member_count(record)) {
                    // Found the next struct member to initialize
                    break;
                }
            }

            // No more elements in this subobject. Walk up
            if (node == root) {
                fatal_token(lexer_token, "Too many initializers in this initializer list.");
            }
            index = node->index;
            node = node->parent;
            assert(index != SIZE_MAX);
        }
    }

    return root;
}

node_t* parse_initializer(type_t* type) {
    if (lexer_is(STR_BRACE_OPEN)) {
        return parse_initializer_list(type);
    }

    node_t* scalar = parse_assignment_expression();

    if (type_is_array(type) && valid_string_array_initializer(type, scalar)) {
        return scalar;
    }

    return node_cast(node_decay(scalar), type, NULL);
}
