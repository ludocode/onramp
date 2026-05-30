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

struct argument_t;

/**
 * An instruction.
 *
 * Instructions contain an opcode and a growable array of arguments.
 * Instructions are owned by a containing block.
 */
typedef struct instruction_t {
    location_t* location;
    opcode_t opcode;
    vector_t arguments;
} instruction_t;

/**
 *
 * Takes ownership of location.
 */
instruction_t* instruction_new(location_t* location, opcode_t opcode);

void instruction_delete(instruction_t* instruction);

void instruction_append(instruction_t* instruction, struct argument_t* argument);

#endif
