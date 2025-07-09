// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <limits.h>

int main(void) {
    if (17ll >> 2u != 4ull) return 1;
    if (17ll >> 2ull != 4ull) return 2;

    if ((long long)0xfedcba9876543210 >> 4u != 0xffedcba987654321ll) return 3;
    if ((long long)0xfedcba9876543210 >> 4ull != 0xffedcba987654321ll) return 4;
}
