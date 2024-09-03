// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

int main(void) {
    // Make sure an array of indeterminate length can be re-declared with a
    // definite length
    extern char hello[];
    extern char hello[6];

    // ...and vice versa.
    extern char world[6];
    extern char world[];

    // They should both be complete types with correct content.
    if (sizeof(hello) != 6) return 1;
    if (sizeof(world) != 6) return 2;
    if (0 != strcmp(hello, "Hello")) return 3;
    if (0 != strcmp(world, "World")) return 4;
}

// See init/init-array-string-indeterminate-extern.c for proper tests of these
// initializers
char hello[6] = "Hello";
char world[6] = "World";
