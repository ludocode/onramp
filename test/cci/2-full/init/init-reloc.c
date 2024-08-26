// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

char x[6] = "Hello";
char* y = &x[2];

int main(void) {
    if (y[-2] != 'H') return 1;
    if (y[-1] != 'e') return 2;
    if (y[ 0] != 'l') return 3;
    if (y[ 1] != 'l') return 4;
    if (y[ 2] != 'o') return 5;
    if (y[ 3] != 0)   return 6;
}
