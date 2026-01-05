/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025-2026 Fraser Heavy Software
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
 * All of the madness for controlling the terminal is put in this file. I don't
 * know if it's right, but it seems to work in the cases I've tested.
 *
 * It turns out that enabling raw input mode while interacting correctly with
 * shell job control and tools like `timeout` is *insane*. This is all just to
 * make doom-cli work. It is a huge waste of time; don't bother to do this in
 * your own VMs.
 *
 * If you change this, make sure you test c-testsuite, and doom-cli, and job
 * control with a long-running app to make sure it works properly. For example:
 */
#if 0
#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("Launch\n");
    for (;;) {
        putchar('a');
        fflush(stdout);
        usleep(100000);
    }
}
#endif
/*
 * With the above app compiled with onrampcc, you should be able to:
 *
 * - Launch it either in foreground or in background
 * - Stop it in foreground with CTRL+Z
 * - Resume it in background with `bg`
 * - Bring it back to the foreground with `fg`
 * - Stop it while backgrounded with `stty tostop`
 * - Resume it while stopped in background with `stty -tostop ; bg`
 * - Kill it with e.g. `kill %1` while it's in background whether it's running
 *   or stopped
 * - Kill it with e.g. `kill <pid>` from another terminal whether it's in
 *   foreground or background and whether it's running or stopped
 *
 * Moreover, with a hello world app that exits immediately (i.e. remove the
 * loop in the above program), you should be able to launch it under `timeout`
 * in the foreground, background, in a shell script, in an `if`, etc. and it
 * should always exit immediately, not after the timeout expires.
 */

#include "terminal.h"

#include <termios.h>
#include <signal.h>

#include "vmcommon.h"

static struct termios original_termios;
static bool raw_input_set;
bool raw_input_enabled;

static void sigcont(int signal);

static void set_signal_handler(int signal, void (*handler)(int)) {
    // Use sigaction() to avoid SA_RESETHAND and to mask all signals during the
    // signal handler
    struct sigaction action = {.sa_handler = handler};
    sigfillset(&action.sa_mask);
    sigaction(signal, &action, NULL);
}

static void sigttou_ignore(int signal) {
    // Ignored. (We can't use SIG_IGN because it's restartable even if
    // SA_RESTART isn't specified. We need this empty function instead.)
    (void)signal;
}

// Sets raw input mode if we're in the foreground of a terminal.
static void enable_raw_input(void) {
    if (!raw_input_enabled) {
        return;
    }
    if (raw_input_set) {
        return;
    }

    if (0 != tcgetattr(STDIN_FILENO, &original_termios)) {
        // If this fails, our stdin is probably not connected to a terminal.
        // Nothing to do.
        return;
    }

    if (getpgrp() != tcgetpgrp(STDOUT_FILENO)) {
        // If we don't control the terminal, avoid trying to set raw input
        // mode.
        return;
    }

    // There's a race condition here where we could be put in the background in
    // between doing the check above and calling tcsetattr(), which would cause
    // us to stop on a SIGTTOU. For this reason we trap SIGTTOU so we can undo
    // it safely. Since SIGTTOU stops the entire process group, we send a
    // SIGCONT to the process group to unblock it.

    // Ignore SIGTTOU with a non-restartable handler while setting terminal
    // state, this way tcsetattr() will fail with EINTR if we're in the
    // background.
    set_signal_handler(SIGTTOU, sigttou_ignore);

    // Set raw input mode.
    struct termios termios = original_termios;
    termios.c_lflag &= ~(ICANON | ECHO); // Non-canonical input, no input echo
    if (0 == tcsetattr(STDIN_FILENO, TCSANOW, &termios)) {
        raw_input_set = true;
    } else {
        // Failed to set terminal state. Our entire process group received a
        // SIGTTOU; we need to send them a SIGCONT to unblock them. Since we
        // also receive the SIGCONT, we need to ignore it.
        set_signal_handler(SIGCONT, SIG_IGN);
        kill(-getpgrp(), SIGCONT);
        set_signal_handler(SIGCONT, sigcont);
    }

    // We're done ignoring SIGTTOU. We don't want to ignore it now because we
    // do want to stop if the user has configured their tty to block background
    // output (e.g. `stty tostop`.)
    signal(SIGTTOU, SIG_DFL);
}

static void disable_raw_input(void) {
    if (raw_input_set) {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
        raw_input_set = false;
    }
}

static void exit_signal(int signal) {
    disable_raw_input();
    _Exit(128 + signal);
}

static void sigcont(int signal) {
    (void)signal;
    enable_raw_input();
}

static void sigtstp(int sig) {
    (void)sig;
    disable_raw_input();

    // We need to raise SIGTSTP again with the default handler so that we stop
    // and our controlling shell is woken up from waitpid(). We unblock SIGTSTP
    // so it fires recursively, this way we can reset the handler when it's
    // done.
    sigset_t newset, oldset;
    sigfillset(&newset);
    sigdelset(&newset, SIGTSTP);
    sigprocmask(SIG_SETMASK, &newset, &oldset);
    signal(SIGTSTP, SIG_DFL);

    // Stop. This will block until we get SIGCONT (i.e. we're foregrounded by
    // the shell), which will re-enable raw input.
    raise(SIGTSTP);

    // We woke up. Reset the handler.
    set_signal_handler(SIGTSTP, sigtstp);
    sigprocmask(SIG_SETMASK, &oldset, NULL);
}

void terminal_setup(void) {

    // Unbuffer stdin so we can poll() it properly
    setvbuf(stdin, NULL, _IONBF, BUFSIZ);

    // Fully buffer output so as not to flicker when animating our debug info.
    setvbuf(stdout, NULL, _IOFBF, BUFSIZ);

    // Restore terminal state on normal exit
    atexit(disable_raw_input);

    // Set up our signal handlers so we can restore the terminal state on
    // signals. (atexit() handlers aren't normally called on termination by
    // signal.)
    set_signal_handler(SIGABRT, exit_signal);
    set_signal_handler(SIGALRM, exit_signal);
    set_signal_handler(SIGBUS, exit_signal);
    set_signal_handler(SIGFPE, exit_signal);
    set_signal_handler(SIGHUP, exit_signal);
    set_signal_handler(SIGILL, exit_signal);
    set_signal_handler(SIGINT, exit_signal);
    set_signal_handler(SIGPIPE, exit_signal);
    set_signal_handler(SIGPROF, exit_signal);
    set_signal_handler(SIGQUIT, exit_signal);
    set_signal_handler(SIGSEGV, exit_signal);
    set_signal_handler(SIGSYS, exit_signal);
    set_signal_handler(SIGTERM, exit_signal);
    set_signal_handler(SIGTRAP, exit_signal);
    set_signal_handler(SIGUSR1, exit_signal);
    set_signal_handler(SIGUSR2, exit_signal);
    set_signal_handler(SIGVTALRM, exit_signal);
    set_signal_handler(SIGXCPU, exit_signal);
    set_signal_handler(SIGXFSZ, exit_signal);

    // On Linux, SIGINFO is an alias of SIGPWR, which by default terminates the
    // program. On macOS, SIGINFO by default is ignored. We register for SIGPWR
    // only on Linux.
    #ifdef __linux__
    #ifdef SIGPWR
    set_signal_handler(SIGPWR, exit_signal);
    #endif
    #endif

    // Same is true for SIGPOLL which terminates on Linux, which is SIGIO and
    // is ignored on macOS.
    #ifdef __linux__
    #ifdef SIGPOLL
    set_signal_handler(SIGPOLL, exit_signal);
    #endif
    #endif

    // Listen to SIGCONT to set raw input mode when we're foregrounded.
    set_signal_handler(SIGCONT, sigcont);

    // Listen to SIGTSTP to clear raw input mode when we're backgrounded.
    set_signal_handler(SIGTSTP, sigtstp);

    // Set raw input mode on startup if possible
    raw_input_enabled = true;
    enable_raw_input();

    // If we failed to set raw input, disable it permanently. We'll declare
    // that we don't have raw input in our PIT capabilities.
    if (!raw_input_set) {
        raw_input_enabled = false;
    }
}
