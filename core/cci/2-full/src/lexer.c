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

#include "lexer.h"

#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#include "libo-error.h"
#include "token.h"
#include "common.h"

token_t* lexer_token;

// A queued token (from lexer_push().)
static token_t* queued_token;

// The `pragma` token from the file that included this one.
static token_t* include_token;

// The next character (the last read from the file), not added to lexer_token
// yet. This is an int because it's -1 (EOF) at the end of the file.
static int lexer_char;

// Tokens are accumulated into this growable buffer.
static char* lexer_buffer;
static size_t lexer_buffer_capacity;
static size_t lexer_buffer_length;
#define LEXER_MINIMUM_CAPACITY 32

static FILE* lexer_file;

// This stores an intern version of current_filename from libo (so we don't
// have to intern it again for each token.) They should always match.
static string_t* lexer_filename;

// TODO move to libo
int hex_to_int(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    fatal("Expected hexadecimal character");
}

static void lexer_buffer_append(char c) {
    if (lexer_buffer_length == lexer_buffer_capacity) {

        // Not enough room for the additional character; we need to reallocate.
        // Calculate new size
        size_t new_capacity = ((lexer_buffer_capacity * 3) / 2);
        if (new_capacity < lexer_buffer_capacity) {
            // overflow
            fatal("Out of memory.");
        }
        if (new_capacity < LEXER_MINIMUM_CAPACITY) {
            new_capacity = LEXER_MINIMUM_CAPACITY;
        }

        // Reallocate
        lexer_buffer = realloc(lexer_buffer, new_capacity);
        if (lexer_buffer == NULL) {
            fatal("Out of memory.");
        }
        lexer_buffer_capacity = new_capacity;
    }

    lexer_buffer[lexer_buffer_length++] = c;
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

static bool lexer_is_end_of_line(int c) {
    return c == '\n' || c == '\r' || c == -1;
}

// Reads the next character, placing it in lexer_char and returning it.
static int lexer_read_char(void) {
    int c = fgetc(lexer_file);
    lexer_char = c;
    return c;
}

/**
 * Initializes the lexer, opening the given `.i` preprocessed C source file.
 */
void lexer_init(const char* filename) {
    lexer_file = fopen(filename, "r");
    if (lexer_file == NULL) {
        fatal("Failed to open input file: %s", filename);
    }

    lexer_filename = string_intern_cstr(filename);
    current_filename = (char*)string_cstr(lexer_filename);
    current_line = 1;

    // Prime the current char with a newline so the first line can be a #line
    // directive or #pragma.
    lexer_char = '\n';
    lexer_consume();
}

/**
 * Destroys the lexer.
 */
void lexer_destroy(void) {
    fclose(lexer_file);
    if (queued_token) {
        token_deref(queued_token);
    }
    if (lexer_token) {
        token_deref(lexer_token);
    }
    current_filename = NULL;
    string_deref(lexer_filename);
    free(lexer_buffer);
}

static void lexer_consume_literal_char(void) {
    int c = lexer_char;

    // not escape sequence
    if (c != '\\') {
        lexer_buffer_append(c);
        lexer_read_char();
        return;
    }

    // escape sequence
    c = lexer_read_char();
    switch (c) {

        // These definitions look circular but they're not: we're relying on the
        // definition of these escape sequences from the previous stage compiler.
        case 'a':   lexer_buffer_append('\a');  break;  // bell
        case 'b':   lexer_buffer_append('\b');  break;  // backspace
        case 't':   lexer_buffer_append('\t');  break;  // horizontal tab
        case 'n':   lexer_buffer_append('\n');  break;  // line feed
        case 'v':   lexer_buffer_append('\v');  break;  // vertical tab
        case 'f':   lexer_buffer_append('\f');  break;  // form feed
        case 'r':   lexer_buffer_append('\r');  break;  // carriage return
        case 'e':   lexer_buffer_append(27);    break;  // escape (extension, not standard C)
        case '"':   lexer_buffer_append('"');   break;  // double quote
        case '\'':  lexer_buffer_append('\'');  break;  // single quote
        case '?':   lexer_buffer_append('?');   break;  // question mark
        case '\\':  lexer_buffer_append('\\');  break;  // backslash

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7': {
            unsigned value = c - '0';
            c = lexer_read_char();
            if (c >= '0' && c <= '7') {
                value <<= 3;
                value += c - '0';
                c = lexer_read_char();
                if (c >= '0' && c <= '7') {
                    value <<= 3;
                    value += c - '0';
                    lexer_read_char();
                }
            }
            if (c > 255) {
                fatal("The maximum octal escape sequence is \\377.");
            }
            lexer_buffer_append((char)value);
            return;
        }

        // TODO all of these
        case 'x':
        case 'X': {
            unsigned value = 0;
            c = lexer_read_char();
            while (isxdigit(c)) {
                value <<= 4;
                value += hex_to_int(c);
                c = lexer_read_char();
            }
            if (value > 0xff) {
                fatal("Hexadecimal unicode escape sequences are not yet supported.");
            }
            lexer_buffer_append((char)value);
            return;
        }
        case 'u':
        case 'U':
            fatal("Unicode escape sequences are not yet supported.");

        default:
            fatal("Unrecognized escape sequence");
    }

    // Cases that break above don't consume the character themselves; we do it
    // here.
    lexer_read_char();
}

static void lexer_consume_string_literal(void) {
    assert(lexer_char == '"');
    lexer_buffer_length = 0;
    lexer_read_char();

    // Collect characters until the closing quote
    while (true) {
        int c = lexer_char;
        if (c == '"') {
            lexer_read_char();
            break;
        }

        if (lexer_is_end_of_line(c)) {
            fatal("Unclosed string literal");
        }

        lexer_consume_literal_char();
    }
}

static void lexer_consume_char_literal(void) {
    assert(lexer_char == '\'');
    lexer_buffer_length = 0;

    // read the char
    char c = lexer_read_char();
    if (c == '\'') {
        fatal("Empty char literal is not allowed.");
    }
    if (lexer_is_end_of_line(c)) {
        fatal("Unclosed character literal.");
    }
    lexer_consume_literal_char();

    // consume the closing quote
    c = lexer_char;
    if (lexer_is_end_of_line(c)) {
        fatal("Unclosed character literal.");
    }
    if (c != '\'') {
        fatal("Only a single character is supported in a char literal.");
    }
    lexer_read_char();
}

static void lexer_consume_optional_horizontal_whitespace(void) {
    while (lexer_char == ' ' || lexer_char == '\t') {
        lexer_read_char();
    }
}

static void lexer_consume_horizontal_whitespace(void) {
    if (lexer_char != ' ' && lexer_char != '\t') {
        fatal("Expected horizontal whitespace");
    }
    lexer_consume_optional_horizontal_whitespace();
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
    // TODO use strtol() base 10, want to check for overflow
    if (!isdigit(lexer_char)) {
        fatal("Expected line number after #line");
    }
    int line = 0;
    while (isdigit(lexer_char)) {
        line = line * 10 + (lexer_char - '0');
        lexer_read_char();
    }

    // Line number is off by 1 because the end of the #line directive will
    // increment it.
    current_line = line - 1;

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
    string_deref(lexer_filename);
    lexer_filename = string_intern_bytes(lexer_buffer, lexer_buffer_length);
    current_filename = (char*)string_cstr(lexer_filename);

    lexer_consume_optional_horizontal_whitespace();
    if (!lexer_is_end_of_line(lexer_char)) {
        fatal("Expected end of line after filename in #line directive");
    }
}

static void lexer_parse_directive(void) {

    // skip the '#'
    assert(lexer_char == '#');
    lexer_read_char();
    lexer_consume_optional_horizontal_whitespace();

    // read the command
    lexer_buffer_length = 0;
    while (isalpha(lexer_char)) {
        lexer_buffer_append(lexer_char);
        lexer_read_char();
    }
    lexer_buffer_append(0);

    // handle a line directive
    if (0 == strcmp(lexer_buffer, "line")) {
        lexer_handle_line_directive();
        return;
    }

    // handle a pragma
    if (0 == strcmp(lexer_buffer, "pragma")) {
        // TODO
    }

    // otherwise ignore it
    lexer_consume_until_newline();
}

static void lexer_consume_end_of_line(void) {
    int c = lexer_char;
    if (c == '\n') {
        ++current_line;
        lexer_read_char();
        return;
    }

    if (c == '\r') {
        ++current_line;
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

// Consumes whitespace, returning true if a newline was found.
static bool lexer_consume_whitespace(void) {

    // (Note that we don't handle comments or escaped newlines. Those need to
    // be filtered out by the preprocessor.)

    bool found_newline = false;
    char c = (char)lexer_char;
    while (isspace(c)) {
        if (c == '\n' || c == '\r') {
            found_newline = true;
            lexer_consume_end_of_line();
            c = (char)lexer_char;
            continue;
        }
        c = lexer_read_char();
    }

    return found_newline;
}

static void lexer_consume_whitespace_and_directives(void) {
    while (1) {
        bool found_newline = lexer_consume_whitespace();
        if (lexer_char != '#') {
            break;
        }
        if (!found_newline) {
            fatal("A `#` preprocessor directive can only appear at the start of a line.");
        }
        lexer_parse_directive();
    }
}

void lexer_consume(void) {
    if (lexer_token) {
        token_deref(lexer_token);
    }

    // If we already have a queued token, use it.
    if (queued_token) {
        lexer_token = queued_token;
        queued_token = NULL;
        return;
    }

    // Skip whitespace and handle #line directives. This brings us to the start
    // of the next real token.
    lexer_consume_whitespace_and_directives();

    // Store the line now so that our line number will be on the line the token
    // starts (in case it's broken by a line continuation.)
    int line = current_line;

    // Check for end of file
    if (lexer_char == EOF) {
        lexer_token = token_new(token_type_end,
                string_intern_cstr(""), token_prefix_none,
                lexer_filename, line, include_token);
        return;
    }

    char c = (char)lexer_char;
    lexer_buffer_length = 0;

    // Alphanumeric (keyword, identifier name or type name)
    if (lexer_is_alphanumeric(c, true)) {
        do {
            lexer_buffer_append(c);
            c = lexer_read_char();
        } while (lexer_is_alphanumeric(c, false));

        // TODO handle prefixed string/character literal
        if (c == '"' || c == '\'') {
            fatal("String and character literal prefixes are not implemented yet.");
        }

        lexer_token = token_new(token_type_alphanumeric,
                string_intern_bytes(lexer_buffer, lexer_buffer_length),
                token_prefix_none, lexer_filename, line, include_token);
        return;
    }

    // Unprefixed string/character literal
    if (c == '"') {
        lexer_consume_string_literal();
        lexer_token = token_new(token_type_string,
                string_intern_bytes(lexer_buffer, lexer_buffer_length),
                token_prefix_none, lexer_filename, line, include_token);
        return;
    }
    if (c == '\'') {
        lexer_consume_char_literal();
        lexer_token = token_new(token_type_character,
                string_intern_bytes(lexer_buffer, lexer_buffer_length),
                token_prefix_none, lexer_filename, line, include_token);
        return;
    }

    // Number
    if (isdigit(c)) {
        // TODO for now we just glob all alphanum characters plus the dot for
        // floats and quote for digit separator.
        // Note that a number is not supposed to end in a digit separator, but
        // there is no situation where a quote is valid after a number so we
        // include it in the token and let the number parser detect the error.
        do {
            lexer_buffer_append(c);
            c = lexer_read_char();
        } while (isalnum(c) || c == '.' || c == '\'');
        lexer_token = token_new(token_type_number,
                string_intern_bytes(lexer_buffer, lexer_buffer_length),
                token_prefix_none, lexer_filename, line, include_token);
        return;
    }

    // Punctuation
    if (NULL != strchr("+-*/%&|^!~<>=()[]{}.?:,;", c)) {
        lexer_buffer_append(c);
        char c0 = c;
        c = lexer_read_char();
        char c1 = c;

        // Two-character operators
        bool is_assign = c1 == '=' && NULL != strchr("+-*/%&|^!<>=", c0);
        bool is_double = c0 == c1 && NULL != strchr("+-&|<>", c0);
        bool is_pointer = c0 == '-' && c1 == '>';
        bool is_variadic = c0 == '.' && c1 == '.';
        if (is_assign || is_double || is_pointer || is_variadic) {
            lexer_buffer_append(c);
            c = lexer_read_char();

            // Three-character operations (<<=, >>=, ...)
            if (((c == '=') & ((c1 == '<') | (c1 == '>'))) | ((c == '.') & (c0 == '.'))) {
                lexer_buffer_append(c);
                c = lexer_read_char();
            }

            if (lexer_buffer_length == 2 && c0 == '.') {
                fatal("`..` is not a valid token.");
            }
        }

        lexer_token = token_new(token_type_punctuation,
                string_intern_bytes(lexer_buffer, lexer_buffer_length),
                token_prefix_none, lexer_filename, line, include_token);
        return;
    }

    fatal("Unexpected character: %c", c);
}

struct token_t* lexer_take(void) {
    token_t* token = token_ref(lexer_token);
    lexer_consume();
    return token;
}

void lexer_push(token_t* token) {
    if (queued_token) {
        fatal_token(token, "Internal error: At most one token can be queued.");
    }
    queued_token = lexer_token;
    lexer_token = token;
}

void lexer_expect(const string_t* token, const char* error_message) {
    if (!lexer_is(token)) {
        if (error_message) {
            fatal_token(lexer_token, error_message);
        }
        if (!error_message) {
            fatal_token(lexer_token, "Expected `%s`", token->bytes);
        }
    }
    lexer_consume();
}

/**
 * Consumes the current token and returns true if it matches the given
 * alphanumeric or punctuation; returns false otherwise.
 *
 * This always returns false if the current token is not alphanumeric or
 * punctuation.
 */
bool lexer_accept(const string_t* token) {
    if (!lexer_is(token)) {
        return false;
    }
    lexer_consume();
    return true;
}

bool lexer_is(const string_t* token) {
    if (lexer_token->type == token_type_punctuation ||
            lexer_token->type == token_type_alphanumeric)
    {
        return string_equal(lexer_token->value, token);
    }
    return false;
}

void lexer_dump_tokens(void) {
    while (lexer_token->type != token_type_end) {
        printf("    token %c %s\n", (char)lexer_token->type,
                string_cstr(lexer_token->value));
        lexer_consume();
    }
}
