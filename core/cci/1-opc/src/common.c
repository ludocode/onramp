/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
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

#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

#if 0
scope_t* current_scope;
function_t* functions;

void variable_destroy(variable_t* variable) {
    free(variable->name);
}

void function_destroy(function_t* function) {
    free(function->name);
    free(function->args);
}

void scope_push(void) {
    scope_t* scope = (scope_t*)malloc(sizeof(scope_t));
    scope->container = current_scope;
    scope->variables = NULL;
    current_scope = scope;
}

void scope_pop(void) {
    scope_t* scope = current_scope;
    current_scope = scope->container;
    for (variable_t* variable = scope->variables; variable != NULL;) {
        variable_t* next = variable->next;
        free(variable);
        variable = next;
    }
    free(scope);
}

void scope_add(type_t type, char* name, bool local) {
    variable_t* variable = (variable_t*)malloc(sizeof(variable_t));
    variable->next = current_scope->variables;
    variable->type = type;
    variable->name = name;
    if (local) {
        if (variable->next == NULL)
            variable->offset = -4;
        else
            variable->offset = variable->next->offset - 4;
    } else {
        variable->offset = 0;
    }
    current_scope->variables = variable;
}

variable_t* scope_find(const char* name) {
    for (scope_t* scope = current_scope; scope; scope = scope->container) {
        for (variable_t* variable = scope->variables; variable; variable = variable->next) {
            if (0 == strcmp(name, variable->name)) {
                return variable;
            }
        }
    }
    return 0;
}
#endif

/*
 * Fatal error functions
 */

void fatal(const char* message) {
    fatal_4(message, "", "", "");
}

void fatal_2(const char* message_1, const char* message_2) {
    fatal_4(message_1, message_2, "", "");
}

void fatal_3(const char* message_1, const char* message_2,
        const char* message_3)
{
    fatal_4(message_1, message_2, message_3, "");
}

void fatal_4(const char* message_1, const char* message_2,
        const char* message_3, const char* message_4)
{
    fputs("ERROR", stderr);
    if (lexer_line != 0) {
        fputs(" at ", stderr);
        fputs(lexer_filename, stderr);
        fputs(":", stderr);
        // TODO
        fprintf(stderr, "%i", lexer_line);
    }
    fputs(": ", stderr);

    fputs(message_1, stderr);
    fputs(message_2, stderr);
    fputs(message_3, stderr);
    fputs(message_4, stderr);
    fputc('\n', stderr);
*(int*)0=1;
    _Exit(1);
}

#ifndef __GNUC__
void assert(bool x) {
    if (!x) {
        fatal("Assertion failed");
    }
}
#endif
