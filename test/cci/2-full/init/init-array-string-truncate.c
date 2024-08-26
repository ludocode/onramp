// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

char x[4] = "abcdefgh"; // raises -Wexcess-initializers
int y = 0x12345678;

int main(void) {
    if (x[0] != 'a') return 1;
    if (x[1] != 'b') return 2;
    if (x[2] != 'c') return 3;
    if (x[3] != 'd') return 4;

    // make sure the initialization of x didn't overwrite y
    if (y != 0x12345678) return 5;
}
