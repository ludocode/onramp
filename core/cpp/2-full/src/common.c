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

#define _GNU_SOURCE

#include "common.h"

#include "token.h"
#include "emit.h"
#include "strings.h"
#include "libo-vector.h"
#include "libo-string.h"
#include "lexer.h"
#include "strings.h"

location_t location_builtin;
location_t location_command_line;

void location_setup(void) {
    location_init(&location_builtin, STR_BUILT_IN, 1, 1, NULL);
    location_init(&location_command_line, STR_COMMAND_LINE, 1, 1, NULL);
}

void location_teardown(void) {
    location_destroy(&location_command_line);
    location_destroy(&location_builtin);
}

void location_init(location_t* location, string_t* filename, int line,
        int column, token_t* source)
{
    location->filename = filename ? string_ref(filename) : NULL;
    location->line = line;
    location->column = column;
    location->source = source ? token_ref(source) : NULL;
}

void location_init_builtin(location_t* location) {
    location_init(location, STR_BUILT_IN, 1, 1, NULL);
}

void location_init_command_line(location_t* location) {
    location_init(location, STR_COMMAND_LINE, 1, 1, NULL);
}

void location_init_copy(location_t* location, location_t* source) {
    location_init(location, source->filename, source->line,
            source->column, source->source);
}

void location_set_copy(location_t* dest, location_t* src) {
    string_ref(src->filename);
    if (src->source)
        token_ref(src->source);
    string_deref(dest->filename);
    if (dest->source)
        token_deref(dest->source);
    *dest = *src;
}

void location_set_filename(location_t* location, string_t* filename) {
    string_ref(filename);
    string_deref(location->filename);
    location->filename = filename;
}

void location_destroy(location_t* location) {
    string_deref(location->filename);
    if (location->source)
        token_deref(location->source);
}



// TODO share these with cci/2. probably we'll be moving location_t into libo.

void fatal_token(struct token_t* token, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfatal_token(token, format, args);
}

void vfatal_token(struct token_t* token, const char* format, va_list args) {
    if (token == NULL) {
        current_line = 0;
        current_filename = NULL;
    } else {
        current_line = token->location.line;
        current_filename = (char*)string_cstr(token->location.filename);
    }
    vfatal(format, args);
}

void output_token(vector_t* /*nullable*/ output, token_t* token) {
    if (output) {
        vector_append(output, token_ref(token));
    } else {
        emit_token(token);
    }
}

string_t* path_dirname(string_t* filename) {
    /*
    if (filename == NULL) {
        return string_ref(STR_DOT);
    }
    */
    char* end = strrchr(filename->bytes, '/');
    if (end == NULL) {
        return string_ref(STR_DOT);
    }
    return string_intern_bytes(filename->bytes, end - filename->bytes);
}

char* path_join(string_t* path, string_t* filename) {
    if (filename->bytes[0] == '/') {
        return strdup(filename->bytes);
    }
    if (string_length(path) == 0) {
        fatal("Internal error: path is empty");
    }
    bool trailing_slash = path->bytes[string_length(path) - 1] == '/';
    char* cstr;
    if (0 > asprintf(&cstr, "%s%s%s",
                path->bytes,
                trailing_slash ? "" : "/",
                filename->bytes))
    {
        fatal("Out of memory.");
    }
    //string_t* str = string_intern_cstr(cstr);
    //free(cstr);
    //return str;
    return cstr;
}

void destroy_string_vector(vector_t* vector) {
    for (size_t i = vector_count(vector); i-- > 0;) {
        string_deref(vector_at(vector, i));
    }
    vector_destroy(vector);
}
