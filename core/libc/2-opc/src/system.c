/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2025 Fraser Heavy Software
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
 * Program start, program exit and other system functions.
 *
 * Implements __start_c(), exit(), system() and more.
 */

#include <stdlib.h>

#include <__onramp/__pit.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "internal.h"
#include "syscalls.h"

int __argc;
char** __argv;
extern char** environ;

// TODO clean these up
_Noreturn void __end(unsigned exit_code, unsigned exit_address);
extern void __environ_setup(void);
extern void __malloc_init(void);
extern void __file_init(void);
extern void __file_destroy(void);
extern void __io_init(void);
extern void __io_destroy(void);
static void exit_flush(void);
extern int main(int argc, char** argv, char** envp);

unsigned* __process_info_table;

extern void* __constructors[];
extern void* __destructors[];
extern void __call_constructor(int argc, char** argv, char** envp, void* func);
extern void __call_destructor(void* func);
static void call_constructors(void);
static void call_destructors(void);

static char* empty_argv[2];

static void __argv_setup(void) {
    __argv = (char**)__process_info_table[__ONRAMP_PIT_ARGS];

    // The VM is allowed to leave argv NULL or point it to an empty
    // (null-terminated) array. We make sure there is always at least
    // a program name.
    if (__argv == NULL || __argv[0] == NULL) {
        // We don't have initializers or local static variables in opC so we
        // have to declare it above and initialize it here.
        empty_argv[0] = "<unnamed>";
        __argv = empty_argv;
    }

    // Count command-line args
    for (__argc = 0; __argv[__argc]; ++__argc) {}
}

#ifdef __onramp__
_Noreturn
void __start_c(unsigned* process_info, unsigned stack_base) {

    // initialize the libc
    __process_info_table = process_info;
    __argv_setup();
    __environ_setup();
    __time_setup();
    __malloc_init(/*process_info[__ONRAMP_PIT_BREAK], stack_base*/);
    __io_init();
    __file_init();

    // run user code. exit() does not return.
    call_constructors();
    exit(main(__argc, __argv, environ));
}
#endif

static void exit_flush(void) {
    // TODO flush open files, etc.
}

_Noreturn void _Exit(int status) {
    __sys_exit(status);
}



/*
 * exit(), quick_exit(), __fatal() and other ways of exiting
 */

// We keep track of whether we're exiting in case a silly programmer calls an
// exit function during an atexit(), at_quick_exit() or destructor callback.
//
// Calling exit() more than once is undefined behaviour. We translate the
// second call to _Exit() (thus skipping the remaining callbacks and exiting
// with the status passed to the second call to exit(), not the first.) The
// same is true of calling quick_exit() twice. (It is not possible to call any
// other method of exiting twice because no other methods return control to any
// user code.)
//
// It's not clear whether at_quick_exit() is allowed in a callback from exit()
// or vice versa (or maybe it is and I just haven't found it in the standard.)
// We support it because exit() and quick_exit() do not share any callbacks so
// there is no possibility of callbacks being called twice. Calling
// quick_exit() during exit() stops performing exit() callbacks and performs
// quick_exit() callbacks only, and vice versa.
static bool exiting;
static bool quick_exiting;

_Noreturn void exit(int status) {
    if (exiting) {
        _Exit(status);
    }
    exiting = true;

    // call user cleanup functions
    __call_atexit();
    call_destructors();

    // close files
    exit_flush();
    __file_destroy();
    __io_destroy();

    _Exit(status);
}

_Noreturn void quick_exit(int status) {
    if (quick_exiting) {
        _Exit(status);
    }
    quick_exiting = true;

    // call user at_quick_exit() functions (no atexit() or destructors)
    __call_at_quick_exit();

    // flush open files but don't bother closing anything
    exit_flush();

    _Exit(status);
}

_Noreturn void __fatal(const char* string) {
    fputs(string, stderr);
    fflush(stderr);
    _Exit(1);
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

static void call_constructors(void) {
    void** constructors = __constructors;
    while (*constructors) {
        __call_constructor(__argc, __argv, environ, *constructors++);
    }
}

static void call_destructors(void) {
    void** destructors = __destructors;
    while (*destructors) {
        __call_destructor(*destructors++);
    }

    #ifndef __onramp__
    // hack for annoying "unused" warning when testing with a native C compiler
    (void)call_constructors;
    (void)__argv_setup;
    #endif
}
