/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
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

#include "opcodes.h"

#include <stdint.h>
#include <string.h>

#include "parse.h"
#include "emit.h"

#include "libo-util.h"

#define ADD   0x70
#define SUB   0x71
#define MUL   0x72
#define DIVU  0x73
#define AND   0x74
#define OR    0x75
#define SHL   0x76
#define SHRU  0x77
#define LDW   0x78
#define STW   0x79
#define LDB   0x7A
#define STB   0x7B
#define IMS   0x7C
#define CMPU  0x7D
#define JZ    0x7E
#define SYS   0x7F

#define R0    0x80
#define RA    0x8A
#define RB    0x8B
#define RSP   0x8C
#define RFP   0x8D
#define RPP   0x8E
#define RIP   0x8F



/*
 * Helpers
 */

// Parses and emits the offset (destination) of a jz instruction.
static void parse_and_emit_jump_offset(void) {
    if (try_parse_invocation_relative()) {
        emit_label(identifier, label_type_invocation_relative, label_flags, -1, -1);
        return;
    }
    fatal("Expected relative label as jump destination.");
}

// TODO these jump instructions are slow, the ones in as/1 are better

static void opcode_jz_je(uint8_t pred) {
    emit_hex_byte(JZ);  // jz
    emit_hex_byte(pred);  // pred
    parse_and_emit_jump_offset();
}

static void opcode_jnz_jne(uint8_t pred) {
    uint8_t bytes[] = {
        JZ, pred, 0x01, 0x00,  // jz pred +1
        JZ, 0x00,              // jz 0 label
    };
    emit_hex_bytes(bytes, sizeof(bytes));
    parse_and_emit_jump_offset();
}

// Jumps if the value matches
static void opcode_jg_jl(uint8_t value) {
    uint8_t reg = parse_register_non_scratch();
    uint8_t bytes[] = {
        CMPU, RB, reg, value,  // cmpu rb reg value
        JZ, RB,              // jz rb label
    };
    emit_hex_bytes(bytes, sizeof(bytes));
    parse_and_emit_jump_offset();
}

// Jumps if the value *doesn't* match
static void opcode_jge_jle(uint8_t value) {
    uint8_t reg = parse_register_non_scratch();
    uint8_t bytes[] = {
        CMPU, RB, reg, value,  // cmpu rb reg value
        JZ, RB, 0x01, 0x00,    // jz 0 +1
        JZ, 0x00,              // jz rb label
    };
    emit_hex_bytes(bytes, sizeof(bytes));
    parse_and_emit_jump_offset();
}

static void opcode_reg_mix_mix(uint8_t opcode) {
    uint8_t dest = parse_register();
    uint8_t src1 = parse_mix();
    uint8_t src2 = parse_mix();
    uint8_t bytes[] = {opcode, dest, src1, src2};
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_mix_mix_mix(uint8_t opcode) {
    uint8_t dest = parse_mix();
    uint8_t src1 = parse_mix();
    uint8_t src2 = parse_mix();
    uint8_t bytes[] = {opcode, dest, src1, src2};
    emit_hex_bytes(bytes, sizeof(bytes));
}



/*
 * Arithmetic
 */

static void opcode_add(void) {
    opcode_reg_mix_mix(ADD);
}

static void opcode_sub(void) {
    opcode_reg_mix_mix(SUB);
}

static void opcode_mul(void) {
    opcode_reg_mix_mix(MUL);
}

static void opcode_divu(void) {
    opcode_reg_mix_mix(DIVU);
}

static void opcode_divs(void) {
    uint8_t dest = parse_register_numbered();
    uint8_t src1 = parse_mix_non_scratch();
    uint8_t src2 = parse_mix_non_scratch();

    // To do a signed division, we convert both arguments to unsigned, do
    // an unsigned division, then make the result negative if exactly one
    // of the arguments was negative.

    // If the destination register is not one of the sources, we can use it as
    // an extra scratch register. Otherwise, we need to push some data to the
    // stack.

    // TODO it's probably possible to do this optimization in all cases, we
    // just need to decide which argument to read first.

    if (dest != src1 && dest != src2) {
        uint8_t bytes[] = {

            // collect sign of src1 in dest
            SHRU, dest, src1, 31,

            // place absolute value of src1 in ra
            JZ, dest, 2, 0,
            SUB, RA, 0, src1,
            JZ, 0, 1, 0,
            ADD, RA, src1, 0,

            // xor sign of src2 into dest
            SHRU, RB, src2, 31,
            ADD, dest, dest, RB,
            AND, dest, dest, 1,

            // place absolute value of src2 in rb
            JZ, RB, 2, 0,
            SUB, RB, 0, src2,
            JZ, 0, 1, 0,
            ADD, RB, src2, 0,

            // do the unsigned division
            DIVU, RA, RA, RB,

            // move to dest with appropriate sign
            JZ, dest, 2, 0,
            SUB, dest, 0, RA,
            JZ, 0, 1, 0,
            ADD, dest, 0, RA,
        };
        emit_hex_bytes(bytes, sizeof(bytes));

    } else {
        uint8_t bytes[] = {

            // make stack space
            SUB, RSP, RSP, 8,

            // collect sign of src1 in ra, store it on the stack
            SHRU, RA, src1, 31,
            STW, RA, RSP, 0,

            // place absolute value of src1 in ra
            JZ, RA, 2, 0,
            SUB, RA, 0, src1,
            JZ, 0, 1, 0,
            ADD, RA, src1, 0,

            // collect sign of src2 in rb, store it on the stack
            SHRU, RB, src2, 31,
            STW, RB, RSP, 4,

            // place absolute value of src2 in rb
            JZ, RB, 2, 0,
            SUB, RB, 0, src2,
            JZ, 0, 1, 0,
            ADD, RB, src2, 0,

            // do the unsigned division
            // (we can write to dest now since we're done reading srcs)
            DIVU, dest, RA, RB,

            // pop and xor signs
            LDW, RA, RSP, 0,
            LDW, RB, RSP, 4,
            ADD, RSP, RSP, 8,
            ADD, RA, RA, RB,
            AND, RA, RA, 1,

            // flip sign of dest if exactly one of src1 and src2 was negative
            JZ, RA, 1, 0,
            SUB, dest, 0, dest,

        };
        emit_hex_bytes(bytes, sizeof(bytes));
    }
}

static void opcode_modu(void) {
    uint8_t dest = parse_register_non_scratch();
    uint8_t src1 = parse_mix_non_scratch();
    uint8_t src2 = parse_mix_non_scratch();
    uint8_t bytes[] = {
        // We just do a division then multiply it back. The difference is the
        // remainder.
        DIVU, RA, src1, src2,
        MUL, RB, RA, src2,
        SUB, dest, src1, RB,
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_mods(void) {
    uint8_t dest = parse_register_numbered();
    uint8_t src1 = parse_mix_non_scratch();
    uint8_t src2 = parse_mix_non_scratch();

    // Modulo in C is defined to use truncated division. We perform unsigned
    // modulo with the magnitudes of the arguments, then set the sign of the
    // remainder to the sign of the dividend.

    // We don't have enough scratch registers so we store the sign on the
    // stack.

    uint8_t bytes[] = {

        // make stack space
        SUB, RSP, RSP, 4,

        // store sign of src1 on the stack
        SHRU, RA, src1, 31,
        STW, RA, RSP, 0,

        // place absolute value of src1 in ra
        JZ, RA, 2, 0,
        SUB, RA, 0, src1,
        JZ, 0, 1, 0,
        ADD, RA, src1, 0,

        // place absolute value of src2 in rb
        SHRU, RB, src2, 31,
        JZ, RB, 2, 0,
        SUB, RB, 0, src2,
        JZ, 0, 1, 0,
        ADD, RB, src2, 0,

        // do the unsigned modulus
        // (we can write to dest now since we're done reading srcs)
        DIVU, dest, RA, RB,
        MUL, RB, dest, RB,
        SUB, dest, RA, RB,

        // pop and flip sign of dest if src1 was negative
        LDW, RA, RSP, 0,
        ADD, RSP, RSP, 4,
        JZ, RA, 1, 0,
        SUB, dest, 0, dest,

    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_zero(void) {
    uint8_t dest = parse_register();
    uint8_t bytes[] = {
        ADD, dest, 0x00, 0x00,  // add dest 0 0
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_inc(void) {
    uint8_t dest = parse_register();
    uint8_t bytes[] = {
        ADD, dest, dest, 0x01,  // add dest dest 1
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_dec(void) {
    uint8_t dest = parse_register();
    uint8_t bytes[] = {
        SUB, dest, dest, 0x01,  // sub dest dest 1
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_sxs(void) {
    uint8_t dest = parse_register_non_scratch();
    uint8_t src = parse_mix();
    uint8_t bytes[] = {

        // branch on the high bit
        SHRU, RA, src, 15,
        AND, RA, RA, 1,
        JZ, RA, 5, 0,

        // negative, set high bits
        SHL, RA, 1, 16,
        SUB, RA, RA, 1,
        SHL, RA, RA, 16,
        OR, dest, RA, src,
        JZ, 0, 3, 0,

        // positive, clear high bits
        SHL, RA, 1, 16,
        SUB, RA, RA, 1,
        AND, dest, src, RA,

    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_sxb(void) {
    uint8_t dest = parse_register_non_scratch();
    uint8_t src = parse_mix();
    uint8_t bytes[] = {

        // branch on the high bit
        SHRU, RA, src, 7,
        AND, RA, RA, 1,
        JZ, RA, 5, 0,

        // negative, set high bits
        SHL, RA, 1, 24,
        SUB, RA, RA, 1,
        SHL, RA, RA, 8,
        OR, dest, RA, src,
        JZ, 0, 3, 0,

        // positive, clear high bits
        SHL, RA, 1, 8,
        SUB, RA, RA, 1,
        AND, dest, src, RA,

    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_trs(void) {
    uint8_t dest = parse_register_non_scratch();
    uint8_t src = parse_mix();
    uint8_t bytes[] = {
        SHL, RA, 1, 16,
        SUB, RA, RA, 1,
        AND, dest, src, RA,
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_trb(void) {
    uint8_t dest = parse_register_non_scratch();
    uint8_t src = parse_mix();
    uint8_t bytes[] = {
        SHL, RA, 1, 8,
        SUB, RA, RA, 1,
        AND, dest, src, RA,
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}



/*
 * Logic
 */

static void opcode_and(void) {
    opcode_reg_mix_mix(AND);
}

static void opcode_or(void) {
    opcode_reg_mix_mix(OR);
}

static void opcode_xor(void) {
    uint8_t dest = parse_register();
    uint8_t arg1 = parse_mix_non_scratch();
    uint8_t arg2 = parse_mix_non_scratch();
    uint8_t bytes[] = {
        OR, RA, arg1, arg2,    // or ra arg1 arg2
        AND, RB, arg1, arg2,   // and rb arg2 arg2
        SUB, dest, RA, RB,     // sub dest ra rb
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

/*
static void opcode_ror(void) {
    opcode_reg_mix_mix(ROR);
}
*/

static void opcode_mov(void) {
    uint8_t dest = parse_register();
    uint8_t src = parse_mix();
    uint8_t bytes[] = {
        ADD, dest, 0x00, src,
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_not(void) {
    uint8_t dest = parse_register();
    uint8_t src = parse_mix();
    uint8_t bytes[] = {
        SUB, dest, 0xFF, src,
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

/*
static void opcode_rol(void) {
    uint8_t dest = parse_register();
    uint8_t src = parse_mix_non_scratch();
    uint8_t bits = parse_mix_non_scratch();
    uint8_t bytes[] = {
        // This is the most straightforward code. The 32 doesn't actually
        // matter, we could have subtracted from 0 or any multiple of 32,
        // because the ror instruction only cares about the bits mod 32.
        SUB, RA, 32, bits,
        ROR, dest, src, RA,
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}
*/

static void opcode_shrs(void) {
    uint8_t dest = parse_register();
    uint8_t src = parse_mix_non_scratch();
    uint8_t bits = parse_mix_non_scratch();
    uint8_t bytes[] = {

        // test the sign bit
        SHRU, RA, src, 31,
        JZ, RA, 7, 0,

        // negative. if bits is zero, jump to unsigned shift (to copy to dest)
        JZ, bits, 6, 0,
        // generate a mask
        SUB, RB, 32, bits,
        SHL, RB, 1, RB,
        SUB, RB, 0, RB,
        // shift and apply inverted mask
        SHRU, RA, src, bits,
        OR, dest, RA, RB,
        JZ, 0, 1, 0,

        // non-negative. do unsigned shift
        SHRU, dest, src, bits,
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_shru(void) {
    opcode_reg_mix_mix(SHRU);
}

static void opcode_shl(void) {
    opcode_reg_mix_mix(SHL);
}

static void opcode_bool(void) {
    uint8_t dest = parse_register();
    uint8_t src = parse_mix();

    // In code emitted by our compilers, src and dest are often the same. We
    // can optimize this a bit.

    // TODO this could be replaced by just lt and sub, see isz

    if (src != dest) {
        uint8_t mov[] = {
            ADD, dest, 0, src,
        };
        emit_hex_bytes(mov, sizeof(mov));
    }

    uint8_t bytes[] = {
        JZ, dest, 1, 0,
        ADD, dest, 0, 1,
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_isz(void) {
    uint8_t dest = parse_register();
    uint8_t src = parse_mix();

    // In code emitted by our compilers, src and dest are often the same. We
    // can optimize this a bit.

    // TODO this could be replaced by a single lt instruction if we ever get
    // around to replacing cmpu

    if (src == dest) {
        uint8_t bytes[] = {
            JZ, dest, 1, 0,
            ADD, dest, 0, 1,
            SUB, dest, 1, dest,
        };
        emit_hex_bytes(bytes, sizeof(bytes));
    } else {
        uint8_t bytes[] = {
            JZ, src, 2, 0,
            ADD, dest, 0, 0,
            JZ, 0, 1, 0,
            ADD, dest, 0, 1,
        };
        emit_hex_bytes(bytes, sizeof(bytes));
    }
}



/*
 * Memory
 */

static void opcode_ldw(void) {
    opcode_reg_mix_mix(LDW);
}

static void opcode_stw(void) {
    opcode_mix_mix_mix(STW);
}

static void opcode_ldb(void) {
    opcode_reg_mix_mix(LDB);
}

static void opcode_stb(void) {
    opcode_mix_mix_mix(STB);
}

static void opcode_lds(void) {
    uint8_t dest = parse_register_non_scratch();
    uint8_t base = parse_mix_non_scratch();
    uint8_t offset = parse_mix_non_scratch();

    // If either base or offset is zero, we can save an instruction; otherwise
    // we have to add them beforehand.

    if (base == 0 || offset == 0) {
        uint8_t addr = (base != 0) ? base : offset;
        uint8_t bytes[] = {
            LDB, RA, addr, 0,
            LDB, RB, addr, 1,
            SHL, RB, RB, 8,
            OR, dest, RA, RB,
        };
        emit_hex_bytes(bytes, sizeof(bytes));

    } else {
        uint8_t bytes[] = {
            ADD, RB, base, offset,
            LDB, RA, RB, 0,
            LDB, RB, RB, 1,
            SHL, RB, RB, 8,
            OR, dest, RA, RB,
        };
        emit_hex_bytes(bytes, sizeof(bytes));
    }
}

static void opcode_sts(void) {
    uint8_t value = parse_mix_non_scratch();
    uint8_t base = parse_mix_non_scratch();
    uint8_t offset = parse_mix_non_scratch();

    // If either base or offset is zero, we can save an instruction; otherwise
    // we have to add them beforehand.

    if (base == 0 || offset == 0) {
        uint8_t addr = (base != 0) ? base : offset;
        uint8_t bytes[] = {
            STB, value, addr, 0,
            SHRU, RA, value, 8,
            STB, RA, addr, 1,
        };
        emit_hex_bytes(bytes, sizeof(bytes));
    } else {
        uint8_t bytes[] = {
            ADD, RB, base, offset,
            STB, value, RB, 0,
            SHRU, RA, value, 8,
            STB, RA, RB, 1,
        };
        emit_hex_bytes(bytes, sizeof(bytes));
    }
}

static void opcode_push(void) {
    uint8_t value = parse_mix();
    uint8_t bytes[] = {
        SUB, RSP, RSP, 0x04,
        STW, value, RSP, 0x00,
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_pop(void) {
    uint8_t reg = parse_register();
    uint8_t bytes[] = {
        LDW, reg, RSP, 0x00,
        ADD, RSP, RSP, 0x04,
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_popd(void) {
    uint8_t bytes[] = {
        ADD, RSP, RSP, 0x04,
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}



/*
 * Control
 */

static void opcode_ims(void) {
    uint8_t reg = parse_register();

    emit_hex_byte(IMS);
    emit_hex_byte(reg);

    if (try_parse_invocation_short()) {
        emit_label(identifier, label_type, label_flags, -1, -1);
        return;
    }

    // number or two quoted bytes
    if (try_parse_and_emit_short()) {
        return;
    }

    uint8_t a, b;
    if (try_parse_character_or_quoted_byte(&a) &&
            try_parse_character_or_quoted_byte(&b))
    {
        emit_hex_byte(a);
        emit_hex_byte(b);
        return;
    }

    fatal("Expected ims value: short invocation, number, or two quoted bytes or single-character strings.");
}

static void opcode_cmpu(void) {
    opcode_reg_mix_mix(CMPU);
}

static void opcode_jz(void) {
    opcode_jz_je(parse_mix());
}

static void opcode_sys(void) {
    uint8_t number = parse_syscall_number();

    // make sure sys is followed by two quoted bytes
    uint8_t arg1, arg2;
    if (!try_parse_quoted_byte(&arg1) || !try_parse_quoted_byte(&arg2) ||
            arg1 != 0 || arg2 != 0)
    {
        fatal("Expected sys instruction to end in two quoted zero bytes.");
    }

    uint8_t bytes[] = {
        SYS, number, 0x00, 0x00,  // sys number 0 0
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_imw(void) {
    uint8_t reg = parse_register();
    if (reg == RIP) {
        // We're loading in two steps so we can't load straight into rip. (Not
        // that it would make much sense anyway since programs have to be
        // position-independent and VMs can decide what regions of memory are
        // mapped.)
        fatal("Register for imw cannot be rip.");
    }

    // number
    int32_t value;
    if (try_parse_number(&value)) {
        uint8_t bytes[] = {
            IMS, reg, (uint8_t)((value >> 16) & 0xFF), (uint8_t)((value >> 24) & 0xFF),  // ims reg byte3 byte4
            IMS, reg, (uint8_t)((value      ) & 0xFF), (uint8_t)((value >>  8) & 0xFF),  // ims reg byte1 byte2
        };
        emit_hex_bytes(bytes, sizeof(bytes));
        return;
    }

    // absolute label
    if (try_parse_invocation_absolute()) {
        emit_imw_absolute(reg);
        return;
    }

    // relative label
    if (try_parse_invocation_relative()) {
        uint8_t bytes[] = {
            ADD, reg, 0x00, 0x00,  // add reg 0 0
            CMPU, reg,              // ims reg &label
        };
        emit_hex_bytes(bytes, sizeof(bytes));
        emit_label(identifier, label_type_invocation_relative, label_flags, -1, -1);
        return;
    }

    // four quoted bytes (in little-endian)
    uint8_t a, b, c, d;
    if (try_parse_character_or_quoted_byte(&a) &&
            try_parse_character_or_quoted_byte(&b) &&
            try_parse_character_or_quoted_byte(&c) &&
            try_parse_character_or_quoted_byte(&d))
    {
        uint8_t bytes[] = {
            IMS, reg, c, d,  // ims reg c d
            IMS, reg, a, b,  // ims reg a b
        };
        emit_hex_bytes(bytes, sizeof(bytes));
        return;
    }

    fatal("Expected imw value: number, absolute or relative label, or four quoted bytes or single-character strings.");
}

static void opcode_cmps(void) {
    uint8_t dest = parse_register();
    uint8_t arg1 = parse_mix_non_scratch();
    uint8_t arg2 = parse_mix_non_scratch();
    uint8_t bytes[] = {
        SHL, RB, 0x01, 0x1F,  // shl rb 1 31       // rb = 0x80000000
        ADD, RA, arg1, RB,    // add ra arg1 rb
        ADD, RB, arg2, RB,    // add rb arg2 rb
        CMPU, dest, RA, RB,   // cmpu dest ra rb
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_ltu(void) {
    // This is a temporary implementation that uses cmpu. Eventually cmpu will
    // be replaced by ltu in the VM.
    uint8_t dest = parse_register();
    uint8_t arg1 = parse_mix_non_scratch();
    uint8_t arg2 = parse_mix_non_scratch();
    uint8_t bytes[] = {
        CMPU, RA, arg1, arg2,  // cmpu ra arg1 arg2
        SHRU, RA, RA, 0x01,    // shru ra ra 1
        AND, dest, RA, 0x01,   // and dest ra 1
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_lts(void) {
    uint8_t dest = parse_register();
    uint8_t arg1 = parse_mix_non_scratch();
    uint8_t arg2 = parse_mix_non_scratch();
    uint8_t bytes[] = {
        SHL, RB, 0x01, 0x1F,  // shl rb 1 31       // rb = 0x80000000
        ADD, RA, arg1, RB,    // add ra arg1 rb
        ADD, RB, arg2, RB,    // add rb arg2 rb
        // This is temporary; the rest of this should become LTU.
        CMPU, RA, RA, RB,     // cmpu ra ra rb
        SHRU, RA, RA, 0x01,   // shru ra ra 1
        AND, dest, RA, 0x01,  // and dest ra 1
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_jnz(void) {
    opcode_jnz_jne(parse_mix());
}

static void opcode_jmp(void) {

    // absolute
    if (try_parse_invocation_absolute()) {
        emit_imw_absolute(RA);  // imw ra ^label
        uint8_t bytes[] = {
            ADD, RIP, RPP, RA,  // add rip rpp ra
        };
        emit_hex_bytes(bytes, sizeof(bytes));
        return;
    }

    // relative
    emit_hex_byte(JZ);  // jz
    emit_hex_byte(0x00);  // 0
    parse_and_emit_jump_offset();
}

static void opcode_je(void) {
    // same as jz except we only accept a register as predicate, not a mix-type
    opcode_jz_je(parse_register());
}

static void opcode_jne(void) {
    // same as jnz except we only accept a register as predicate, not a mix-type
    opcode_jnz_jne(parse_register());
}

static void opcode_jg(void) {
    opcode_jg_jl(0x01);
}

static void opcode_jl(void) {
    opcode_jg_jl(0xFF);
}

static void opcode_jge(void) {
    // The value is what we *don't* want to jump on
    opcode_jge_jle(0xFF);
}

static void opcode_jle(void) {
    // The value is what we *don't* want to jump on
    opcode_jge_jle(0x01);
}

static void opcode_enter(void) {
    uint8_t bytes[] = {
        SUB, RSP, RSP, 0x04,  // push rfp
        STW, RFP, 0x00, RSP,  // ^^^
        ADD, RFP, RSP, 0x00,  // mov rfp rsp
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_leave(void) {
    uint8_t bytes[] = {
        ADD, RSP, RFP, 0x00,  // mov rsp rfp
        LDW, RFP, 0x00, RSP,  // pop rfp
        ADD, RSP, RSP, 0x04,  // ^^^
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}

static void opcode_call(void) {

    // absolute
    if (try_parse_invocation_absolute()) {
        emit_imw_absolute(RA); // imw ra ^label
        uint8_t bytes[] = {
            SUB, RSP, RSP, 4,  // push return address
            ADD, RB, RIP, 8,   // ^^^
            STW, RB, 0, RSP,   // ^^^
            ADD, RIP, RPP, RA, // jump
            ADD, RSP, RSP, 4,  // pop return address
        };
        emit_hex_bytes(bytes, sizeof(bytes));
        return;
    }

    // indirect register
    uint8_t reg;
    if (try_parse_register(&reg)) {
        uint8_t bytes[] = {
            SUB, RSP, RSP, 4,  // push return address
            ADD, RB, RIP, 8,   // ^^^
            STW, RB, 0, RSP,   // ^^^
            ADD, RIP, 0, reg,  // jump
            ADD, RSP, RSP, 4,  // pop return address
        };
        emit_hex_bytes(bytes, sizeof(bytes));
        return;
    }

    fatal("Expected absolute invocation or register as argument to call instruction.");
}

static void opcode_ret(void) {
    uint8_t bytes[] = {
        LDW, RIP, 0x00, RSP,
    };
    emit_hex_bytes(bytes, sizeof(bytes));
}



/*
 * Tables
 */

typedef struct opcode_fn_t {
    const char* name;
    void (*function)(void);
} opcode_fn_t;

static opcode_fn_t opcodes_list[] = {

    // arithmetic
    {"add", opcode_add},
    {"sub", opcode_sub},
    {"mul", opcode_mul},
    {"divu", opcode_divu},
    {"divs", opcode_divs},
    {"modu", opcode_modu},
    {"mods", opcode_mods},
    {"zero", opcode_zero},
    {"inc", opcode_inc},
    {"dec", opcode_dec},
    {"sxs", opcode_sxs},
    {"sxb", opcode_sxb},
    {"trs", opcode_trs},
    {"trb", opcode_trb},

    // logic
    {"and", opcode_and},
    {"or", opcode_or},
    {"xor", opcode_xor},
    {"not", opcode_not},
    {"shl", opcode_shl},
    {"shru", opcode_shru},
    {"shrs", opcode_shrs},
    //{"rol", opcode_rol},
    //{"ror", opcode_ror},
    {"mov", opcode_mov},
    {"bool", opcode_bool},
    {"isz", opcode_isz},

    // memory
    {"ldw", opcode_ldw},
    {"lds", opcode_lds},
    {"ldb", opcode_ldb},
    {"stw", opcode_stw},
    {"sts", opcode_sts},
    {"stb", opcode_stb},
    {"push", opcode_push},
    {"pop", opcode_pop},
    {"popd", opcode_popd},

    // control
    {"ims", opcode_ims},
    {"imw", opcode_imw},
    {"cmpu", opcode_cmpu},
    {"cmps", opcode_cmps},
    {"ltu", opcode_ltu},
    {"lts", opcode_lts},
    {"jz", opcode_jz},
    {"jnz", opcode_jnz},
    {"je", opcode_je},
    {"jne", opcode_jne},
    {"jl", opcode_jl},
    {"jg", opcode_jg},
    {"jle", opcode_jle},
    {"jge", opcode_jge},
    {"jmp", opcode_jmp},
    {"call", opcode_call},
    {"ret", opcode_ret},
    {"enter", opcode_enter},
    {"leave", opcode_leave},
    {"sys", opcode_sys},

};

// We create a simple fixed-size hashtable that uses open addressing with
// linear probing.
#define OPCODES_TABLE_CAPACITY 256
static opcode_fn_t opcodes_table[OPCODES_TABLE_CAPACITY];

void opcodes_init(void) {
    size_t count = sizeof(opcodes_list) / sizeof(*opcodes_list);
    size_t mask = OPCODES_TABLE_CAPACITY - 1;

    for (size_t i = 0; i < count; ++i) {
        opcode_fn_t* fn = &opcodes_list[i];
        size_t i = fnv1a_cstr(fn->name) & mask;
        while (opcodes_table[i].name != NULL) {
            i = (i + 1) & mask;
        }
        //opcodes_table[i] = *fn; // TODO implement struct assignment in cci/2; in the meantime we fallback to memcpy
        memcpy(&opcodes_table[i],fn,sizeof(*fn));
    }
}

void opcodes_destroy(void) {
    // nothing
}

void opcodes_dispatch(const char* name) {
    size_t mask = OPCODES_TABLE_CAPACITY - 1;
    size_t i = fnv1a_cstr(name) & mask;
    while (1) {
        opcode_fn_t* fn = &opcodes_table[i];
        if (fn->name == NULL) {
            break;
        }
        if (0 == strcmp(name, fn->name)) {
            fn->function();
            return;
        }
        i = (i + 1) & mask;
    }
    fatal("Invalid opcode: %s", name);
}
