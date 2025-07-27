/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Haelwenn (lanodan) Monnier
 * Copyright (c) 2025 Fraser Heavy Software
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

#ifndef __ONRAMP_LIBC_SIGNAL_H_INCLUDED
#define __ONRAMP_LIBC_SIGNAL_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

#include <sys/types.h>

/* SIG values taken from Linux */
#define SIGINT  2
#define SIGILL  4
#define SIGABRT 6
#define SIGFPE  8
#define SIGSEGV 11
#define SIGTERM 15
#define __SIG_MAX 32

#define SIG_ERR  ((void (*)(int))-1)
#define SIG_DFL  ((void (*)(int)) 0)
#define SIG_IGN  ((void (*)(int)) 1)

#define SA_SIGINFO (1 << 0)
#define SA_RESETHAND (1 << 1)

int raise(int);

// Most of this file requires the final stage C compiler.
#ifndef __onramp_cci_omc__
#ifndef __onramp_cci_opc__
typedef int sigset_t; // TODO sigset properly

typedef int sig_atomic_t;

union sigval {
    int sival_int;
    void* sival_ptr;
};

typedef struct {
    int si_signo;
    int si_code;
    pid_t si_pid;
    uid_t si_uid;
    void* si_addr;
    int si_status;
    union sigval si_value;
} siginfo_t;

struct sigaction {
    union {
        void (*sa_handler)(int __signo);
        void (*sa_sigaction)(int __signo, siginfo_t* __info, void* __context);
    };
    int sa_flags;
    sigset_t sa_mask;
};

int sigaction(int __signo,
        const struct sigaction* restrict __action,
        struct sigaction* restrict __previous_action);

void (*signal(int __signo, void (*__handler)(int)))(int);
#endif
#endif

#endif /* __ONRAMP_LIBC_SIGNAL_H_INCLUDED */
