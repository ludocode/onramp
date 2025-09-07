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

#include "instruction.h"

#include <assert.h>
#include <stdlib.h>

#include "libo-error.h"
#include "emit.h"
#include "common.h"
#include "token.h"
#include "options.h"

static const char* opcode_to_string(opcode_t opcode) {
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
        case INC: return "inc";
        case DEC: return "dec";
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
        case IMW: return "imw";
        case LTU: return "ltu";
        case LTS: return "lts";
        case JZ: return "jz";
        case JNZ: return "jnz";
        case JMP: return "jmp";
        case CALL: return "call";
        case RET: return "ret";
        case ENTER: return "enter";
        case LEAVE: return "leave";
        case SYS: return "sys";
    }

    fatal("Internal error: no such opcode: %i", (int)opcode);
}

#ifdef CCI2_IR
void instruction_init(instruction_t* instruction, token_t* token, opcode_t opcode, size_t arg_count) {
    instruction->token = token ? token_ref(token) : NULL;
    instruction->opcode = opcode;
    instruction->argument_count = arg_count;
    if (arg_count == 0) {
        instruction->arguments = NULL;
    } else {
        instruction->arguments = malloc(arg_count * sizeof(argument_t));
        if (!instruction->arguments) {
            fatal("Out of memory.");
        }
    }
}
#endif // CCI2_IR

#ifndef CCI2_IR
void instruction_init(instruction_t* instruction) {
    memset(instruction, 0, sizeof(*instruction));
}
#endif // !CCI2_IR

void instruction_destroy(instruction_t* instruction) {
    if (instruction->token) {
        token_deref(instruction->token);
    }

    #ifdef CCI2_IR
    free(instruction->arguments);
    #endif
}

#ifdef CCI2_IR
void instruction_set_arg_number(instruction_t* instruction, size_t arg, uint32_t number) {
    argument_t* argument = instruction_argument(instruction, arg);
    argument->type = argument_type_number;
    argument->number = number;
}

void instruction_set_arg_sentinel(instruction_t* instruction, size_t arg) {
    argument_t* argument = instruction_argument(instruction, arg);
    argument->type = argument_type_sentinel;
}

void instruction_set_arg_temporary(instruction_t* instruction, size_t arg, string_t* temporary) {
    argument_t* argument = instruction_argument(instruction, arg);
    argument->type = argument_type_temporary;
    argument->string = temporary;
}

void instruction_set_arg_absolute(instruction_t* instruction, size_t arg, string_t* label) {
    argument_t* argument = instruction_argument(instruction, arg);
    argument->type = argument_type_absolute;
    argument->string = label;
}

void instruction_set_arg_relative(instruction_t* instruction, size_t arg, uint32_t label) {
    argument_t* argument = instruction_argument(instruction, arg);
    argument->type = argument_type_relative;
    argument->number = label;
}
#endif // CCI2_IR

#ifndef CCI2_IR
void instruction_vset(instruction_t* instruction, token_t* token,
        opcode_t opcode, va_list args)
{
    #ifdef GENERATE_DEBUG
    extern int debug_depth;
    for (int i = 0; i < debug_depth; ++i)
        fputs("  ", stdout);
    printf("%s\n", opcode_to_string(opcode));
    #endif

    if (token)
        token_ref(token);
    if (instruction->token)
        token_deref(instruction->token);
    instruction->token = token;

    instruction->opcode = opcode;
    switch (opcode) {
        case NOP:
            break;
        case VALUE:
            fatal("TODO vset VALUE opcode");
            break;

        // three register or mix arguments
        case ADD:
        case SUB:
        case MUL:
        case DIVU:
        case DIVS:
        case MODU:
        case MODS:
        case AND:
        case OR:
        case XOR:
        case NOT:
        case SHL:
        case SHRU:
        case SHRS:
        case ROL:
        case ROR:
        case LDW:
        case LDS:
        case LDB:
        case STW:
        case STS:
        case STB:
        case LTU:
        case LTS:
        case SYS:
            // TODO might be a bug in cci1 here, check it
            instruction->arg1 = (int8_t)va_arg(args, int);
            instruction->arg2 = (int8_t)va_arg(args, int);
            instruction->arg3 = (int8_t)va_arg(args, int);
            break;

        // two register or mix arguments
        case SXS:
        case SXB:
        case TRS:
        case TRB:
        case MOV:
        case BOOL:
        case ISZ:
            instruction->arg1 = (int8_t)va_arg(args, int);
            instruction->arg2 = (int8_t)va_arg(args, int);
            break;

        // one register or mix argument
        case ZERO:
        case INC:
        case DEC:
        case PUSH:
        case POP:
            instruction->arg1 = (int8_t)va_arg(args, int);
            break;

        // no args
        case POPD:
        case RET:
        case ENTER:
        case LEAVE:
            break;

        // register and either number or invocation
        case IMW: {
            instruction->argtypes = va_arg(args, instruction_argtypes_t);
            instruction->arg1 = (int8_t)va_arg(args, int);
            if (instruction->argtypes == ARGTYPE_NUMBER) {
                instruction->number = va_arg(args, int);
            } else if (instruction->argtypes == ARGTYPE_NAME) {
                instruction->invocation_type = (char)va_arg(args, int);
                instruction->invocation_label = va_arg(args, const char*);
            } else if (instruction->argtypes == ARGTYPE_GENERATED) {
                instruction->invocation_type = (char)va_arg(args, int);
                instruction->invocation_prefix = va_arg(args, const char*);
                instruction->invocation_number = va_arg(args, int);
            } else {
                fatal("Internal error: Invalid ARGTYPE for IMW instruction");
            }
            break;
        }

        // register/mix and invocation
        case JZ:
        case JNZ:
            instruction->arg1 = (int8_t)va_arg(args, int);
            instruction->invocation_type = (char)va_arg(args, int);
            instruction->invocation_prefix = va_arg(args, const char*);
            instruction->invocation_number = va_arg(args, int);
            break;

        // invocation only
        case JMP:
            instruction->invocation_type = (char)va_arg(args, int);
            instruction->invocation_prefix = va_arg(args, const char*);
            instruction->invocation_number = va_arg(args, int);
            break;
        case CALL:
            instruction->argtypes = va_arg(args, instruction_argtypes_t);
            if (instruction->argtypes == ARGTYPE_REGISTER) {
                instruction->arg1 = (int8_t)va_arg(args, int);
            } else if (instruction->argtypes == ARGTYPE_NAME) {
                instruction->invocation_type = (char)va_arg(args, int);
                instruction->invocation_label = va_arg(args, const char*);
            } else {
                fatal("Internal error: Invalid ARGTYPE for CALL instruction");
            }
            break;

        default:
            fatal("Internal error: no such opcode: %i", (int)opcode);
    }
}

void instruction_set(instruction_t* instruction, token_t* token, opcode_t opcode, ...) {
    va_list args;
    va_start(args, opcode);
    instruction_vset(instruction, token, opcode, args);
    va_end(args);
}
#endif // !CCI2_IR

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

    #ifdef CCI2_IR
    for (size_t i = 0; i < instruction->argument_count; ++i) {
        emit_char(' ');
        argument_t* argument = instruction_argument(instruction, i);
        switch (argument->type) {
            case argument_type_sentinel:
                emit_char('$');
                break;
            case argument_type_temporary:
                emit_string(argument->string);
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
                emit_string(argument->string);
                break;
        }
    }

    if (instruction->opcode == CALL) {
        emit_cstr(" end");
    }
    #endif

    #ifndef CCI2_IR
    switch (instruction->opcode) {
        case NOP:
        case VALUE:
            fatal("Internal error, emit opcode unreachable");
            break;

        // three register or mix arguments
        case ADD:
        case SUB:
        case MUL:
        case DIVU:
        case DIVS:
        case MODU:
        case MODS:
        case AND:
        case OR:
        case XOR:
        case SHL:
        case SHRU:
        case SHRS:
        case ROL:
        case ROR:
        case LDW:
        case LDS:
        case LDB:
        case STW:
        case STS:
        case STB:
        case LTU:
        case LTS:
        case SYS:
            emit_arg_mix(instruction->arg1);
            emit_arg_mix(instruction->arg2);
            emit_arg_mix(instruction->arg3);
            break;

        // two register or mix arguments
        case SXS:
        case SXB:
        case TRS:
        case TRB:
        case NOT:
        case MOV:
        case BOOL:
        case ISZ:
            emit_arg_mix(instruction->arg1);
            emit_arg_mix(instruction->arg2);
            break;

        // one register or mix argument
        case ZERO:
        case INC:
        case DEC:
        case PUSH:
        case POP:
            emit_arg_mix(instruction->arg1);
            break;

        // no args
        case POPD:
        case RET:
        case ENTER:
        case LEAVE:
            break;

        // register and number
        case IMW:
            emit_arg_mix(instruction->arg1);
            if (instruction->argtypes == ARGTYPE_NUMBER) {
                emit_arg_number(instruction->number);
            } else if (instruction->argtypes == ARGTYPE_NAME) {
                emit_arg_invocation(
                        instruction->invocation_type,
                        instruction->invocation_label);
            } else if (instruction->argtypes == ARGTYPE_GENERATED) {
                emit_arg_invocation_prefix(
                        instruction->invocation_type,
                        instruction->invocation_prefix,
                        instruction->invocation_number);
            } else {
                fatal("Internal error: Invalid ARGTYPE for IMW instruction");
            }
            break;

        // register/mix and invocation
        case JZ:
        case JNZ:
            emit_arg_mix(instruction->arg1);
            emit_arg_invocation_prefix(
                    instruction->invocation_type,
                    instruction->invocation_prefix,
                    instruction->invocation_number);
            break;

        // invocation only
        case JMP:
            emit_arg_invocation_prefix(
                    instruction->invocation_type,
                    instruction->invocation_prefix,
                    instruction->invocation_number);
            break;
        case CALL:
            if (instruction->argtypes == ARGTYPE_REGISTER) {
                emit_arg_mix(instruction->arg1);
            } else if (instruction->argtypes == ARGTYPE_NAME) {
                emit_arg_invocation(
                        instruction->invocation_type,
                        instruction->invocation_label);
            } else {
                fatal("Internal error: Invalid ARGTYPE for CALL instruction");
            }
            break;

        default:
            fatal("Internal error: no such opcode: %i", (int)instruction->opcode);
    }
    #endif

    emit_newline();
}
