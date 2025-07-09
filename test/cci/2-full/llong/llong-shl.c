// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <limits.h>

int main(void) {
    if (1ll << 2 != 4ll) return 1;
    if (1ll << 2ll != 4ll) return 2;

    if (0xfedcba9876543210ll << 4 != 0xedcba98765432100ll) return 3;
    if (0xfedcba9876543210ll << 4ll != 0xedcba98765432100ll) return 4;
}
