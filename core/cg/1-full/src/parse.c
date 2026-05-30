/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025-2026 Fraser Heavy Software
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

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "argument.h"
#include "block.h"
#include "common.h"
#include "instruction.h"
#include "libo-error.h"
#include "libo-reader.h"
#include "libo-unicode.h"
#include "libo-vector.h"
#include "location.h"
#include "symbol.h"

// TODO parse utf8, this should be using libo-reader
//static reader_t reader;

static int current_char;

char* identifier;
static size_t identifier_capacity;

void parse_next_char(void) {
    if (current_char == EOF) {
        fatal("Internal error: cannot consume EOF.");
    }
    current_char = fgetc(input_file);
    if (current_char == EOF) {
        if (ferror(input_file)) {
            fatal("Failed to read from input file.");
        }
    }
}

// TODO the whitespace and comment parsing is adapted from as/2

static bool try_parse_whitespace(void) {
    if (!isspace(current_char))
        return false;

    bool was_carriage_return = current_char == '\r';
    if (was_carriage_return || current_char == '\n') {
        ++current_line;
    }
    parse_next_char();

    // carriage return followed by line feed counts as a single line ending.
    if (was_carriage_return && current_char == '\n') {
        parse_next_char();
    }

    return true;
}

static bool try_parse_comment(void) {
    if (current_char != ';')
        return false;

    // comment found. consume it
    do {
        parse_next_char();
    } while (current_char != '\r' && current_char != '\n');

    // note that we don't consume the carriage return or line feed. we let
    // try_parse_whitespace() do it so it handles line endings correctly.

    return true;
}

static bool try_parse_debug(void) {
    if (current_char != '#')
        return false;

    // debug line found.
    // TODO need to parse #line directives and update location
    // in the meantime we ignore it
    do {
        parse_next_char();
    } while (current_char != '\r' && current_char != '\n');

    // as above, we don't consume the line ending.

    return true;
}

static void parse_whitespace_and_comments(void) {
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

bool is_identifier_char(uint32_t c, bool first_char, bool percent) {
    if (percent && c == '%')
        return true;
    return uchar_is_identifier(current_char, first_char);
}

/**
 * Parses an identiifer.
 *
 * If `percent` is true, the percent symbol ('%') is allowed. This is used for
 * temporaries.
 */
bool try_parse_identifier(bool percent) {
    printf("%s() %s:%i\n", __func__, __FILE__, __LINE__);
    if (!is_identifier_char(current_char, true, percent))
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
        if (!is_identifier_char(current_char, false, percent)) {
            identifier[identifier_length] = 0;
            break;
        }

        // otherwise append the char
        identifier[identifier_length++] = current_char;
        parse_next_char();
    }

    printf("%s() %s:%i parsed identifier %s\n", __func__, __FILE__, __LINE__, identifier);
    return true;
}

static void parse_identifier(bool percent) {
    printf("%s() %s:%i\n", __func__, __FILE__, __LINE__);
    if (!try_parse_identifier(percent)) {
        fatal("Expected an identifier.");
    }
}

static void parse_parameters(void) {
    for (;;) {
        parse_whitespace_and_comments();
        if (current_char == '%') {
            // TODO
            fatal("Parameter parsing is not implemented yet.");
        }
        if (current_char == ':') {
            return;
        }
        if (current_char == 'v') {
            parse_identifier(false);
            if (0 != strcmp(identifier, "varargs")) {
                break;
            }
        }
        break;
    }

    fatal("Expected a parameter, a label or `varargs` in the preamble of this function.");
}

static opcode_t parse_opcode(void) {
    printf("%s() %s:%i current_char %c\n", __func__, __FILE__, __LINE__, current_char);
    if (!try_parse_identifier(false)) {
        fatal("Expected an instruction opcode.");
    }
    return opcode_from_identifier(identifier);
}

static uint32_t parse_register() {
    if (current_char != 'r') {
        fatal("Expected register.");
    }
    parse_next_char();
    
    uint32_t reg_number;
    if (isdigit(current_char)) {
        reg_number = current_char - '0';
        parse_next_char();
    } else {
        switch (current_char) {
            case 'a': reg_number = 10; break;
            case 'b': reg_number = 11; break;
            case 's': reg_number = 12; break;
            case 'f': reg_number = 13; break;
            case 'p': reg_number = 14; break;
            case 'i': reg_number = 15; break;
            default: fatal("Expected register.");
        }
        parse_next_char();
        if (reg_number >= 12) {
            if (current_char != 'p') {
                fatal("Expected register.");
            }
            parse_next_char();
        }
    }

    return reg_number;
}

static uint32_t parse_number(void) {
    printf("%s() %s:%i current_char %c\n", __func__, __FILE__, __LINE__, current_char);
    char buffer[32];
    if (!isdigit(current_char)) {
        fatal("Expected number.");
    }

    // TODO hex

    size_t i = 0;
    do {
        buffer[i++] = current_char - '0';
        if (i == sizeof(buffer)) {
            fatal("Number out of bounds.");
        }
        parse_next_char();
    } while (isdigit(current_char));

    return strtol(buffer, NULL, 10);
}

static argument_t* /*nullable*/ try_parse_argument(void) {
    parse_whitespace_and_comments();

    if (current_char == '%') {
        parse_identifier(true);
        if (identifier[1] == 0) {
            return argument_new_sentinel();
        }
        return argument_new_string(argument_type_temporary, string_intern_cstr(identifier));
    }

    if (current_char == '^') {
        // TODO need to handle flags, move this to e.g. parse_invocation()
        parse_next_char();
        parse_identifier(false);
        return argument_new_string(argument_type_absolute, string_intern_cstr(identifier));
    }

    if (current_char == '&') {
        // TODO flags as above
        parse_next_char();
        parse_identifier(false);
        return argument_new_string(argument_type_relative, string_intern_cstr(identifier));
    }

    if (current_char == 'r') {
        return argument_new_number(argument_type_register, parse_register());
    }

    if (isdigit(current_char)) {
        return argument_new_number(argument_type_number, parse_number());
    }

    fatal("Expected identifier.");
}

static argument_t* parse_argument(void) {
    argument_t* argument = try_parse_argument();
    if (!argument) {
        // TODO location
        fatal("Expected instruction argument.");
    }
    return argument;
}

static instruction_t* parse_instruction(void) {
    parse_whitespace_and_comments();
    opcode_t opcode = parse_opcode();
    instruction_t* instruction = instruction_new(location_new_current(), opcode);

    switch (opcode) {
        case opcode_ret:
            instruction_append(instruction, parse_argument());
            break;
        default:
            fatal("Internal error: invalid/unimplemented opcode");
    }

    return instruction;
}

static void parse_block(symbol_t* symbol) {
    assert(current_char == ':');
    parse_next_char();
    printf("%s() %s:%i\n", __func__, __FILE__, __LINE__);
    parse_identifier(false);
    block_t* block = block_new(identifier, location_new_current());
    vector_append(symbol->blocks, block);

    for (;;) {
        parse_whitespace_and_comments();
        if (current_char == '=' || current_char == ':' || current_char == EOF)
            break;
    printf("%s() %s:%i current_char %c\n", __func__, __FILE__, __LINE__, current_char);
        vector_append(block->instructions, parse_instruction());
    }

    if (vector_count(block->instructions) == 0) {
        fatal("Blocks must contain at least one instruction.");
    }

    // Make sure all but the last instruction do not end the block
    for (size_t i = vector_count(block->instructions) - 1; i-- > 0;) {
        instruction_t* instruction = vector_at(block->instructions, i);
        if (!opcode_is_block_end(instruction->opcode)) {
            // TODO location
            fatal("This instruction is only valid at the end of a block.");
        }
    }

    // Make sure the last instruction ends the block
    instruction_t* last = vector_last(block->instructions);
    if (!opcode_is_block_end(last->opcode)) {
        fatal("Blocks must end in a `ret`, `jmp` or `br` instruction.");
    }
}

symbol_t* /*nullable*/ try_parse_symbol(void) {
    printf("%s() %s:%i\n", __func__, __FILE__, __LINE__);
    if (current_char == EOF) {
        return NULL;
    }
    parse_whitespace_and_comments();

    // parse the symbol name
    if (current_char != '=') {
        fatal("Expected a symbol declaration.");
    }
    location_t* location = location_new_current();
    parse_next_char();
    parse_identifier(false);
    symbol_t* symbol = symbol_new(identifier, location);
    parse_whitespace_and_comments();

    // parse preamble (containing a temporary for each parameter, including
    // possibly a varargs parameter)
    printf("%s() %s:%i\n", __func__, __FILE__, __LINE__);
    parse_parameters();
    printf("%s() %s:%i\n", __func__, __FILE__, __LINE__);

    // parse instructions and labels
    for (;;) {
        if (current_char == '=' || current_char == EOF)
            break;

        // a label starts a new block
        if (current_char == ':') {
            parse_block(symbol);
            continue;
        }

        fatal("Expected a symbol or block.");
    }

    return symbol;
}

void parse_setup(const char* input_filename) {
    string_t* filename_str = string_intern_cstr(input_filename);
    set_current_filename_string(filename_str);
    string_deref(filename_str);
    current_line = 1;

    // prime parser
    parse_next_char();
}

void parse_teardown(void) {
}
