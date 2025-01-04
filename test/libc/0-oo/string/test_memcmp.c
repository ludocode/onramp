// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

int main(void) {
    if (0 != memcmp("abcd", "abcd", 4)) {
        return 1;
    }
    if (0 >= memcmp("abdd", "abcd", 4)) {
        return 2;
    }
    if (0 <= memcmp("abbd", "abcd", 4)) {
        return 3;
    }

    if (0 != memcmp("aaa", "abc", 1)) {
        return 4;
    }

    // pointers need not be valid when length is 0
    if (0 != memcmp(NULL, (void*)0xFFFFFFFF, 0)) {
        return 5;
    }

    return 0;
}
