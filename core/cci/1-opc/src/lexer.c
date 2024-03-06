/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
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

#include "lexer.h"

#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "emit.h"

char* lexer_filename;
int lexer_line;
char* lexer_token;
lexer_type_t lexer_type;

// The next character (the last read from the file), not added to lexer_token
// yet. This is an int because it's -1 (EOF) at the end of the file.
static int lexer_char;

// lexer_token grows as needed. Its capacity is stored here.
static size_t lexer_token_capacity;

#define LEXER_MINIMUM_CAPACITY 32

FILE* lexer_file;

const char* lexer_type_to_string(lexer_type_t type) {
    if (type == lexer_type_alphanumeric) {return "lexer_type_alphanumeric";}
    if (type == lexer_type_number) {return "lexer_type_number";}
    if (type == lexer_type_character) {return "lexer_type_character";}
    if (type == lexer_type_string) {return "lexer_type_string";}
    if (type == lexer_type_punctuation) {return "lexer_type_punctuation";}
    if (type == lexer_type_end) {return "lexer_type_end";}
    return "<unknown lexer type>";
}

// TODO move to libo
// Returns true if the given character is valid for an alphanumeric token (i.e.
// a keyword or an identifier.)
static bool lexer_is_alphanumeric(int c, bool first) {
    if (c == EOF) {
        return false;
    }

    // The first character of an alphanumeric cannot be a numerical digit.
    if (!!first & !!isdigit(c)) {
        return false;
    }

    // Note, we allow $ as an extension for compatibility with GNU C.
    return isalnum(c) | ((c == '_') | (c == '$'));
}

// Reads the next character, placing it in lexer_char and returning it.
static int lexer_read_char(void) {
    int c = fgetc(lexer_file);
    lexer_char = c;
    return c;
}

static bool lexer_is_end_of_line(int c) {
    return ((c == '\n') | (c == '\r')) | (c == -1);
}

// TODO don't take length here, we're making each individual token type track
// its own size, should just have a global lexer_token_size, could also expose
// it publicly to avoid null-termination in some cases
// TODO actually this is an issue of correctness as well, a literal string or
// char could contain a null byte, we need to use a separate token_length to
// parse it correctly
// TODO we don't need to worry about a null byte in earlier stages, there is no
// way to input one. we only need to worry about null bytes in literals in the
// final cci stage.
// TODO the initial allocation should be a static buffer (say 128 bytes) that
// we memdup() on take. this will greatly reduce the number of mallocs we do,
// and also will make it possible for the first stage libc malloc to be a stack
// allocator. (the way it is now we always allocate a new buffer on any take so
// it's never possible for a stack allocator to free memory.)
static void lexer_token_append(size_t length, char c) {
    if (length == lexer_token_capacity) {

        // Not enough room for the additional character; we need to reallocate.
        // Calculate new size
        size_t new_capacity = ((lexer_token_capacity * 3) / 2);
        if (new_capacity < lexer_token_capacity) {
            // overflow
            fatal("Out of memory");
        }
        if (new_capacity < LEXER_MINIMUM_CAPACITY) {
            new_capacity = LEXER_MINIMUM_CAPACITY;
        }

        // Reallocate
        lexer_token = realloc(lexer_token, new_capacity);
        if ((lexer_token == NULL) | (new_capacity < lexer_token_capacity)) {
            fatal("Out of memory");
        }
        lexer_token_capacity = new_capacity;
    }

    *(lexer_token + length) = c;
}

void lexer_init(const char* filename) {
    lexer_file = fopen(filename, "r");
    if (lexer_file == NULL) {
        fatal_2("Failed to open input file: ", filename);
    }

    lexer_line = 1;
    lexer_filename = strdup(filename);
    emit_line_directive();

    lexer_read_char();
    lexer_consume();
}

void lexer_destroy(void) {
    fclose(lexer_file);
    free(lexer_token);
    free(lexer_filename);
}

// TODO it's not going to be straightforward to share this.
//
// For a string literal we'll just convert it to UTF-8 (maybe?) This might not
// work because, say in a 16-bit wide string, you should be allowed to define
// arbitrary 16-bit values (like unpaired surrogates) which don't convert
// properly to UTF-8 and back. That means our lexer will need to support
// different string widths instead of keeping/converting everything in UTF-8
// and letting the compiler convert it back.
//
// For a char literal, if we want to support multichar as an extension, it will
// depend on the width of this. For example you should be able to do \u twice
// to declare a surrogate pair (I guess? not sure why you'd want to but it
// probably works with GCC.)
//
// Maybe in the full compiler we'll need different string vars for each width.
// In the opC stage we just don't allow unicode/numerical escape sequences.
static char lexer_consume_escape_sequence(void) {
    int c = lexer_read_char();

    // These definitions look circular but they're not: we're relying on the
    // definition of these escape sequences from the previous stage compiler.
    if (c == 'a')   {return '\a';}   // bell
    if (c == 'b')   {return '\b';}   // backspace
    if (c == 't')   {return '\t';}   // horizontal tab
    if (c == 'n')   {return '\n';}   // line feed
    if (c == 'v')   {return '\v';}   // vertical tab
    if (c == 'f')   {return '\f';}   // form feed
    if (c == 'r')   {return '\r';}   // carriage return
    if (c == 'e')   {return 27;}     // escape (extension, not standard C)
    if (c == '"')   {return '"';}    // double quote
    if (c == '\'')  {return '\'';}   // single quote
    if (c == '?')   {return '?';}    // question mark
    if (c == '\\')  {return '\\';}   // backslash

    if (c == '0') {
        // TODO parse octal, this is not supposed to be just null.
        // IIRC it's supposed to be followed by exactly three octal
        // digits but GCC allows less, which is why \0 with no
        // additional digits is zero (I think?)
    }

    // We don't bother to support X, u and U in opC.
    fatal("Unrecognized escape sequence");
}

static void lexer_consume_string_literal(void) {
    assert(lexer_char == '"');

    // Size, including null-terminator
    size_t size = 0;

    // Collect characters until the closing quote
    while (true) {
        int c = lexer_read_char();
        if (c == '"') {
            lexer_read_char();
            break;
        }

        if (lexer_is_end_of_line(c)) {
            fatal("Unclosed string literal");
        }

        if (c == '\\') {
            c = lexer_consume_escape_sequence();
        }

        lexer_token_append(size, c);
        size = (size + 1);
    }

    // Append null-terminator
    lexer_token_append(size, 0);
    size = (size + 1);
}

static void lexer_consume_char_literal(void) {
    assert(lexer_char == '\'');

    // read the char
    char c = lexer_read_char();
    if (c == '\'') {
        fatal("Empty char literal is not allowed.");
    }
    if (lexer_is_end_of_line(c)) {
        fatal("Unclosed character literal.");
    }
    if (c == '\\') {
        c = lexer_consume_escape_sequence();
    }

    // place it in the token with a null-terminator
    lexer_token_append(0, c);
    lexer_token_append(1, 0);

    // read the closing quote
    c = lexer_read_char();
    if (lexer_is_end_of_line(c)) {
        fatal("Unclosed character literal.");
    }
    if (c != '\'') {
        fatal("Only a single character is supported in a char literal.");
    }
    lexer_read_char();
}

static void lexer_consume_string_char_literal(void) {
    if (lexer_char == '"') {
        lexer_type = lexer_type_string;
        lexer_consume_string_literal();
        return;
    }
    lexer_type = lexer_type_character;
    lexer_consume_char_literal();
}

static void lexer_consume_optional_horizontal_whitespace(void) {
    while ((lexer_char == ' ') | (lexer_char == '\t')) {
        lexer_read_char();
    }
}

static void lexer_consume_horizontal_whitespace(void) {
    if (!((lexer_char == ' ') | (lexer_char == '\t'))) {
        fatal("Expected horizontal whitespace");
    }
    lexer_consume_optional_horizontal_whitespace();
}

static void lexer_consume_end_of_line(void) {
    int c = lexer_char;
    if (c == '\n') {
        lexer_line = (lexer_line + 1);
        lexer_read_char();
        return;
    }

    if (c == '\r') {
        lexer_line = (lexer_line + 1);
        c = lexer_read_char();
        if (c == '\n') {
            lexer_read_char();
        }
        return;
    }

    if (c == -1) {
        return;
    }

    fatal("Expected end of line.");
}

static void lexer_consume_whitespace(void) {
    // (Note that we don't handle comments or escaped newlines. Those need to
    // be filtered out by the preprocessor.)
    char c = (char)lexer_char;
    while (isspace(c)) {
        if ((c == '\n') | (c == '\r')) {
            lexer_consume_end_of_line();
            emit_line_increment_directive();
            c = (char)lexer_char;
            continue;
        }
        c = lexer_read_char();
    }
}

static void lexer_consume_until_newline(void) {
    int c = lexer_char;
    while (!lexer_is_end_of_line(c)) {
        c = lexer_read_char();
    }
}

static void lexer_handle_line_directive(void) {
    lexer_consume_horizontal_whitespace();

    // parse line number
    // (it's always decimal even if it has leading zeroes so parsing is real
    // simple.)
    if (!isdigit(lexer_char)) {
        fatal("Expected line number after #line");
    }
    int line = 0;
    while (isdigit(lexer_char)) {
        line = (line * 10);
        line = (line + (lexer_char - '0'));
        lexer_read_char();
    }

    // Line number is off by 1 because the end of the #line directive will
    // increment it.
    lexer_line = (line - 1);

    // Line number must be followed by space or end of line
    if (lexer_is_end_of_line(lexer_char)) {
        return;
    }
    lexer_consume_horizontal_whitespace();
    if (lexer_is_end_of_line(lexer_char)) {
        return;
    }

    // We have a filename. It must be surrounded in quotes. We assume it has
    // the same syntax as a string literal (so we can re-use the parse
    // function.)
    if (lexer_char != '"') {
        fatal("Filename in #line directive must be double-quoted.");
    }
    lexer_consume_string_literal();
    free(lexer_filename);
    lexer_filename = strdup(lexer_token);

    lexer_consume_optional_horizontal_whitespace();
    if (!lexer_is_end_of_line(lexer_char)) {
        fatal("Expected end of line after filename in #line directive");
    }
    lexer_consume_end_of_line();

    emit_line_directive();
}

static void lexer_parse_directive(void) {

    // skip the '#'
    lexer_read_char();
    lexer_consume_optional_horizontal_whitespace();

    // read the command
    size_t len = 0;
    while (isalpha(lexer_char)) {
        lexer_token_append(len, lexer_char);
        len = (len + 1);
        lexer_read_char();
    }
    lexer_token_append(len, 0);

    // handle a line directive
    if (0 == strcmp(lexer_token, "line")) {
        lexer_handle_line_directive();
        return;
    }

    // otherwise ignore it
    lexer_consume_until_newline();
}

static void lexer_consume_whitespace_and_directives(void) {
    while (1) {
        lexer_consume_whitespace();
        if (lexer_char != '#') {
            break;
        }
        lexer_parse_directive();
    }
}

void lexer_consume(void) {

    // Skip whitespace and handle #line directives. This brings us to the start
    // of the next real token.
    lexer_consume_whitespace_and_directives();

    // Check for end of file
    if (lexer_char == EOF) {
        lexer_token_append(0, 0);
        lexer_type = lexer_type_end;
        return;
    }

    char c = (char)lexer_char;

    // Symbol
    if (lexer_is_alphanumeric(c, true)) {
        size_t len = 0;
        while (1) {
            lexer_token_append(len, c);
            len = (len + 1);
            c = lexer_read_char();
            if (!lexer_is_alphanumeric(c, false)) {
                break;
            }
        }

        lexer_token_append(len, 0);
        lexer_type = lexer_type_alphanumeric;
        return;
    }

    // Unprefixed string/character literal
    if ((c == '"') | (c == '\'')) {
        lexer_consume_string_char_literal();
        return;
    }

    // Number
    if (isdigit(c)) {
        size_t len = 0;

        // TODO for now we just glob all alphanum characters plus the dot for
        // floats.
        while (1) {
            lexer_token_append(len, c);
            len = (len + 1);
            c = lexer_read_char();
            if (!isalnum(c) & (c != '.')) {
                break;
            }
        }

        lexer_token_append(len, 0);
        lexer_type = lexer_type_number;
        return;
    }

    // Punctuation
    if (NULL != strchr("+-*/%&|^!~<>=()[]{}.?:,;", c)) {
        size_t len = 1;
        lexer_token_append(0, c);
        char c0 = c;
        c = lexer_read_char();
        char c1 = c;

        // Two-character operators
        bool is_assign = ((c1 == '=') & (NULL != strchr("+-*/%&|^!<>=", c0)));
        bool is_double = ((c0 == c1) & (NULL != strchr("+-&|<>", c0)));
        bool is_pointer = ((c0 == '-') & (c1 == '>'));
        if ((is_assign | is_double) | is_pointer) {
            lexer_token_append(1, c);
            c = lexer_read_char();
            len = 2;

            // Three-character operations
            if ((c == '=') & ((c1 == '<') | (c1 == '>'))) {   // <<=, >>=
                lexer_token_append(2, c);
                c = lexer_read_char();
                len = 3;
            }
        }

        lexer_token_append(len, 0);
        lexer_type = lexer_type_punctuation;
        return;
    }

    // Fail
//printf("      Unexpected character: %i\n", c);
    lexer_token_append(0, c);
    lexer_token_append(1, 0);
    fatal_2("Unexpected character: ", lexer_token);
}

bool lexer_is(const char* token) {
    if (((lexer_type == lexer_type_string) |
        (lexer_type == lexer_type_character)) |
        (lexer_type == lexer_type_end))
    {
        return false;
    }
    return 0 == strcmp(token, lexer_token);
}

void lexer_expect(const char* token, const char* error_message) {
    if (!lexer_is(token)) {
        if (error_message) {
            fatal(error_message);
        }
        if (!error_message) {
            fatal_2("Expected ", token);
        }
    }
    lexer_consume();
}

bool lexer_accept(const char* token) {
    //printf("      try %s %s\n",token,lexer_token);
    if (!lexer_is(token)) {
        return false;
    }
    lexer_consume();
    return true;
}

char* lexer_take(void) {
    if (lexer_token == NULL) {
        // This should not be possible.
        fatal("Internal error, no token??");
    }
    //printf("      taking %s\n",lexer_token);

    // Truncate it to size
    char* ret = realloc(lexer_token, strlen(lexer_token) + 1);
    if (ret == NULL) {
        fatal("Out of memory");
    }

    lexer_token = NULL;
    lexer_token_capacity = 0;
    lexer_consume();
    return ret;
}
