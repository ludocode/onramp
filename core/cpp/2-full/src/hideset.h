/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Fraser Heavy Software
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

#ifndef HIDESET_H_INCLUDED
#define HIDESET_H_INCLUDED

#include "libo-table.h"
#include "libo-string.h"

/**
 * A hideset.
 *
 * The hideset of a token contains the set of names of macros that were used in
 * the expansion of that token. This prevents tokens from being recursively
 * expanded.
 *
 * The hideset is immutable and reference counted so it can be shared.
 *
 * The hideset is the same for every token expanded from a macro. For this
 * reason we generate a hideset at the start of macro expansion and share it
 * with each token that is expanded.
 */
typedef struct hideset_t {
    unsigned refcount;

    // TODO we should make an open hashtable in libo. there's a lot of overhead
    // here in both code (due to manually walking buckets) and runtime (due to
    // all the entry memory allocations.) the libo closed table is good for big
    // tables (like the intern string table) but for tables that are expected
    // to be small, open is probably better.)
    table_t table;
} hideset_t;

static inline hideset_t* hideset_ref(hideset_t* hideset) {
    ++hideset->refcount;
    return hideset;
}

void hideset_deref(hideset_t* hideset);

/**
 * Creates a new hideset by cloning the given hideset (if non-null) and adding
 * the given string to it.
 */
hideset_t* hideset_new(hideset_t* /*nullable*/ old, string_t* string);

/**
 * Creates a new hideset by cloning the given hideset, intersecting it with the
 * second given hideset (intended for the closing parenthesis of a
 * function-like macro invocation), and adding the given string to it.
 *
 * If either hideset is null, it is considered the same as empty, and the
 * resulting hideset will contain only the given string. If both hidesets are
 * non-null, the result is the intersection of the hidesets plus the given
 * string.
 */
hideset_t* hideset_new_intersection(hideset_t* /*nullable*/ old, hideset_t*
        /*nullable*/ closing_paren, string_t* string);

bool hideset_contains(hideset_t* hideset, string_t* name);

#endif
