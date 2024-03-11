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

#include <stdlib.h>

#include "internal.h"

#include <stdint.h>


// TODO clean this up


#if 0
#ifndef ONRAMP_LIBC_OVERLAY
_Noreturn
void __start_c(void) {
    extern void __malloc_init(void);
    __malloc_init();

    const char* argv = "";
    extern int main(int argc, const char** argv);
    int ret = main(1, &argv);
    exit(ret);
}

#if defined(__linux__) && defined(__i386__)
void _start(void) {
    // TODO, probably need some inline assembly, just to put the command-line
    // args and env var pointers in globals, then we call __start2(). In onramp
    // this is in assembly in start.os, will also store heap bounds.
    __start2();
}
#endif
#endif
#endif


uint32_t* __process_info;
int __argc;
char** __argv;
char** __environ; // TODO POSIX alias is just environ

_Noreturn void __end(uint32_t exit_code, uint32_t exit_address);

static void __call_constructors(void) {
    // TODO make sure to pass argc, argv, environ to __attribute__((constructor)) functions. same arguments as main()
}

static void __call_destructors(void) {
    // TODO
}

// TODO put these in an onramp header, we use them in cc.c
#define PIT_VERSION 0
#define PIT_BREAK 1
#define PIT_EXIT 2
#define PIT_INPUT 3
#define PIT_OUTPUT 4
#define PIT_ERROR 5
#define PIT_ARGS 6
#define PIT_ENVIRON 7
#define PIT_WORKDIR 8

#ifdef __onramp__
_Noreturn
void __start_c(uint32_t* process_info, uint32_t stack_base) {

    // store environment
    __process_info = process_info;
    __argv = process_info[PIT_ARGS];
    __environ = process_info[PIT_ENVIRON];

    // count command-line args
    for (__argc = 0; __argv[__argc]; ++__argc) {}

    // initialize the libc
    __malloc_init(process_info[PIT_BREAK], stack_base);
    __stdio_init();

    // run user code. exit() does not return.
    __call_constructors();
    exit(main(__argc, __argv, __environ));
}
#endif



/*
 * There are many ways to exit a C program. Onramp implements four different
 * ways:
 *
 * - exit() calls atexit() functions, then calls __attribute__((destructor))
 *   functions, then forwards to _Exit().
 *
 * - quick_exit() calls at_quick_exit() functions, then forwards to _Exit().
 *
 * - _Exit() flushes and closes open files and jumps to the exit address (which
 *   returns to the parent Onramp process or exits the Onramp VM.)
 *
 * - abort() halts the entire VM. Since it is not safe to close open files, we
 *   cannot return to our parent process safely, so we halt the VM in order to
 *   take out not only our process but all parent Onramp processes as well.
 *
 * Other ways to exit are mapped to the above four functions:
 *
 * - returning from main() calls exit();
 *
 * - _exit() is an alias of _Exit();
 *
 * - __builtin_trap() and other fatal errors call abort().
 */



// TODO detect if the user calls exit() or quick_exit() while they are already
// executing. if so, call _Exit()? or abort?

/*
 * atexit() and at_quick_exit() calls are stored in linked lists.
 */

typedef struct exit_call_t {
    struct exit_call_t* next;
    void (*func)(void);
} exit_call_t;

static exit_call_t* exit_calls = NULL;
static exit_call_t* quick_exit_calls = NULL;

int atexit(void (*func)(void)) {
    exit_call_t* new_call = (exit_call_t*)malloc(sizeof(exit_call_t));
    if (new_call == NULL)
        return -1;
    new_call->func = func;
    new_call->next = exit_calls;
    exit_calls = new_call;
    return 0;
}

int at_quick_exit(void (*func)(void)) {
    exit_call_t* new_call = (exit_call_t*)malloc(sizeof(exit_call_t));
    if (new_call == NULL)
        return -1;
    new_call->func = func;
    new_call->next = quick_exit_calls;
    quick_exit_calls = new_call;
    return 0;
}

static void exit_flush(void) {
    // TODO flush open files, etc.
}

_Noreturn void exit(int status) {
    for (exit_call_t* call = exit_calls; call != NULL; call = call->next) {
        call->func();
    }
    __call_destructors();
    _Exit(status);
}

_Noreturn void quick_exit(int status) {
    for (exit_call_t* call = quick_exit_calls; call != NULL; call = call->next) {
        call->func();
    }
    _Exit(status);
}

_Noreturn void _Exit(int status) {

    // clean up libc. we need to flush and close open files; our parent process
    // can't do it for us.
    __stdio_destroy();

    // we're done. end the process
    __end(__exit_code, __process_info[PIT_EXIT]);

    #if 0
    #if defined(__onramp__)
    #elif defined(__linux__) && defined(__i386__)
        __asm__(
                "movl $1, %%eax\n"  // 1 == SYS_exit on i386
                "movl %[status], %%ebx\n"
                "int $0x80\n"
                : // outputs
                // none
                : // inputs
                [status] "rm" (status)
                : // registers clobbered
                "eax",
                "ebx"
        );
        __builtin_unreachable();
    #endif
    #endif
}

// TODO move this to libc/1, cc needs it
char* getenv(const char* name) {
    for (char** p = __environ; p; ++p) {
        char* start = *p;

        // get the end of the key
        char* end = strchr(start, '=');
        if (end == NULL) {
            continue;
        }

        // check if it matches
        if (end - start != strlen(name)) {
            continue;
        }
        if (0 != memcmp(start, name, end - start)) {
            continue;
        }

        // found!
        return end + 1;
    }

    // not found
    return NULL;
}

int system(const char* string) {

    /* A null string is used to query whether the platform has a command
     * processor. For now we say it doesn't. */
    if (string == NULL)
        return 0;

    /* Since we've declared that we don't have a command processor, a non-null
     * string is undefined behaviour. */
    abort();

    // TODO when hosted on known platforms we could implement this for real.
    // our vm has an external spawn call and we should be able to tell when
    // we're wrapped for posix or windows. this should let us do an external
    // spawn to a shell or batch script that runs the given string as a
    // command.
    //     see: https://en.cppreference.com/w/cpp/utility/program/system
    // almost all of this will be the same between posix and windows, only
    // difference is the shell (/usr/bin/sh or cmd.exe). means it would in
    // theory be easy to port this to arbitrary other platforms as well.

}
