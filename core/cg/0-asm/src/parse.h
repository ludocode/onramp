/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Fraser Heavy Software
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

#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

/*
 * The parser.
 *
 * This includes most of the code to parse and emit functions. All instructions
 * in the function are parsed into a single `instructions` array. Label
 * declarations and constants are treated as their own instructions.
 */

#include "instruction.h"

static char* string_buffer;
#define string_buffer_capacity 256

static char function_prefix;
static char* function_name;

static void consume_whitespace_and_comments(void) {
    //printf("consume_whitespace_and_comments()\n");
    while (1) {
        if (current_char == EOF) {
            break;
        }

        // skip whitespace
        if (isspace(current_char)) {
            read_char();
            continue;
        }

        // skip comments
        // TODO we could forward debug info but we'd have to store it in the
        // instruction list. For now cci/0 doesn't produce any debug info and
        // we don't really care about cci/1.
        if ((current_char == '#') | (current_char == ';')) {
            while (1) {
                read_char();
                if ((current_char == '\n') | (current_char == EOF)) {
                    break;
                }
            }
            continue;
        }

        // done
        break;
    }
    //printf("consume_whitespace_and_comments() done\n");
}

// Tries to parse an identifier into string_buffer.
static bool try_parse_identifier(void) {
    if (!is_identifier_char(current_char, true)) {
        return false;
    }

    size_t len = 0;
    while (1) {
        *(string_buffer + len) = current_char;
        len = (len + 1);

        if (len == string_buffer_capacity) {
            fatal("Label or symbol name is too long.");
        }

        read_char();
        if (!is_identifier_char(current_char, false)) {
            break;
        }
    }
    *(string_buffer + len) = 0;
    return true;
}

// Parses a string into string_buffer.
static void parse_string(void) {
    if (current_char != '"') {
        fatal("Expected a string.");
    }
    read_char();

    size_t len = 0;
    while (1) {
        if (len == string_buffer_capacity) {
            fatal("String is too long.");
        }
        if (current_char == EOF) {
            fatal("Expected '\"' at the end of this string");
        }
        if (current_char == '"') {
            read_char();
            break;
        }
        *(string_buffer + len) = current_char;
        len = (len + 1);
        read_char();
    }
    *(string_buffer + len) = 0;
}

static void parse_declaration(void) {

    // make sure it's a declaration
    if (current_char != '=') {
        if (current_char != '@') {
            fatal("Expected a symbol declaration ('=').");
        }
    }
    function_prefix = current_char;
    read_char();
    //printf("new function\n");

    if (!try_parse_identifier()) {
        fatal("Expected an identifier after '='");
    }
}

static bool is_linker_directive(char c) {
    if (current_char == '^') {return true;}
    if (current_char == '&') {return true;}
    if (current_char == '<') {return true;}
    if (current_char == '>') {return true;}
    if (current_char == ':') {return true;}
    if (current_char == '=') {return true;}
    return false;
}

static void instruction_collect_label(instruction_t* instruction) {
    assert(is_linker_directive(current_char));
    assert(instruction_label(instruction) == NULL);
    instruction_set_label_prefix(instruction, current_char);
    read_char();
    if (!try_parse_identifier()) {
        fatal("Expected an identifier as part of this linker directive");
    }
    instruction_set_label(instruction, string_buffer);
}

// TODO put this in libo
static int hex_to_int(char c) {
    if ((c >= '0') & (c <= '9')) {
        return c - '0';
    }
    if ((c >= 'a') & (c <= 'f')) {
        return (c - 'a') + 10;
    }
    if ((c >= 'A') & (c <= 'F')) {
        return (c - 'A') + 10;
    }
    fatal("Expected hexadecimal character");
}

// TODO the below two functions are adapted from as/2. we need unsigned in cci/0
int parse_decimal(void) {
    int value = 0;

    while (isdigit(current_char)) {
        int new_value = ((value * 10) + (current_char - '0'));
        if (new_value < value) {
            fatal("Decimal number is out of range.");
        }
        value = new_value;
        read_char();
    }

    return value;
}

int parse_hexadecimal(void) {
    int ret = 0;
    int digits = 0;

    while (isxdigit(current_char)) {
        digits = (digits + 1);
        if (digits > 8) {
            fatal("Hexadecimal number must be at most 8 hexadecimal digits.");
        }
        ret = ((ret << 4) | hex_to_int(current_char));
        read_char();
    }

    if (digits == 0) {
        fatal("Number starting with \"0x\" must be followed by at least one hexadecimal digit.");
    }

    return ret;
}

// TODO this is also adapted from as/2
int parse_number(void) {
    bool negative = false;

    if (current_char == '-') {
        negative = true;
        read_char();
    }

    // if first digit is non-zero, it's a decimal number
    bool is_zero = (current_char == '0');
    if (!is_zero) {
        int value = parse_decimal();
        if (negative) {
            value = -value;
        }
        return value;
    }

    // otherwise it's either a plain zero or it's hexadecimal
    read_char();
    if ((current_char != 'x') & (current_char != 'X')) {
        if (isxdigit(current_char)) {
            fatal("Invalid number");
        }
        // it's zero
        return 0;
    }

    // it's hexadecimal
    read_char();
    int value = parse_hexadecimal();
    if (negative) {
        value = -value;
    }
    return value;
}

// Parses a register out of the identifier that has already been read into the
// identifier buffer. Returns the register's byte value (0x80-0x8F).
// TODO this could be faster if we adapt register_name_to_number() from as/2,
// except it's written for cci/2, not cci/0. need something in libo
int parse_register(void) {
    char* p = string_buffer;
    if (*p == 'r') {
        p = (p + 1);
        bool numbered = isdigit(*p);
        if (numbered & (*(p + 1) == 0)) {
            if (*p == '0') {return 0x80;}
            if (*p == '1') {return 0x81;}
            if (*p == '2') {return 0x82;}
            if (*p == '3') {return 0x83;}
            if (*p == '4') {return 0x84;}
            if (*p == '5') {return 0x85;}
            if (*p == '6') {return 0x86;}
            if (*p == '7') {return 0x87;}
            if (*p == '8') {return 0x88;}
            if (*p == '9') {return 0x89;}
        }
        if (!numbered) {
            if (0 == strcmp(p, "a")) {return 0x8A;}
            if (0 == strcmp(p, "b")) {return 0x8B;}
            if (0 == strcmp(p, "sp")) {return 0x8C;}
            if (0 == strcmp(p, "fp")) {return 0x8D;}
            if (0 == strcmp(p, "pp")) {return 0x8E;}
            if (0 == strcmp(p, "ip")) {return 0x8F;}
        }
    }
    fatal("Alphanumeric argument is not a register.");
}

static instruction_t* parse_instruction(int index) {
    instruction_t* instruction = instruction_new(index);

    // Check for "virtual" instructions first (basically anything that isn't
    // really an instruction)

    // Parse a linker directive outside of an instruction. This is still stored
    // as an instruction but with a "virtual opcode".
    if (is_linker_directive(current_char)) {
        if (current_char == ':') {
            instruction_set_opcode(instruction, OP_DECLARATION);
        }
        if (current_char != ':') {
            instruction_set_opcode(instruction, OP_INVOCATION);
        }
        instruction_collect_label(instruction);
        return instruction;
    }

    // Parse a string
    if (current_char == '"') {
        parse_string();
        instruction_set_opcode(instruction, OP_STRING);
        instruction_set_label(instruction, string_buffer);
        instruction_set_label_prefix(instruction, '"');
        return instruction;
    }

    // Parse a number
    if (isdigit(current_char) | (current_char == '-')) {
        instruction_set_opcode(instruction, OP_NUMBER);
        instruction_set_arg0(instruction, parse_number());
        return instruction;
    }

    // Parse a quoted byte
    if (current_char == '\'') {
        read_char();
        instruction_set_opcode(instruction, OP_BYTE);
        instruction_set_arg0(instruction, parse_hexadecimal());
        return instruction;
    }

    // Parse a real instruction

    // Collect the opcode
    if (!try_parse_identifier()) {
        fatal("Expected a label, an opcode or a value");
    }
    opcode_t opcode = opcode_name_to_value(string_buffer);
    instruction_set_opcode(instruction, opcode);

    // Collect the args
    size_t argcount = opcode_argcount(opcode);
    size_t arg = 0;
    while (arg < argcount) {
        consume_whitespace_and_comments();

        // Parse string argument
        if (current_char == '"') {
            parse_string();
            if ((*string_buffer == 0) | (*(string_buffer + 1) != 0)) {
                fatal("Instruction argument string must be a single character.");
            }
            instruction_set_arg(instruction, arg, *string_buffer);
            arg = (arg + 1);
            continue;
        }

        // Parse invocation argument
        if (is_linker_directive(current_char)) {
            if (arg != (argcount - 1)) {
                fatal("A label should be the last argument of this instruction.");
            }
            instruction_collect_label(instruction);
            break;
        }

        // Parse number argument
        if (isdigit(current_char) | (current_char == '-')) {
            int number = parse_number();
            if (opcode != OP_IMW) {
                // TODO it would be nice to check that numbers are in the range
                // of a mix-type byte. For now we just truncate. The input is
                // generated by cci/0 so we don't need much error checking.
                number = (number & 0xFF);
            }
            instruction_set_arg(instruction, arg, number);
            arg = (arg + 1);
            continue;
        }

        // Parse quoted byte argument
        if (current_char == '\'') {
            read_char();
            // TODO it would be nice to check that we only have two hex chars.
            // For now we just truncate. The input is generated by cci/0 so we
            // don't need much error checking.
            instruction_set_arg(instruction, arg, parse_hexadecimal() & 0xFF);
            return instruction;
        }

        // Parse register argument
        if (try_parse_identifier()) {
            instruction_set_arg(instruction, arg, parse_register());
            arg = (arg + 1);
            continue;
        }

        fatal("Expected an argument for this instruction.");
    }

    return instruction;
}

static void parse_clear(void) {
    size_t i = 0;
    while (i != instructions_count) {
        //printf("delete instruction %zi of %zi\n", i, instructions_count);
        instruction_delete(*(instructions + i));
        i = (i + 1);
    }
    instructions_count = 0;

    free(function_name);
    function_name = NULL;
}

static bool parse_function(void) {
    //printf("parse()\n");
    consume_whitespace_and_comments();

    if (current_char == EOF) {
        //printf("eof\n");
        return false;
    }

    // clear the block table
    size_t i = 0;
    while (i < BLOCKS_BUCKETS) {
        *(blocks + i) = NULL;
        i = (i + 1);
    }

    parse_declaration();
    //printf("parsing function: %s\n", string_buffer);
    function_name = strdup(string_buffer);

    size_t index = 0;
    while (1) {
        consume_whitespace_and_comments();

        // if we reach the next function or end of file, we're done
        if (current_char == '=') {
            break;
        }
        if (current_char == '@') {
            break;
        }
        if (current_char == EOF) {
            break;
        }

        // parse an instruction
        instruction_t* instruction = parse_instruction(index);
        instructions_append(instruction);
        index = (index + 1);
        //printf("  parsed instruction: "); instruction_print(*(instructions + (instructions_count - 1)));

        // if this instruction is a label declaration, add it to the block table
        if (instruction_opcode(instruction) == OP_DECLARATION) {
            size_t bucket = (fnv1a_cstr(instruction_label(instruction)) & (BLOCKS_BUCKETS - 1));
            //printf("inserting declaration %s into bucket %zi\n", instruction_label(instruction), bucket);
            instruction_set_opt_vp(instruction, *(blocks + bucket));
            *(blocks + bucket) = instruction;
        }
    }

    //printf("returning true\n");
    return true;
}

static void parse_setup(void) {
    string_buffer = malloc(string_buffer_capacity);
}

static void parse_teardown(void) {
    free(instructions);
    free(string_buffer);
}

#endif
