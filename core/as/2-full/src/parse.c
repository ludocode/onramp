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

#include "parse.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "emit.h"
#include "opcodes.h"



/*
 * Global Vars
 */

FILE* input_file;

char* identifier;
static size_t identifier_capacity;

char current_char;

int label_flags;
label_type_t label_type;



/*
 * Helpers
 */

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




/*
 * Parse functions
 */

static bool try_parse_character(uint8_t* out);

bool try_parse_and_emit_short(void) {
    int32_t offset;
    if (try_parse_number(&offset)) {
        if (offset < INT16_MIN || offset > INT16_MAX)
            fatal("Number is out of range of relative jump: %i", offset);
        emit_hex_byte((uint8_t)(offset & 0xFF));
        emit_hex_byte((uint8_t)((offset >> 8) & 0xFF));
        return true;
    }
    return false;
}

static bool try_parse_whitespace(void) {
    if (!isspace(current_char))
        return false;

    bool was_carriage_return = current_char == '\r';
    if (was_carriage_return || current_char == '\n') {
        emit_char('\n');
        ++current_line;
    }
    read_char();

    // carriage return followed by line feed counts as a single line ending.
    if (was_carriage_return && current_char == '\n')
        read_char();

    return true;
}

static bool try_parse_comment(void) {
    if (current_char != ';')
        return false;

    // comment found. consume it
    do {
        read_char();
    } while (current_char != '\r' && current_char != '\n');

    // note that we don't consume the carriage return or line feed. we let
    // try_parse_whitespace() do it so it handles line endings correctly.

    return true;
}

static bool try_parse_debug(void) {
    if (current_char != '#')
        return false;

    // debug line found. consume it and feed it verbatim to the output
    // TODO also parse the line info, handle manual mode
    do {
        emit_char(current_char);
        read_char();
    } while (current_char != '\r' && current_char != '\n');

    // as above, we don't consume the line ending.

    return true;
}

void consume_whitespace_and_comments(void) {
    for (;;) {
        if (try_parse_whitespace())
            continue;
        if (try_parse_comment())
            continue;
        if (try_parse_debug())
            continue;
        break;
    }
}

void read_char(void) {
    if (current_char == -1) {
        fatal("Internal error: cannot consume EOF");
    }
    size_t count = fread(&current_char, 1, 1, input_file);
    if (count != 1) {
        if (feof(input_file)) {
            current_char = -1;
            return;
        }
        fatal("Failed to read from input file");
    }
}

static label_type_t label_type_from_char(char c) {
    switch (c) {
        case '^': return label_type_invocation_absolute;
        case '<': return label_type_invocation_high;
        case '>': return label_type_invocation_low;
        case '&': return label_type_invocation_relative;
        case ':': return label_type_definition_label;
        case '=': return label_type_definition_symbol;
        case '@': return label_type_definition_static;
        default: break;
    }
    fatal("Unrecognized label type");
}

static int label_flag_from_char(char c) {
    switch (c) {
        case '?': return LABEL_FLAG_WEAK;
        case '+': return LABEL_FLAG_ZERO;
        case '{': return LABEL_FLAG_CONSTRUCTOR;
        case '}': return LABEL_FLAG_DESTRUCTOR;
    }
    fatal("Unrecognized label flag");
}

bool try_parse_label(void) {
    consume_whitespace_and_comments();

    // consume the label type
    if (NULL == strchr("^<>&:=@", current_char))
        return false;
    label_type = label_type_from_char(current_char);
    read_char();

    // collect flags
    label_flags = 0;
    while (NULL != strchr("?+{}", current_char)) {
        label_flags |= label_flag_from_char(current_char);
        read_char();
    }

    // collect identifier
    if (!try_parse_identifier()) {
        fatal("Label symbol must be followed by flags or identifier");
    }

    return true;
}

bool try_parse_invocation_absolute(void) {
    consume_whitespace_and_comments();
    if (current_char != '^')
        return false;
    return try_parse_label();
}

bool try_parse_invocation_relative(void) {
    consume_whitespace_and_comments();
    if (current_char != '&')
        return false;
    return try_parse_label();
}

bool try_parse_invocation_short(void) {
    consume_whitespace_and_comments();
    if (current_char != '&' && current_char != '<' && current_char != '>')
        return false;
    return try_parse_label();
}

// TODO move to libo
static bool is_identifier_char(int c, bool first_char) {
    if (isalpha(current_char) || current_char == '_')
        return true;

    // We accept $ in identifiers as an extension.
    if (current_char == '$')
        return true;

    // Identifiers cannot start with digits.
    if (!first_char && isdigit(current_char))
        return true;

    return false;
}

bool try_parse_identifier(void) {
    consume_whitespace_and_comments();

    if (!is_identifier_char(current_char, true))
        return false;

    size_t identifier_length = 0;
    for (;;) {

        // grow identifier buffer if necessary
        if (identifier_length == identifier_capacity) {
            size_t new_capacity = (identifier_capacity == 0) ?
                    32 : identifier_capacity * 2;
            char* new_identifier = realloc(identifier, new_capacity);
            if (new_identifier == NULL) {
                fatal("Memory allocation failed.");
            }
            identifier = new_identifier;
            identifier_capacity = new_capacity;
        }

        // if we've reached the end of the identifier, we're done
        if (!is_identifier_char(current_char, false)) {
            identifier[identifier_length] = 0;
            break;
        }

        // otherwise append the char
        identifier[identifier_length++] = current_char;
        read_char();

    }

    return true;
}

uint32_t parse_decimal(void) {
    uint32_t value = 0;

    while (isdigit(current_char)) {
        uint32_t new_value = value * 10 + current_char - '0';
        if (new_value < value) {
            fatal("Decimal number is out of range.");
        }
        value = new_value;
        read_char();
    }

    return value;
}

uint32_t parse_hexadecimal(void) {
    uint32_t ret = 0;
    int digits = 0;

    while (isxdigit(current_char)) {
        if (++digits > 8) {
            fatal("Hexadecimal number must be at most 8 hexadecimal digits.");
        }

        ret <<= 4;
        if (current_char <= '9') {
            ret |= current_char - '0';
        } else if (current_char <= 'F') {
            ret |= current_char - 'A' + 10;
        } else {
            ret |= current_char - 'a' + 10;
        }

        read_char();
    }

    if (digits == 0) {
        fatal("Number starting with \"0x\" must be followed by at least one hexadecimal digit.");
    }

    return ret;
}

bool try_parse_number(int32_t* out) {
    consume_whitespace_and_comments();

    bool negative = false;
    uint32_t value;

    // handle sign
    if (!isdigit(current_char)) {
        if (current_char == '-') {
            negative = true;
            read_char();
        } else if (current_char == '+') {
            // TODO remove support for '+', don't want to bother supporting it in as/1
            read_char();
        } else {
            return false;
        }

        // sign must be followed by a digit
        if (!isdigit(current_char)) {
            fatal("+ or - sign must be followed by at least one digit.");
        }
    }

    // if first digit is non-zero, it's a decimal number
    if (current_char != '0') {
        value = parse_decimal();

    // otherwise it's either a plain zero or it's hexadecimal
    } else {
        read_char();
        if (current_char != 'x' && current_char != 'X') {
            if (isxdigit(current_char)) {
                fatal("Number starting with '0' must be followed by 'x' for hexadecimal. Decimal numbers cannot start with '0'. Octal and binary are not supported.");
            }
            // it's zero
            *out = 0;
            return true;
        }
        // it's hexadecimal
        read_char();
        value = parse_hexadecimal();
    }

    // apply sign, make sure it's in range
    if (negative) {
        if (value > 0x80000000) {
            fatal("Negative number is out of range.");
        }
        *out = -(int32_t)value;
    } else {
        // we don't care if this wraps, we're about to convert it to hex anyway
        *out = (int32_t)value;
    }
    return true;
}

static bool try_parse_and_emit_number(void) {
    int32_t number;
    if (!try_parse_number(&number)) {
        return false;
    }
    emit_hex_byte((uint8_t)(number & 0xFF));
    emit_hex_byte((uint8_t)((number >> 8) & 0xFF));
    emit_hex_byte((uint8_t)((number >> 16) & 0xFF));
    emit_hex_byte((uint8_t)((number >> 24) & 0xFF));
    return true;
}

bool try_parse_quoted_byte(uint8_t* out) {
    consume_whitespace_and_comments();
    if (current_char != '\'')
        return false;

    read_char();
    int ret = hex_to_int(current_char) << 4;
    read_char();
    ret |= hex_to_int(current_char);
    read_char();

    *out = ret;
    return true;
}

bool try_parse_character_or_quoted_byte(uint8_t* out) {
    if (try_parse_quoted_byte(out))
        return true;
    if (try_parse_character(out))
        return true;
    return false;
}

uint8_t parse_quoted_byte(void) {
    uint8_t ret;
    if (!try_parse_quoted_byte(&ret))
        fatal("Expected quoted byte");
    return ret;
}

// This is hopefully a bit faster than a hashtable or a bunch of string
// comparisons. We avoid strlen(), strcmp(), fnv1a(), etc.
bool register_name_to_number(const char* name, uint8_t* number) {
    if (name[0] == 0)
        return false;
    if (name[1] == 0)
        return false;
    if (name[0] != 'r')
        return false;

    // r0-r9, ra, rb
    if (name[2] == 0) {
        if (isdigit(name[1])) {
            *number = (uint8_t)(0x80 + name[1] - '0');
            return true;
        }
        if (name[1] == 'a') {
            *number = 0x8A;
            return true;
        }
        if (name[1] == 'b') {
            *number = 0x8B;
            return true;
        }
        return false;
    }

    if (name[3] != 0)
        return false;
    if (name[2] != 'p')
        return false;

    // rsp, rfp, rpp, rip
    if (name[1] == 's') {
        *number = 0x8C;
        return true;
    }
    if (name[1] == 'f') {
        *number = 0x8D;
        return true;
    }
    if (name[1] == 'p') {
        *number = 0x8E;
        return true;
    }
    if (name[1] == 'i') {
        *number = 0x8F;
        return true;
    }

    return false;
}

static const char* register_error = "Expected register name or quoted register byte";

uint8_t parse_register(void) {
    uint8_t ret;

    if (try_parse_quoted_byte(&ret)) {
        if ((ret & 0xF0) != 0x80)
            fatal(register_error);
        return ret;
    }

    if (!try_parse_identifier())
        fatal(register_error);
    if (!register_name_to_number(identifier, &ret))
        fatal(register_error);
    return ret;
}

uint8_t parse_mix(void) {

    // number
    int32_t value;
    if (try_parse_number(&value)) {
        if (value < -112 || value > 127) {
            fatal("Decimal number for mix-type argument is out of range (-112 to 127).");
        }
        return (uint8_t)value;
    }

    uint8_t ret;

    // identifier
    if (try_parse_identifier()) {
        if (!register_name_to_number(identifier, &ret))
            fatal("Mix-type argument identifier is not the name of a register.");
        return ret;
    }

    // character
    if (try_parse_character(&ret)) {
        return ret;
    }

    // quoted byte
    if (try_parse_quoted_byte(&ret)) {
        return ret;
    }

    fatal("Mix-type argument must be register name, decimal "
            "number, single-character string or quoted byte.");
}

uint8_t parse_register_non_scratch(void) {
    uint8_t value = parse_register();
    if (value == 0x8A || value == 0x8B) {
        fatal("Register argument cannot be ra or rb.");
    }
    return value;
}

uint8_t parse_register_numbered(void) {
    uint8_t value = parse_register();
    if (value >= 0x8A) {
        fatal("Register argument must be a numbered register.");
    }
    return value;
}

uint8_t parse_mix_non_scratch(void) {
    uint8_t value = parse_mix();
    if (value == 0x8A || value == 0x8B) {
        fatal("Mix-type argument cannot be ra or rb.");
    }
    return value;
}

static uint8_t syscall_number(const char* name) {
    // we don't bother with a hashtable; sys is rarely used.

    // system
    if (0 == strcmp(name, "halt")) return 0x00;
    if (0 == strcmp(name, "time")) return 0x01;
    if (0 == strcmp(name, "spawn")) return 0x02;

    // files
    if (0 == strcmp(name, "fopen")) return 0x03;
    if (0 == strcmp(name, "fclose")) return 0x04;
    if (0 == strcmp(name, "fread")) return 0x05;
    if (0 == strcmp(name, "fwrite")) return 0x06;
    if (0 == strcmp(name, "fseek")) return 0x07;
    if (0 == strcmp(name, "ftell")) return 0x08;
    if (0 == strcmp(name, "ftrunc")) return 0x09;

    // directories
    if (0 == strcmp(name, "dopen")) return 0x0A;
    if (0 == strcmp(name, "dclose")) return 0x0B;
    if (0 == strcmp(name, "dread")) return 0x0C;

    // filesystem
    if (0 == strcmp(name, "stat")) return 0x0D;
    if (0 == strcmp(name, "rename")) return 0x0E;
    if (0 == strcmp(name, "symlink")) return 0x0F;
    if (0 == strcmp(name, "unlink")) return 0x10;
    if (0 == strcmp(name, "chmod")) return 0x11;
    if (0 == strcmp(name, "mkdir")) return 0x12;
    if (0 == strcmp(name, "rmdir")) return 0x13;

    fatal("Argument to sys instruction is not a syscall.");
}

uint8_t parse_syscall_number(void) {
    consume_whitespace_and_comments();
    if (try_parse_identifier()) {
        return syscall_number(identifier);
    }

    int32_t value;
    if (try_parse_number(&value)) {
        if (value < 0 || value > UINT8_MAX)
            fatal("Syscall number out of bounds.");
        return (uint8_t)value;
    }

    uint8_t byte;
    if (try_parse_quoted_byte(&byte)) {
        return byte;
    }

    fatal("Expected a syscall name, number or quoted byte.");
}

static bool is_string_char_valid(char c) {
    // we don't check for double-quote, that should have been checked first

    // these characters are invalid in a string
    if (c == '\\' || c == '\r' || c == '\n')
        return false;

    // otherwise it must be a printable character
    return isprint(c);
}

bool try_parse_and_emit_string(size_t* out_length) {
    consume_whitespace_and_comments();

    // match an opening quote
    if (current_char != '"')
        return false;

    size_t length = 0;
    for (;;) {
        read_char();

        // on a closing quote, we're done
        if (current_char == '"') {
            read_char();
            break;
        }

        // make sure the character is allowed
        if (!is_string_char_valid(current_char)) {
            fatal("Forbidden character in string");
        }

        // emit it as hex
        emit_hex_byte(current_char);
        ++length;
    }

    if (out_length) {
        *out_length = length;
    }
    return true;
}

static bool try_parse_character(uint8_t* out) {
    consume_whitespace_and_comments();

    // match an opening quote
    if (current_char != '"')
        return false;
    read_char();

    // check the character
    if (current_char == '"') {
        fatal("String argument cannot be empty.");
    }
    if (!is_string_char_valid(current_char)) {
        fatal("Forbidden character in string");
    }
    *out = current_char;
    read_char();

    // make sure the string closes properly
    if (current_char != '"') {
        fatal("String argument must consist of a single character.");
    }
    read_char();
    return true;
}

bool parse(void) {
    consume_whitespace_and_comments();
    if (current_char == -1)
        return false;

    // parse an identifier. if found, it must be an opcode.
    if (try_parse_identifier()) {
        if (output_alignment != 0) {
            fatal("Misaligned instruction");
        }
        opcodes_dispatch(identifier);
        return true;
    }

    // quoted byte
    uint8_t byte;
    if (try_parse_quoted_byte(&byte)) {
        emit_hex_byte(byte);
        return true;
    }

    // string
    if (try_parse_and_emit_string(NULL))
        return true;

    // label
    if (try_parse_label()) {
        emit_label(identifier, label_type, label_flags);
        return true;
    }

    // number
    if (try_parse_and_emit_number()) {
        return true;
    }

    fatal("Unrecognized character.");
}
