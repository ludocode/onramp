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

#include <stdlib.h>

#include "libo-error.h"

token_t* token_new(
        token_type_t type,
        string_t* value,
        token_prefix_t prefix,
        string_t* filename,
        int line,
        struct token_t* source)
{
    token_t* token = malloc(sizeof(token_t));
    if (!token) {
        fatal("Out of memory.");
    }
    token->refcount = 1;
    token->type = type;
    token->value = value;
    token->prefix = prefix;
    token->filename = string_ref(filename);
    token->line = line;
    if (source) {
        token->source = token_ref(source);
        token->depth = source->depth + 1;
    } else {
        token->source = NULL;
        token->depth = 0;
    }
    //printf("new token: %s\n", value->bytes);
    return token;
}

void token_deref(token_t* token) {
    if (--token->refcount != 0) {
        return;
    }

    string_deref(token->value);
    string_deref(token->filename);
    if (token->source) {
        token_deref(token->source);
    }
    free(token);
}

void token_print(token_t* token) {
    char prefix[2];
    prefix[0] = token->prefix;
    prefix[1] = 0;
    printf("<token: %c %s\"%s\">\n", token->type, prefix,
            token->value ? string_cstr(token->value) : "");
}
