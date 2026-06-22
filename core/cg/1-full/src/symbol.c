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

#include "symbol.h"

#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "libo-vector.h"
#include "location.h"

symbol_t* symbol_new(const char* name, location_t* location, bool is_static) {
    symbol_t* symbol = calloc(1, sizeof(symbol_t));
    symbol->name = strdup(name);
    symbol->location = location;
    symbol->is_static = is_static;
    symbol->parameters = vector_new();
    symbol->blocks = vector_new();
    symbol->frame_size = 0;
    return symbol;
}

void symbol_delete(symbol_t* symbol) {
    vector_delete(symbol->blocks);
    vector_delete(symbol->parameters);
    location_delete(symbol->location);
    free(symbol->name);
    free(symbol);
}
