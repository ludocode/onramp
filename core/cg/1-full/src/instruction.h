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

#ifndef INSTRUCTION_H_INCLUDED
#define INSTRUCTION_H_INCLUDED

#include "location.h"
#include "opcode.h"
#include "libo-vector.h"
#include "argument.h"

/*
 * Instructions and Arguments
 *
 * The instruction and argument types here are similar to cci/2 except there
 * is some additional stuff useful for codegen. For example there is an
 * additional argument type "register" for when temporaries are converted to
 * registers.
 *
 * Instructions are also allocated individually in order to make it more
 * efficient to move them around in vectors. They are generally owned by the
 * block that contains them.
 *
 * (There are a lot of things here that use pointers unnecessarily, for example
 * location is a pointer when it should just be inline. There's a good reason
 * for this: I'm planning to upgrade cci/0 to be able to compile this in the
 * future and it will have struct pointer but not struct value support. It
 * will make sense later.)
 */

/**
 * An instruction.
 *
 * Instructions contain an opcode and a growable array of arguments.
 * Instructions are owned by a containing block.
 */
typedef struct instruction_t {
    location_t* location;
    opcode_t opcode;
    vector_t* arguments;
    size_t index; // index in global order for live interval analysis
} instruction_t;

/**
 *
 * Takes ownership of location.
 */
instruction_t* instruction_new(location_t* location, opcode_t opcode);

void instruction_delete(instruction_t* instruction);

static inline argument_t* instruction_append(instruction_t* instruction, argument_t* argument) {
    vector_append(instruction->arguments, argument);
    return argument;
}

static inline argument_t* instruction_argument(instruction_t* instruction, size_t argument) {
    return vector_at(instruction->arguments, argument);
}

/**
 * True if the first argument of the given instruction is an output.
 */
bool instruction_has_output_arg(instruction_t* instruction);

/**
 * Returns the mode of the first argument, i.e. whether the instructions reads,
 * writes, or both its first argument,
 *
 * If the instruction has no argument, this returns argument_mode_read.
 */
argument_mode_t instruction_mode(instruction_t* instruction);

#endif
