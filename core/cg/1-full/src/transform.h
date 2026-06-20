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

/*
 * Functions to transform IR into assembly.
 */

#ifndef TRANSFORM_H_INCLUDED
#define TRANSFORM_H_INCLUDED

struct symbol_t;

/**
 * Inserts instructions to convert all parameters from IR to assembly.
 */
void transform_parameters(struct symbol_t* symbol);

/**
 * Convert all `var` instructions and function parameters to variables.
 */
void transform_vars(struct symbol_t* symbol);

/**
 * Convert the entry point of the function to assembly.
 *
 * This adds `enter` and a `sub rsp` instruction to allocate stack space for
 * all variables.
 */
void transform_entry(struct symbol_t* symbol);

/**
 * Convert all `ret` and `br` instructions from IR to assembly.
 *
 * `ret` instructions place their argument in r0 (if any) and have `leave`
 * inserted. `br` instructions are converted to a `jz+jmp` pair.
 *
 * After this is run, all blocks end in `jmp` or `ret`.
 */
void transform_control_flow(struct symbol_t* symbol);

/**
 * Convert all temporaries to registers and all variables to constants,
 * inserting spill instructions where necessary.
 *
 * - Temporaries that have been assigned a register are replaced by their
 *   register.
 *
 * - Temporaries that are spilled are replaced by `r0` or `r1`, inserting `ldw`
 *   and `stw` instructions.
 *
 * - Variables whose offset fits in a mix-type byte are replaced by their
 *   offset in-place.
 *
 * - Variables whose offset does not fit in a mix-type byte are replaced by
 *   `r0` or `r1`, inserting an `imw` instruction.
 */
void transform_registers(struct symbol_t* symbol);

#endif
