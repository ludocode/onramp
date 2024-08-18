// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

// TODO test ordering

int main(void) {
    if (0 != strncmp("hello", "goodbye", 0)) return 1;
    if (0 == strncmp("hello", "goodbye", 1)) return 2;

    if (0 != strncmp("hello", "hello", 1)) return 3;
    if (0 != strncmp("hello", "hello", 5)) return 4;
    if (0 != strncmp("hello", "hello", 4)) return 5;
    if (0 != strncmp("hello", "hellX", 4)) return 6;

    if (0 == strncmp("hello", "heXlo", 5)) return 7;
}
