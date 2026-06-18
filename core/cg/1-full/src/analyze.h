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
 * Functions to perform analysis on the IR.
 */

#ifndef ANALYZE_H_INCLUDED
#define ANALYZE_H_INCLUDED

struct symbol_t;
struct otable_t;
struct instruction_t;

/**
 * Updates the given live_temps table with liveness changes made by this
 * instruction.
 */
void analyze_liveness_instruction(struct otable_t* live_temps, struct instruction_t* instruction);

/**
 * Perform a liveness analysis of temporaries.
 *
 * Once complete, each block's live_temps will contain a complete set of all
 * temporaries that may be alive at the end of the block.
 */
void analyze_liveness(struct symbol_t* symbol);

#endif
