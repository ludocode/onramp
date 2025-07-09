// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <limits.h>

int main(void) {
    if (17llu >> 2u != 4ull) return 1;
    if (17llu >> 2ull != 4ull) return 2;

    if (0xfedcba9876543210llu >> 4u != 0xfedcba987654321ll) return 3;
    if (0xfedcba9876543210llu >> 4ull != 0xfedcba987654321ll) return 4;
}
