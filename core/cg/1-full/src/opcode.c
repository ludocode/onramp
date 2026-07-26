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
        case opcode_nop: return "nop";

        // arithmetic
        case opcode_add: return "add";
        case opcode_sub: return "sub";
        case opcode_mul: return "mul";
        case opcode_divu: return "divu";
        case opcode_divs: return "divs";
        case opcode_modu: return "modu";
        case opcode_mods: return "mods";
        case opcode_zero: return "zero";
        case opcode_inc: return "inc";
        case opcode_dec: return "dec";
        case opcode_sxs: return "sxs";
        case opcode_sxb: return "sxb";
        case opcode_trs: return "trs";
        case opcode_trb: return "trb";

        // logic
        case opcode_and: return "and";
        case opcode_or: return "or";
        case opcode_xor: return "xor";
        case opcode_not: return "not";
        case opcode_shl: return "shl";
        case opcode_shru: return "shru";
        case opcode_shrs: return "shrs";
        case opcode_rol: return "rol";
        case opcode_ror: return "ror";
        case opcode_mov: return "mov";
        case opcode_bool: return "bool";
        case opcode_isz: return "isz";
        case opcode_ltu: return "ltu";
        case opcode_lts: return "lts";

        // memory
        case opcode_sym: return "sym";
        case opcode_ldw: return "ldw";
        case opcode_lds: return "lds";
        case opcode_ldb: return "ldb";
        case opcode_stw: return "stw";
        case opcode_sts: return "sts";
        case opcode_stb: return "stb";

        // control
        case opcode_imw: return "imw";
        case opcode_enter: return "enter";
        case opcode_leave: return "leave";
        case opcode_var: return "var";
        case opcode_alloc: return "alloc";
        case opcode_free: return "free";
        case opcode_call: return "call";
        case opcode_volatile: return "volatile";
        case opcode_ret: return "ret";
        case opcode_jmp: return "jmp";
        case opcode_br: return "br";
        case opcode_jz: return "jz";

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

static void opcode_insert(const char* identifier, opcode_t value) {
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
            printf("%s\n",identifier);
            fflush(stdout);
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

    opcode_insert("nop", opcode_nop);

    // arithmetic
    opcode_insert("add", opcode_add);
    opcode_insert("sub", opcode_sub);
    opcode_insert("mul", opcode_mul);
    opcode_insert("divu", opcode_divu);
    opcode_insert("divs", opcode_divs);
    opcode_insert("modu", opcode_modu);
    opcode_insert("mods", opcode_mods);
    opcode_insert("zero", opcode_zero);
    opcode_insert("inc", opcode_inc);
    opcode_insert("dec", opcode_dec);
    opcode_insert("sxs", opcode_sxs);
    opcode_insert("sxb", opcode_sxb);
    opcode_insert("trs", opcode_trs);
    opcode_insert("trb", opcode_trb);

    // logic
    opcode_insert("and", opcode_and);
    opcode_insert("or", opcode_or);
    opcode_insert("xor", opcode_xor);
    opcode_insert("not", opcode_not);
    opcode_insert("shl", opcode_shl);
    opcode_insert("shru", opcode_shru);
    opcode_insert("shrs", opcode_shrs);
    opcode_insert("rol", opcode_rol);
    opcode_insert("ror", opcode_ror);
    opcode_insert("mov", opcode_mov);
    opcode_insert("bool", opcode_bool);
    opcode_insert("isz", opcode_isz);
    opcode_insert("ltu", opcode_ltu);
    opcode_insert("lts", opcode_lts);

    // memory
    opcode_insert("sym", opcode_sym);
    opcode_insert("ldw", opcode_ldw);
    opcode_insert("lds", opcode_lds);
    opcode_insert("ldb", opcode_ldb);
    opcode_insert("stw", opcode_stw);
    opcode_insert("sts", opcode_sts);
    opcode_insert("stb", opcode_stb);

    // control
    opcode_insert("enter", opcode_enter);
    opcode_insert("leave", opcode_leave);
    opcode_insert("var", opcode_var);
    opcode_insert("alloc", opcode_alloc);
    opcode_insert("free", opcode_free);
    opcode_insert("call", opcode_call);
    opcode_insert("volatile", opcode_volatile);
    opcode_insert("ret", opcode_ret);
    opcode_insert("jmp", opcode_jmp);
    opcode_insert("br", opcode_br);
    opcode_insert("jz", opcode_jz);
}

void opcode_teardown(void) {
    free(opcodes_identifier);
    free(opcodes_value);
}
