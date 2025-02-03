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

#include "lexer.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "libo-error.h"
#include "libo-unicode.h"

lexer_t* lexer_current;

static void lexer_consume_char(lexer_t* lexer);
static void lexer_parse(lexer_t* lexer);
static void lexer_complete_token(lexer_t* lexer, token_type_t type);

static string_t* lexer_string_include;

void lexer_setup(void) {
    lexer_string_include = string_intern_cstr("include");
}

void lexer_teardown(void) {
    string_deref(lexer_string_include);
}

static lexer_t* lexer_new(string_t* filename, token_t* /*nullable*/ source) {
    lexer_t* lexer = calloc(1, sizeof(lexer_t));
    if (lexer == NULL) {
        fatal("Out of memory.");
    }

    lexer->buffer_capacity = 32;
    lexer->buffer_count = 0;
    lexer->buffer = malloc(lexer->buffer_capacity);
    
    lexer->start_of_line = true;
    location_init(&lexer->char_location, filename, 1, 0, source);
    lexer->next_char = READER_EOF;

    location_init_copy(&lexer->token_location, &lexer->char_location);

    return lexer;
}

lexer_t* lexer_new_file(string_t* filename, FILE* /*nullable*/ file, token_t* /*nullable*/ source) {
    lexer_t* lexer = lexer_new(filename, source);
    reader_init_file(&lexer->reader, filename, file);
    lexer_consume_char(lexer);
    lexer_parse(lexer); // TODO should defer parse as much as possible
    return lexer;
}

lexer_t* lexer_new_bytes(string_t* filename, char* bytes, size_t length) {
    lexer_t* lexer = lexer_new(filename, NULL);
    reader_init_bytes(&lexer->reader, (char8_t*)bytes, length);
    lexer_consume_char(lexer);
    lexer_parse(lexer); // TODO should defer parse as much as possible
    return lexer;
}

void lexer_delete(lexer_t* lexer) {
    token_deref(lexer->token);
    free(lexer->buffer);
    location_destroy(&lexer->token_location);
    location_destroy(&lexer->char_location);
    reader_destroy(&lexer->reader);
    free(lexer);
}

// Sets the location in libo-error for a lexer call to fatal(). This is set to
// either the current char location or the start of the token depending on the
// error.
static void lexer_set_error_location(location_t* location) {
    current_filename = location->filename->bytes;
    current_line = location->line;
}

/**
 * Consumes a character and parses a new one, performing end-of-line
 * normalization.
 *
 * - A carriage return followed by a line feed, as well as a carriage return
 *   alone, are converted to a single line feed.
 *
 * - Other end-of-line characters from Unicode UAX-31 are converted to a single
 *   line feed.
 *
 * - A backslash followed by any of the end-of-line patterns above are deleted.
 *
 * - A line feed is generated at the end of input if the file did not end in one.
 */
static void lexer_consume_char(lexer_t* lexer) {

    // Clear state on a newline
    /*
    if (lexer->next_char == '\n') {
        lexer->start_of_line = true;
        if (lexer->include_state == lexer_include_state_include) {
            lexer->include_state = lexer_include_state_none;
        }
    }
    */

    // If the last char was a newline, move to next line
    if (lexer->next_char == '\n') {
        ++lexer->char_location.line;
        lexer->char_location.column = 1;
    } else {
        // TODO figure out what we should do for column numbers for unicode.
        // currently we increment column per code point.
        ++lexer->char_location.column;
    }

    // Store the previous character
    lexer->previous_char = lexer->next_char;

    for (;;) {
        lexer->next_char = reader_peek(&lexer->reader, 0);

        // Check for missing line ending at end of file
        if (lexer->next_char == READER_EOF) {
            if (!uchar_is_end_of_line(lexer->previous_char)) {
                lexer->next_char = '\n';
            }
            return;
        }

        // Skip escaped newlines
        if (lexer->next_char == '\\') {
            char32_t eol = reader_peek(&lexer->reader, 1);
            if (uchar_is_end_of_line(eol)) {
                bool crlf = eol == '\r' && reader_peek(&lexer->reader, 2) == '\n';
                reader_consume(&lexer->reader, crlf ? 3 : 2);
                ++lexer->char_location.line;
                lexer->char_location.column = 1;
                continue;
            }
        }

        // Convert carriage return and line feed to single line feed; convert
        // all other line endings to single line feed
        if (uchar_is_end_of_line(lexer->next_char)) {
            bool crlf = lexer->next_char == '\r' && reader_peek(&lexer->reader, 1) == '\n';
            reader_consume(&lexer->reader, crlf ? 2 : 1);
            lexer->next_char = '\n';
            break;
        }

        // Otherwise we don't handle it here
        reader_consume(&lexer->reader, 1);
        break;
    }
}

static void lexer_buffer_append(lexer_t* lexer, char32_t c) {
    if (lexer->buffer_capacity - lexer->buffer_count < 4) {
        size_t new_capacity = lexer->buffer_capacity * 2;
        if (new_capacity <= lexer->buffer_capacity) {
            lexer_set_error_location(&lexer->char_location);
            fatal("Out of memory.");
        }
        lexer->buffer_capacity = new_capacity;
        lexer->buffer = realloc(lexer->buffer, lexer->buffer_capacity);
        if (lexer->buffer == NULL) {
            lexer_set_error_location(&lexer->char_location);
            fatal("Out of memory.");
        }
    }
    lexer->buffer_count += utf8_encode(lexer->buffer + lexer->buffer_count, c);
}

/**
 * Parses a string or character literal (including an angle-bracketed filename
 * after an include directive.)
 */
static void lexer_parse_literal(lexer_t* lexer, token_prefix_t prefix) {
    char32_t end_quote = lexer->next_char;
    if (end_quote == '<')
        end_quote = '>';
    assert(end_quote == '"' || end_quote == '\'' || end_quote == '>');
    lexer_consume_char(lexer);

    // We allow multi-character literals of any length. The compiler can decide
    // what it will support.
    while (lexer->next_char != end_quote) {
        if (lexer->next_char == '\\') {
            // Note: We don't attempt to parse escape sequences at all. We just
            // append it and the character after it in case it's escaping the
            // end quote. We leave it up to the compiler to interpret them.
            // (An escape sequence in the filename of an #include directive,
            // whether quoted or angle-bracketed, is implementation defined, so
            // we don't need to implement this at all. We may eventually want
            // to for compatibility with other compilers though.)
            lexer_buffer_append(lexer, lexer->next_char);
            lexer_consume_char(lexer);
        }

        // Note that READER_END is not possible since the lexer inserts a
        // trailing newline at the end of the file if necessary.
        if (lexer->next_char == '\n') {
            lexer_set_error_location(&lexer->token_location);
            fatal("Unclosed string or character literal.");
        }

        lexer_buffer_append(lexer, lexer->next_char);
        lexer_consume_char(lexer);
    }
    lexer_consume_char(lexer);

    token_type_t type;
    switch (end_quote) {
        case '\'': type = token_type_character; break;
        case '"': type = token_type_string; break;
        case '>': type = token_type_angle_include; break;
        default: abort(); // unreachable
    }
    lexer_complete_token(lexer, type);
    lexer->token->prefix = prefix;
}

/**
 * Parse a punctuation token.
 *
 * If `have_slash` is true, we've already consumed a starting slash.
 *
 * Note that there's a bit of a hack here: this always consumes a character and
 * appends it to the buffer even if it fails. This must be called last in the
 * lexer (except for the invalid token handler.) TODO I should probably fix
 * this at some point, just haven't bothered right now.
 */
static bool lexer_try_parse_punctuation(lexer_t* lexer, bool have_slash) {
    char32_t c0;
    if (have_slash) {
        c0 = '/';
    } else {
        c0 = lexer->next_char;
        lexer_consume_char(lexer);
    }
    lexer_buffer_append(lexer, c0);
    char32_t c1 = lexer->next_char;

    // check for compound assignments and two-character comparisons first (any
    // punctuation where `=` is the second character)
    if (c1 == '=') {
        switch (c0) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '%':
            case '&':
            case '|':
            case '^':
            case '!':
            case '<': // TODO in order to preprocess C++20 we should handle spaceship operator here
            case '>':
            case '=':
                goto two;
            default:
                goto done;
        }
    }

    // check for doubled operators
    if (c0 == c1) {
        if (c1 == '<' || c1 == '>') {
            lexer_buffer_append(lexer, c1);
            lexer_consume_char(lexer);
            if (lexer->next_char == '=') {
                // `<<=` or `>>=`
                lexer_buffer_append(lexer, '=');
                lexer_consume_char(lexer);
            }
            goto done;
        }
        switch (c0) {
            case '+':
            case '-':
            case '#':
            case '&':
            case '|':
            case '=':
                goto two;
            default:
                break;
        }
    }

    // anything else
    switch (c0) {

        // individual operators (not handled above)
        case '+':
        case '*':
        case '/':
        case '%':
        case '&':
        case '|':
        case '^':
        case '!':
        case '~':
        case '<':
        case '>':
        case '=':
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case '?':
        case ':':
        case ',':
        case ';':
            goto done;

        // `#` is either punctuation or directive
        case '#':
            if (lexer->start_of_line) {
                lexer_complete_token(lexer, token_type_directive);
                return true;
            }
            goto done;

        // `-` or `->`
        case '-':
            if (c1 == '>')
                goto two;
            goto done;

        // `.` or `...`
        case '.':
            if (c1 == '.') {
                lexer_consume_char(lexer);
                if (lexer->next_char != '.') {
                    // `..` is not a valid token, but it is allowed to occur in
                    // a directive in an untaken conditional branch.
                    lexer_complete_token(lexer, token_type_invalid);
                    return true;
                }
                lexer_consume_char(lexer);
                lexer_buffer_append(lexer, '.');
                lexer_buffer_append(lexer, '.');
            }
            goto done;

        default:
            break;
    }

    return false;

two:
    // the second character is also part of the token
    lexer_buffer_append(lexer, c1);
    lexer_consume_char(lexer);
done:
    // all necessary characters have been appended to the buffer
    lexer_complete_token(lexer, token_type_punctuation);
    return true;
}

/**
 * Parses a C-style comment. The starting `/` has already been consumed.
 */
static void lexer_parse_c_comment(lexer_t* lexer) {
    assert(lexer->next_char == '*');
    lexer_consume_char(lexer);

    for (;;) {
        char32_t c = lexer->next_char;
        if (c == READER_EOF) {
            lexer_set_error_location(&lexer->token_location);
            fatal("Unterminated block comment.");
        }
        if (c == '*') {
            lexer_consume_char(lexer);
            if (lexer->next_char == '/') {
                lexer_consume_char(lexer);
                break;
            }
            continue;
        }
        lexer_consume_char(lexer);
    }

    lexer_buffer_append(lexer, ' ');
    lexer_complete_token(lexer, token_type_space);
}

/**
 * Parses a C++-style comment. The starting `/` has already been consumed.
 */
static void lexer_parse_cxx_comment(lexer_t* lexer) {
    assert(lexer->next_char == '/');
    lexer_consume_char(lexer);

    // Note that we don't include the newline in the comment. It gets put in
    // its own newline token.
    while (lexer->next_char != '\n') {
        lexer_consume_char(lexer);
    }

    lexer_buffer_append(lexer, ' ');
    lexer_complete_token(lexer, token_type_space);
}

/**
 * Parses an invalid token.
 *
 * The only place this is allowed to occur is in (TODO a directive in?) an
 * untaken conditional branch. We ignore the rest of the line and produce a
 * single invalid token; the parser will reject if it it's not in an untaken
 * directive.
 */
static void lexer_parse_invalid(lexer_t* lexer) {
    while (lexer->next_char != '\n') {
        lexer_consume_char(lexer);
    }
    lexer_complete_token(lexer, token_type_invalid);
}

/**
 * Completes parsing of a token which has been read into the buffer, updating
 * any relevant state.
 */
static void lexer_complete_token(lexer_t* lexer, token_type_t type) {
    lexer->token = token_new_bytes(type, lexer->buffer, lexer->buffer_count, &lexer->token_location);

    lexer->buffer_count = 0;
    location_set_copy(&lexer->token_location, &lexer->char_location);

    // update start of line flag
    if (type == token_type_newline) {
        if (lexer->include_state == lexer_include_state_include) {
            lexer->include_state = lexer_include_state_none;
        }
        lexer->start_of_line = true;
    } else if (type != token_type_space) {
        lexer->start_of_line = false;
    }

    // update include state
    switch (lexer->include_state) {
        case lexer_include_state_none:
            if (type == token_type_directive) {
                lexer->include_state = lexer_include_state_directive;
            }
            break;
        case lexer_include_state_directive:
            if (type == token_type_alphanumeric && string_equal(lexer->token->value, lexer_string_include)) {
                lexer->include_state = lexer_include_state_include;
            } else {
                lexer->include_state = lexer_include_state_none;
            }
            break;
        case lexer_include_state_include:
            // this state is exited when a newline is consumed
            break;
    }
}

/**
 * Decodes the prefix in the buffer.
 */
static token_prefix_t lexer_literal_prefix(lexer_t* lexer) {
    assert(lexer->buffer_count > 0);

    if (lexer->next_char == '<') {
        // no string prefixes are valid on an angle-bracketed include path.
        return token_prefix_none;
    }

    // null-terminate the buffer
    lexer_buffer_append(lexer, 0);
    --lexer->buffer_count;

    // check with strcmp
    // TODO check the language standard for these. e.g. u8 is not valid until
    // C23, u/U are not valid until C11
    if (0 == strcmp((char*)lexer->buffer, "L")) return token_prefix_L;
    if (0 == strcmp((char*)lexer->buffer, "u")) return token_prefix_u;
    if (0 == strcmp((char*)lexer->buffer, "U")) return token_prefix_U;
    if (0 == strcmp((char*)lexer->buffer, "u8")) return token_prefix_u8;

    return token_prefix_none;
}

/**
 * Parses a token.
 */
static void lexer_parse(lexer_t* lexer) {

    // End of file
    if (lexer->next_char == READER_EOF) {
        lexer->token = token_ref(token_end);
        return;
    }

    // Whitespace
    if (uchar_is_whitespace(lexer->next_char)) {
        bool newline = false;
        do {
            if (lexer->next_char == '\n')
                newline = true;
            lexer_consume_char(lexer);
        } while (uchar_is_whitespace(lexer->next_char));
        lexer_complete_token(lexer, newline ? token_type_newline : token_type_space);
        return;
    }

    // String
    if (lexer->next_char == '"' || lexer->next_char == '\'' ||
            (lexer->next_char == '<' && lexer->include_state == lexer_include_state_include))
    {
        lexer_parse_literal(lexer, token_prefix_none);
        return;
    }

    // Identifier (or a string prefixed by an identifier)
    if (uchar_is_identifier(lexer->next_char, true)) {
        do {
            lexer_buffer_append(lexer, lexer->next_char);
            lexer_consume_char(lexer);
        } while (lexer->next_char != READER_EOF && uchar_is_identifier(lexer->next_char, false));

        if (lexer->next_char == '"' || lexer->next_char == '\'' ||
                (lexer->next_char == '<' && lexer->include_state == lexer_include_state_include))
        {
            // We only consume the string now if the prefix is valid. If not,
            // the prefix is its own token.
            token_prefix_t prefix = lexer_literal_prefix(lexer);
            if (prefix != token_prefix_none) {
                lexer->buffer_count = 0;
                lexer_parse_literal(lexer, prefix);
                return;
            }
        }

        lexer_complete_token(lexer, token_type_alphanumeric);
        return;
    }

    // Comment (or punctutation starting with '/')
    if (lexer->next_char == '/') {
        lexer_consume_char(lexer);
        if (lexer->next_char == '*') {
            lexer_parse_c_comment(lexer);
            return;
        }
        if (lexer->next_char == '/') {
            lexer_parse_cxx_comment(lexer);
            return;
        }

        // Not a comment, parse punctuation
        lexer_try_parse_punctuation(lexer, true);
        return;
    }

    // Number
    if (isdigit(lexer->next_char)) {
        // TODO like cci/2, for now we just glob all alphanum plus dot and quote.
        do {
            lexer_buffer_append(lexer, lexer->next_char);
            lexer_consume_char(lexer);
        } while (lexer->next_char != READER_EOF &&
                (isalnum(lexer->next_char) || lexer->next_char == '.' || lexer->next_char == '\''));
        lexer_complete_token(lexer, token_type_number);
        return;
    }

    // Punctuation
    // Note that there's a bit of a hack here: this always consumes a character
    // and appends it to the buffer even if it fails. I'm too lazy to fix this
    // right now so this must be called last in the lexer (except for the
    // invalid token handler.)
    if (lexer_try_parse_punctuation(lexer, false)) {
        return;
    }

    // Otherwise, it's an invalid token. (This is still a token.)
    lexer_parse_invalid(lexer);
}

token_t* lexer_peek(lexer_t* lexer) {
    assert(lexer->token);
    return lexer->token;
}

token_t* lexer_take(lexer_t* lexer) {
    assert(lexer->token);
    token_t* token = lexer->token;
    lexer_parse(lexer);
    return token;
}

void lexer_consume(lexer_t* lexer) {
    assert(lexer->token);
    token_deref(lexer->token);
    lexer_parse(lexer);
}
