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

#include "token.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "libo-error.h"
#include "libo-vector.h"
#include "hideset.h"
#include "strings.h"

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

    printf("<token: %s:%i:%i %s %s\"%s\"",
            token->location.filename->bytes,
            token->location.line,
            token->location.column,
            token_type_to_string(token->type),
            prefix,
            string_cstr(token->value));

    if (token->hideset) {
        fputs(" hideset:", stdout);
        hideset_print(token->hideset);
    }

    fputs(">\n", stdout);
}

bool token_is_punctuation(token_t* token, string_t* punctuation) {
    return token->type == token_type_punctuation && string_equal(token->value, punctuation);
}

bool token_is_keyword(token_t* token, string_t* keyword) {
    return token->type == token_type_alphanumeric && string_equal(token->value, keyword);
}

bool token_is_whitespace(token_t* token) {
    return token->type == token_type_space || token->type == token_type_newline;
}

string_t* stringify_string(token_t* token) {
    //trace("Stringifying token: "); token_print(token);

    bool is_str = token->type == token_type_string;

    char* old_str = token->value->bytes;
    size_t old_len = token->value->length;
    size_t new_len = old_len + (is_str ? 4 : 2);

    // count quotes and backslashes
    for (size_t i = 0; i < old_len; ++i) {
        if (old_str[i] == '\\' || old_str[i] == '"')
            ++new_len;
    }

    // allocate a new string
    char* new_str = malloc(new_len);
    if (new_str == NULL) {
        fatal("Out of memory.");
    }

    // append start and end quotes
    size_t new_i;
    if (is_str) {
        new_str[0] = '\\';
        new_str[1] = '"';
        new_str[new_len - 2] = '\\';
        new_str[new_len - 1] = '"';
        new_i = 2;
    } else {
        new_str[0] = '\'';
        new_str[new_len - 1] = '\'';
        new_i = 1;
    }

    // copy characters, escaping quotes and backslashes
    for (size_t old_i = 0; old_i < old_len; ++old_i) {
        if (old_str[old_i] == '\\' || old_str[old_i] == '"')
            new_str[new_i++] = '\\';
        new_str[new_i++] = old_str[old_i];
    }

    // intern it
    // TODO the extra copy is unnecessary here, we should just return the buffer
    string_t* string = string_intern_bytes(new_str, new_len);
    free(new_str);
    return string;
}

token_t* token_new_stringify(vector_t* tokens, hideset_t* hideset) {
    //trace("Stringified vector with %zi tokens\n", vector_count(tokens));

    // TODO we could really use a byte buffer in libo.
    size_t result_length = 0;
    char8_t* result = malloc(1);
    if (result == NULL) {
        fatal("Out of memory.");
    }

    location_t* location = &location_builtin;
    bool last_space = false;



    // TODO this whole function is a mess. needs to be cleaned up badly.



    void** start = vector_start(tokens);
    void** end = vector_end(tokens);
    if (start == end)
        goto done;

    // Find the first and last non-whitespace tokens
    if (token_is_whitespace(*start)) {
        start = token_next(start, end);
    }
    void** last = end - 1;
    if (token_is_whitespace(*last)) {
        last = token_previous(last, start);
    }
    end = last + 1;



    for (void** p = start; p != end; ++p) {
        token_t* token = *p;
        string_t* append;

        switch (token->type) {

            // Our lexer creates space tokens for comments and doesn't coalesce
            // them so we need to do so here.
            case token_type_space:
            case token_type_newline:
                if (last_space)
                    continue;
                last_space = true;
                append = string_ref(STR_SPACE);
                break;

            case token_type_alphanumeric:
            case token_type_punctuation:
            case token_type_number:
            case token_type_string:
            case token_type_character:
                last_space = false;

                if (token->type == token_type_string || token->type == token_type_character) {
                    append = stringify_string(token);
                } else {
                    append = string_ref(token->value);
                }

                //trace("Appending token \"%s\"\n", append->bytes);

                // We use the location of the first non-whitespace non-empty
                // token as the stringified token's location.
                if (location == &location_builtin) {
                    location = &token->location;
                }
                break;


            default:
                fatal_token(token, "Internal error: cannot stringify token type '%c'.", token->type);
        }

        size_t new_length = result_length + string_length(append);
        result = realloc(result, new_length);
        if (result == NULL) {
            fatal("Out of memory.");
        }
        memcpy(result + result_length, append->bytes, string_length(append));
        result_length = new_length;
        string_deref(append);
    }

done:;
    token_t* token = token_new_bytes(token_type_string, result, result_length, location);
    free(result);
    token->hideset = hideset ? hideset_ref(hideset) : NULL;

    //trace("Generated stringified token: "); token_print(token);
    return token;
}

void** token_next(void** p, void** end) {
    for (void** q = p + 1; q != end; ++q) {
        if (!token_is_whitespace(*q)) {
            return q;
        }
    }
    return NULL;
}

void** token_previous(void** p, void** start) {
    for (void** q = p; q-- != start;) {
        if (!token_is_whitespace(*q)) {
            return q;
        }
    }
    return NULL;
}

token_t* token_new_int(int value, location_t* location, hideset_t* /*nullable*/ hideset) {
    char buf[16];
    sprintf(buf, "%i", value);
    string_t* numstr = string_intern_cstr(buf);
    token_t* token = token_new(token_type_number, numstr, location);
    token->hideset = hideset ? hideset_ref(hideset) : NULL;
    string_deref(numstr);
    return token;
}
