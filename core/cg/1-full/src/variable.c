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

#include "variable.h"

#include <stdlib.h>

#include "libo-vector.h"

static vector_t* variables;

// Takes ownership of name
variable_t* variable_new(void) {
    variable_t* variable = calloc(1, sizeof(variable_t));
    variable->id = vector_count(variables);
    vector_append(variables, variable);
    return variable;
}

static void variable_delete(variable_t* variable) {
    // This is only called when we cleanup variables; we don't remove from the
    // variables vector because it's being cleared.
    free(variable);
}

variable_t* variable_get(size_t id) {
    return vector_at(variables, id);
}

void variables_setup(void) {
    variables = vector_new();
}

void variables_teardown(void) {
    variables_clear();
    vector_delete(variables);
}

void variables_clear(void) {
    size_t count = vector_count(variables);
    for (size_t i = 0; i < count; ++i) {
        variable_delete(vector_at(variables, i));
    }
    vector_remove_all(variables);
}
