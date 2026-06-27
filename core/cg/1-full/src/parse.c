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
#include "emit.h"
#include "instruction.h"
#include "libo-error.h"
#include "libo-reader.h"
#include "libo-unicode.h"
#include "libo-vector.h"
#include "location.h"
#include "symbol.h"
#include "temporary.h"

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
    //printf("%s() %s:%i\n", __func__, __FILE__, __LINE__);
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

    //printf("%s() %s:%i parsed identifier %s\n", __func__, __FILE__, __LINE__, identifier);
    return true;
}

static void parse_identifier(bool percent) {
    //printf("%s() %s:%i\n", __func__, __FILE__, __LINE__);
    if (!try_parse_identifier(percent)) {
        fatal("Expected an identifier.");
    }
}

static void parse_parameters(symbol_t* symbol) {
    for (;;) {
        parse_whitespace_and_comments();
        if (current_char == '%') {
            parse_identifier(true);
            temporary_t* temporary = temporary_create(identifier);
            if (!temporary) {
                fatal("Duplicate parameter.");
            }
            vector_append(symbol->parameters, temporary);
            continue;
        }

        if (current_char == ':') {
            return;
        }

        if (current_char == 'v') {

            // the only keyword allowed is varargs
            parse_identifier(false);
            if (0 != strcmp(identifier, "varargs")) {
                break;
            }

            // parse the variadic temporary
            parse_whitespace_and_comments();
            if (current_char != '%') {
                fatal("Expected a temporary after `varargs`.");
            }
            parse_identifier(true);
            temporary_t* temporary = temporary_create(identifier);
            if (!temporary) {
                fatal("Duplicate parameter.");
            }
            symbol->varargs = temporary;

            // block must follow
            parse_whitespace_and_comments();
            if (current_char != ':') {
                fatal("Expected a block after varargs temporary.");
            }
            return;
        }

        // not recognized. error
        break;
    }

    fatal("Expected a parameter, a label or `varargs` in the preamble of this function.");
}

static opcode_t parse_opcode(void) {
    //printf("%s() %s:%i current_char %c\n", __func__, __FILE__, __LINE__, current_char);
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
    //printf("%s() %s:%i current_char %c\n", __func__, __FILE__, __LINE__, current_char);
    char buffer[32];
    if (!isdigit(current_char)) {
        fatal("Expected number.");
    }

    // TODO hex

    size_t i = 0;
    do {
        buffer[i++] = current_char;
        if (i == sizeof(buffer)) {
            fatal("Number out of bounds.");
        }
        parse_next_char();
    } while (isdigit(current_char));
    buffer[i] = 0;

    return strtol(buffer, NULL, 10);
}

static argument_t* /*nullable*/ try_parse_argument(void) {
    parse_whitespace_and_comments();

    if (current_char == '%') {
        parse_identifier(true);
        if (identifier[1] == 0) {
            return argument_new_sentinel();
        }
        return argument_new_temporary(temporary_find_or_create(identifier));
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

    //printf("%s() %s:%i current_char %c\n", __func__, __FILE__, __LINE__, current_char);
    fatal("Expected argument.");
}

static argument_t* parse_argument(void) {
    argument_t* argument = try_parse_argument();
    if (!argument) {
        // TODO location
        fatal("Expected instruction argument.");
    }
    return argument;
}

/**
 * Parses a mix-type argument.
 */
static void parse_argument_mix(instruction_t* instruction) {
    argument_t* argument = instruction_append(instruction, parse_argument());
    if (!argument_type_is_mix(argument->type)) {
        fatal("Expected temporary or number argument.");
    }
}

/**
 * Parses an optional mix-type argument.
 */
static void parse_argument_mix_opt(instruction_t* instruction) {
    argument_t* argument = instruction_append(instruction, parse_argument());
    if (!argument_type_is_mix(argument->type) && argument->type != argument_type_sentinel) {
        fatal("Expected temporary or number or sentinel argument.");
    }
}

/**
 * Parse a temporary argument.
 */
static void parse_argument_temp(instruction_t* instruction) {
    argument_t* argument = instruction_append(instruction, parse_argument());
    if (argument->type != argument_type_temporary) {
        fatal("Expected temporary argument.");
    }
}

/**
 * Parse an optional temporary argument.
 */
static void parse_argument_temp_opt(instruction_t* instruction) {
    argument_t* argument = instruction_append(instruction, parse_argument());
    if (argument->type != argument_type_temporary && argument->type != argument_type_sentinel) {
        fatal("Expected temporary or sentinel argument.");
    }
}

/**
 * Parse a number argument.
 */
static void parse_argument_number(instruction_t* instruction) {
    argument_t* argument = instruction_append(instruction, parse_argument());
    if (argument->type != argument_type_number) {
        fatal("Expected number argument.");
    }
}

/**
 * Parse an optional number argument.
 */
static void parse_argument_number_opt(instruction_t* instruction) {
    argument_t* argument = instruction_append(instruction, parse_argument());
    if (argument->type != argument_type_number && argument->type != argument_type_sentinel) {
        fatal("Expected number or sentinel argument.");
    }
}

/**
 * Parse a relative invocation argument.
 */
static void parse_argument_relative(instruction_t* instruction) {
    argument_t* argument = instruction_append(instruction, parse_argument());
    if (argument->type != argument_type_relative) {
        fatal("Expected relative invocation (label) argument.");
    }
}

static void parse_argument_absolute(instruction_t* instruction) {
    argument_t* argument = instruction_append(instruction, parse_argument());
    if (argument->type != argument_type_absolute) {
        fatal("Expected absolute invocation (symbol) argument.");
    }
}

static void parse_argument_temporary_or_absolute(instruction_t* instruction) {
    argument_t* argument = instruction_append(instruction, parse_argument());
    if (argument->type != argument_type_absolute && argument->type != argument_type_temporary) {
        fatal("Expected absolute invocation (symbol) or temporary.");
    }
}

static void parse_call_arguments(instruction_t* instruction) {

    // Return value is a temporary or sentinel (if ignored)
    parse_argument_temp_opt(instruction);

    // Next is the function. This can be either an absolute invocation (for a
    // typical function call) or a temporary (for a function pointer.)
    parse_argument_temporary_or_absolute(instruction);

    // Keep parsing arguments until we reach keyword "end".
    for (;;) {
        parse_whitespace_and_comments();
        if (current_char == 'e') {
            parse_identifier(false);
            if (0 != strcmp(identifier, "end")) {
                fatal("Expected `end` or a function argument.");
            }
            break;
        }
        parse_argument_mix_opt(instruction);
    }
}

static instruction_t* parse_instruction(void) {
    parse_whitespace_and_comments();
    opcode_t opcode = parse_opcode();
    //printf("%s() %s:%i parsed opcode %i\n", __func__, __FILE__, __LINE__,opcode);
    instruction_t* instruction = instruction_new(location_new_current(), opcode);

    //printf("%s() %s:%i switch on opcode %i\n", __func__, __FILE__, __LINE__,opcode);
    switch (opcode) {

        // temp-mix-mix instructions
        case opcode_add:
        case opcode_sub:
        case opcode_mul:
        case opcode_divu:
        case opcode_divs:
        case opcode_modu:
        case opcode_mods:
        case opcode_and:
        case opcode_or:
        case opcode_xor:
        case opcode_shl:
        case opcode_shru:
        case opcode_shrs:
        case opcode_rol:
        case opcode_ror:
        case opcode_ltu:
        case opcode_lts:
            parse_argument_temp(instruction);
            parse_argument_mix(instruction);
            parse_argument_mix(instruction);
            break;

        // temp-mix instructions
        case opcode_sxs:
        case opcode_sxb:
        case opcode_trs:
        case opcode_trb:
        case opcode_not:
        case opcode_mov:
        case opcode_bool:
        case opcode_isz:
        case opcode_alloc:
            parse_argument_temp(instruction);
            parse_argument_mix(instruction);
            break;

        // temp-temp instructions
        case opcode_ldw:
        case opcode_lds:
        case opcode_ldb:
            parse_argument_temp(instruction);
            parse_argument_temporary_or_absolute(instruction);
            break;

        // mix-temp instructions
        case opcode_stw:
        case opcode_sts:
        case opcode_stb:
            parse_argument_mix(instruction);
            parse_argument_temporary_or_absolute(instruction);
            break;

        // temp instructions
        case opcode_zero:
        case opcode_inc:
        case opcode_dec:
        case opcode_volatile:
            parse_argument_temp(instruction);
            break;

        // mix instructions
        case opcode_free:
        case opcode_ret:
            parse_argument_mix(instruction);
            break;

        // misc
        case opcode_var:
            parse_argument_temp(instruction);
            parse_argument_number(instruction);
            parse_argument_number_opt(instruction);
            break;
        case opcode_jmp:
            parse_argument_relative(instruction);
            break;
        case opcode_br:
            parse_argument_mix(instruction);
            parse_argument_relative(instruction);
            parse_argument_relative(instruction);
            break;
        case opcode_call:
            parse_call_arguments(instruction);
            break;
        case opcode_sym:
            parse_argument_temp(instruction);
            parse_argument_absolute(instruction);
            break;

        case opcode_enter:
        case opcode_leave:
        case opcode_jz:
            fatal("This assembly opcode cannot appear in IR.");
            break;

        default:
            fprintf(stderr, "opcode: %s\n", opcode_to_string(opcode));
            fatal("Internal error: invalid/unimplemented opcode");
    }

    return instruction;
}

static void parse_block(symbol_t* symbol) {
    assert(current_char == ':');
    parse_next_char();
    //printf("%s() %s:%i\n", __func__, __FILE__, __LINE__);
    parse_identifier(false);
    block_t* block = block_new(identifier, location_new_current());
    vector_append(symbol->blocks, block);

    for (;;) {
        parse_whitespace_and_comments();
        if (current_char == '=' || current_char == '@' ||
                current_char == ':' || current_char == EOF)
        {
            break;
        }
        //printf("%s() %s:%i current_char %c\n", __func__, __FILE__, __LINE__, current_char);
        vector_append(block->instructions, parse_instruction());
    }

    if (vector_count(block->instructions) == 0) {
        fatal("Blocks must contain at least one instruction.");
    }

    // Make sure all but the last instruction do not end the block
    for (size_t i = vector_count(block->instructions) - 1; i-- != 0;) {
        instruction_t* instruction = vector_at(block->instructions, i);
        if (opcode_is_block_end(instruction->opcode)) {
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

/**
 * Parses a symbol that contains data.
 *
 * A data symbol cannot contain instructions or labels.
 *
 * The data is not stored; it is forwarded directly to the output.
 */
static void parse_data_symbol(symbol_t* symbol) {
    symbol->is_data = true;
    emit_symbol_name(symbol);

    for (;;) {
        parse_whitespace_and_comments();

        if (isdigit(current_char)) {
            emit_char(' ');
            do {
                emit_char(current_char);
                parse_next_char();
            } while (isdigit(current_char));
            emit_char('\n');
            continue;
        }

        if (current_char == '\'') {
            emit_char(' ');
            emit_char('\'');
            parse_next_char();
            char first_char = current_char;
            parse_next_char();
            if (!isxdigit(first_char) || !isxdigit(current_char)) {
                fatal("Expected two hex characters in this quoted byte.");
            }
            emit_char(first_char);
            emit_char(current_char);
            parse_next_char();
            emit_char('\n');
            continue;
        }

        if (current_char == '"') {
            emit_char(' ');
            emit_char('"');
            do {
                parse_next_char();
                if (current_char == EOF) {
                    fatal("Unterminated string");
                }
                emit_char(current_char);
            } while (current_char != '"');
            parse_next_char();
            emit_char('\n');
            continue;
        }

        // TODO need to parse debug directives

        if (current_char == '=' || current_char == '@' || current_char == EOF) {
            break;
        }

        if (current_char == ':') {
            fatal("Labels are not allowed in data symbols.");
        }
        fatal("Expected a number, a hex byte or a string in this data symbol.");
    }

    emit_char('\n');
}

symbol_t* /*nullable*/ try_parse_symbol(void) {
    //printf("%s() %s:%i\n", __func__, __FILE__, __LINE__);
    if (current_char == EOF) {
        return NULL;
    }
    parse_whitespace_and_comments();

    // parse the symbol name
    if (current_char != '=' && current_char != '@') {
        fatal("Expected a symbol declaration.");
    }
    bool is_static = current_char == '@';
    location_t* location = location_new_current();
    parse_next_char();
    parse_identifier(false);
    symbol_t* symbol = symbol_new(identifier, location, is_static);
    parse_whitespace_and_comments();

    if (isdigit(current_char) || current_char == '"' || current_char == '\'') {
        parse_data_symbol(symbol);
        return symbol;
    }

    // parse preamble (containing a temporary for each parameter, including
    // possibly a varargs parameter)
    //printf("%s() %s:%i\n", __func__, __FILE__, __LINE__);
    parse_parameters(symbol);
    //printf("%s() %s:%i\n", __func__, __FILE__, __LINE__);

    // parse instructions and labels
    for (;;) {
        if (current_char == '=' || current_char == '@' || current_char == EOF)
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
