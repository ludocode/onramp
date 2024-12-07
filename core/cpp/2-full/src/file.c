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

#include "file.h"

#include <assert.h>
#include <stdlib.h>

#include "lexer.h"

file_t* file_current;

file_t* file_new(string_t* filename, FILE* cfile, struct token_t* source) {
    file_t* file = malloc(sizeof(file_t));
    file->lexer = lexer_new_file(filename, cfile, source);

    file->conditionals_count = 0;
    file->conditionals_capacity = 4;
    file->conditionals = malloc(file->conditionals_capacity * sizeof(conditional_t));
    if (file->conditionals == NULL) {
        fatal("Out of memory.");
    }

    file->guard_state = guard_state_start;
    file->guard_name = NULL;

    return file;
}

void file_delete(file_t* file) {
    if (file->guard_name) {
        string_deref(file->guard_name);
    }

    if (file->conditionals_count != 0) {
        token_t* token = file->conditionals[file->conditionals_count - 1].token;
        fatal_token(token, "Unclosed #%s conditional directive; expected #endif.", token->value->bytes);
    }
    free(file->conditionals);

    lexer_delete(file->lexer);
    free(file);
}

void file_conditional_push(file_t* file, token_t* token) {
    if (file->conditionals_count == file->conditionals_capacity) {
        size_t new_capacity = file->conditionals_capacity * 2;
//printf("previous cap %zi new cap %zi\n",conditionals_capacity,new_capacity);
        if (new_capacity <= file->conditionals_capacity) {
            fatal("Out of memory.");
        }
        file->conditionals = realloc(file->conditionals, new_capacity * sizeof(conditional_t));
        if (file->conditionals == NULL) {
            fatal("Out of memory.");
        }
        file->conditionals_capacity = new_capacity;
    }

    file->conditionals[file->conditionals_count].token = token_ref(token);
    file->conditionals[file->conditionals_count].taken = false;
    ++file->conditionals_count;
}

void file_conditional_pop(file_t* file, token_t* token) {
    if (file->conditionals_count == 0) {
        fatal_token(token, "`#endif` without matching `#if`");
    }
    token_deref(file->conditionals[file->conditionals_count - 1].token);
    --file->conditionals_count;
}
