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
 * instruction.h) and manipulates it in place. Each pass is independent and
 * self-contained; it takes in valid assembly and it must leave the array as
 * valid assembly with observable behaviour unchanged.
 */

#include <stdbool.h>

#include "register.h"



/*
 * Stack push/pop elimination
 *
 * (replace push/pop pairs with mov into external register)
 */

static void optimize_push_pop(void) {
}



/*
 * Redundant load optimization
 *
 * (replace unnecessary ldw with mov)
 */

static void optimize_load(void) {
}



/*
 * Constant propagation
 */

static void optimize_propagate(void) {
}



/*
 * Leaf stack frame elimination
 *
 * Eliminates stack frames from branchless leaf functions that do not require
 * stack space.
 */

static void optimize_leaf(void) {
}



/*
 * Dead store elimination
 */



// TODO: It would be better if each pass just allocated whatever memory it
// needs while it's running. libc/0 doesn't reclaim memory yet so for now we
// allocate all this up front.

static void optimize_setup(void) {
}

static void optimize_teardown(void) {
}

#endif
