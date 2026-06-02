/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Fraser Heavy Software
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

#include "opcode.h"

#include <stdlib.h>
#include <string.h>

#include "libo-util.h"
#include "libo-error.h"

bool opcode_is_block_end(opcode_t opcode) {
    switch (opcode) {
        case opcode_ret:
        case opcode_jmp:
        case opcode_br:
            return true;
        default:
            break;
    }
    return false;
}

const char* opcode_to_string(opcode_t opcode) {

    // Some of these exist only for debugging purposes. For example "br" cannot
    // appear in assembly, but we can print it in debug output.

    switch (opcode) {
        case opcode_mov: return "mov";

        // function
        case opcode_enter: return "enter";
        case opcode_leave: return "leave";

        // branches
        case opcode_ret: return "ret";
        case opcode_jmp: return "jmp";
        case opcode_br:  return "br";
    }
    fatal("Unrecognized opcode.");
}

// The opcodes hashtable. Simple open addressing with linear probing using
// the FNV-1a hash, same as most other hashtables in Onramp.
static const char** opcodes_identifier;
static opcode_t* opcodes_value;
enum opcodes_constants {
    opcodes_capacity = 256,
};

static void opcode_add(const char* identifier, opcode_t value) {
    size_t i = fnv1a_cstr(identifier) & (opcodes_capacity - 1);
    for (;;) {
        if (opcodes_identifier[i] == NULL) {
            opcodes_identifier[i] = identifier;
            opcodes_value[i] = value;
            return;
        }
        i = (i + 1) & (opcodes_capacity - 1);
    }
}

opcode_t opcode_from_identifier(const char* identifier) {
    size_t i = fnv1a_cstr(identifier) & (opcodes_capacity - 1);
    for (;;) {
        if (opcodes_identifier[i] == NULL) {
            // TODO location
            fatal("Unrecognized opcode.");
        }
        if (0 == strcmp(opcodes_identifier[i], identifier)) {
            return opcodes_value[i];
        }
        i = (i + 1) & (opcodes_capacity - 1);
    }
}

void opcode_setup(void) {
    opcodes_identifier = calloc(opcodes_capacity, sizeof(char*));
    opcodes_value = calloc(opcodes_capacity, sizeof(opcode_t));

    // Note: We don't include any opcodes that can't appear in the IR, for
    // example "enter" and "leave".

    opcode_add("mov", opcode_ret);

    opcode_add("ret", opcode_ret);
    opcode_add("jmp", opcode_jmp);
    opcode_add("br", opcode_br);
}

void opcode_teardown(void) {
    free(opcodes_identifier);
    free(opcodes_value);
}
