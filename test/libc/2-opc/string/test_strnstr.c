// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

int main(void) {
    // These tests and strnstr() are copied from Ghost:
    //     https://github.com/ludocode/ghost/blob/develop/test/unit/src/string/test_ghost_strnstr.c
    const char* haystack = "abcdefgh";
    if (haystack + 3 != strnstr(haystack, "de", 99)) return 1;
    if (haystack + 3 != strnstr(haystack, "de", 5)) return 2;
    if (NULL != strnstr(haystack, "de", 4)) return 3;
    if (haystack + 7 != strnstr(haystack, "h", 8)) return 4;
    if (NULL != strnstr(haystack, "h", 7)) return 5;
    if (NULL != strnstr(haystack, "a", 0)) return 6;
    if (NULL != strnstr(haystack, "z", 99)) return 7;
    if (NULL != strnstr(haystack, "z", 4)) return 8;
    if (NULL != strnstr(haystack, "z", 0)) return 9;
}
