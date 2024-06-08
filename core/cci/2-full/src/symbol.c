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

#include "symbol.h"

#include <stdlib.h>

#include "type.h"
#include "token.h"

symbol_t* symbol_new(symbol_kind_t kind, type_t* type, token_t* name, string_t* asm_name) {
    symbol_t* symbol = calloc(1, sizeof(symbol_t));
    symbol->refcount = 1;
    symbol->kind = kind;
    symbol->type = type_ref(type);
    symbol->token = token_ref(name);
    symbol->name = string_ref(name->value);
    symbol->asm_name = string_ref(asm_name);
    symbol->offset = SYMBOL_OFFSET_GLOBAL;
    return symbol;
}

void symbol_deref(symbol_t* symbol) {
    if (--symbol->refcount != 0) {
        return;
    }
    string_deref(symbol->asm_name);
    string_deref(symbol->name);
    token_deref(symbol->token);
    type_deref(symbol->type);
    free(symbol);
}
