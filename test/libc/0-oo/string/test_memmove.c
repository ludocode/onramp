// The MIT License (MIT)
// Copyright (c) 2023-2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>
#include <stdio.h>

int main(void) {

    // move down
    {
        char bytes[] = "Hello world";
        memmove(bytes + 5, bytes + 8, 4);
        if (0 != strcmp(bytes, "Hellorld")) return 1;
    }
    {
        char bytes[] = "Hello world";
        memmove(bytes + 1, bytes + 3, 7);
        if (0 != strcmp(bytes, "Hlo worlrld")) return 1;
    }
    {
        // move by words
        char bytes[] = "abcdefghijklmnopqrstuvwxyz";
        memmove(bytes + 4, bytes + 8, 15);
        if (0 != strcmp(bytes, "abcdijklmnopqrstuvwtuvwxyz")) return 1;
    }

    // move up
    {
        char bytes[] = "Hello world";
        memmove(bytes + 3, bytes, 4);
        if (0 != strcmp(bytes, "HelHellorld")) return 1;
    }
    {
        char bytes[] = "Hello world";
        memmove(bytes + 1, bytes, 7);
        if (0 != strcmp(bytes, "HHello wrld")) return 1;
    }
    {
        // move by words
        char bytes[] = "abcdefghijklmnopqrstuvwxyz";
        memmove(bytes + 8, bytes + 4, 15);
        if (0 != strcmp(bytes, "abcdefghefghijklmnopqrsxyz")) return 1;
    }

    return 0;
}
