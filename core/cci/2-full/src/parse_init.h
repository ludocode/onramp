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

#ifndef PARSE_INIT_H_INCLUDED
#define PARSE_INIT_H_INCLUDED

struct type_t;
struct node_t;

/**
 * Parses an initializer for the given type.
 *
 * The initializer can be an expression or an initializer list.
 *
 * In the case that the initializer is a string or character literal, we just
 * parse it as an expression. The reason for this is because GCC allows
 * parenthesized string literal initializers for char arrays as an extension
 * (though it warns about it under -Wpedantic.) For example:
 *
 *     char x[] = ("foo");
 *
 * This is not legal C but GCC accepts it so we do too. We do type checking on
 * the initializer after parsing.
 */
struct node_t* parse_initializer(struct type_t* type);

#endif
