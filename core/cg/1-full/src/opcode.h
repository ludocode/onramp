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

#ifndef OPCODE_INCLUDED
#define OPCODE_INCLUDED

#include <stdbool.h>

typedef enum opcode_t {
    opcode_nop,

    // arithmetic
    opcode_add,
    opcode_sub,
    opcode_mul,
    opcode_divu,
    opcode_divs,
    opcode_modu,
    opcode_mods,
    opcode_zero,
    opcode_inc,
    opcode_dec,
    opcode_sxs,
    opcode_sxb,
    opcode_trs,
    opcode_trb,

    // logic
    opcode_and,
    opcode_or,
    opcode_xor,
    opcode_not,
    opcode_shl,
    opcode_shru,
    opcode_shrs,
    opcode_rol,
    opcode_ror,
    opcode_mov,
    opcode_bool,
    opcode_isz,
    opcode_ltu,
    opcode_lts,

    // memory
    opcode_sym,
    opcode_ldw,
    opcode_lds,
    opcode_ldb,
    opcode_stw,
    opcode_sts,
    opcode_stb,

    // control
    opcode_imw,
    opcode_enter,
    opcode_leave,
    opcode_var,
    opcode_alloc,
    opcode_free,
    opcode_call,
    opcode_volatile,
    opcode_ret,
    opcode_jmp,
    opcode_br,
    opcode_jz,

} opcode_t;

/**
 * Returns true if the opcode is valid for the end of a block.
 */
bool opcode_is_block_end(opcode_t opcode);

opcode_t opcode_from_identifier(const char* identifier);
const char* opcode_to_string(opcode_t opcode);

void opcode_setup(void);
void opcode_teardown(void);

#endif
