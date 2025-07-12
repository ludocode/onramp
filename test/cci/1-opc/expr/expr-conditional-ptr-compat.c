// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

int main(void) {
    char      * x = "x";
    char const* y = "y";
    if (0 != strcmp("x", 1 ? x : y)) return 1;
    if (0 != strcmp("y", 0 ? x : y)) return 2;
}
