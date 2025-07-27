// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <signal.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static int test_sigfpe_count = 0;

static void test_sigfpe(int signo) {
    if (signo != SIGFPE) {
        fprintf(stderr, "Invalid signal() signo, expected SIGFPE, got %i\n", signo);
        exit(1);
    }

    // make sure signal handler was reset before we were called
    void (*old_handler)(int) = signal(signo, SIG_DFL);
    if (old_handler != SIG_DFL) {
        fprintf(stderr, "signal() was not reset to SIG_DFL before calling handler\n");
        exit(1);
    }

    ++test_sigfpe_count;
}

static void test_sigfpe_action(int signo, siginfo_t* info, void* context) {
    if (signo != SIGFPE) {
        fprintf(stderr, "Invalid sigaction() signo, expected SIGFPE, got %i\n", signo);
        exit(1);
    }

    if (info->si_signo != signo) {
        fprintf(stderr, "Invalid si_signo, expected %i, got %i\n", signo, info->si_signo);
        exit(1);
    }

    ++test_sigfpe_count;
}

static void test_signal(void) {
    void (*old_handler)(int) = signal(SIGFPE, test_sigfpe);
    if (old_handler != SIG_DFL) {
        fprintf(stderr, "Previous signal handler is not SIG_DFL\n");
        exit(1);
    }

    test_sigfpe_count = 0;
    raise(SIGFPE);
    if (test_sigfpe_count != 1) {
        fprintf(stderr, "SIGFPE handler not called, or called too many times\n");
        exit(1);
    }
}

static void test_sigaction_noreset(void) {
    struct sigaction act = {};
    act.sa_sigaction = test_sigfpe_action;
    act.sa_flags |= SA_SIGINFO;

    struct sigaction oact = {};

    if (0 != sigaction(SIGFPE, &act, &oact)) {
        perror("sigaction(SIGFPE) failed");
        exit(1);
    }
    if (oact.sa_handler != SIG_DFL) {
        fprintf(stderr, "Previous sigaction is not SIG_DFL\n");
        exit(1);
    }

    test_sigfpe_count = 0;
    raise(SIGFPE);
    if (test_sigfpe_count != 1) {
        fprintf(stderr, "SIGFPE sigaction not called, or called too many times\n");
        exit(1);
    }

    struct sigaction dfl = {};
    dfl.sa_handler = SIG_DFL;
    struct sigaction oact2 = {};

    if (0 != sigaction(SIGFPE, &dfl, &oact2)) {
        perror("sigaction(SIGFPE, SIG_DFL) failed");
        exit(1);
    }
    if (oact2.sa_sigaction != test_sigfpe_action) {
        fprintf(stderr, "sigaction was reset despite no SA_RESETHAND\n");
        exit(1);
    }
}

static void test_sigaction_reset(void) {
    struct sigaction act = {};
    act.sa_sigaction = test_sigfpe_action;
    act.sa_flags |= SA_SIGINFO | SA_RESETHAND;

    struct sigaction oact = {};

    if (0 != sigaction(SIGFPE, &act, &oact)) {
        perror("sigaction(SIGFPE) failed");
        exit(1);
    }
    if (oact.sa_handler != SIG_DFL) {
        fprintf(stderr, "Previous sigaction is not SIG_DFL\n");
        exit(1);
    }

    test_sigfpe_count = 0;
    raise(SIGFPE);
    if (test_sigfpe_count != 1) {
        fprintf(stderr, "SIGFPE sigaction not called, or called too many times\n");
        exit(1);
    }

    struct sigaction dfl = {};
    dfl.sa_handler = SIG_DFL;
    struct sigaction oact2 = {};

    if (0 != sigaction(SIGFPE, &dfl, &oact2)) {
        perror("sigaction(SIGFPE, SIG_DFL) failed");
        exit(1);
    }
    if (oact2.sa_handler != SIG_DFL) {
        fprintf(stderr, "sigaction was not reset despite SA_RESETHAND\n");
        exit(1);
    }
}

static void test_sigaction_null_oact(void) {
    struct sigaction dfl = {};
    dfl.sa_handler = SIG_DFL;

    // test of passing NULL for oact
    if (0 != sigaction(SIGFPE, &dfl, NULL)) {
        perror("sigaction(SIGFPE, SIG_DFL, NULL) failed");
        exit(1);
    }
}

static void test_signal_invalid(void) {
    errno = 0;
    if (SIG_ERR != signal(__SIG_MAX, SIG_DFL)) {
        fprintf(stderr, "Expected error from signal() with invalid signo");
        exit(1);
    }
    if (errno != EINVAL) {
        fprintf(stderr, "Expected EINVAL from signal() with invalid signo");
        exit(1);
    }
}

static void test_sigaction_invalid(void) {
    errno = 0;
    if (SIG_ERR != sigaction(__SIG_MAX, (struct sigaction*)-1, (struct sigaction*)-1)) {
        fprintf(stderr, "Expected error from sigaction() with invalid signo");
        exit(1);
    }
    if (errno != EINVAL) {
        fprintf(stderr, "Expected EINVAL from sigaction() with invalid signo");
        exit(1);
    }
}

int main(void) {
    test_signal();
    test_signal_invalid();

    test_sigaction_noreset();
    test_sigaction_reset();
    test_sigaction_null_oact();
    test_sigaction_invalid();
}
