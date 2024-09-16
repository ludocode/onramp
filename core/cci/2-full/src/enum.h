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

#ifndef ENUM_H_INCLUDED
#define ENUM_H_INCLUDED

struct token_t;

/**
 * An enum definition.
 *
 * Values of the enum are collected here for the purpose of generating warnings
 * (for example, `-Wswitch` ensures that all enum values appear in a switch.)
 *
 * Eventually we also aim to support C23 enums with fixed underlying types.
 */
typedef struct enum_t {
    unsigned refcount;
    struct token_t* tag; // null if anonymous

    // TODO underlying type

    // TODO collect values
    //symbol_t** values;
    //size_t values_count;
} enum_t;

enum_t* enum_new(struct token_t* /*nullable*/ tag);

static inline enum_t* enum_ref(enum_t* enum_) {
    ++enum_->refcount;
    return enum_;
}

void enum_deref(enum_t* enum_);

//void enum_set_values(symbol_t** values, size_t values_count);

#endif
