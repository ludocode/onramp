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

/*
 * This file implements atexit() and at_quick_exit(). They require function
 * pointers so they are implemented here in libc/3.
 */

#include <stdlib.h>

// callbacks are stored in linked lists.
typedef struct exit_call_t {
    struct exit_call_t* next;
    void (*func)(void);
} exit_call_t;

static exit_call_t* exit_calls = NULL;
static exit_call_t* quick_exit_calls = NULL;

int atexit(void (*func)(void)) {
    exit_call_t* new_call = malloc(sizeof(exit_call_t));
    if (new_call == NULL)
        return -1;
    new_call->func = func;
    new_call->next = exit_calls;
    exit_calls = new_call;
    return 0;
}

// TODO implement on_exit() maybe. it's the same as atexit() except with
// arguments.

int at_quick_exit(void (*func)(void)) {
    exit_call_t* new_call = malloc(sizeof(exit_call_t));
    if (new_call == NULL)
        return -1;
    new_call->func = func;
    new_call->next = quick_exit_calls;
    quick_exit_calls = new_call;
    return 0;
}

static void call_atexit(void) {
    for (exit_call_t* call = exit_calls; call != NULL; call = call->next) {
        call->func();
    }
}

static void call_at_quick_exit(void) {
    for (exit_call_t* call = quick_exit_calls; call != NULL; call = call->next) {
        call->func();
    }
}
