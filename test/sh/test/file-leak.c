// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <sys/syscall.h>
#include <stdio.h>

int main(void) {

    // open three files
    unsigned a = __sys_fopen("test/sh/test/a.txt", 0);
    unsigned b = __sys_fopen("test/sh/test/b.txt", 0);
    unsigned c = __sys_fopen("test/sh/test/c.txt", 0);

    // make sure none failed to open (high bit indicates error)
    if ((a | b | c) >> 31) {
        fputs("Failed to open files!", stderr);
        return 1;
    }

    // close the first one
    __sys_fclose(a);

    // the parent program should close the other two
}
