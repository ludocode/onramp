// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

char x[6] = "Hello";

int main(void) {
    if (x[0] != 'H') return 1;
    if (x[1] != 'e') return 2;
    if (x[2] != 'l') return 3;
    if (x[3] != 'l') return 4;
    if (x[4] != 'o') return 5;
    if (x[5] != 0)   return 6;
}
