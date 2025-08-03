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

#ifndef INSTRUCTION_H_INCLUDED
#define INSTRUCTION_H_INCLUDED

#include "opcode.h"

typedef void instruction_t;

#define INSTRUCTION_OPCODE 0
#define INSTRUCTION_ARG0 1
#define INSTRUCTION_ARG1 2
#define INSTRUCTION_ARG2 3
#define INSTRUCTION_LABEL 4
#define INSTRUCTION_LABEL_PREFIX 5
#define INSTRUCTION_SIZE 6

static instruction_t** instructions;
static size_t instructions_count;
static size_t instructions_capacity;

// TODO free() does nothing in libc/0. I have plans to fix this in libc/0;
// in the meantime we pool instructions. This can be removed once libc/0
// has some means of reclaiming memory.
#ifdef __onramp__
#define INSTRUCTION_POOL
#endif
static instruction_t* instruction_pool;

static char* instruction_label(instruction_t* instruction);

//static instruction_t* instruction_new(opcode_t opcode, int arg0, int arg1, int arg2) {
static instruction_t* instruction_new(void) {

    // pop an instruction off the free list
    instruction_t* instruction;
    if (!instruction_pool) {
        instruction = calloc(INSTRUCTION_SIZE, sizeof(size_t));
    }
    if (instruction_pool) {
        instruction = instruction_pool;
        instruction_pool = *(instruction_t**)instruction;
    }

    // initialize it
    /*
    *(opcode_t*)((size_t*)instruction + INSTRUCTION_OPCODE) = opcode;
    *(int*)((size_t*)instruction + INSTRUCTION_ARG0) = arg0;
    *(int*)((size_t*)instruction + INSTRUCTION_ARG1) = arg1;
    *(int*)((size_t*)instruction + INSTRUCTION_ARG2) = arg2;
    */
    *(char**)((size_t*)instruction + INSTRUCTION_LABEL) = NULL;
    return instruction;
}

static void instruction_delete(instruction_t* instruction) {

    // free the contents
    free(instruction_label(instruction));

    // free (or pool) the instruction
    #ifdef INSTRUCTION_POOL
    *(instruction_t**)instruction = instruction_pool;
    instruction_pool = instruction;
    #endif
    #ifndef INSTRUCTION_POOL
    // We don't use the free list when compiling natively so we can properly
    // test for leaks.
    free(instruction);
    #endif
}

static void instruction_set_opcode(instruction_t* instruction, opcode_t opcode) {
    *(opcode_t*)((size_t*)instruction + INSTRUCTION_OPCODE) = opcode;
}

static void instruction_set_arg0(instruction_t* instruction, int arg0) {
    *(int*)((size_t*)instruction + INSTRUCTION_ARG0) = arg0;
}

static void instruction_set_arg1(instruction_t* instruction, int arg1) {
    *(int*)((size_t*)instruction + INSTRUCTION_ARG1) = arg1;
}

static void instruction_set_arg2(instruction_t* instruction, int arg2) {
    *(int*)((size_t*)instruction + INSTRUCTION_ARG2) = arg2;
}

static void instruction_set_arg(instruction_t* instruction, int arg, int value) {
    *(int*)((size_t*)instruction + (INSTRUCTION_ARG0 + arg)) = value;
}

static void instruction_set_label(instruction_t* instruction, const char* label) {
    free(*(char**)((size_t*)instruction + INSTRUCTION_LABEL));
    *(char**)((size_t*)instruction + INSTRUCTION_LABEL) = strdup(label); // TODO check oom
}

static void instruction_set_label_prefix(instruction_t* instruction, char prefix) {
    *(char*)((size_t*)instruction + INSTRUCTION_LABEL_PREFIX) = prefix;
}

static opcode_t instruction_opcode(instruction_t* instruction) {
    return *(opcode_t*)((size_t*)instruction + INSTRUCTION_OPCODE);
}

static int instruction_arg0(instruction_t* instruction) {
    return *(int*)((size_t*)instruction + INSTRUCTION_ARG0);
}

static int instruction_arg1(instruction_t* instruction) {
    return *(int*)((size_t*)instruction + INSTRUCTION_ARG1);
}

static int instruction_arg2(instruction_t* instruction) {
    return *(int*)((size_t*)instruction + INSTRUCTION_ARG2);
}

static int instruction_arg(instruction_t* instruction, int arg) {
    return *(int*)((size_t*)instruction + (INSTRUCTION_ARG0 + arg));
}

static char* instruction_label(instruction_t* instruction) {
    return *(char**)((size_t*)instruction + INSTRUCTION_LABEL);
}

static char instruction_label_prefix(instruction_t* instruction) {
    return *(char*)((size_t*)instruction + INSTRUCTION_LABEL_PREFIX);
}

static void instruction_eliminate(instruction_t* instruction) {
    *(opcode_t*)((size_t*)instruction + INSTRUCTION_OPCODE) = OP_NOP;
}

static void print_register(int b, FILE* file) {
    fputc('r', file);

    if (b <= 0x89) {
        fputc('0' + (b - 0x80), file);
        return;
    }

    if (b <= 0x8B) {
        fputc('a' + (b - 0x8A), file);
        return;
    }

    if (b == 0x8C) {fputc('s', file);}
    if (b == 0x8D) {fputc('f', file);}
    if (b == 0x8E) {fputc('p', file);}
    if (b == 0x8F) {fputc('i', file);}
    fputc('p', file);
}

static void instruction_write(instruction_t* instruction, FILE* file) {
    opcode_t opcode = instruction_opcode(instruction);
    if (opcode == OP_NOP) {
        return;
    }

    if (opcode == OP_NUMBER) {
        // TODO should output in hex usually, unless 0<=number<=9
        fputd(instruction_arg(instruction, 0), file);
        fputc('\n', file);
        return;
    }
    if (opcode == OP_BYTE) {
        int arg = instruction_arg0(instruction);
        fputc('\'', file);
        fputc(int_to_hex(arg >> 4), file);
        fputc(int_to_hex(arg & 0xf), file);
        fputc('\n', file);
        return;
    }

    if (opcode > OP_VIRTUAL_MAX) {
        fputs(opcode_value_to_name(opcode), file);
        fputc(' ', file);
    }

    const char* label = instruction_label(instruction);
    bool args_printed = false;

    // These instructions can take 32-bit immediate numbers. We can't just
    // assume that all arguments are mix-type.
    if (opcode == OP_IMW) {
        print_register(instruction_arg(instruction, 0), file);
        fputc(' ', file);
        if (label == NULL) {
            // TODO should output in hex usually, unless 0<=number<=9
            fputd(instruction_arg(instruction, 1), file);
        }
        args_printed = true;
        // we don't return here in case it's a label.
    }

    // For all other opcodes, all non-label arguments are either registers or
    // mix-type. We print them all as mix-type.
    if (!args_printed) {
        int argcount = opcode_argcount(opcode);
        if (label != NULL) {
            argcount = (argcount - 1);
        }

        int i = 0;
        while (i < argcount) {
            int arg = instruction_arg(instruction, i);
            bool is_register = ((arg >= 0x80) & (arg <= 0x8F));
            if (is_register) {
                print_register(arg, file);
            }
            if (!is_register) {
                // TODO should output in hex usually, unless 0<=number<=9
                fputd(arg, file);
            }
            fputc(' ', file);
            i = (i + 1);
        }
    }

    // For all instructions that take labels, the label is always at the end.
    // (This also prints strings.)
    if (label != NULL) {
        if (label != NULL) {
            char prefix = instruction_label_prefix(instruction);
            fputc(prefix, file);
            fputs(label, file);
            if (prefix == '"') {
                fputc(prefix, file);
            }
        }
    }

    fputc('\n', file);
}

static void instruction_print(instruction_t* instruction) {
    if (instruction_opcode(instruction) == OP_NOP) {
        puts("; nop");
        return;
    }
    instruction_write(instruction, stdout);
}

static void instruction_emit(instruction_t* instruction) {
    fputc(' ', output_file);
    instruction_write(instruction, output_file);
}

/*
static void instruction_setup(void) {
}

static void instruction_teardown(void) {
}
*/

static void instructions_append(instruction_t* instruction) {

    // grow if needed
    if (instructions_count == instructions_capacity) {
        size_t new_capacity;
        if (instructions_capacity == 0) {
            new_capacity = 2;
        }
        if (instructions_capacity != 0) {
            new_capacity = (instructions_capacity * 2);
        }
        // We don't have realloc(). We have to grow manually.
        instruction_t** new_block = malloc(sizeof(instruction_t*) * new_capacity);
        if (new_block == NULL) {
            fatal("Out of memory.");
        }
        memcpy(new_block, instructions, sizeof(instruction_t*) * instructions_count);
        free(instructions);
        instructions = new_block;
        instructions_capacity = new_capacity;
    }

    // append the instruction
    *(instructions + instructions_count) = instruction;
    instructions_count = (instructions_count + 1);
}

#endif
