// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

struct {
    char a, b, c;
    char d[4];
    char e;
} x = {1,2,3,"abcd",4};

int main(void) {
    if (sizeof(x) != 8) return 1;
    if (x.a != 1) return 2;
    if (x.b != 2) return 3;
    if (x.c != 3) return 4;
    if (x.d[0] != 'a') return 5;
    if (x.d[1] != 'b') return 6;
    if (x.d[2] != 'c') return 7;
    if (x.d[3] != 'd') return 8;
    if (x.e != 4) return 9;
}

