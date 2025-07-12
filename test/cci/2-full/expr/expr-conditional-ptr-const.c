// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

int main(void) {
    char      * x = "x";
    char const* y = "y";
    char* z = 1 ? x : y; // ERROR, y is const so expression is const, even if branch is not taken
}
