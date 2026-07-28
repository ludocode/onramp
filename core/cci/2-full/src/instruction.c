/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2026 Fraser Heavy Software
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

#include "instruction.h"

#include <assert.h>
#include <stdlib.h>

#include "common.h"
#include "emit.h"
#include "function.h"
#include "generate.h"
#include "libo-error.h"
#include "options.h"
#include "token.h"

const char* opcode_to_string(opcode_t opcode) {
    switch (opcode) {
        case NOP: return "nop";
        case VALUE: return "<value>"; // not a real opcode but we want a name for debugging

        // arithmetic
        case ADD: return "add";
        case SUB: return "sub";
        case MUL: return "mul";
        case DIVU: return "divu";
        case DIVS: return "divs";
        case MODU: return "modu";
        case MODS: return "mods";
        case ZERO: return "zero";
        case INC: return "inc"; // TODO remove
        case DEC: return "dec"; // TODO remove
        case SXS: return "sxs";
        case SXB: return "sxb";
        case TRS: return "trs";
        case TRB: return "trb";

        // logic
        case AND: return "and";
        case OR: return "or";
        case XOR: return "xor";
        case NOT: return "not";
        case SHL: return "shl";
        case SHRU: return "shru";
        case SHRS: return "shrs";
        case ROL: return "rol";
        case ROR: return "ror";
        case MOV: return "mov";
        case BOOL: return "bool";
        case ISZ: return "isz";

        // memory
        case SYM: return "sym";
        case VAR: return "var";
        case ALLOC: return "alloc";
        case FREE: return "free";
        case LDW: return "ldw";
        case LDS: return "lds";
        case LDB: return "ldb";
        case STW: return "stw";
        case STS: return "sts";
        case STB: return "stb";
        case PUSH: return "push";
        case POP: return "pop";
        case POPD: return "popd";

        // control
        case LTU: return "ltu";
        case LTS: return "lts";
        case BR: return "br";
        case JMP: return "jmp";
        case CALL: return "call";
        case RET: return "ret";
        case ENTER: return "enter"; // TODO remove
        case LEAVE: return "leave"; // TODO remove
        case SYS: return "sys";
    }

    fatal("Internal error: no such opcode: %i", (int)opcode);
}

void instruction_init(instruction_t* instruction, token_t* token, opcode_t opcode, size_t arg_count) {
    instruction->token = token ? token_ref(token) : NULL;
    instruction->opcode = opcode;
    instruction->argument_count = arg_count;
    if (arg_count == 0) {
        instruction->arguments = NULL;
    } else {
        instruction->arguments = calloc(arg_count, sizeof(argument_t));
        if (!instruction->arguments) {
            fatal("Out of memory.");
        }
    }
}

void instruction_destroy(instruction_t* instruction) {
    if (instruction->token) {
        token_deref(instruction->token);
    }

    free(instruction->arguments);
}

void instruction_set_arg_number(instruction_t* instruction, size_t arg, uint32_t number) {
    argument_t* argument = instruction_argument(instruction, arg);
    argument->type = argument_type_number;
    argument->number = number;
}

void instruction_set_arg_sentinel(instruction_t* instruction, size_t arg) {
    argument_t* argument = instruction_argument(instruction, arg);
    argument->type = argument_type_sentinel;
}

void instruction_set_arg_temporary(instruction_t* instruction, size_t arg, int temporary) {
    assert(temporary != TEMPORARY_INVALID);
    argument_t* argument = instruction_argument(instruction, arg);
    argument->type = argument_type_temporary;
    argument->number = temporary;
}

void instruction_set_arg_absolute(instruction_t* instruction, size_t arg, string_t* label) {
    argument_t* argument = instruction_argument(instruction, arg);
    argument->type = argument_type_absolute;
    argument->string = label;
}

void instruction_set_arg_absolute_cstr(instruction_t* instruction, size_t arg, const char* label) {
    string_t* string = string_intern_cstr(label);
    instruction_set_arg_absolute(instruction, arg, string);
    function_take_string(current_function, string);
}

void instruction_set_arg_relative(instruction_t* instruction, size_t arg, uint32_t label) {
    argument_t* argument = instruction_argument(instruction, arg);
    argument->type = argument_type_relative;
    argument->number = label;
}

void instruction_set_arg_varargs(instruction_t* instruction, size_t arg) {
    argument_t* argument = instruction_argument(instruction, arg);
    argument->type = argument_type_varargs;
}

void instruction_set_args_tt(instruction_t* instruction, int a, int b) {
    instruction_set_arg_temporary(instruction, 0, a);
    instruction_set_arg_temporary(instruction, 1, b);
}

void instruction_set_args_ttt(instruction_t* instruction, int a, int b, int c) {
    instruction_set_arg_temporary(instruction, 0, a);
    instruction_set_arg_temporary(instruction, 1, b);
    instruction_set_arg_temporary(instruction, 2, c);
}

void instruction_set_args_ttn(instruction_t* instruction, int temp_a, int temp_b, uint32_t number) {
    instruction_set_arg_temporary(instruction, 0, temp_a);
    instruction_set_arg_temporary(instruction, 1, temp_b);
    instruction_set_arg_number(instruction, 2, number);
}

void instruction_emit(instruction_t* instruction) {
    if (instruction->opcode == NOP)
        return;
    if (instruction->token)
        emit_source_location(instruction->token);
    emit_cstr(ASM_INDENT);

    if (instruction->opcode == VALUE) {
        // need to decide what types can value have, probably need number and absolute invocation
        fatal("TODO emit VALUE instruction");
        //emit_number(instruction->number);
        emit_newline();
        return;
    }

    emit_cstr(opcode_to_string(instruction->opcode));

    for (size_t i = 0; i < instruction->argument_count; ++i) {
        emit_char(' ');
        argument_t* argument = instruction_argument(instruction, i);
        switch (argument->type) {
            case argument_type_sentinel:
                emit_char('%');
                break;
            case argument_type_temporary:
                emit_string(temporary_name(argument->number));
                break;
            case argument_type_number:
                // Small numbers are printed in decimal for readability. Large
                // numbers are printed in hexadecimal for efficiency.
                if (argument->number <= 9 || argument->number >= (uint32_t)(-9)) {
                    emit_number(argument->number);
                } else {
                    emit_cstr("0x");
                    emit_hex_number(argument->number);
                }
                break;
            case argument_type_absolute:
                emit_char('^');
                emit_string(argument->string);
                break;
            case argument_type_relative:
                emit_char('&');
                emit_cstr(JUMP_LABEL_PREFIX);
                emit_hex_number(argument->number);
                break;
            case argument_type_varargs:
                emit_cstr("varargs");
                break;
            default:
                fatal("Internal error: invalid argument type");
                break;
        }
    }

    if (instruction->opcode == CALL) {
        emit_cstr(" end");
    }

    emit_newline();
}
