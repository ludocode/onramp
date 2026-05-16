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

#ifndef SYMBOL_H_INCLUDED
#define SYMBOL_H_INCLUDED

#include <stdbool.h>

#include "common.h"

void symbol_setup(void);

void symbol_teardown(void);

void symbol_clear(void);

/**
 * Returns true if the current position is correctly aligned for an instruction.
 */
bool symbol_is_aligned(void);

/**
 * Appends the given bytes as hexadecimal to the current symbol.
 */
void symbol_add_hex_bytes(const uint8_t* bytes, size_t count);

/**
 * Appends the given byte as hexadecimal to the current symbol.
 */
void symbol_add_hex_byte(uint8_t byte);

void symbol_add_byte(uint8_t byte);

/**
 * Appends the given label to the current symbol.
 */
void symbol_add_label(const char* name, label_type_t type, int flags);

/**
 * Emits the symbol, resolving any relative labels.
 */
void symbol_emit(void);

#endif
