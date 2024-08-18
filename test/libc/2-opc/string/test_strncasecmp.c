// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <strings.h>

// TODO test ordering

int main(void) {
    if (0 != strncasecmp("hello", "goodbye", 0)) return 1;
    if (0 == strncasecmp("hello", "goodbye", 1)) return 2;

    if (0 != strncasecmp("hello", "hello", 1)) return 3;
    if (0 != strncasecmp("hello", "Hello", 1)) return 4;
    if (0 != strncasecmp("hello", "hello", 5)) return 5;
    if (0 != strncasecmp("hello", "HELLO", 5)) return 6;
    if (0 != strncasecmp("hello", "hellO", 4)) return 7;
    if (0 != strncasecmp("hello", "HellX", 4)) return 8;

    if (0 == strncasecmp("hello", "heXlo", 5)) return 9;
}
