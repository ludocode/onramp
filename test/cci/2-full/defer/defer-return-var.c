// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdio.h>

int main(void) {
    int x = 0;
    defer x = 1;
    return x; // evaluates x before defer
}
