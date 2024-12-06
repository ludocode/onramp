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

#include "token.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "libo-error.h"
#include "hideset.h"

token_t* token_end;

const char* token_type_to_string(token_type_t type) {
    switch (type) {
        case token_type_directive: return "directive";
        case token_type_alphanumeric: return "alphanumeric";
        case token_type_number: return "number";
        case token_type_character: return "character";
        case token_type_string: return "string";
        case token_type_angle_include: return "angle_include";
        case token_type_punctuation: return "punctuation";
        case token_type_space: return "space";
        case token_type_newline: return "newline";
        case token_type_end: return "end";
        case token_type_invalid: return "invalid";
        default: break;
    }
    return "<unknown>";
}

void token_setup(void) {
    location_t builtin;
    location_init_builtin(&builtin);
    string_t* empty = string_intern_cstr("");
    token_end = token_new(token_type_end, empty, &builtin);
    string_deref(empty);
    location_destroy(&builtin);
}

void token_teardown(void) {
    token_deref(token_end);
}

token_t* token_new(token_type_t type, string_t* value, location_t* location) {
    token_t* token = (token_t*) calloc(1, sizeof(token_t));
    if (!token)
        fatal("Out of memory.");
    token->refcount = 1;
    token->type = type;
    token->value = string_ref(value);
    location_init_copy(&token->location, location);
    token->hideset = NULL;
    return token;
}

token_t* token_new_bytes(token_type_t type, const char8_t* bytes, size_t count, location_t* location) {
    string_t* string = string_intern_bytes((const char*)bytes, count);
    token_t* token = token_new(type, string, location);
    string_deref(string);
    return token;
}

token_t* token_new_expansion(token_t* old_token, location_t* location, struct hideset_t* hideset) {
    token_t* new_token = token_new(old_token->type, old_token->value, location);
    new_token->prefix = old_token->prefix;
    new_token->hideset = hideset_ref(hideset);
    return new_token;
}

void token_deref(token_t* token) {
    assert(token);
    if (--token->refcount != 0)
        return;

    if (token->hideset)
        hideset_deref(token->hideset);
    location_destroy(&token->location);
    string_deref(token->value);
    free(token);
}

/*
token_t* token_new_as_expansion(token_t* token, token_t* macro) {
    token_t* ret = token_new(token->type, token->value, &token->location);
    ret->prefix = token->prefix;
    ret->hideset = hideset_new_intersection

    // add the macro to the hideset
    ret->hideset = table_new();
    token_add_to_hideset(ret, macro->value);

    // copy entries from macro's hideset
    table_t* old_hideset = macro->hideset;
    if (old_hideset) {
        for (table_entry_t** bucket = table_first_bucket(old_hideset); bucket;
                bucket = table_next_bucket(old_hideset, bucket))
        {
            for (table_entry_t* entry = *bucket; entry; entry = table_entry_next(entry)) {
                token_add_to_hideset(ret, ((token_hideset_entry_t*)entry)->string);
            }
        }
    }

    return ret;
}
*/

void token_print(token_t* token) {
    char prefix[2] = {token->prefix, 0};

    printf("<token: %s:%i:%i %s %s\"%s\">\n",
            token->location.filename->bytes,
            token->location.line,
            token->location.column,
            token_type_to_string(token->type),
            prefix,
            string_cstr(token->value));
}

bool token_is_punctuation(token_t* token, string_t* punctuation) {
    return token->type == token_type_punctuation && string_equal(token->value, punctuation);
}

bool token_is_keyword(token_t* token, string_t* keyword) {
    return token->type == token_type_alphanumeric && string_equal(token->value, keyword);
}
