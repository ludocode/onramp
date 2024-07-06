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

#include "common.h"

#include <stdlib.h>

#include "libo-error.h"
#include "token.h"

// TODO
void enum_deref(enum_t* enum_) {
    if (--enum_->refcount != 0) {
        return;
    }
    //TODO
    free(enum_);
}

void fatal_token(struct token_t* token, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfatal_token(token, format, args);
}

void vfatal_token(struct token_t* token, const char* format, va_list args) {
    current_line = token->line;
    current_filename = (char*)string_cstr(token->filename);
    vfatal(format, args);
}

bool is_pow2(int n) {
    return n && !(n & (n - 1));
}
