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

#include <signal.h>

#include "internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

static void signal_abort(int signo) {
    fprintf(stderr, "Unhandled signal: %i\n", signo);
    exit(1);
}

// All signal handlers are initially SIG_DFL (zero).
static struct sigaction signals[__SIG_MAX];

void (*signal(int signo, void (*handler)(int)))(int) {
    if (signo < 0 || signo >= __SIG_MAX) {
        errno = EINVAL;
        return SIG_ERR;
    }

    // TODO don't allow changing signals that can't be caught (e.g. SIGKILL)

    struct sigaction* sa = &signals[signo];
    void (*original)(int) = sa->sa_handler;
    sa->sa_handler = handler;
    sa->sa_flags |= SA_RESETHAND; // signal() is always reset before the handler is called
    return original;
}

int sigaction(int signo,
        const struct sigaction *restrict act,
        struct sigaction *restrict oact)
{
    if (signo < 0 || signo >= __SIG_MAX) {
        errno = EINVAL;
        return -1;
    }

    // TODO don't allow changing signals that can't be caught (e.g. SIGKILL)

    struct sigaction* sa = &signals[signo];
    if (oact) {
        *oact = *sa;
    }

    // TODO check that argument is valid
    *sa = *act;
    return 0;
}

int raise(int signo) {
    if (signo < 0 || signo >= __SIG_MAX) {
        errno = EINVAL;
        return -1;
    }

    struct sigaction* sa = &signals[signo];

    // handle default actions
    if (sa->sa_handler == SIG_IGN) {
        return 0;
    }
    if (sa->sa_handler == SIG_ERR || sa->sa_handler == SIG_DFL) {
        // TODO not all signals should abort by default. for now we do
        signal_abort(signo);
    }

    // TODO: for now we ignore the SA_SIGINFO flag and just always pass the
    // extra arguments to sa_sigaction. They will be ignored by old-style
    // handlers.
    void (*func)(int __signo, siginfo_t* __info, void* __context) = sa->sa_sigaction;

    // reset if necessary
    if (sa->sa_flags & SA_RESETHAND) {
        sa->sa_handler = SIG_DFL;
        sa->sa_flags &= ~SA_SIGINFO;
    }

    /* TODO compound initializers not working
    siginfo_t info = {
        // TODO: most of these fields will be zero for now. Onramp doesn't have
        // the concept of process IDs, user IDs, etc.
        .si_signo = signo,
    };
    */
    siginfo_t info = {};
    info.si_signo = signo;

    func(signo, &info, NULL);
    return 0;
}
