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

#include "emit.h"

#include <assert.h>

#include "token.h"
#include "options.h"
#include "libo-error.h"

// Maximum number of newlines to emit instead of emitting a #line directive.
// Larger numbers make the output more efficient (within reason); smaller
// numbers make the output more legible.
#define MAX_NEWLINES_TO_EMIT 3

static FILE* output_file;

// The location of the last token that was emitted.
static location_t last_location;

// The last character that was emitted. We keep track of this to determine
// whether we need to emit a newline before emitting a #line or #pragma
// directive. We also use it to optimize whitespace a bit.
static char last_char;

static token_type_t last_token_type; // TODO unused?

static void emit_char(char c) {
    fputc(c, output_file);
    last_char = c;
}

static void emit_bytes(const char* bytes, size_t length) {
    if (length > 0) {
        fwrite(bytes, 1, length, output_file);
        last_char = bytes[length - 1];
    }
}

static void emit_cstr(const char* cstr) {
    emit_bytes(cstr, strlen(cstr));
}

void emit_setup(void) {
    last_char = '\n';
    location_init_builtin(&last_location);
}

void emit_open(void) {
    assert(output_file == NULL);
    assert(options_output_filename != NULL);
    output_file = fopen(options_output_filename, "wb");
    if (output_file == NULL) {
        fatal("Failed to open output file: %s", output_file);
    }
}

void emit_teardown(void) {
    if (last_char != '\n') {
        emit_char('\n');
    }
    fclose(output_file);
    location_destroy(&last_location);
}

static void emit_newline_if_needed(void) {
    if (last_char != '\n') {
        emit_char('\n');
        last_char = '\n';
        ++last_location.line;
    }
}

void emit_string(const string_t* string) {
    emit_bytes(string->bytes, string->length);
}

static void emit_token_string(token_t* token, char delimiter) {
    assert(token->type == token_type_character || token->type == token_type_string);

    switch (token->prefix) {
        case token_prefix_none: break;
        case token_prefix_L: emit_char('L'); break;
        case token_prefix_u: emit_char('u'); break;
        case token_prefix_U: emit_char('U'); break;
        case token_prefix_u8: emit_bytes("u8", 2); break;
    }

    // The value already contains escaped quotes and other escape sequences; we
    // never unescaped it. We just need to put quotes around it.
    emit_char(delimiter);
    emit_string(token->value);
    emit_char(delimiter);
}

void emit_location(location_t* location) {

    // If the filenames match, we can optimize a bit.
    if (string_equal(last_location.filename, location->filename)) {
        if (location->line == last_location.line) {
            return;
        }
        if (location->line > last_location.line && location->line - last_location.line <= MAX_NEWLINES_TO_EMIT) {
            for (size_t i = location->line - last_location.line; i-- > 0;) {
                emit_char('\n');
            }
        } else {
            emit_newline_if_needed();
            fprintf(output_file, "#line %i\n", location->line);
            last_char = '\n';
        }
        last_location.line = location->line;
        return;
    }

    location_set_copy(&last_location, location);
    emit_newline_if_needed();
    // TODO escape file string
    fprintf(output_file, "#line %i \"%s\"\n", location->line, location->filename->bytes);
    last_char = '\n';
}

void emit_pragma_file_push(void) {
    emit_newline_if_needed();
    fprintf(output_file, "#pragma onramp file push\n");
    last_char = '\n';
}

void emit_pragma_file_pop(void) {
    emit_newline_if_needed();
    fprintf(output_file, "#pragma onramp file pop\n");
    last_char = '\n';
}

void emit_inline_pragma(token_t* token) {
    emit_newline_if_needed();
    emit_location(&token->location);
    emit_cstr("#pragma ");

    // TODO we need to decode escape sequences! for now we don't bother; this
    // works for all the pragmas we currently care about and the compiler will
    // ignore the rest. We'll need to implement this to support GCC diagnostics
    // in _Pragma().
    emit_string(token->value);

    emit_newline_if_needed();
}

void emit_token_at(token_t* token, location_t* location) {
    switch (token->type) {
        case token_type_character:
            emit_location(location);
            emit_token_string(token, '\'');
            break;
        case token_type_string:
            emit_location(location);
            emit_token_string(token, '"');
            break;
        // Invalid tokens are emitted verbatim. This provides some possibility
        // of preprocessing files that aren't valid C. We're behaving the same
        // way GCC and Clang do.
        case token_type_invalid:
        case token_type_alphanumeric:
        case token_type_number:
        case token_type_punctuation:
            // The string may be empty in the case of a token pasting
            // placeholder that wasn't concatenated.
            if (!string_is_empty(token->value)) {
                emit_location(location);
                emit_string(token->value);
            }
            break;
        case token_type_space:
            if (last_char == ' ' || last_char == '\n')
                break;
            emit_location(location);
            emit_char(' ');
            break;
        case token_type_newline:
            // emit nothing. we insert newlines and linemarkers as needed.
            break;
        case token_type_pragma:
            emit_inline_pragma(token);
            break;
        case token_type_directive:
        case token_type_angle_include:
        case token_type_end:
            fatal("Internal error: cannot emit token type '%c'.", token->type);
            break;
    }

    last_token_type = token->type;
}
