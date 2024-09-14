// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <limits.h>

int main(void) {
    unsigned long long a = 0x7777777777777777ull;
    unsigned long long x = 0;
    unsigned long long b = 0xCCCCCCCCCCCCCCCCull;

    if (x++ != 0) {return 1;}
    if (x-- != 1) {return 2;}
    if (x-- != 0) {return 3;}
    if (x++ != ULLONG_MAX) {return 4;}
    if (x   != 0) {return 5;}

    // check that canaries weren't clobbered
    if (a != 0x7777777777777777ull) {return 6;}
    if (b != 0xCCCCCCCCCCCCCCCCull) {return 7;}
}
