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
#include "scope.h"

symbol_t* symbol_new(symbol_kind_t kind, type_t* type, token_t* name, string_t* /*nullable*/ asm_name) {
    symbol_t* symbol = calloc(1, sizeof(symbol_t));
    symbol->refcount = 1;
    symbol->kind = kind;

    // only builtins have no type
    assert((type == NULL) == (kind == symbol_kind_builtin));
    if (type)
        symbol->type = type_ref(type);

    symbol->token = token_ref(name);
    symbol->name = string_ref(name->value);
    symbol->asm_name = string_ref(asm_name ? asm_name : symbol->name);
    return symbol;
}

void symbol_deref(symbol_t* symbol) {
    if (--symbol->refcount != 0) {
        return;
    }
    string_deref(symbol->asm_name);
    string_deref(symbol->name);
    token_deref(symbol->token);
    if (symbol->type)
        type_deref(symbol->type);
    free(symbol);
}

static void symbol_create_builtin(const char* cname, builtin_t builtin) {
    token_t* token = token_new_builtin(cname);
    symbol_t* symbol = symbol_new(symbol_kind_builtin, NULL, token, NULL);
    symbol->builtin = builtin;
    scope_add_symbol(scope_global, symbol);
    symbol_deref(symbol);
    token_deref(token);
}

void symbol_create_builtins(void) {
    symbol_create_builtin("__builtin_va_arg", BUILTIN_VA_ARG);
    symbol_create_builtin("__builtin_va_start", BUILTIN_VA_START);
    symbol_create_builtin("__builtin_va_end", BUILTIN_VA_END);
    symbol_create_builtin("__builtin_va_copy", BUILTIN_VA_COPY);
    symbol_create_builtin("__func__", BUILTIN_FUNC);
    symbol_create_builtin("__FUNCTION__", BUILTIN_FUNC);
}
