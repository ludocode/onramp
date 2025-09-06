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

#ifndef OPTIMIZE_H_INCLUDED
#define OPTIMIZE_H_INCLUDED

/*
 * This file contains the various optimization passes.
 *
 * An optimization pass walks through the `instructions` array (see
 * instruction.h) and manipulates it in place. Each pass takes in valid
 * assembly and it must leave the array as valid assembly with observable
 * behaviour unchanged (except of course for performance.)
 *
 * See the README.md for a description of each optimization performed.
 */

#include <stdbool.h>

#include "register.h"



/*
 * Stack push/pop elimination
 *
 * We replace push/pop pairs with movs to unused registers. See the README.md
 * for details.
 *
 * This uses the extra fields in `instruction_t` to store a stack of push
 * instructions. The `opt_vp` field stores the stack of push instructions as an
 * intrusive linked list. The `opt_int` field stores the lowest numbered
 * register that can be used by a push/pop pair.
 *
 * Note that we don't bother to optimize things like `popd` since cci/0 doesn't
 * emit those. We avoid generic optimizations in general; the transformations
 * are highly tailored to cci/0's output. TODO we should not even parse popd
 */

static bool optimize_push_pop_blocked(instruction_t* instruction, opcode_t opcode) {

    // These instructions either do not preserve registers or require us to
    // preserve the stack. If any of these are encountered, we block any
    // optimizations that might cross it by discarding our push stack.
    if (opcode == OP_DECLARATION) {return true;}
    if (opcode == OP_CALL) {return true;}
    if (opcode == OP_RET) {return true;}
    if (opcode == OP_JZ) {return true;}
    if (opcode == OP_JNZ) {return true;}
    if (opcode == OP_JMP) {return true;}
    if (opcode == OP_POPD) {return true;}

    // If anything reads or writes rsp, we discard the push stack.
    if (instruction_uses_register(instruction, REG_RSP)) {return true;}

    return false;
}

static void optimize_push_pop(void) {
    instruction_t* last_push = NULL;

    size_t i = 0;
    while (i < instructions_count) {
        instruction_t* instruction = *(instructions + i);
        opcode_t opcode = instruction_opcode(instruction);
        i = (i + 1);

        // If we encounter anything that could touch rsp, we dump our push
        // stack. This includes jumps, labels, calls, and any instruction that
        // takes rsp as an argument.
        if (optimize_push_pop_blocked(instruction, opcode)) {
            last_push = NULL;
            continue;
        }

        // On a push, insert the instruction to the front of the stack.
        if (opcode == OP_PUSH) {
            instruction_set_opt_vp(instruction, last_push);
            last_push = instruction;
            // We don't know about any nested pushes yet. The lowest numbered
            // register we can use is r4.
            instruction_set_opt_int(instruction, 0x84);
            continue;
        }

        // We otherwise only care about pops, and only as long as they are
        // paired with a push in the same basic block.
        if (opcode != OP_POP) {
            continue;
        }
        if (last_push == NULL) {
            continue;
        }

        // We've found a pop with a matching push that we can optimize.

        // Convert the push to a mov into the temporary register
        int reg = instruction_opt_int(last_push);
        instruction_set_arg1(last_push, instruction_arg0(last_push));
        instruction_set_arg0(last_push, reg);
        instruction_set_opcode(last_push, OP_MOV);

        // Convert the pop to a mov from the temporary register
        instruction_set_arg1(instruction, reg);
        instruction_set_opcode(instruction, OP_MOV);

        // We're done with the original push instruction
        last_push = instruction_opt_vp(last_push);

        // If we have an outer push instruction, bump the register it can use
        // (but only if it's not already larger)
        if (last_push) {
            int new_reg = (1 + reg);
            if (new_reg > 0x88) {
                // No free registers. We're not going to be able to pair up
                // anything else in our stack; just clear it.
                last_push = NULL;
                continue;
            }
            int old_reg = instruction_opt_int(last_push);
            if (new_reg > old_reg) {
                instruction_set_opt_int(last_push, new_reg);
            }
        }
    }
}



/*
 * Forward propagation
 *
 * The forward propagation pass includes the following optimizations:
 *
 * - propagation of inline additions
 * - propagation of constants and registers
 * - constant folding
 * - unreachable code elimination
 *
 * As we walk through the function, we keep track of the "horizon", which is
 * the index of one past the last instruction that quashed all registers. In
 * other words, it's the index of the first instruction we can use for
 * optimizations.
 *
 * Instructions that change the horizon include labels and calls: control flow
 * jumps to them and all registers have indeterminate values afterwards. If a
 * register was last written to before the horizon, we can't use it for
 * optimization. This saves us from having to clear all registers every time we
 * reach a label or call.
 */

/*
 * Called by constant folding when we've detected that an instruction writes a
 * constant value to a register.
 *
 * If the register already has this value, the instruction is eliminated.
 *
 * If we can make the instruction shorter, or if we can eliminate a register
 * dependency (the force parameter), the instruction is replaced by MOV or IMW
 * depending on whether it fits in a mix-type byte.
 *
 * Finally we record the fact that the register contains this constant for
 * later optimizations.
 */
void optimize_constant(instruction_t* instruction, int value, bool force, size_t horizon) {

    //printf("Found a constant value %i for: ", value); instruction_print(instruction);

    // This instruction must write to arg0
    assert((opcode_style(instruction_opcode(instruction)) == STYLE_REG_MIX) |
        ((opcode_style(instruction_opcode(instruction)) == STYLE_REG_CON) |
        (opcode_style(instruction_opcode(instruction)) == STYLE_REG)));
    int arg0 = instruction_arg0(instruction);
    assert(is_register(arg0));

    // Check if the last assignment to the destination register gave it this
    // same constant value. If it does, and it was assigned later than the
    // horizon, this instruction is redundant.
    reg_t* dest = register_get(arg0);
    if (register_content_type(dest) == REGISTER_CONTENT_CONSTANT) {
        if (register_value(dest) == value) {
            if (instruction_index(register_instruction(dest)) > horizon) {
                instruction_set_opcode(instruction, OP_NOP);
                return;
            }
        }
    }

    // If the instruction would be smaller, or if we can eliminate a register
    // dependency (force is true), replace it with IMW or MOV.
    if (force | (opcode_size(instruction_opcode(instruction)) != 1)) {
        opcode_t opcode = OP_NOP; // TODO use else here once we get it in cci/0, this is confusing
        if ((value >= -0x70) & (value <= 0x7F)) {
            opcode = OP_MOV;
        }
        if (opcode == OP_NOP) {
            opcode = OP_IMW;
        }
        instruction_set_opcode(instruction, opcode);
        instruction_set_arg1(instruction, value);
    }

    // Record the fact that this register contains this constant for later
    // optimizations.
    register_set_constant(dest, value, instruction);
}

/**
 * Called by constant folding when we detect that an instruction is equivalent
 * to a mov from one register to another.
 *
 * If the register already contained a mov from that register and that register
 * hasn't changed, the instruction is eliminated.
 *
 * Otherwise the instruction is replaced by mov, and we record the fact that
 * the register contains the value of another register.
 */
void optimize_register_mov(instruction_t* instruction, int src_name, size_t horizon) {

    // This instruction must write to arg0
    assert((opcode_style(instruction_opcode(instruction)) == STYLE_REG_MIX) |
        ((opcode_style(instruction_opcode(instruction)) == STYLE_REG_CON) |
        (opcode_style(instruction_opcode(instruction)) == STYLE_REG)));

    // If this instruction effectively replaces a register with itself, delete
    // it. (e.g. `divu r0 r0 1` is really `mov r0 r0` which is useless; it is
    // deleted here.)
    int arg0 = instruction_arg0(instruction);
    if (arg0 == src_name) {
        instruction_set_opcode(instruction, OP_NOP);
        return;
    }

    // Check if the last assignment to the destination register was already a
    // mov from the source register. If it was, and we assigned later than the
    // horizon, and the source register hasn't been modified since, this
    // instruction is redundant.
    reg_t* dest = register_get(arg0);
    if (register_content_type(dest) == REGISTER_CONTENT_REGISTER) {
        if (register_value(dest) == src_name) {
            if (instruction_index(register_instruction(dest)) > horizon) {
                instruction_t* src = register_instruction(register_get(src_name));
                size_t src_index = 0;
                if (src) {
                    src_index = instruction_index(src);
                }
                if (src_index < instruction_index(register_instruction(dest))) {
                    instruction_set_opcode(instruction, OP_NOP);
                    return;
                }
            }
        }
    }

    // Turn this into a mov
    instruction_set_opcode(instruction, OP_MOV);
    instruction_set_arg1(instruction, src_name);

    // Record the fact that this register contains the contents of another
    // register for later optimizations.
    register_set_register(dest, src_name, instruction);
}

/*
 * Performs constant folding.
 *
 * If the instruction writes a constant value to a register, we record that the
 * register contains that value for later optimizations. Furthurmore, if the
 * resulting instruction would be shorter or takes register arguments, it is
 * replaced with IMW or MOV in order to eliminate register dependencies and
 * minimize bytecode.
 *
 * If the instruction performs arithmetic with a constant that has the effect
 * of assigning another register to a destination register, it is replaced with
 * MOV, eliminating the unnecessary constant argument. (For example `divs r0 r1
 * 1` is replaced by `mov r0 r1`, eliminating the constant.)
 *
 * This also replaces jnz and jz with jmp if the predicate is a constant, and
 * simplifies some other bitwise operations with constants (e.g. xor -1 is
 * replaced with not.)
 *
 * This optimization relies on constant propagation having replaced any
 * register arguments with constants. Unfortunately that means it only works
 * when the input constants fit in a mix-type byte.
 */
static void optimize_constant_fold(instruction_t* instruction, size_t horizon) {
    opcode_t opcode = instruction_opcode(instruction);
    if (opcode <= OP_VIRTUAL_MAX) {
        return;
    }

    if (opcode <= OP_ARITHMETIC_MAX) {

        if (opcode == OP_ADD) {
            int arg1 = instruction_arg1(instruction);
            int arg2 = instruction_arg2(instruction);
            if (!is_register(arg1) & !is_register(arg2)) {
                // arguments are constants
                optimize_constant(instruction, mix_to_int(arg1) + mix_to_int(arg2), false, horizon);
                return;
            }
            if (arg1 == 0) {
                // first argument is zero, second is register
                optimize_register_mov(instruction, arg2, horizon);
                return;
            }
            if (arg2 == 0) {
                // second argument is zero, first is register
                optimize_register_mov(instruction, arg1, horizon);
            }
            return;
        }

        if (opcode == OP_SUB) {
            int arg1 = instruction_arg1(instruction);
            int arg2 = instruction_arg2(instruction);
            if (!is_register(arg1) & !is_register(arg2)) {
                // arguments are constants
                optimize_constant(instruction, mix_to_int(arg1) - mix_to_int(arg2), false, horizon);
                return;
            }
            if (arg2 == 0) {
                // second argument is zero, first is register
                optimize_register_mov(instruction, arg1, horizon);
            }
            return;
        }

        if (opcode == OP_MUL) {
            int arg1 = instruction_arg1(instruction);
            int arg2 = instruction_arg2(instruction);
            if (!is_register(arg1) & !is_register(arg2)) {
                // arguments are constants
                optimize_constant(instruction, mix_to_int(arg1) * mix_to_int(arg2), false, horizon);
                return;
            }
            if ((arg1 == 0) | (arg2 == 0)) {
                // an argument is zero; result is zero
                optimize_constant(instruction, 0, true, horizon);
                return;
            }
            if (arg1 == 1) {
                // first argument is one, second is register
                optimize_register_mov(instruction, arg2, horizon);
                return;
            }
            if (arg2 == 1) {
                // second argument is one, first is register
                optimize_register_mov(instruction, arg1, horizon);
            }
            return;
        }

        if ((opcode == OP_DIVU) | (opcode == OP_DIVS)) {
            int arg1 = instruction_arg1(instruction);
            int arg2 = instruction_arg2(instruction);
            if (!is_register(arg1) & !is_register(arg2)) {
                // arguments are constants
                // TODO we need unsigned in cci/0. in the meantime we can't
                // constant fold divu.
                if (opcode != OP_DIVU) {
                    optimize_constant(instruction, mix_to_int(arg1) / mix_to_int(arg2), false, horizon);
                    return;
                }
            }
            if (arg1 == 0) {
                // dividend is 0. quotient is always 0.
                optimize_constant(instruction, 0, true, horizon);
                return;
            }
            if (arg2 == 1) {
                // divisor is 1. quotient is always the dividend (which is a
                // register.)
                optimize_register_mov(instruction, arg1, horizon);
            }
            return;
        }

        // TODO we don't constant fold modu/mods yet, we could but there's
        // probably no point right now, simpler to add it after we add unsigned
        // to cci/0

        if (opcode == OP_ZERO) {
            // Record the fact that this register contains zero for later
            // optimizations.
            reg_t* dest = register_get(instruction_arg0(instruction));
            register_set_constant(dest, instruction_immediate(instruction), instruction);
            return;
        }

        // nothing we can do for inc and dec

        // TODO eventually we should remove sxs and trs. cci/1 supports them so
        // we keep them for now. but if we upgrade cci/0 and downgrade cci/2
        // enough to skip cci/1, probably this would be done by adding short
        // in cci/2 rather than in cci/0, so there would be no need for these
        // instructions in cg/0.

        if ((opcode == OP_SXS) | (opcode == OP_SXB)) {
            int arg1 = instruction_arg1(instruction);
            if (!is_register(arg1)) {
                // argument is constant
                optimize_constant(instruction, mix_to_int(arg1), false, horizon);
                return;
            }
        }

        if ((opcode == OP_TRS) | (opcode == OP_TRB)) {
            int arg1 = instruction_arg1(instruction);
            if (!is_register(arg1)) {
                // argument is constant
                int mask;
                if (opcode == OP_TRS) {
                    mask = 0xFFFF;
                }
                if (opcode != OP_TRS) {
                    mask = 0xFF;
                }
                optimize_constant(instruction, mix_to_int(arg1) & mask, false, horizon);
                return;
            }
        }

        return;
    }

    if (opcode <= OP_LOGIC_MAX) {

        // TODO it would be nice to clean up some of the duplication here.
        // Maybe we should improve our "style" enum to handle all reg-mix-mix
        // instructions together. This would also make this code run faster
        // since we don't have a switch.

        if (opcode == OP_AND) {
            int arg1 = instruction_arg1(instruction);
            int arg2 = instruction_arg2(instruction);
            if (!is_register(arg1) & !is_register(arg2)) {
                // arguments are constants
                optimize_constant(instruction, mix_to_int(arg1) & mix_to_int(arg2), false, horizon);
                return;
            }
            if ((arg1 == 0) | (arg2 == 0)) {
                // either argument is zero, result is zero
                optimize_constant(instruction, 0, true, horizon);
            }
            if (!is_register(arg1)) {
                // TODO the mix_to_int() here is redundant, we should be able
                // to just check against 0xFF, same with all the other checks
                // below. The problem is we don't have unsigned in cci/0 and
                // char is signed, if any char conversions happen in our args
                // they would sign extend. This needs to be fixed, that should
                // never happen, and later we'll make char unsigned anyway.
                if (mix_to_int(arg1) == (int)0xFFFFFFFF) {
                    // first argument is -1, second is register
                    optimize_register_mov(instruction, arg2, horizon);
                }
                return;
            }
            if (!is_register(arg2)) {
                if (mix_to_int(arg2) == (int)0xFFFFFFFF) {
                    // second argument is -1, first is register
                    optimize_register_mov(instruction, arg1, horizon);
                }
            }
            return;
        }

        if (opcode == OP_OR) {
            int arg1 = instruction_arg1(instruction);
            int arg2 = instruction_arg2(instruction);
            if (!is_register(arg1) & !is_register(arg2)) {
                // arguments are constants
                optimize_constant(instruction, mix_to_int(arg1) | mix_to_int(arg2), false, horizon);
            }
            if (!is_register(arg1)) {
                if (arg1 == 0) {
                    // first argument is 0, result is second argument register
                    optimize_register_mov(instruction, arg2, horizon);
                    return;
                }
                arg1 = mix_to_int(arg1);
                if (arg1 == (int)0xFFFFFFFF) {
                    // first argument is -1, result is -1
                    optimize_constant(instruction, 0xFFFFFFFF, true, horizon);
                }
                return;
            }
            if (!is_register(arg2)) {
                if (arg2 == 0) {
                    // second argument is 0, result is first argument register
                    optimize_register_mov(instruction, arg1, horizon);
                    return;
                }
                arg2 = mix_to_int(arg2);
                if (arg2 == (int)0xFFFFFFFF) {
                    // second argument is -1, result is -1
                    optimize_constant(instruction, 0xFFFFFFFF, true, horizon);
                }
            }
            return;
        }

        if (opcode == OP_XOR) {
            int arg1 = instruction_arg1(instruction);
            int arg2 = instruction_arg2(instruction);
            if (!is_register(arg1) & !is_register(arg2)) {
                // arguments are constants
                optimize_constant(instruction, mix_to_int(arg1) ^ mix_to_int(arg2), false, horizon);
            }
            if (!is_register(arg1)) {
                if (arg1 == 0) {
                    // first argument is 0, second is register
                    optimize_register_mov(instruction, arg2, horizon);
                    return;
                }
                if (mix_to_int(arg1) == (int)0xFFFFFFFF) {
                    // first argument is -1, second is register. not is shorter than xor.
                    instruction_set_opcode(instruction, OP_NOT);
                    instruction_set_arg1(instruction, arg2);
                }
            }
            if (!is_register(arg2)) {
                if (arg2 == 0) {
                    // second argument is 0, first is register
                    optimize_register_mov(instruction, arg1, horizon);
                    return;
                }
                if (mix_to_int(arg2) == (int)0xFFFFFFFF) {
                    // second argument is -1, first is register. not is shorter than xor.
                    instruction_set_opcode(instruction, OP_NOT);
                }
            }
            return;
        }

        if (opcode == OP_NOT) {
            int arg1 = instruction_arg1(instruction);
            if (!is_register(arg1)) {
                // argument is constant
                optimize_constant(instruction, ~mix_to_int(arg1), false, horizon);
            }
            return;
        }

        if (opcode == OP_SHL) {
            int arg1 = instruction_arg1(instruction);
            int arg2 = instruction_arg2(instruction);
            if (!is_register(arg1) & !is_register(arg2)) {
                // arguments are constants
                if (arg2 & ~0x1f) {
                    fatal("A shl instruction shifts by a constant outside the range 0-31.");
                }
                optimize_constant(instruction, mix_to_int(arg1) << arg2, false, horizon);
                return;
            }
            if (arg2 == 0) {
                // first is register, second is zero. no shift.
                optimize_register_mov(instruction, arg1, horizon);
            }
            return;
        }

        if (opcode == OP_SHRU) {
            int arg1 = instruction_arg1(instruction);
            int arg2 = instruction_arg2(instruction);
            if (!is_register(arg1) & !is_register(arg2)) {
                // arguments are constants
                if (arg2 & ~0x1f) {
                    fatal("A shrs instruction shifts by a constant outside the range 0-31.");
                }
                // TODO we don't do shru yet, need unsigned
                //optimize_constant(instruction, (unsigned)mix_to_int(arg1) << arg2, false, horizon);
                // in the meantime just optimize zero
                if (arg2 == 0) {
                    optimize_constant(instruction, mix_to_int(arg1), false, horizon);
                }
                return;
            }
            if (arg2 == 0) {
                // first is register, second is zero. no shift.
                optimize_register_mov(instruction, arg1, horizon);
            }
            return;
        }

        if (opcode == OP_SHRS) {
            int arg1 = instruction_arg1(instruction);
            int arg2 = instruction_arg2(instruction);
            if (!is_register(arg1) & !is_register(arg2)) {
                // arguments are constants
                if (arg2 & ~0x1f) {
                    fatal("A shrs instruction shifts by a constant outside the range 0-31.");
                }
                optimize_constant(instruction, mix_to_int(arg1) >> arg2, false, horizon);
                return;
            }
            if (arg2 == 0) {
                // first is register, second is zero. no shift.
                optimize_register_mov(instruction, arg1, horizon);
            }
            return;
        }

        // TODO no rol/ror, shouldn't even parse them, cci/0 and cci/1 don't
        // emit them. probably even remove them from as/1, we will only emit
        // them from cci/2 from rotate builtins or optimizations

        if (opcode == OP_MOV) {
            // Record the fact that the destination register contains another
            // register or a constant for later optimizations.
            int arg1 = instruction_arg1(instruction);
            if (is_register(arg1)) {
                optimize_register_mov(instruction, arg1, horizon);
                return;
            }
            optimize_constant(instruction, mix_to_int(arg1), false, horizon);
            return;
        }

        if (opcode == OP_BOOL) {
            int arg1 = instruction_arg1(instruction);
            if (!is_register(arg1)) {
                // argument is constant
                optimize_constant(instruction, !!arg1, false, horizon);
                return;
            }
            return;
        }

        if (opcode == OP_ISZ) {
            int arg1 = instruction_arg1(instruction);
            if (!is_register(arg1)) {
                // argument is constant
                optimize_constant(instruction, !arg1, false, horizon);
                return;
            }
            return;
        }

        return;
    }

    if (opcode <= OP_MEMORY_MAX) {
        // none of these instructions can do constant folding.
        return;
    }

    // the rest are control instructions.

    if (opcode == OP_IMW) {
        if (instruction_label(instruction) == NULL) {
            // Pass to optimize_constant(). This will change it to MOV if it
            // fits in a mix-type byte and records it for later optimizations.
            optimize_constant(instruction, instruction_immediate(instruction), false, horizon);
        }
        return;
    }

    if (opcode == OP_LTU) {
        int arg2 = instruction_arg2(instruction);
        if (arg2 == 0) {
            // result is always false
            optimize_constant(instruction, 0, false, horizon);
            return;
        }
        int arg1 = instruction_arg1(instruction);
        if (arg1 == 0xFF) {
            // result is always false
            optimize_constant(instruction, 0, true, horizon);
            return;
        }
        if (!is_register(arg1) & !is_register(arg2)) {
            // arguments are constants
            arg1 = mix_to_int(arg1);
            arg2 = mix_to_int(arg2);
            // TODO I need to add unsigned to cci/0, this is totally ridiculous
            arg1 = (arg1 - (1 << 31));
            arg2 = (arg2 - (1 << 31));
            optimize_constant(instruction, arg1 < arg2, true, horizon);
        }
        return;
    }

    if (opcode == OP_LTS) {
        int arg1 = instruction_arg1(instruction);
        int arg2 = instruction_arg2(instruction);
        if (!is_register(arg1) & !is_register(arg2)) {
            // arguments are constants
            arg1 = mix_to_int(arg1);
            arg2 = mix_to_int(arg2);
            optimize_constant(instruction, arg1 < arg2, true, horizon);
        }
        return;
    }

    if ((opcode == OP_JZ) | (opcode == OP_JNZ)) {
        int pred = instruction_arg0(instruction);
        if (!is_register(pred)) {
            if (opcode == OP_JZ) {
                pred = !pred;
            }
            if (pred) {
                // result is always true
                instruction_set_opcode(instruction, OP_JMP);
            }
            if (!pred) {
                // result is always false
                instruction_set_opcode(instruction, OP_NOP);
            }
        }
        return;
    }

    // no optimizations possible for jmp, call, ret, enter, leave
}

/**
 * This is called on instructions that take two arguments and add them
 * together, i.e. add, ld* and st*. It's also called for mov because it can be
 * changed to add.
 *
 * TODO the inline addition optimization is pretty limited at the moment. It
 * can't optimize this:
 *
 *     add ra rpp ra
 *     ldw r0 ra 0
 *
 * The reason is because ra is both read and written by the add instruction.
 * Probably it would need to reorder them into this:
 *
 *     ldw r0 rpp ra
 *     add ra rpp ra
 *
 * This isn't implemented yet. Probably we can do this safely as long as there
 * are no instructions in between (or, as long as there are no instructions
 * that touch r0 in between. It's not like we make any promises about memory
 * ordering, and we don't support fences or anything at this stage; as long as
 * we check that none of the used registers change in between, we're safe to
 * swap.)
 */
static void optimize_inline_addition(instruction_t* target, opcode_t opcode, size_t horizon) {

    // We need one of our arguments to be a register and the other to be a
    // plain zero. (We could improve this to handle a register plus a constant
    // in some cases but it would complicate this too much so it's not worth
    // doing.)
    int reg;
    int zero;

    // If this is a MOV, it's implicitly an add of a register and zero.
    if (opcode == OP_MOV) {
        reg = instruction_arg1(target);
        zero = 0;
    }

    // Otherwise we have two arguments.
    // We permute the args if necessary. If both are constants we skip it; it
    // will be handled by constant folding.
    if (opcode != OP_MOV) {
        reg = instruction_arg1(target);
        zero = instruction_arg2(target);
        if (is_register(zero)) {
            if (is_register(reg)) {
                return;
            }
            int temp = reg;
            reg = zero;
            zero = temp;
        }
    }

    if (!is_register(reg)) {
        return;
    }
    if (zero != 0) {
        return;
    }

    // Find the last instruction that wrote to the source register.
    instruction_t* src = register_instruction(register_get(reg));
    if (!src) {
        return;
    }

    // If we can't find one since the horizon, we can't use it.
    size_t src_index = instruction_index(src);
    if (src_index < horizon) {
        return;
    }

    // If it's not an add instruction, we can't use it.
    if (instruction_opcode(src) != OP_ADD) {
        return;
    }

    // We need to make sure each register argument has not been written to
    // in or after this instruction. If they have, we can't use it.
    int i = 1;
    while (i <= 2) {
        int arg = instruction_arg(src, i);
        if (is_register(arg)) {
            instruction_t* instruction = register_instruction(register_get(arg));
            if (instruction) {
                if (instruction_index(instruction) >= src_index) {
                    return;
                }
            }
            assert(arg != reg); // shouldn't be possible, index check should catch it
        }
        i = (i + 1);
    }

    // Success! Substitute the arguments.
    //printf("Found an add substitution for: "); instruction_print(target);
    if (opcode == OP_MOV) {
        instruction_set_opcode(target, OP_ADD);
    }
    instruction_set_arg1(target, instruction_arg1(src));
    instruction_set_arg2(target, instruction_arg2(src));
    //printf("Add substitution result: "); instruction_print(target);
}

/**
 * For each input register, check if we can replace it with a known value.
 *
 * If the input register is an alias of another register, and the register
 * hasn't been modified since, replace it with that register.
 *
 * If the input register is a constant, and it fits in a mix-type byte, replace
 * it with that constant. Unfortunately this means constant propagation will
 * only work for constants that fit in a mix-type byte.
 */
static void optimize_propagate_single_inputs(instruction_t* instruction, size_t horizon) {
    opcode_t opcode = instruction_opcode(instruction);
    style_t style = opcode_style(opcode);
    int i = 0;

    if (opcode <= OP_VIRTUAL_MAX) {
        return;
    }

    // imw takes an immediate input and no mix-type inputs.
    if (opcode == OP_IMW) {
        return;
    }

    // If the first argument is a destination register, skip it.
    if (((style == STYLE_REG_MIX) | (style == STYLE_IN_PLACE)) |
            ((style == STYLE_REG_CON) | (style == STYLE_REG)))
    {
        i = 1;
    }

    // If we have a label, we have one less mix-type argument.
    int argcount = opcode_argcount(opcode);
    if (instruction_label(instruction)) {
        argcount = (argcount - 1);
    }

    // For each input argument, if it's a register, see if we can replace it.
    while (i < argcount) {
        int arg = instruction_arg(instruction, i);
        if (!is_register(arg)) {
            i = (i + 1);
            continue;
        }

        // Make sure the register was set after the horizon
        reg_t* reg = register_get(arg);
        instruction_t* src_instruction = register_instruction(reg);
        if (!src_instruction) {
            i = (i + 1);
            continue;
        }
        if (instruction_index(src_instruction) <= horizon) {
            i = (i + 1);
            continue;
        }

        int type = register_content_type(reg);

        if (type == REGISTER_CONTENT_CONSTANT) {
            int value = register_value(reg);
            if ((value >= -0x70) & (value <= 0x7F)) {
                // Register contains a constant that fits. Replace it.
                instruction_set_arg(instruction, i, value & 0xFF);
            }
        }

        if (type == REGISTER_CONTENT_REGISTER) {
            int src_name = register_value(reg);
            instruction_t* src = register_instruction(register_get(src_name));
            size_t src_index = 0;
            if (src) {
                src_index = instruction_index(src);
            }
            if (src_index < instruction_index(register_instruction(reg))) {
                // Register contains another register that has not been
                // modified. Replace it.
                instruction_set_arg(instruction, i, src_name);
            }
        }

        i = (i + 1);
        continue;
    }
}

static void optimize_forward(void) {
    registers_clear();
    size_t horizon = 0;
    bool dead = false;

    size_t i = 0;
    while (i < instructions_count) {
        instruction_t* instruction = *(instructions + i);
        i = (i + 1);

        //puts("");
        //instruction_print(instruction);

        opcode_t opcode = instruction_opcode(instruction);

        // When we reach a label, intra-block dead code elimination ends.
        if (opcode == OP_DECLARATION) {
            if (!dead) {
                // We've fallen through to this block. Mark it used.
                instruction_set_opt_int(instruction, 1);
            }
            dead = false;
        }

        // Otherwise, if we're eliminating dead code, remove it.
        if (dead) {
            if (opcode != OP_DECLARATION) {
                //printf("eliminating instruction dead code\n");
                instruction_set_opcode(instruction, OP_NOP);
                continue;
            }
        }

        // Label declarations and call instructions change our horizon
        if ((opcode == OP_DECLARATION) | (opcode == OP_CALL)) {
            horizon = instruction_index(instruction);
        }

        // If this function takes two inputs that it adds together, see if we
        // can replace both. (Optimizes add, load and store instructions.)
        if (opcode_adds(opcode) | (opcode == OP_MOV)) {
            optimize_inline_addition(instruction, opcode, horizon);
        }

        // For each input of this instruction, see if we can replace that input.
        optimize_propagate_single_inputs(instruction, horizon);

        // Perform constant folding and related optimizations (e.g. `jnz 1` -> `jmp`)
        optimize_constant_fold(instruction, horizon);

        // Optimize instructions that have register outputs.
        // (opcode and style might have changed due to above optimizations so
        // we need to get them again.)
        // TODO put this in a function or change style to an output flag or something, this is copy-pasted in a few places
        opcode = instruction_opcode(instruction);
        style_t style = opcode_style(opcode);
        if (((style == STYLE_REG_MIX) | (style == STYLE_IN_PLACE)) |
                ((style == STYLE_REG_CON) | (style == STYLE_REG)))
        {
            // If the instruction hasn't been eliminated and we haven't
            // already recorded what it does to its destination register,
            // record the fact that the destination register now contains an
            // unknown value.
            if (opcode != OP_NOP) {
                reg_t* dest = register_get(instruction_arg0(instruction));
                if (register_instruction(dest) != instruction) {
                    register_set_unknown(dest, instruction);
                }
            }
        }

        // If the final instruction has a label, and that label references a
        // block in this function, mark the block used.
        if ((opcode != OP_DECLARATION) & (opcode != OP_STRING)) {
            const char* label = instruction_label(instruction);
            if (label) {
                size_t bucket = (fnv1a_cstr(label) & (BLOCKS_BUCKETS - 1));
                //printf("found label %s, looking in bucket %zi\n", label, bucket);
                instruction_t* block = *(blocks + bucket);
                while (block) {
                    if (0 == strcmp(label, instruction_label(block))) {
                        // Mark the block used.
                        //printf("found block %s, marking used: ", label); instruction_print(block);
                        instruction_set_opt_int(block, 1);
                        break;
                    }
                    block = instruction_opt_vp(block);
                }
            }
        }

        // If the final instruction is a jmp or ret, the rest of this block is
        // unreachable.
        if ((opcode == OP_JMP) | (opcode == OP_RET)) {
            dead = true;
        }
    }
}



/**
 * Backwards optimizations
 *
 * The backward optimization pass performs dead store elimination and trivial
 * jump optimizations.
 */

/**
 * Perform dead store elimination on this instruction.
 */
static void optimize_dead_store(instruction_t* instruction) {
    opcode_t opcode = instruction_opcode(instruction);

    // Jumps may be passing along any registers so we need to preserve all of
    // them.
    if ((opcode >= OP_JZ) & (opcode <= OP_JMP)) {
        registers_set_used_up_to(9);
        return;
    }

    // A call instruction may use registers r0-r3. Other registers are not
    // used in the standard call convention. (This is another reason this
    // optimizer is only safe to run on code generated by cci/0 and cci/1.)
    if (opcode == OP_CALL) {
        registers_set_used_up_to(3);
        return;
    }

    // Functions only return values in r0.
    if (opcode == OP_RET) {
        registers_set_used_up_to(0);
        return;
    }

    size_t i = 0;

    // Handle instructions that write to a register.
    style_t style = opcode_style(opcode);
    if (((style == STYLE_REG_MIX) | (style == STYLE_IN_PLACE)) |
            ((style == STYLE_REG_CON) | (style == STYLE_REG)))
    {
        int arg0 = instruction_arg0(instruction);
        i = (i + 1);

        // We only care about writes to numbered registers.
        if (is_register(arg0)) {
            if (arg0 <= 0x89) {

                // If the output register is unused, eliminate it.
                if (!register_used(register_get(arg0))) {
                    instruction_set_opcode(instruction, OP_NOP);
                    return;
                }

                // Otherwise, as long as this is not an in-place instruction, mark the
                // output register as unused. (Since we are writing to it, anything that
                // writes to it before this is a dead store.)
                reg_t* reg = register_get(arg0);
                if (style == STYLE_IN_PLACE) {
                    register_set_used(reg, true);
                    return;
                }
                register_set_used(reg, false);
            }
        }
    }

    // These instructions don't have input registers.
    if (opcode <= OP_VIRTUAL_MAX) {
        return;
    }
    if (opcode == OP_IMW) {
        // skip imw in case we change it later to not store its immediate in arg1
        return;
    }

    // Any remaining arguments are inputs. Mark any that are registers as used.
    // (If one of these matches the output register, this will make it used
    // again.)
    size_t argcount = opcode_argcount(opcode);
    if (instruction_label(instruction)) {
        argcount = (argcount - 1);
    }
    while (i < argcount) {
        int arg = instruction_arg(instruction, i);
        if (is_register(arg)) {
            register_set_used(register_get(arg), true);
        }
        i = (i + 1);
    }
}

static void optimize_backward(void) {
    //instructions_print();
    registers_clear();

    size_t i = instructions_count;
    while (i > 0) {
        i = (i - 1);
        instruction_t* instruction = *(instructions + i);

        //puts("");
        //instruction_print(instruction);

        // Perform dead store elimination
        optimize_dead_store(instruction);

        // Perform trivial jump elimination. If this is a label and the
        // previous instruction is a jmp to it, we can eliminate the jmp.
        if (instruction_opcode(instruction) == OP_DECLARATION) {
            size_t j = i;
            while (j > 1) {
                j = (j - 1);
                instruction_t* previous = *(instructions + j);
                opcode_t opcode = instruction_opcode(previous);
                if ((opcode == OP_NOP) | (opcode == OP_DECLARATION)) {
                    i = j;
                    continue;
                }
                if (instruction_opcode(previous) == OP_JMP) {
                    if (0 == strcmp(instruction_label(instruction), instruction_label(previous))) {
                        i = j;
                        instruction_set_opcode(previous, OP_NOP);
                    }
                }
                break;
            }
        }
    }
}



/**
 * Eliminates a block.
 *
 * All instructions from this label to the next are changed to NOP.
 */
static void optimize_eliminate_block(instruction_t* instruction) {
    //printf("eliminating block %s\n", instruction_label(instruction));
    size_t i = instruction_index(instruction);
    while (1) {
        instruction_set_opcode(instruction, OP_NOP);
        i = (i + 1);
        if (i == instructions_count) {
            break;
        }
        instruction = *(instructions + i);
        if (instruction_opcode(instruction) == OP_DECLARATION) {
            break;
        }
    }
}

/*
 * Block elimination pass
 *
 * This iterates through all blocks and deletes any that were not marked used
 * during the forward propagation pass.
 *
 * Label declaration instructions are stored in the `blocks` hashtable. The
 * instruction's opt_int is the used flag and the opt_vp pointer is the next
 * block in the hashtable collision sequence.
 */
static void optimize_blocks(void) {
    size_t bucket = 0;
    while (bucket < BLOCKS_BUCKETS) {
        instruction_t* block = *(blocks + bucket);
        while (block) {
            if (!instruction_opt_int(block)) {
                optimize_eliminate_block(block);
            }
            block = instruction_opt_vp(block);
        }
        bucket = (bucket + 1);
    }
}



static void optimize_setup(void) {
}

static void optimize_teardown(void) {
}

#endif
