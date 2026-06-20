/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025-2026 Fraser Heavy Software
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

struct block_t;
struct vector_t;
struct location_t;

/**
 * All data for the current symbol being compiled.
 *
 * This is mostly gathered together as a convenience so we don't have stuff
 * scattered everywhere.
 *
 * TODO actually currently we are using globals, this is pretty useless
 */
typedef struct symbol_t {
    char* name;
    struct location_t* location;
    struct vector_t* blocks;
    size_t frame_size;
} symbol_t;

//extern symbol_t* current_symbol;

/**
 * Creates a symbol.
 *
 * Takes ownership of the given location.
 */
symbol_t* symbol_new(const char* name, struct location_t* location);

void symbol_delete(symbol_t* symbol);

#endif
