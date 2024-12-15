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

#include "preprocess.h"

#include <stdlib.h>
#include <assert.h>

#include "libo-vector.h"
#include "emit.h"
#include "file.h"
#include "lexer.h"
#include "token.h"
#include "macro.h"
#include "stream.h"
#include "directive.h"
#include "strings.h"
#include "options.h"

/**
 * Maximum number of input files to keep open at a time.
 *
 * The preprocessor will close least-recently-used files and re-open them as
 * needed in order to limit the total number of open files.
 *
 * This can't be less than 2 because we never close the original input file
 * (since in a hosted environment it may be a stream.)
 */
#define MAX_OPEN_INPUT_FILES 5
// TODO make sure we never unload the input file in case it's a stream.

/**
 * Stack of files currently waiting on `#include` directives. Each time we
 * reach an `#include`, the current file is pushed onto this stack and a new
 * file is created. Whenever we reach the end of the current file, we pop the
 * top file off the stack and continue parsing.
 */
static vector_t files;

void preprocess_setup(void) {
    vector_init(&files);
}

void preprocess_teardown(void) {
    assert(vector_is_empty(&files));
    vector_destroy(&files);
}

/**
 * Prepare to include a new file.
 *
 * This may unload an open file to make room.
 */
static void preprocess_prepare_include(void) {
    // TODO if this is > MAX_OPEN_INPUT_FILES, unload n - max + 1 (the +1 keeps the initial file)
    // TODO actually need to do the unload before including the file
}

/**
 * Performs an include of the given file.
 *
 * Note that includes are not recursive. This points the stream to the
 * new file and then returns.
 */
static void preprocess_include_file(struct string_t* filename, FILE* file, struct token_t* source) {
    //trace("pushing file %s\n", filename->bytes);
    if (file_current != NULL) {
        vector_append(&files, file_current);
    }
    file_current = file_new(filename, file, source);
    lexer_current = file_current->lexer;
}

/*
 * Tries to include the given file.
 */
static bool preprocess_try_include(string_t* path, string_t* filename, token_t* source) {
    char* full_path_cstr = path_join(path, filename);
    FILE* file = fopen(full_path_cstr, "rb");

    if (!file) {
        free(full_path_cstr);
        return false;
    }

    // TODO if this is `#include_next`, walk the lexer stack to make sure we
    // aren't already including this file.

    string_t* full_path_str = string_intern_cstr(full_path_cstr);
    free(full_path_cstr);
    preprocess_include_file(full_path_str, file, source);
    string_deref(full_path_str);
    return true;
}

static bool preprocess_include_search_paths(token_t* source, vector_t* paths) {
    size_t count = vector_count(paths);
    for (size_t i = 0; i < count; ++i) {
        if (preprocess_try_include(vector_at(paths, i), source->value, source)) {
            return true;
        }
    }
    return false;
}

void preprocess_include_search(stream_t* stream, token_t* token) {
    preprocess_prepare_include();
    string_t* filename = token->value;
    bool is_quoted = token->type == token_type_string;

    //trace("#include searching for file: %s\n", filename->bytes);

    // same directory and -iquote are only searched for quoted filenames (GCC's
    // docs for #include_next say it doesn't distinguish between angle brackets
    // and quotes. It's not clear whether we should search for -iquote paths.)
    if (is_quoted) {
        string_t* dir = path_dirname(lexer_current->reader.filename);
        bool found = preprocess_try_include(dir, filename, token);
        string_deref(dir);
        if (found)
            return;
    }

    if (preprocess_include_search_paths(token, &options_include_paths)) return;
    if (preprocess_try_include(STR_DOT, token->value, token)) return;

    fatal_token(token, "Include file not found: %s\n", filename->bytes);
}

/**
 * Runs until the lexer stack is empty.
 */
static void preprocess_run(stream_t* stream) {
    for (;;) {
        token_t* token = stream_take(stream);
        //trace("\npreprocessing token: "); token_print(token);
        if (token->type == token_type_end) {
            token_deref(token);
            //trace("popping file %s\n", file_current->lexer->reader.filename->bytes);
            file_delete(file_current);

            // If there are no more files, we're done
            if (vector_is_empty(&files)) {
                break;
            }

            file_current = vector_remove_last(&files);
            lexer_current = file_current->lexer;
            continue;
        }

        if (token->type == token_type_directive) {
            // It's a directive. Parse and handle it
            directive_parse(stream, token);
        } else if (token->type == token_type_alphanumeric) {
            // It might be a macro. Expand it
            macro_expand(stream, NULL, token);
        } else {
            // Just a plain non-alphanumeric token. Output it.
            output_token(NULL, token);
        }
        token_deref(token);
    }
}

void preprocess(void) {
    // TODO emit a linemarker for root filename before doing anything

    stream_t stream;
    stream_init(&stream, true, NULL);

    // Parse all force-includes
    for (size_t i = 0; i < vector_count(&options_force_includes); ++i) {
        location_t location;
        location_init_command_line(&location);
        token_t* token = token_new(token_type_alphanumeric,
                vector_at(&options_force_includes, i), &location);
        preprocess_include_search(&stream, token);
        token_deref(token);
        location_destroy(&location);
        preprocess_run(&stream);
    }

    //trace("pushing root file %s\n", options_input_filename);
    string_t* infile = string_intern_cstr(options_input_filename);
    file_current = file_new(infile, NULL, NULL);
    lexer_current = file_current->lexer;
    string_deref(infile);

//stream_dump_tokens(stream);
    preprocess_run(&stream);

    assert(vector_is_empty(&files));

    stream_destroy(&stream);
}
