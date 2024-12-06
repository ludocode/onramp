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

#include "expression.h"

#include <stdint.h>

#include "stream.h"

typedef struct value_t {
    bool is_signed;
    union {
        int64_t s;  // signed
        uint64_t u; // unsigned
    };
} value_t;

static void expression_parse_primary(stream_t* stream, value_t* out) {
    // defined, parens, literal number, literal char.
}

static void expression_parse_unary(stream_t* stream, value_t* out) {
    (void)expression_parse_primary;
}

static void expression_parse_binary(stream_t* stream, value_t* out) {
    (void)expression_parse_unary;
}

static void expression_parse_conditional(stream_t* stream, value_t* out) {
    // TODO
    (void)expression_parse_binary;
}

bool expression_evaluate(stream_t* stream) {
    value_t value;
    expression_parse_conditional(stream, &value);
    return !!value.u;
}
