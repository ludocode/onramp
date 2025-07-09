// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <limits.h>

int main(void) {
    unsigned long long a = 0x7777777777777777ull;
    unsigned long long x = 1;
    unsigned long long y = 2;
    unsigned long long b = 0xCCCCCCCCCCCCCCCCull;

    if (x << y != 4ull) return 1;
    x <<= y;
    if (x != 4ull) return 2;

    // check that canaries weren't clobbered
    if (a != 0x7777777777777777ull) {return 6;}
    if (b != 0xCCCCCCCCCCCCCCCCull) {return 7;}
}
