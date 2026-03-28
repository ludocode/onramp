// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#define _DEFAULT_SOURCE

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

char wd[PATH_MAX];

// Tests one path. If expected is relative it is appended to the working
// directory.
static bool test(const char* path, const char* expected) {
    char outbuf[PATH_MAX]; // output buffer
    char exbuf[PATH_MAX]; // expected buffer

    if (outbuf != realpath(path, outbuf)) {
        perror("realpath() returned NULL");
        return false;
    }

    // We support VMs that don't provide a working directory.
    if (*wd == 0 || *expected == '/') {
        return 0 == strcmp(outbuf, expected);
    } else {
        snprintf(exbuf, sizeof(exbuf), "%s/%s", wd, expected);
        return 0 == strcmp(outbuf, exbuf);
    }
}

int main(void) {
    if (!getcwd(wd, sizeof(wd))) {
        perror("Failed to get working directory");
        exit(1);
    }

    if (!test("foo", "foo")) {return 10;}
    if (!test("foo/.", "foo")) {return 11;}
    if (!test("./foo", "foo")) {return 12;}
    if (!test("./foo/.", "foo")) {return 13;}
    if (!test("././foo/././bar/./.", "foo/bar")) {return 14;}
    if (!test("foo/../bar", "bar")) {return 15;}
    if (!test("foo/bar/../baz", "foo/baz")) {return 16;}
    if (!test("foo/../bar/./../baz", "baz")) {return 17;}

    chdir("/");
    if (!getcwd(wd, sizeof(wd))) {
        perror("Failed to get working directory");
        exit(2);
    }

    // trying to avoid paths that might actually exist in case they are symlinks
    if (!test("/onramp/test", "/onramp/test")) {return 30;}
    if (!test("/onramp/test1/../test2", "/onramp/test2")) {return 31;}
    if (!test("/onramp/../onramp", "/onramp")) {return 32;}
    if (!test("/onramp/../../onramp", "/onramp")) {return 33;}
    if (!test("/onramp/test1/../../../onramp", "/onramp")) {return 34;}
}
