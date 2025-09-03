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
#define INSTRUCTION_OPT_VP 6   // void* for optimization passes
#define INSTRUCTION_OPT_INT 7  // int for optimization passes
#define INSTRUCTION_INDEX 8
//#define INSTRUCTION_IMMEDIATE 9 // value of imw instruction (not an arg) TODO using arg1 for now
#define INSTRUCTION_SIZE 9

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
static instruction_t* instruction_new(int index) {

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
    *(size_t*)((size_t*)instruction + INSTRUCTION_INDEX) = index;
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

static void instruction_set_opt_vp(instruction_t* instruction, void* vp) {
    *(void**)((size_t*)instruction + INSTRUCTION_OPT_VP) = vp;
}

static void instruction_set_opt_int(instruction_t* instruction, int opt_int) {
    *(int*)((size_t*)instruction + INSTRUCTION_OPT_INT) = opt_int;
}

/*
static void instruction_set_immediate(instruction_t* instruction, int immediate) {
    *(int*)((size_t*)instruction + INSTRUCTION_IMMEDIATE) = immediate;
}
*/

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

static void* instruction_opt_vp(instruction_t* instruction) {
    return *(void**)((size_t*)instruction + INSTRUCTION_OPT_VP);
}

static int instruction_opt_int(instruction_t* instruction) {
    return *(int*)((size_t*)instruction + INSTRUCTION_OPT_INT);
}

static size_t instruction_index(instruction_t* instruction) {
    return *(size_t*)((size_t*)instruction + INSTRUCTION_INDEX);
}

// TODO for now immediate is stored in arg1
#define instruction_immediate instruction_arg1
/*
static int instruction_immediate(instruction_t* instruction) {
    return *(int*)((size_t*)instruction + INSTRUCTION_IMMEDIATE);
}
*/

/**
 * Replaces this instruction with one that sets its arg0 to a constant value.
 *
 * If the value fits in a mix-type byte, mov will be used; otherwise imw will
 * be used.
 *
 * Note that we don't change the instruction's arg0. This can only be used to
 * change instructions that write to their arg0.
 */
/*
static void instruction_set_constant(instruction_t* instruction, int value) {


}
*/

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
        // TODO for now we print the nops into the output. It makes it much
        // easier to debug since the optimized output lines up exactly with the
        // unoptimized input.
        fputs("; nop\n", file);
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
        size_t argcount = opcode_argcount(opcode);
        if ((argcount != 0) & (label != NULL)) {
            argcount = (argcount - 1);
        }

        size_t i = 0;
        while (i != argcount) {
            int arg = instruction_arg(instruction, i);
            bool is_reg = is_register(arg);
            if (is_reg) {
                print_register(arg, file);
            }
            if (!is_reg) {
                // TODO should output in hex usually, unless 0<=number<=9
                fputd(mix_to_int(arg), file);
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
    /*
    if (instruction_opcode(instruction) == OP_NOP) {
        puts("; nop");
        return;
    }
    */
    instruction_write(instruction, stdout);
}

static void instructions_print(void) {
    size_t i = 0;
    while (i < instructions_count) {
        instruction_print(*(instructions + i));
        i = (i + 1);
    }
}

static void instruction_emit(instruction_t* instruction) {
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

static bool instruction_uses_register_impl(instruction_t* instruction, int reg, bool input, bool output) {
    opcode_t opcode = instruction_opcode(instruction);
    style_t style = opcode_style(opcode);

    if (style == STYLE_NONE) {
        return false;
    }

    // The in-place style uses its single argument as both input and output.
    if (style == STYLE_IN_PLACE) {
        if (instruction_arg0(instruction) == reg) {
            return true;
        }
        return false;
    }

    // The reg and reg-con styles have only a register output and no register
    // inputs.
    if ((style == STYLE_REG) | (style == STYLE_REG_CON)) {
        if (output) {
            if (instruction_arg0(instruction) == reg) {
                return true;
            }
        }
        return false;
    }

    // get number of non-label arguments
    size_t argcount = opcode_argcount(opcode);
    if (instruction_label(instruction)) {
        // We checked for STYLE_NONE above so argcount can't be zero.
        argcount = (argcount - 1);
    }
    // TODO we should also check here if this instruction is imw, and if so,
    // also decrement the argcount because the arg is an immediate value, not a
    // mix-type byte. Then we could get rid of the STYLE_REG and STYLE_REG_CON
    // styles and collapse them both into STYLE_REG_MIX.
    if (argcount == 0) {
        return false;
    }

    // The first argument of the remaining styles is either input or output.
    // Only the mix style uses it as input; all other remaining styles use it
    // as output.
    int arg0 = instruction_arg0(instruction);
    if (arg0 == reg) {
        if (style == STYLE_MIX) {
            if (input) {
                return true;
            }
        }
        if (style != STYLE_MIX) {
            if (output) {
                return true;
            }
        }
    }

    // The rest of the arguments are inputs for all remaining styles.
    if (input) {
        size_t i = 1;
        while (i != argcount) {
            int arg = instruction_arg(instruction, i);
            if (arg == reg) {
                return true;
            }
            i = (i + 1);
        }
    }
    return false;
}

static bool instruction_uses_register(instruction_t* instruction, int reg) {
    return instruction_uses_register_impl(instruction, reg, true, true);
}

static bool instruction_reads_register(instruction_t* instruction, int reg) {
    return instruction_uses_register_impl(instruction, reg, true, false);
}

static bool instruction_writes_register(instruction_t* instruction, int reg) {
    return instruction_uses_register_impl(instruction, reg, false, true);
}

#endif
